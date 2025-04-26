#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Wire.h>
#include <stdarg.h>
#include <ADC.h>
#include "JX8P.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // note, for WIRE on Teensy 4.1
                                                                                             // SDA = 18, SCL = 19
ADC adc;

// Console config
#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8
#define MAX_COLS (SCREEN_WIDTH / CHAR_WIDTH)     // 21
#define MAX_ROWS (SCREEN_HEIGHT / CHAR_HEIGHT)   // 8
#define BUFFER_ROWS 64                           // Scrollback buffer depth

char textBuffer[BUFFER_ROWS][MAX_COLS + 1];
int head = 0;
int count = 0;
int scrollOffset = 0;

// Buttons
#define SCROLL_UP_PIN 31
#define SCROLL_DOWN_PIN 32
#define BUTTON_A 29
#define BUTTON_B 30

#define U2_ENABLE_PIN 4
#define U3_ENABLE_PIN 5
#define U4_ENABLE_PIN 6
#define U6_ENABLE_PIN 7
#define U2_ANALOG_IN_PIN 23
#define U3_ANALOG_IN_PIN 22
#define U4_ANALOG_IN_PIN 21
#define U6_ANALOG_IN_PIN 20
#define S0_PIN 0
#define S1_PIN 1
#define S2_PIN 2 
#define S3_PIN 3
#define NUM_MUXES 4
#define READYOUT_PIN 10
#define CLOCK_IN_PIN 11
#define DATA_OUT_PIN 12
#define OLED_I2C_ADDRESS 0x3c

uint enablePins[NUM_MUXES] = { U2_ENABLE_PIN, U3_ENABLE_PIN, U4_ENABLE_PIN, U6_ENABLE_PIN };
uint MuxAddressPins[4] = { S0_PIN, S1_PIN, S2_PIN, S3_PIN };
uint analogInPins[NUM_MUXES] = { U2_ANALOG_IN_PIN, U3_ANALOG_IN_PIN, U4_ANALOG_IN_PIN , U6_ANALOG_IN_PIN};

uint16_t AnalogValues[16][NUM_MUXES];
uint16_t oldAnalogValues[16][NUM_MUXES];

bool stickyScrollEnabled = true;
static unsigned long lastScreenUpdateTime = 0;
static unsigned long lastPotScanTime = 0;
static unsigned long lastButtonScan = 0;
static uint16_t paramCtr = 0x00;
static uint16_t oldParamCtr = 0xff;
static uint16_t paramValue = 0xff;
static uint16_t oldParamValue = 0xff;

volatile uint16_t sendBuffer = 0;
volatile int bitIndex = -1;

bool bSending = false;

/* --------------------------------------------------------------
   |                                                            |
   |  Set the muxes address value                               |
   |      Invoked by: gatherPotentiometerValues()               |
   |                                                            |
   -------------------------------------------------------------- */
void setAddressPins(uint val) {
  if(val & 0x01) digitalWrite(S0_PIN, HIGH); else digitalWrite(S0_PIN, LOW);
  if(val & 0x02) digitalWrite(S1_PIN, HIGH); else digitalWrite(S1_PIN, LOW);
  if(val & 0x04) digitalWrite(S2_PIN, HIGH); else digitalWrite(S2_PIN, LOW);
  if(val & 0x08) digitalWrite(S3_PIN, HIGH); else digitalWrite(S3_PIN, LOW);
  delayMicroseconds(5);
}


/* --------------------------------------------------------------
   |                                                            |
   |  Print the values of the potentiometer buffer              |
   |  Invoked by: updateConsoleScreen()                         |
   |                                                            |
   -------------------------------------------------------------- */
