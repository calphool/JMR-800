#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Wire.h>
#include <stdarg.h>
#include <ADC.h>
#include "JX8P.h"
#include <Encoder.h>

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

// LED pattern buffer
uint8_t ledPattern = 0b00000000;

// MUX enable pins
#define U2_ENABLE_PIN 4
#define U3_ENABLE_PIN 5
#define U4_ENABLE_PIN 6
#define U6_ENABLE_PIN 7

// ADC input pins
#define U2_ANALOG_IN_PIN 23
#define U3_ANALOG_IN_PIN 22
#define U4_ANALOG_IN_PIN 21
#define U6_ANALOG_IN_PIN 20

// MUX address pins
#define S0_PIN 0
#define S1_PIN 1
#define S2_PIN 2 
#define S3_PIN 3

#define NUM_MUXES 4

// synthesizer input pins
#define READYOUT_PIN 10
#define CLOCK_IN_PIN 11
#define DATA_OUT_PIN 12

// OLED screen I2C address
#define OLED_I2C_ADDRESS 0x3c

// Button pins
#define PUSH_BTN_SW4_PIN 29
#define PUSH_BTN_SW3_PIN 28
#define PUSH_BTN_SW2_PIN 27
#define PUSH_BTN_SW1_PIN 26

// array of button pins
#define NUM_BUTTONS 4
const uint8_t buttonPins[NUM_BUTTONS] = {
  PUSH_BTN_SW1_PIN,
  PUSH_BTN_SW2_PIN,
  PUSH_BTN_SW3_PIN,
  PUSH_BTN_SW4_PIN
};

#define MODE_TEST 0
#define MODE_CONFIG 1
#define MODE_RUNNING 2

#define SUBMODE_1 0
#define SUBMODE_2 1

uint8_t systemMode;
uint8_t systemSubMode;

// button state variables
uint8_t buttonStates[NUM_BUTTONS] = {0}; // Current button state (0 or 1)
uint8_t prevButtonStates[NUM_BUTTONS] = {0}; // Previous button state

// button lights shift register pins
#define SHIFT_REG_SER 32
#define SHIFT_REG_SRCLK 31
#define SHIFT_REG_RCLK 30

// Encoder pins
#define ENCODER_CLK_PIN 13
#define ENCODER_DT_PIN 14
#define ENCODER_SW_PIN 15

Encoder encoderKnob(ENCODER_CLK_PIN, ENCODER_DT_PIN);

// encoder position
long lastEncoderPosition = 0;

// test screen refresh counter
long lastdrawTestScreen = 0;

// array of pins for mux enable
uint enablePins[NUM_MUXES] = { U2_ENABLE_PIN, U3_ENABLE_PIN, U4_ENABLE_PIN, U6_ENABLE_PIN };

// array of pins for MUX addresses
uint MuxAddressPins[4] = { S0_PIN, S1_PIN, S2_PIN, S3_PIN };

// array of analog ADC input pins
uint analogInPins[NUM_MUXES] = { U2_ANALOG_IN_PIN, U3_ANALOG_IN_PIN, U4_ANALOG_IN_PIN , U6_ANALOG_IN_PIN};

// buffer for values from ADC, and shadow copy for detecting changes
uint16_t AnalogValues[16][NUM_MUXES];
uint16_t oldAnalogValues[16][NUM_MUXES];

// timer value for monitoring frequency of potentiometer scans
static unsigned long lastPotScanTime = 0;

// button statuses
static bool bEncoderBtn = false;
static bool bPrevEncoderBtn = false;

// used for bit banging PG800 serial protocol
volatile int bitIndex = -1;
volatile uint16_t sendBuffer = 0;



// Cycles through 0b00 → 0b10 → 0b01 → 0b00 (off → left → right → off)
uint8_t cycleLedState(uint8_t current) {
  if (current == 0b00) return 0b10;
  if (current == 0b10) return 0b01;
  return 0b00;
}

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
   |  Gather potentiometer values every quarter second          |
   |  Invoked by: loop()                                        |
   -------------------------------------------------------------- */
