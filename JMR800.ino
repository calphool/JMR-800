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
uint8_t ledPattern = 0b00000001;


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

// synthesizer input pins
#define READYOUT_PIN 10
#define CLOCK_IN_PIN 11
#define DATA_OUT_PIN 12

#define OLED_I2C_ADDRESS 0x3c

// Buttons
#define PUSH_BTN_SW4_PIN 29
#define PUSH_BTN_SW3_PIN 28
#define PUSH_BTN_SW2_PIN 27
#define PUSH_BTN_SW1_PIN 26

// button lights shift register
#define SHIFT_REG_SER 32
#define SHIFT_REG_SRCLK 31
#define SHIFT_REG_RCLK 30

// Encoder
#define ENCODER_CLK_PIN 13
#define ENCODER_DT_PIN 14
#define ENCODER_SW_PIN 15

uint enablePins[NUM_MUXES] = { U2_ENABLE_PIN, U3_ENABLE_PIN, U4_ENABLE_PIN, U6_ENABLE_PIN };
uint MuxAddressPins[4] = { S0_PIN, S1_PIN, S2_PIN, S3_PIN };
uint analogInPins[NUM_MUXES] = { U2_ANALOG_IN_PIN, U3_ANALOG_IN_PIN, U4_ANALOG_IN_PIN , U6_ANALOG_IN_PIN};

uint16_t AnalogValues[16][NUM_MUXES];
uint16_t oldAnalogValues[16][NUM_MUXES];

bool stickyScrollEnabled = true;
//static unsigned long lastScreenUpdateTime = 0;
static unsigned long lastPotScanTime = 0;
//static unsigned long lastButtonScan = 0;

static int ENC_counter = 0;
static int ENC_currentStateCLK;
static int ENC_lastStateCLK;

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
/*
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
*/ 


/* --------------------------------------------------------------
   |                                                            |
   |  handle scrolling and stickiness                           |
   |  Invoked by: loop()                                        |
   |                                                            |
   -------------------------------------------------------------- */
/*
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
*/

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
          raw = raw >> 2;          // (1024 / 4 = 256)
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
/*
void updateConsoleScreen() {
  // Read analogs every 2 seconds and print
  if (millis() - lastScreenUpdateTime > 2000) {
    lastScreenUpdateTime = millis();
    printAnalogValuesAsHexCompactReordered();
    addLine("\n");
  }
}
*/


/* --------------------------------------------------------------
   |                                                            |
   |  Reset the console buffer                                  |
   |  Invoked by:  nowhere at this time                         |
   |                                                            |
   -------------------------------------------------------------- */
/*
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
*/

/* --------------------------------------------------------------
   |                                                            |
   |  Add a single line to the console                          |
   |  Invoked by: wrapAndAddLines() and updateConsoleScreen()   |
   |                                                            |
   -------------------------------------------------------------- */
/*
void addLine(const char* line) {
  strncpy(textBuffer[head], line, MAX_COLS);
  textBuffer[head][MAX_COLS] = '\0';
  head = (head + 1) % BUFFER_ROWS;
  if (count < BUFFER_ROWS) count++;
}
*/

/* --------------------------------------------------------------
   |                                                            |
   |  Add a line and handle wrapping                            |
   |  Invoked by: consolePrint()                                |
   |                                                            |
   -------------------------------------------------------------- */
/*
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
*/


/* --------------------------------------------------------------
   |                                                            |
   |  console print and handle scrolling stickiness             |
   |  Invoked by: consolePrintf()                               |
   |                                                            |
   -------------------------------------------------------------- */
/*
void consolePrint(const char* text) {
  // Only scroll if sticky scroll is enabled
  wrapAndAddLines(text);
  if (stickyScrollEnabled) {
    scrollOffset = 0;
  }
  renderBuffer();
}
*/

/* --------------------------------------------------------------
   |                                                            |
   |  print a formatted string to the console                   |
   |  Invoked by: nothing at this time
   |                                                            |
   -------------------------------------------------------------- */
/*
void consolePrintf(const char* fmt, ...) {
  char buffer[256];  // long enough for most uses
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  consolePrint(buffer);
}
*/