void printAnalogValuesAsHexCompactReordered() {
  char line[22];  // 7 values * 3 chars ("XX ") = 21 + null
  int idx = 0;
  int count = 0;

  for (int col = 0; col < NUM_MUXES; col++) {
    for (int row = 0; row < 16; row++) {
      if (count % 7 == 0) {
        if (count != 0) {
          line[idx] = '\0';
          consolePrint(line);
        }
        idx = 0;
      }

      uint8_t val = AnalogValues[row][col];
      idx += snprintf(&line[idx], sizeof(line) - idx, "%02X ", val);
      count++;
    }
  }

  // Print any remaining values
  if (idx > 0) {
    line[idx] = '\0';
    consolePrint(line);
  }
}


/* --------------------------------------------------------------
   |                                                            |
   |  handle scrolling and stickiness                           |
   |  Invoked by: loop()                                        |
   |                                                            |
   -------------------------------------------------------------- */
void handleScrolling() {
  // Handle scroll buttons
  if (digitalRead(SCROLL_UP_PIN) == LOW) {
    if (scrollOffset + MAX_ROWS < count) {
      scrollOffset++;
      stickyScrollEnabled = false;  // Disable sticky scroll on manual scroll
      renderBuffer();
      delay(200);
    }
  } else if (digitalRead(SCROLL_DOWN_PIN) == LOW) {
    if (scrollOffset > 0) {
      scrollOffset--;
      renderBuffer();
      delay(200);

      // If we've scrolled all the way back to the bottom, enable sticky scrolling
      if (scrollOffset == 0) {
        stickyScrollEnabled = true;
      }
    }
  }
}


/* --------------------------------------------------------------
   |                                                            |
   |  Gather potentiometer values every quarter second          |
   |  Invoked by: loop()                                        |
   |                                                            |
   -------------------------------------------------------------- */
static int iGatherCtr = 0;
void gatherPotentiometerValues() {
  if(millis() - lastPotScanTime > 50) {   // scan only periodically
    iGatherCtr++;
    lastPotScanTime = millis();
    for(uint8_t muxCtr = 0; muxCtr < NUM_MUXES; muxCtr++) {
        digitalWrite(enablePins[muxCtr], LOW);  // enable this mux
        for(uint8_t addr = 0; addr < 16; addr++) {
          setAddressPins(addr);
          uint16_t raw = adc.adc0->analogRead(analogInPins[muxCtr]);
          raw = raw >> 3;
          raw = (AnalogValues[addr][muxCtr] * 7 + raw) >> 3;  // smoothed value (multiplies original value by 7 then adds new, then averages)
          AnalogValues[addr][muxCtr] = raw;
          if(iGatherCtr % 4 == 0) // every 1/5 second, store a copy of the current AnalogValues()
              oldAnalogValues[addr][muxCtr] = AnalogValues[addr][muxCtr];
        }
        digitalWrite(enablePins[muxCtr], HIGH); // disable this mux
        delayMicroseconds(5);
    }
  }
}

/* --------------------------------------------------------------
   |                                                            |
   |  Updates the console screen every 2 seconds                |
   |  Invoked by:  loop()                                       |
   |                                                            |
   -------------------------------------------------------------- */
void updateConsoleScreen() {
  // Read analogs every 2 seconds and print
  if (millis() - lastScreenUpdateTime > 2000) {
    lastScreenUpdateTime = millis();
    printAnalogValuesAsHexCompactReordered();
    addLine("\n");
  }
}

/* --------------------------------------------------------------
   |                                                            |
   |  Reset the console buffer                                  |
   |  Invoked by:  nowhere at this time                         |
   |                                                            |
   -------------------------------------------------------------- */
void clearConsoleBuffer() {
  head = 0;
  count = 0;
  scrollOffset = 0;
  stickyScrollEnabled = true;

  for (int i = 0; i < BUFFER_ROWS; i++) {
    textBuffer[i][0] = '\0';
  }

  renderBuffer();
}

/* --------------------------------------------------------------
   |                                                            |
   |  Add a single line to the console                          |
   |  Invoked by: wrapAndAddLines() and updateConsoleScreen()   |
   |                                                            |
   -------------------------------------------------------------- */