static int iGatherCtr = 0;
void gatherPotentiometerValues() {
  if(millis() - lastPotScanTime < 50)
    return;

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



/* 
   --------------------------------------------------------------
   |  Logging routine                                           |
   -------------------------------------------------------------- 
*/
void log(const String& s) {
  Serial.println(s);
}


/* 
   --------------------------------------------------------------
   |  Shift register routine for LEDs                           |
   -------------------------------------------------------------- 
*/
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

/* 
   --------------------------------------------------------------
   |  sendParameter -- code that sends data to synthesizer      |
   -------------------------------------------------------------- 
*/
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


/* 
   --------------------------------------------------------------
   |  onPG800ClockFall -- loop that synchronizes the controller |
   |  with the synthesizer                                      |
   -------------------------------------------------------------- 
*/
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

/* 
   --------------------------------------------------------------
   |  sendPG800Message -- Routine that sends message and shows  |
   |  what the parameter is on the screen                       |
   -------------------------------------------------------------- 
*/
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
   ---------------------------------------------------------------
   |  gatherControlSettings -- handles button depresses and sets |
   |  LED status                                                 |
   --------------------------------------------------------------- 
*/
void gatherControlSettings() {
  gatherPotentiometerValues();

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonStates[i] = !digitalRead(buttonPins[i]); // Active low

    if (buttonStates[i] && !prevButtonStates[i]) {
      // Rising edge: button just pressed
      log("button " + String(i + 1));

      uint8_t shift = (3 - i) * 2; // Maps button 0 to bits 6-7, 1 to 4-5, etc.
      uint8_t mask = 0b11 << shift;
      uint8_t current = (ledPattern & mask) >> shift;
      uint8_t next = cycleLedState(current);

      ledPattern = (ledPattern & ~mask) | (next << shift);
    }

    prevButtonStates[i] = buttonStates[i]; // Update for next check
  }

  bPrevEncoderBtn = bEncoderBtn;
  bEncoderBtn = !digitalRead(ENCODER_SW_PIN);
  updateEncoder();
}

/* 
   ---------------------------------------------------------------
   |  updateEncoder -- encoder handler                           |
   --------------------------------------------------------------- 
*/
void updateEncoder() {
  long newPosition = encoderKnob.read();

  if (newPosition != lastEncoderPosition) {
    if (newPosition > lastEncoderPosition) {
      log("CCW " + String(lastEncoderPosition>>2));
    } else {
      log("CW  " + String(lastEncoderPosition>>2));
    }

    lastEncoderPosition = newPosition;
  }
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

  if(i != -1) {
    // i ranges from 0 - 255
    int j = (float)(360 * ((float)i/255));
    drawKnobArrow(x,y,j);
  }
  else {
    if(bEncoderBtn)
        display.fillRect(x+2, y+1, 6, 4, SH110X_BLACK);
  }
}

void drawButton(int x, int y, bool red, bool green, uint8_t depressed) {
  display.fillRect(x+1, y, 8, 6, SH110X_WHITE);
  display.drawLine(x+2, y+2, x+7, y+2, SH110X_BLACK);
  
  if(depressed)
    display.fillRect(x+2, y+2, 6, 3, SH110X_BLACK);

  if(green) 
    display.drawLine(x+2,y+1, x+3, y+1, SH110X_BLACK);
  if(red)
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

/* --------------------------------------------------------------
   |  drawTestScreen()                                          |
   -------------------------------------------------------------- */
void drawTestScreen() {
    uint knobIX = 0;
    uint btnCtr = 3;
    uint knobpos;

    if(millis() - lastdrawTestScreen < 33) 
      return;

    lastdrawTestScreen = millis();

    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_BLACK); // clear screen
    display.setCursor(10,0);                                           // position for title
    display.printf("JMR800 TEST SCREEN");                              // title
    display.drawRect(0, 9, SCREEN_WIDTH, 54, SH110X_WHITE);            // box around UI
    display.drawRect(88, 15, 21, 11, SH110X_WHITE);                    // small screen rectangle
    display.setCursor(90, 17);
    display.printf("%03d", abs((lastEncoderPosition>>2) % 1000));
    for(int i=7; i<78; i = i + 10) {                                   // knobs on the left side
      for(int j=14; j<62; j = j + 8) {
          knobIX++;
          knobpos = getKnobValue(knobIX-1);
          drawKnob(i,j,knobpos);
      }
    }
    for(int i=87; i < 98; i = i + 10) {                               // knobs on the right side
      for(int j=30; j < 62; j = j + 8) {
        knobIX++;
        knobpos = getKnobValue(knobIX-1);
        drawKnob(i,j,knobpos);
      }
    }
    drawKnob(109, 18, -1);                                              // encoder knob

    for(int j=30; j < 62; j = j + 8) {                                 // buttons
      if(btnCtr == 3)
        drawButton(109, j, ledPattern & 0b10000000, ledPattern & 0b01000000, buttonStates[0]);
      else
      if(btnCtr == 2)
        drawButton(109, j, ledPattern & 0b00100000, ledPattern & 0b00010000, buttonStates[1]);
      else
      if(btnCtr == 1)
        drawButton(109, j, ledPattern & 0b00001000, ledPattern & 0b00000100, buttonStates[2]);
      else
        drawButton(109, j, ledPattern & 0b00000010, ledPattern & 0b00000001, buttonStates[3]);
      btnCtr--;
    }
    display.display();
}