/* --------------------------------------------------------------
   |                                                            |
   |  Draw scroll bar                                           |
   |  Invoked by: renderBuffer()                                |
   |                                                            |
   -------------------------------------------------------------- */
/*
void drawScrollBar() {
  if (count <= MAX_ROWS) return;  // No need for scrollbar

  int trackX = SCREEN_WIDTH - 1;
  int trackHeight = SCREEN_HEIGHT;
  int thumbHeight = max((MAX_ROWS * trackHeight) / count, 4);  // Minimum size
  int thumbY = (scrollOffset * trackHeight) / count;

  display.drawLine(trackX, 0, trackX, trackHeight - 1, SH110X_WHITE);   // Draw the track
  display.drawFastVLine(trackX, thumbY, thumbHeight, SH110X_BLACK);  // Draw the thumb, Invert to make it stand out
}
*/

/* --------------------------------------------------------------
   |                                                            |
   |  render buffer (console onto screen)                       |
   |  Invoked by: handleScrolling(), clearConsoleBuffer(),      |
   |              consolePrint()                                |
   |                                                            |
   -------------------------------------------------------------- */
/*
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
*/



/* --------------------------------------------------------------
   |                                                            |
   |  Initialization                                            |
   |                                                            |
   -------------------------------------------------------------- */
void setup() {
  Serial.begin(9600);
  pinMode(PUSH_BTN_SW4_PIN, INPUT_PULLUP);
  pinMode(PUSH_BTN_SW3_PIN, INPUT_PULLUP);
  pinMode(PUSH_BTN_SW2_PIN, INPUT_PULLUP);
  pinMode(PUSH_BTN_SW1_PIN, INPUT_PULLUP);
  pinMode(ENCODER_SW_PIN,   INPUT_PULLUP);
  pinMode(ENCODER_CLK_PIN,  INPUT);
  pinMode(ENCODER_DT_PIN,   INPUT);

  pinMode(SHIFT_REG_SER,    OUTPUT);
  pinMode(SHIFT_REG_SRCLK,  OUTPUT);
  pinMode(SHIFT_REG_RCLK,   OUTPUT);

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

  log("Setting up ADC...");

  adc.adc0->setAveraging(4);
  adc.adc0->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED);
  adc.adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED); 

  ENC_lastStateCLK = digitalRead(ENCODER_CLK_PIN);

  log("Attaching interrupts...");

  attachInterrupt(digitalPinToInterrupt(CLOCK_IN_PIN), onPG800ClockFall, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK_PIN), updateEncoder, CHANGE);

  log("Setting up DAC tables...");

  for(uint8_t i=0; i< NUM_MUXES; i++) {
      for(uint8_t j=0;j<16;j++) {
          AnalogValues[j][i] = 0;
          oldAnalogValues[j][i] = 0;
      }
  }

  log("Initialized.");
}

void log(char* s) {
  Serial.printf("%s\n", s);
}

void updateEncoder() {
  ENC_currentStateCLK = digitalRead(ENCODER_CLK_PIN);

  if (ENC_currentStateCLK != ENC_lastStateCLK && ENC_currentStateCLK == 1) {
    if (digitalRead(ENCODER_DT_PIN) != ENC_currentStateCLK) {
      ENC_counter--;
    } else {
      ENC_counter++;
    }
  }

  ENC_lastStateCLK = ENC_currentStateCLK;
}