void addLine(const char* line) {
  strncpy(textBuffer[head], line, MAX_COLS);
  textBuffer[head][MAX_COLS] = '\0';
  head = (head + 1) % BUFFER_ROWS;
  if (count < BUFFER_ROWS) count++;
}


/* --------------------------------------------------------------
   |                                                            |
   |  Add a line and handle wrapping                            |
   |  Invoked by: consolePrint()                                |
   |                                                            |
   -------------------------------------------------------------- */
void wrapAndAddLines(const char* text) {
  int len = strlen(text);
  char segment[MAX_COLS + 1];

  while (len > 0) {
    int wrapLen = min(MAX_COLS, len);
    int cut = wrapLen;

    if (wrapLen < len) {
      // Try to break at a space
      for (int i = wrapLen - 1; i > 0; i--) {
        if (text[i] == ' ') {
          cut = i;
          break;
        }
      }
    }

    strncpy(segment, text, cut);
    segment[cut] = '\0';
    addLine(segment);

    text += cut;
    while (*text == ' ') text++;  // Skip leading spaces
    len = strlen(text);
  }
}



/* --------------------------------------------------------------
   |                                                            |
   |  console print and handle scrolling stickiness             |
   |  Invoked by: consolePrintf()                               |
   |                                                            |
   -------------------------------------------------------------- */
void consolePrint(const char* text) {
  // Only scroll if sticky scroll is enabled
  wrapAndAddLines(text);
  if (stickyScrollEnabled) {
    scrollOffset = 0;
  }
  renderBuffer();
}


/* --------------------------------------------------------------
   |                                                            |
   |  print a formatted string to the console                   |
   |  Invoked by: nothing at this time
   |                                                            |
   -------------------------------------------------------------- */
void consolePrintf(const char* fmt, ...) {
  char buffer[256];  // long enough for most uses
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  consolePrint(buffer);
}


/* --------------------------------------------------------------
   |                                                            |
   |  Draw scroll bar                                           |
   |  Invoked by: renderBuffer()                                |
   |                                                            |
   -------------------------------------------------------------- */
void drawScrollBar() {
  if (count <= MAX_ROWS) return;  // No need for scrollbar

  int trackX = SCREEN_WIDTH - 1;
  int trackHeight = SCREEN_HEIGHT;
  int thumbHeight = max((MAX_ROWS * trackHeight) / count, 4);  // Minimum size
  int thumbY = (scrollOffset * trackHeight) / count;

  display.drawLine(trackX, 0, trackX, trackHeight - 1, SH110X_WHITE);   // Draw the track
  display.drawFastVLine(trackX, thumbY, thumbHeight, SH110X_BLACK);  // Draw the thumb, Invert to make it stand out
}


/* --------------------------------------------------------------
   |                                                            |
   |  render buffer (console onto screen)                       |
   |  Invoked by: handleScrolling(), clearConsoleBuffer(),      |
   |              consolePrint()                                |
   |                                                            |
   -------------------------------------------------------------- */
void renderBuffer() {
  display.clearDisplay();
  int visible = min(MAX_ROWS, count - scrollOffset);
  int startIdx = (head - visible - scrollOffset + BUFFER_ROWS) % BUFFER_ROWS;

  for (int i = 0; i < visible; i++) {
    int idx = (startIdx + i) % BUFFER_ROWS;
    display.setCursor(0, i * CHAR_HEIGHT);
    display.print(textBuffer[idx]);
    Serial.print(textBuffer[idx]);
  }
  drawScrollBar();
  display.display();
}



/* --------------------------------------------------------------
   |                                                            |
   |  Initialization                                            |
   |                                                            |
   -------------------------------------------------------------- */