void drawConfigKnobScreen() {
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_BLACK); // clear screen
  display.drawRect(87, 27, 1, 1, SH110X_WHITE);
  display.drawPixel(87, 27, SH110X_BLACK);
  display.drawRect(0, 8, 128, 56, SH110X_BLACK);
  display.drawRect(0, 0, 128, 64, SH110X_WHITE);
  display.drawLine(0, 0, 127, 0, SH110X_BLACK);
  display.drawLine(0, 0, 0, 8, SH110X_BLACK);
  display.drawLine(1, 9, 127, 10, SH110X_WHITE);
  display.drawLine(65, 9, 127, 9, SH110X_WHITE);
  display.drawLine(65, 10, 126, 10, SH110X_BLACK);
  display.drawLine(127, 8, 127, 1, SH110X_BLACK);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(4, 1);
  display.print("JMR800 Config Screen");
  display.setCursor(4, 11);
  display.setTextColor(SH110X_WHITE);
  display.print("Knob #1");
  display.setCursor(4, 20);
  display.setTextColor(SH110X_WHITE);
  display.print("Name:");
  display.setCursor(4, 29);
  display.setTextColor(SH110X_WHITE);
  display.print("CMD Byte:");
  display.setCursor(4, 38);
  display.setTextColor(SH110X_WHITE);
  display.print("Type:");
  display.drawRect(34, 38, 31, 7, SH110X_WHITE);
  display.drawLine(58, 39, 58, 44, SH110X_WHITE);
  display.drawLine(60, 41, 62, 41, SH110X_WHITE);
  display.drawLine(61, 42, 61, 42, SH110X_WHITE);
  display.drawLine(60, 40, 62, 40, SH110X_WHITE);
  display.setCursor(34, 53);
  display.setTextColor(SH110X_WHITE);
  display.print("OK");
  display.setCursor(62, 53);
  display.setTextColor(SH110X_WHITE);
  display.print("Cancel");
  display.drawRect(32, 51, 15, 10, SH110X_WHITE);
  display.drawPixel(32, 51, SH110X_WHITE);
  display.drawPixel(32, 51, SH110X_BLACK);
  display.drawPixel(46, 51, SH110X_BLACK);
  display.drawPixel(45, 60, SH110X_BLACK);
  display.drawPixel(44, 60, SH110X_BLACK);
  display.drawPixel(43, 60, SH110X_BLACK);
  display.drawPixel(42, 60, SH110X_BLACK);
  display.drawPixel(41, 60, SH110X_BLACK);
  display.drawPixel(40, 60, SH110X_BLACK);
  display.drawPixel(38, 60, SH110X_BLACK);
  display.drawPixel(39, 60, SH110X_BLACK);
  display.drawPixel(37, 60, SH110X_BLACK);
  display.drawPixel(36, 60, SH110X_BLACK);
  display.drawPixel(35, 60, SH110X_BLACK);
  display.drawPixel(34, 60, SH110X_BLACK);
  display.drawPixel(33, 60, SH110X_BLACK);
  display.drawLine(33, 61, 45, 61, SH110X_WHITE);
  display.drawRect(60, 51, 39, 11, SH110X_WHITE);
  display.drawPixel(60, 51, SH110X_BLACK);
  display.drawPixel(60, 61, SH110X_BLACK);
  display.drawPixel(98, 61, SH110X_BLACK);
  display.drawPixel(98, 51, SH110X_BLACK);

  display.display();
}