void setLEDs(uint8_t data) {
  digitalWrite(SHIFT_REG_RCLK, LOW);  // Start by disabling latch

  // Shift out 8 bits, MSB first
  for (int8_t i = 7; i >= 0; i--) {
    digitalWrite(SHIFT_REG_SRCLK, LOW);  // Prepare for clock

    bool bitVal = data & (1 << i);       // Extract current bit
    digitalWrite(SHIFT_REG_SER, bitVal); // Set data line

    digitalWrite(SHIFT_REG_SRCLK, HIGH); // Rising edge clocks bit into register
  }

  digitalWrite(SHIFT_REG_RCLK, HIGH);    // Latch data to output pins
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


// Called on each synth CLOCK fall
void onPG800ClockFall() {
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

/*
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
*/

void gatherControlSettings() {
  gatherPotentiometerValues();

  //TODO: gather buttons, gather encoder, gather encoder switch setting
}

void drawKnobArrow(int x, int y, int deg) {         // TODO: rewrite this to use precomputed values 
  // Center of the knob sprite
  int cx = x + 4;
  int cy = y + 3;
  
  // Convert degrees to radians
  float rad = deg * (PI / 180.0);

  // Adjust for your rotation: 0° = south, 90° = west, 180° = north, 270° = east
  float adj_rad = rad + PI*.33; // rotate 90 degrees counterclockwise

  // Length of the pointer
  int length = 3;

  // Calculate end point
  int ex = cx + round(length * cos(adj_rad));
  int ey = cy + round(length * sin(adj_rad));

  // Draw line from center to end point
  display.drawLine(cx, cy, ex, ey, SH110X_BLACK);
}



void drawKnob(int x, int y, int i) {
  display.fillRect(x+2, y+1, 6, 4, SH110X_WHITE);
  display.drawLine(x+3, y, x+6, y, SH110X_WHITE);
  display.drawLine(x+3, y+5, x+6, y+5, SH110X_WHITE);
  display.drawLine(x+1, y+2, x+1, y+3, SH110X_WHITE);
  display.drawLine(x+8, y+2, x+8, y+3, SH110X_WHITE);

  // i ranges from 0 - 255
  int j = (float)(360 * ((float)i/255));
  drawKnobArrow(x,y,j);
}

void drawButton(int x, int y, bool red, bool green) {
  display.fillRect(x+1, y, 8, 6, SH110X_WHITE);
  display.drawLine(x+2, y+2, x+7, y+2, SH110X_BLACK);
  
  if(red) 
    display.drawLine(x+2,y+1, x+3, y+1, SH110X_BLACK);
  if(green)
    display.drawLine(x+6,y+1, x+7, y+1, SH110X_BLACK);
}

uint knobXformer[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
 32,33,34,35,36,37,38,39,40,41, 42, 43, 44, 45, 46, 47,
 16,17,18,19,20,21,22,23,24,25, 26, 27, 28, 29, 30, 31,
 48,49,50,51,52,53,54,55
};

inline uint getKnobValue(uint knobIX) {
  knobIX = knobXformer[knobIX];              // transform this index (necessary because of wiring) 
  uint mux = knobIX >> 4;
  uint mux_ix = knobIX & 0x0F;

  return 255-AnalogValues[mux_ix][mux];
}

void drawScreen() {
    uint knobIX = 0;
    uint knobpos;
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_BLACK); // clear screen
    display.setCursor(10,0);                                           // position for title
    display.printf("JMR800 TEST SCREEN");                              // title
    display.drawRect(0, 9, SCREEN_WIDTH, 54, SH110X_WHITE);            // box around UI
    display.drawRect(90, 16, 18, 10, SH110X_WHITE);                    // small screen rectangle
    for(int i=9; i<80; i = i + 10) {                                   // knobs on the left side
      for(int j=14; j<62; j = j + 8) {
          knobIX++;
          knobpos = getKnobValue(knobIX-1);
          drawKnob(i,j,knobpos);
      }
    }
    for(int i=89; i < 100; i = i + 10) {                               // knobs on the right side
      for(int j=30; j < 62; j = j + 8) {
        knobIX++;
        knobpos = getKnobValue(knobIX-1);
        drawKnob(i,j,knobpos);
      }
    }
    drawKnob(109, 18, 0);                                              // encoder knob

    for(int j=30; j < 62; j = j + 8) {                                 // buttons
      drawButton(109, j, false, false);
    }
    display.display();
}



void shiftLights() {
  setLEDs(ledPattern);
  ledPattern = (ledPattern << 1) | (ledPattern >> 7);
}

/* --------------------------------------------------------------
   |                                                            |
   |  Main loop                                                 |
   |                                                            |
   -------------------------------------------------------------- */
void loop() {
  gatherControlSettings();
  drawScreen();
  shiftLights();
  delay(10);
}