void setup() {
  pinMode(SCROLL_UP_PIN, INPUT_PULLUP);
  pinMode(SCROLL_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  for(uint i = 0; i < NUM_MUXES; i++) {
    pinMode(enablePins[i], OUTPUT);             // set enable pins as output
    digitalWrite(enablePins[i], HIGH);          // disable enable pins
  }
  for(uint i = 0; i < 4; i++) {
    pinMode(MuxAddressPins[i], OUTPUT);         // set mux pins as output
    digitalWrite(MuxAddressPins[i], LOW);       // set them all low
  }

  pinMode(READYOUT_PIN, OUTPUT);
  digitalWrite(READYOUT_PIN, LOW);

  pinMode(CLOCK_IN_PIN, INPUT_PULLUP);

  pinMode(DATA_OUT_PIN, OUTPUT);
  digitalWrite(READYOUT_PIN, LOW);
  
  display.begin(OLED_I2C_ADDRESS, false);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setTextWrap(false);
  display.display();

  adc.adc0->setAveraging(4);
  adc.adc0->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED);
  adc.adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED); 

  attachInterrupt(digitalPinToInterrupt(CLOCK_IN_PIN), onClockFall, FALLING);

  for(uint8_t i=0; i< NUM_MUXES; i++) {
      for(uint8_t j=0;j<16;j++) {
          AnalogValues[j][i] = 0;
          oldAnalogValues[j][i] = 0;
      }
  }

  consolePrint("Initialized.");
}



void sendParameter(uint8_t paramID, uint8_t value) {
  while (bitIndex != -1);  // Wait until previous transfer is complete

  noInterrupts();
  sendBuffer = ((uint16_t)paramID << 8) | value;
  bitIndex = 15;
  bool bit = (sendBuffer >> bitIndex) & 1;
  digitalWrite(DATA_OUT_PIN, bit ? LOW : HIGH);
  bitIndex--;
  
  digitalWrite(READYOUT_PIN, HIGH);  // Begin transmission
  //delayMicroseconds(5);
  interrupts();
}


// Called on each rising edge of CLOCK
void onClockFall() {
  if (bitIndex < 0) return;  // Not currently sending

  delayMicroseconds(10);
  // DATA is negative logic, so we invert the bit
  bool bit = (sendBuffer >> bitIndex) & 1;
  digitalWrite(DATA_OUT_PIN, bit ? LOW : HIGH);  // inverted logic

  bitIndex--;
  if (bitIndex < 0) {
    delayMicroseconds(5);
    digitalWrite(READYOUT_PIN, LOW);           // End of transfer
    delayMicroseconds(10);
    digitalWrite(DATA_OUT_PIN, LOW);           // Idle state
  }
}


void sendPG800Message(uint8_t parmIX, uint8_t value) {
    display.fillRect(0, 0, SCREEN_WIDTH, 12, SH110X_BLACK);
    display.setCursor(0,0);
    display.printf("(%d %d)", parmIX, value);
    display.fillRect(0, 12, SCREEN_WIDTH, 12, SH110X_BLACK);
    display.setCursor(0,12);
    display.printf("%s %02X", 
        jx8p_param_names[paramIndexTable[parmIX]], 
        (uint8_t) (value));
    sendParameter((uint8_t) (paramIndexTable[parmIX]), (uint8_t) (value));
    display.display();
}

void handleButtons() {
  if(millis() - lastButtonScan > 100) { // only scan every 1/10 second
    lastButtonScan = millis();

    uint8_t paramCtr = 0;
    for(uint8_t i = 0; i < NUM_MUXES; i++) {
      for(uint8_t j = 0; j < 16; j++) {
        if(oldAnalogValues[j][i] != AnalogValues[j][i]) {
          if(activeParams[paramCtr]) {
              sendPG800Message(paramCtr, AnalogValues[j][i]);
          }
        }
        paramCtr++;
      }
    }
  }
}

/* --------------------------------------------------------------
   |                                                            |
   |  Main loop                                                 |
   |                                                            |
   -------------------------------------------------------------- */
void loop() {
  //handleScrolling();
  handleButtons();
  gatherPotentiometerValues();
  //updateConsoleScreen();
}