void drawConfigSelectionScreen() {
    uint btnCtr = 3;

    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_BLACK); // clear screen
    display.setCursor(0,0);                                           // position for title
    display.printf("JMR800 Config- Pick");                              // title
    display.drawRect(0, 9, SCREEN_WIDTH, 54, SH110X_WHITE);            // box around UI
    display.drawRect(88, 15, 21, 11, SH110X_WHITE);                    // small screen rectangle
    for(int i=7; i<78; i = i + 10) {                                   // knobs on the left side
      for(int j=14; j<62; j = j + 8) {
          drawKnob(i,j,0);
      }
    }
    for(int i=87; i < 98; i = i + 10) {                               // knobs on the right side
      for(int j=30; j < 62; j = j + 8) {
        drawKnob(i,j,0);
      }
    }
    drawKnob(109, 18, -1);                                              // encoder knob

    for(int j=30; j < 62; j = j + 8) {                                 // buttons
      if(btnCtr == 3)
        drawButton(109, j, ledPattern & 0b10000000, ledPattern & 0b01000000, buttonStates[0]);
      else
      if(btnCtr == 2)
        drawButton(109, j, ledPattern & 0b00100000, ledPattern & 0b00010000, buttonStates[1]);
      else
      if(btnCtr == 1)
        drawButton(109, j, ledPattern & 0b00001000, ledPattern & 0b00000100, buttonStates[2]);
      else
        drawButton(109, j, ledPattern & 0b00000010, ledPattern & 0b00000001, buttonStates[3]);
      btnCtr--;
    }
    display.display();
}

void drawConfigScreen() {
  if(millis() - lastdrawTestScreen < 33) 
    return;

  lastdrawTestScreen = millis();

  if(systemSubMode == SUBMODE_2) {
    drawConfigKnobScreen();
  }
  else {
    drawConfigSelectionScreen();
  }
}

void drawRunningScreen() {
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_BLACK); // clear screen
  display.setCursor(0,0);
  display.print("Run screen here");
  display.display();
}

/* --------------------------------------------------------------
   |  Initialization                                            |
   -------------------------------------------------------------- */
void setup() {
  Serial.begin(115200);

  systemMode = MODE_TEST;
  systemSubMode = SUBMODE_1;

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

  log("Attaching interrupts...");

  attachInterrupt(digitalPinToInterrupt(CLOCK_IN_PIN), onPG800ClockFall, FALLING);

  log("Setting up DAC tables...");

  for(uint8_t i=0; i< NUM_MUXES; i++) {
      for(uint8_t j=0;j<16;j++) {
          AnalogValues[j][i] = 0;
          oldAnalogValues[j][i] = 0;
      }
  }

  log("Initialized.");
}


/* --------------------------------------------------------------
   |  Main loop                                                 |
   -------------------------------------------------------------- */
void loop() {
  gatherControlSettings();

  if(systemMode == MODE_CONFIG) {
    if(systemSubMode == SUBMODE_1) {
      if(bPrevEncoderBtn != bEncoderBtn) {
        systemSubMode = SUBMODE_2;
      }
    }
  }

  if(buttonStates[0] && buttonStates[1]) {
    if(systemMode == MODE_TEST) {
      systemMode = MODE_CONFIG;
      systemSubMode = SUBMODE_1;
      delay(500);
    }
    else if (systemMode == MODE_CONFIG) {
      systemMode = MODE_RUNNING;
      delay(500);
    }
    else if (systemMode == MODE_RUNNING) {
      systemMode = MODE_TEST;
      delay(500);
    }
  }

  if(systemMode == MODE_TEST) {
    drawTestScreen();
    setLEDs(ledPattern);
    delay(10);
  }
  else if (systemMode == MODE_CONFIG) {
    drawConfigScreen();
    setLEDs(0b00000000);
    delay(10);
  }
  else { // RUNNING MODE
    drawRunningScreen();
    setLEDs(0b00000000);
    delay(10);
  }
}

