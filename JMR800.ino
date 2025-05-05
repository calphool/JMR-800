#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Wire.h>
#include <stdarg.h>
#include <ADC.h>
#include "JX8P.h"
#include <Encoder.h>
#include <EEPROM.h>
#include <Fonts/TomThumb.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // note, for WIRE on Teensy 4.1
                                                                                             // SDA = 18, SCL = 19
ADC adc;


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


// TYPE related definitions and structures
#define NUM_TYPE_CODES 9

#define TYPE_CODE_RANGE 0
#define TYPE_CODE_WAVE_FORM 1
#define TYPE_CODE_3_2_1_OFF 2
#define TYPE_CODE_0_TO_10 3
#define TYPE_CODE_OCTAVE 4
#define TYPE_CODE_MODE 5
#define TYPE_CODE_ENV2_GATE 6
#define TYPE_CODE_LFO_WAVE_FORM 7
#define TYPE_CODE_2_1_OFF 8

typedef struct {
  char typeCodeName[15];
  uint cd;
}
typeCode;

typeCode typeCodes[NUM_TYPE_CODES];



// system modes
#define MODE_TEST 0
#define MODE_CONFIG 1
#define MODE_RUNNING 2

// Config knob selector screen
#define SUBMODE_1 0

// Config knob update screen
#define SUBMODE_2 1

// Config knob update name field
#define SUBMODE_2_NAME 2

// Config knob update CMD Byte
#define SUBMODE_2_CMD 3

// Config knob update Type
#define SUBMODE_2_TYPE 4

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

// toggle used for flickering in UI
int toggle = -1;

// cursor position for text editing
uint textCursorPos = 0;

// active knob for configuration screen
long configKnobID = -1;


// Knob configuration related defines and structures
#define KNOB_CONFIG_NAME_HIGHLIGHTED 0
#define KNOB_CONFIG_CMD_HIGHLIGHTED 1
#define KNOB_CONFIG_TYPECMD_HIGHLIGHTED 2
#define KNOB_CONFIG_OKAY_HIGHLIGHTED 3
#define KNOB_CONFIG_CANCEL_HIGHLIGHTED 4

// knob configuration structure
typedef struct {
  char name[15];
  uint8_t cmdbyte;
  uint8_t typecode;
} knobConfig;

// array of knob configurations
#define NUM_KNOBS 56
knobConfig knobConfigurations[NUM_KNOBS];
knobConfig knobConfigurations_bkup[NUM_KNOBS];
knobConfig lastKnobConfig;

// small text buffer used in various places, mostly for text buffers
char buffer[25];


// check if the knobconfiguration has changed at this location
bool knobChanged(int i) {
  return memcmp(&knobConfigurations[i], &knobConfigurations_bkup[i], sizeof(knobConfig)) != 0;
}

// Cycles through 0b00 → 0b10 → 0b01 → 0b00 (off → left → right → off)
uint8_t cycleLedState(uint8_t current) {
  if (current == 0b00) return 0b10;
  if (current == 0b10) return 0b01;
  return 0b00;
}

/* --------------------------------------------------------------
   |  Set the muxes address value                               |
   |      Invoked by: gatherPotentiometerValues()               |
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



/* --------------------------------------------------------------
   |  Logging routine                                           |
   -------------------------------------------------------------- */
void log(const String& s) {
  Serial.println(s);
  Serial.flush();
}


/* --------------------------------------------------------------
   |  Shift register routine for LEDs                           |
   -------------------------------------------------------------- */
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

/* --------------------------------------------------------------
   |  sendParameter -- code that sends data to synthesizer      |
   -------------------------------------------------------------- */
void sendParameter(uint8_t paramID, uint8_t value) {
  long start = millis();
  while (bitIndex != -1 && (millis() - start < 500) );  // Wait until previous transfer is complete
  if(millis() - start >= 500) {
    log("problem waiting for bitIndex in sendParameter()");
    return;
  }

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


/* --------------------------------------------------------------
   |  onPG800ClockFall -- loop that synchronizes the controller |
   |  with the synthesizer                                      |
   -------------------------------------------------------------- */
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

/* --------------------------------------------------------------
   |  sendPG800Message -- Routine that sends message and shows  |
   |  what the parameter is on the screen                       |
   -------------------------------------------------------------- */
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

/* ---------------------------------------------------------------
   |  gatherControlSettings -- handles button depresses and sets |
   |  LED status                                                 |
   --------------------------------------------------------------- */
void gatherControlSettings() {
  gatherPotentiometerValues();

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonStates[i] = !digitalRead(buttonPins[i]); // Active low

    if (buttonStates[i] && !prevButtonStates[i]) {
      // Rising edge: button just pressed
      //log("button " + String(i + 1));

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

/* ---------------------------------------------------------------
   |  updateEncoder -- encoder handler                           |
   --------------------------------------------------------------- */
void updateEncoder() {
  long newPosition = encoderKnob.read();

  if(newPosition < 0) {
    encoderKnob.write(0);
    newPosition = 0;
  }

  if (newPosition != lastEncoderPosition) {
    if (newPosition > lastEncoderPosition) {
      //log("CCW " + String(lastEncoderPosition>>2));
    } else {
      //log("CW  " + String(lastEncoderPosition>>2));
    }

    lastEncoderPosition = newPosition;
  }
}



/* ---------------------------------------------------------------
   |  drawKnobArrow - draws the arrow on the knob                |
   --------------------------------------------------------------- */
void drawKnobArrow(int x, int y, int deg) {
  int cx = x + 4;
  int cy = y + 3;

  deg = deg - 45;
  if(deg < 0)
    deg += 360;

  // Apply 120-degree rotation offset (like original float math)
  int index = deg % 360;

  int ex = cx + arrow_dx[index];
  int ey = cy + arrow_dy[index];

  display.drawLine(cx, cy, ex, ey, SH110X_BLACK);
}


/* -------------------------------------------------------------------------
   |  getActiveKnob() - returns the active knob modded by the value passed |
   ------------------------------------------------------------------------- */
long getActiveKnob(long divisor) {
    long i = (lastEncoderPosition>>2) % divisor; 
    if(i < 0)
      i = 0;
    if(i > divisor)
      i = divisor-1;
    
    return i;
}


/* .-----------------------------------------------------------------------------------------------------.
   |  drawKnob() - draws a knob at a given x, y location with a given i value from 0 - 255 for the arrow |
   '-----------------------------------------------------------------------------------------------------' */
void drawKnob(int knobid, int x, int y, int i) {
  display.fillRect(x+2, y+1, 6, 4, SH110X_WHITE);
  display.drawLine(x+3, y, x+6, y, SH110X_WHITE);
  display.drawLine(x+3, y+5, x+6, y+5, SH110X_WHITE);
  display.drawLine(x+1, y+2, x+1, y+3, SH110X_WHITE);
  display.drawLine(x+8, y+2, x+8, y+3, SH110X_WHITE);

  if(systemMode == MODE_CONFIG) {
    if(getActiveKnob(NUM_KNOBS) == knobid) {
      toggle = -toggle;
      if(toggle > 0) {
          display.fillRect(x+2, y+1, 6, 4, SH110X_BLACK);
      }
      else {
          display.fillRect(x+2, y+1, 6, 4, SH110X_WHITE);
      }
    }
  }

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

/* .-------------------------------------------------------------------------------------------------------------.
   |  drawButton() - Draws a push button at a given x, y position, with the LEDs drawn and whether its depressed |
   '-------------------------------------------------------------------------------------------------------------' */
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


/* .---------------------------------------------------------------------------------------------------------------.
   |  getKnobValue() - returns the 0-255 value of a given potentiometer (translates wiring issues through Xformer) |
   '---------------------------------------------------------------------------------------------------------------' */
inline uint getKnobValue(uint knobIX) {
  knobIX = knobXformer[knobIX];              // transform this index (necessary because of wiring) 
  uint mux = knobIX >> 4;
  uint mux_ix = knobIX & 0x0F;

  return 255-AnalogValues[mux_ix][mux];
}

bool knobValueChanged(uint knobIX) {
  knobIX = knobXformer[knobIX];              // transform this index (necessary because of wiring) 
  uint mux = knobIX >> 4;
  uint mux_ix = knobIX & 0x0F;

/*
  if(knobIX == 0) {
    log(String(AnalogValues[mux_ix][mux]) + " / " + oldAnalogValues[mux_ix][mux]);
  }
*/
  if(AnalogValues[mux_ix][mux] == oldAnalogValues[mux_ix][mux])
    return false;

  return true;
}

uint knobValueAt(uint knobIX) {
  knobIX = knobXformer[knobIX];              // transform this index (necessary because of wiring) 
  uint mux = knobIX >> 4;
  uint mux_ix = knobIX & 0x0F;

  return AnalogValues[mux_ix][mux] >> 1; // 0 - 127 returned value
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
          drawKnob(knobIX-1,i,j,knobpos);
      }
    }
    for(int i=87; i < 98; i = i + 10) {                               // knobs on the right side
      for(int j=30; j < 62; j = j + 8) {
        knobIX++;
        knobpos = getKnobValue(knobIX-1);
        drawKnob(knobIX-1,i,j,knobpos);
      }
    }
    drawKnob(-1, 109, 18, -1);                                              // encoder knob

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
  long highlight = getActiveKnob(5);
  toggle = -toggle;
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
  display.print("Config - Update");
  display.setCursor(4, 11);
  display.setTextColor(SH110X_WHITE);
  display.print("Knob #" + String(configKnobID));

  if(highlight == KNOB_CONFIG_NAME_HIGHLIGHTED && systemSubMode == SUBMODE_2) {
    if(toggle > 0)
      display.drawRect(38,19,84,11, SH110X_WHITE);
    else
      display.drawRect(38,19,84,11, SH110X_BLACK);
  }
  display.setCursor(4, 21);
  display.setTextColor(SH110X_WHITE);
  display.print("Name: " + String(knobConfigurations[configKnobID].name));
  
  if(highlight == KNOB_CONFIG_CMD_HIGHLIGHTED && systemSubMode == SUBMODE_2) {
    if(toggle > 0)
      display.drawRect(61,28,29,11, SH110X_WHITE);
    else
      display.drawRect(61,28,29,11, SH110X_BLACK);
  }
  display.setCursor(4, 30);
  display.setTextColor(SH110X_WHITE);
  sprintf(buffer, "CMD Byte: 0x%02X",  knobConfigurations[configKnobID].cmdbyte);
  display.print(buffer);

  if(highlight == KNOB_CONFIG_TYPECMD_HIGHLIGHTED && systemSubMode == SUBMODE_2) {
    if(toggle > 0)
      display.drawRect(32,37,90,11, SH110X_WHITE);
    else
      display.drawRect(32,37,90,11, SH110X_BLACK);
  }
  display.setCursor(4, 39);
  display.setTextColor(SH110X_WHITE);
  display.print("Type:" + String(typeCodes[knobConfigurations[configKnobID].typecode].typeCodeName));

  if(highlight == KNOB_CONFIG_OKAY_HIGHLIGHTED && systemSubMode == SUBMODE_2) {
    if(toggle > 0)
      display.drawRect(30,51,19,11, SH110X_WHITE);
    else
      display.drawRect(30,51,19,11, SH110X_BLACK);
  }

  display.setCursor(34, 53);
  display.setTextColor(SH110X_WHITE);
  display.print("OK");

  if(highlight == KNOB_CONFIG_CANCEL_HIGHLIGHTED && systemSubMode == SUBMODE_2) {
    if(toggle > 0)
      display.drawRect(58,51,44,11, SH110X_WHITE);
    else
      display.drawRect(58,51,44,11, SH110X_BLACK);
  }

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

  if(systemSubMode == SUBMODE_2_CMD ) {
    display.fillRect (15,15,100,45,SH110X_BLACK);
    display.drawRect (15,15,100,45,SH110X_WHITE);
    display.drawPixel(15,15,SH110X_BLACK);
    display.drawPixel(15,60,SH110X_BLACK);
    display.drawPixel(115,15,SH110X_BLACK);
    display.drawPixel(115,60,SH110X_BLACK);
    display.setCursor(55,33);
    sprintf(buffer, "0x%02X",  knobConfigurations[configKnobID].cmdbyte);
    display.print(buffer);
  }
  else
  if(systemSubMode == SUBMODE_2_TYPE) {
    display.fillRect (20,8,80,56,SH110X_BLACK);
    display.drawRect (20,8,80,56,SH110X_WHITE);
    display.setFont(&TomThumb);
    display.setTextColor(SH110X_WHITE);

    for(uint i=0; i < NUM_TYPE_CODES; i++) {
      display.setCursor(40, 14 + i*6);
      display.print(typeCodes[i].typeCodeName);
      if(i == textCursorPos) {
        if(toggle > 0) {
          display.setTextColor(SH110X_WHITE);
          display.setCursor(35, 14 + i*6);
          display.print(">");
        }
        else {
          display.setTextColor(SH110X_BLACK);
          display.setCursor(35, 14 + i*6);
          display.print(">");
          display.setTextColor(SH110X_WHITE);
        }
      }
    }

    display.setFont(); 
    display.setTextColor(SH110X_WHITE);
  }
  else
  if(systemSubMode == SUBMODE_2_NAME) {
    display.fillRect (7,12,114,48,SH110X_BLACK);
    display.drawRect (7,12,114,48,SH110X_WHITE);
    display.setCursor(8,50);
    display.print(" B1=Left  B2=Right");
    display.setCursor(21,21);
    uint slen = strlen(knobConfigurations[configKnobID].name);
    
    for(uint i=0; i < 15; i++) {
      display.setCursor(13 + i*7, 21);
      if(i < slen) {
        display.write(knobConfigurations[configKnobID].name[i]);
      }
      if(i == textCursorPos) {
        if(toggle > 0)
          display.drawLine(13 + i*7, 30, 17 + i*7, 30, SH110X_WHITE);
        else
          display.drawLine(13 + i*7, 30, 17 + i*7, 30, SH110X_BLACK);
      }
      else 
          display.drawLine(13 + i*7, 30, 17 + i*7, 30, SH110X_WHITE);
    }
  }

  display.display();
}

void drawConfigSelectionScreen() {
    uint btnCtr = 3;
    uint8_t iKnobCtr = 0;

    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_BLACK); // clear screen
    display.setCursor(0,0);                                            // position for title
    display.printf("Config - Pick Knob");                              // title
    display.drawRect(0, 9, SCREEN_WIDTH, 54, SH110X_WHITE);            // box around UI
    display.drawRect(88, 15, 21, 11, SH110X_WHITE);                    // small screen rectangle
    for(int i=7; i<78; i = i + 10) {                                   // knobs on the left side
      for(int j=14; j<62; j = j + 8) {
          drawKnob(iKnobCtr, i,j,0);
          iKnobCtr++;
      }
    }
    for(int i=87; i < 98; i = i + 10) {                               // knobs on the right side
      for(int j=30; j < 62; j = j + 8) {
        drawKnob(iKnobCtr, i,j,0);
        iKnobCtr++;
      }
    }
    drawKnob(-1,109, 18, -1);                                              // encoder knob
    iKnobCtr++;


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

  if(systemSubMode == SUBMODE_2 || systemSubMode == SUBMODE_2_NAME || systemSubMode == SUBMODE_2_CMD || systemSubMode == SUBMODE_2_TYPE) {
    drawConfigKnobScreen();
  }
  else {
    drawConfigSelectionScreen();
  }
}

static uint lastChangedKnob = 0;
void drawRunningScreen() {
  if(millis() - lastdrawTestScreen < 33) 
    return;

  lastdrawTestScreen = millis();

  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_BLACK); // clear screen
  display.setTextColor(SH110X_WHITE);
  //display.setCursor(20,10);
  //display.print("Running...");
  if(millis() < 10000) {
    display.setCursor(20,10);
    display.print("Booting...");
    display.display();
    return;
  }

  display.setCursor(20,20);
  boolean bOneChanged = false;
  for(uint i = 0; i < NUM_KNOBS; i++) {
    if(knobValueChanged(i) && !bOneChanged) {
        bOneChanged = true;
        lastChangedKnob = i;
        display.print(knobConfigurations[i].name);
        display.setCursor(20,30);
        display.print(typeCodes[knobConfigurations[i].typecode].typeCodeName);
        display.setCursor(20,40);
        display.print(knobValueAt(i));
    }
  }

  if(bOneChanged == false) {
        display.print(knobConfigurations[lastChangedKnob].name);
        display.setCursor(20,30);
        display.print(typeCodes[knobConfigurations[lastChangedKnob].typecode].typeCodeName);
        display.setCursor(20,40);
        display.print(knobValueAt(lastChangedKnob));
  }

  display.display();
}



/* .-------------------------------------------------------------------.
   |  saveKnobs() - saves knob settings to EEPROM if they have changed |
   '-------------------------------------------------------------------' */
void saveKnobs() {
  for (int i = 0; i < NUM_KNOBS; i++) {
    while(strlen(knobConfigurations[i].name) < 14)
      strcat(knobConfigurations[i].name," ");
    int addr = i * sizeof(knobConfig);

    if(knobChanged(i)) { // optimization that reduces wear on EEPROM
      EEPROM.put(addr, knobConfigurations[i]);
      //log("knob changed:     " + String(i));
    }
    else {
      //log("knob not changed: " + String(i));
    }
  }
  memcpy(knobConfigurations_bkup, knobConfigurations, sizeof(knobConfigurations));
}

/* .------------------------------------------------------------.
   |  loadKnobs() - loads knob settings from EEPROM             |
   '------------------------------------------------------------' */
void loadKnobs() {
  for (int i = 0; i < NUM_KNOBS; i++) {
    int addr = i * sizeof(knobConfig);
    EEPROM.get(addr, knobConfigurations[i]);
    while(strlen(knobConfigurations[i].name) < 14)
      strcat(knobConfigurations[i].name," ");
  }

  memcpy(knobConfigurations_bkup, knobConfigurations, sizeof(knobConfigurations));
}

/* .------------------------------------------------------------.
   |  Initialization                                            |
   '------------------------------------------------------------' */
void setup() {
  Serial.begin(9600);
  EEPROM.begin();
  loadKnobs(); 

  typeCodes[TYPE_CODE_RANGE].cd = TYPE_CODE_RANGE;
  strcpy(typeCodes[TYPE_CODE_RANGE].typeCodeName, "Range");
  typeCodes[TYPE_CODE_WAVE_FORM].cd = TYPE_CODE_WAVE_FORM;
  strcpy(typeCodes[TYPE_CODE_WAVE_FORM].typeCodeName, "Wave Form");
  typeCodes[TYPE_CODE_3_2_1_OFF].cd = TYPE_CODE_3_2_1_OFF;
  strcpy(typeCodes[TYPE_CODE_3_2_1_OFF].typeCodeName, "3-2-1-Off");
  typeCodes[TYPE_CODE_0_TO_10].cd = TYPE_CODE_0_TO_10;
  strcpy(typeCodes[TYPE_CODE_0_TO_10].typeCodeName, "0 To 10");
  typeCodes[TYPE_CODE_OCTAVE].cd = TYPE_CODE_OCTAVE;
  strcpy(typeCodes[TYPE_CODE_OCTAVE].typeCodeName, "Octave");
  typeCodes[TYPE_CODE_MODE].cd = TYPE_CODE_MODE;
  strcpy(typeCodes[TYPE_CODE_MODE].typeCodeName, "Mode");
  typeCodes[TYPE_CODE_ENV2_GATE].cd = TYPE_CODE_ENV2_GATE;
  strcpy(typeCodes[TYPE_CODE_ENV2_GATE].typeCodeName, "Env2-Gate");
  typeCodes[TYPE_CODE_LFO_WAVE_FORM].cd = TYPE_CODE_LFO_WAVE_FORM;
  strcpy(typeCodes[TYPE_CODE_LFO_WAVE_FORM].typeCodeName, "LFO Wave Form");
  typeCodes[TYPE_CODE_2_1_OFF].cd = TYPE_CODE_2_1_OFF;
  strcpy(typeCodes[TYPE_CODE_2_1_OFF].typeCodeName, "2-1-Off");

  systemMode = MODE_RUNNING;
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

  // code to initialize the EEProm configuration if it hasn't already been set -- don't run this unless you're on a new Teensy, and only do it once
  /*
  strcpy(knobConfigurations[0].name, "DCO1 Range"); knobConfigurations[0].cmdbyte = 0x80;
  strcpy(knobConfigurations[1].name, "DCO1 Waveform"); knobConfigurations[1].cmdbyte = 0x81;
  strcpy(knobConfigurations[2].name, "DCO1 Tune"); knobConfigurations[2].cmdbyte = 0x82;
  strcpy(knobConfigurations[3].name, "DCO1 LFO Depth"); knobConfigurations[3].cmdbyte = 0x83;
  strcpy(knobConfigurations[4].name, "DCO1 Env Depth"); knobConfigurations[4].cmdbyte = 0x84;
  strcpy(knobConfigurations[5].name, "DCO2 Range"); knobConfigurations[5].cmdbyte = 0x85;
  strcpy(knobConfigurations[6].name, "DCO2 Waveform"); knobConfigurations[6].cmdbyte = 0x86;
  strcpy(knobConfigurations[7].name, "DCO XMOD"); knobConfigurations[7].cmdbyte = 0x87;
  strcpy(knobConfigurations[8].name, "DCO2 Tune"); knobConfigurations[8].cmdbyte = 0x88;
  strcpy(knobConfigurations[9].name, "DCO2 Fine Tune"); knobConfigurations[9].cmdbyte = 0x89;
  strcpy(knobConfigurations[10].name, "DCO2 LFO Depth"); knobConfigurations[10].cmdbyte = 0x8A;
  strcpy(knobConfigurations[11].name, "DCO2 Env Depth"); knobConfigurations[11].cmdbyte = 0x8B;
  strcpy(knobConfigurations[12].name, "DCO Dynamics"); knobConfigurations[12].cmdbyte = 0x8F;
  strcpy(knobConfigurations[13].name, "DCO EG Mode"); knobConfigurations[13].cmdbyte = 0x90;
  strcpy(knobConfigurations[14].name, "Mix DCO1"); knobConfigurations[14].cmdbyte = 0x91;
  strcpy(knobConfigurations[15].name, "Mix DCO2"); knobConfigurations[15].cmdbyte = 0x92;
  strcpy(knobConfigurations[16].name, "Mix Env"); knobConfigurations[16].cmdbyte = 0x93;
  strcpy(knobConfigurations[17].name, "Mix Dynamics"); knobConfigurations[17].cmdbyte = 0x94;
  strcpy(knobConfigurations[18].name, "Mix EG Mode"); knobConfigurations[18].cmdbyte = 0x95;
  strcpy(knobConfigurations[19].name, "VCF HPF"); knobConfigurations[19].cmdbyte = 0x96;
  strcpy(knobConfigurations[20].name, "VCF Freq"); knobConfigurations[20].cmdbyte = 0x97;
  strcpy(knobConfigurations[21].name, "VCF Resonance"); knobConfigurations[21].cmdbyte = 0x98;
  strcpy(knobConfigurations[22].name, "VCF LFO"); knobConfigurations[22].cmdbyte = 0x99;
  strcpy(knobConfigurations[23].name, "VCF Env"); knobConfigurations[23].cmdbyte = 0x9A;
  strcpy(knobConfigurations[24].name, "VCF Key"); knobConfigurations[24].cmdbyte = 0x9B;
  strcpy(knobConfigurations[25].name, "VCF Dynamics"); knobConfigurations[25].cmdbyte = 0x9C;
  strcpy(knobConfigurations[26].name, "VCF EG Mode"); knobConfigurations[26].cmdbyte = 0x9D;
  strcpy(knobConfigurations[27].name, "VCA Level"); knobConfigurations[27].cmdbyte = 0x9E;
  strcpy(knobConfigurations[28].name, "VCA Dynamics"); knobConfigurations[28].cmdbyte = 0x9F;
  strcpy(knobConfigurations[29].name, "Chorus"); knobConfigurations[29].cmdbyte = 0xA0;
  strcpy(knobConfigurations[30].name, "LFO Waveform"); knobConfigurations[30].cmdbyte = 0xA1;
  strcpy(knobConfigurations[31].name, "LFO Delay"); knobConfigurations[31].cmdbyte = 0xA2;
  strcpy(knobConfigurations[32].name, "LFO Rate"); knobConfigurations[32].cmdbyte = 0xA3;
  strcpy(knobConfigurations[33].name, "EG Env1 Attk"); knobConfigurations[33].cmdbyte = 0xA4;
  strcpy(knobConfigurations[34].name, "EG Env1 Decay"); knobConfigurations[34].cmdbyte = 0xA5;
  strcpy(knobConfigurations[35].name, "EG Env1 Sust"); knobConfigurations[35].cmdbyte = 0xA6;
  strcpy(knobConfigurations[36].name, "EG Env1 Rel"); knobConfigurations[36].cmdbyte = 0xA7;
  strcpy(knobConfigurations[37].name, "EG Env1 Key"); knobConfigurations[37].cmdbyte = 0xA8;
  strcpy(knobConfigurations[38].name, "EG Env2 Attk"); knobConfigurations[38].cmdbyte = 0xA9;
  strcpy(knobConfigurations[39].name, "EG Env2 Decay"); knobConfigurations[39].cmdbyte = 0xAA;
  strcpy(knobConfigurations[40].name, "EG Env2 Sust"); knobConfigurations[40].cmdbyte = 0xAB;
  strcpy(knobConfigurations[41].name, "EG Env2 Rel"); knobConfigurations[41].cmdbyte = 0xAC;
  strcpy(knobConfigurations[42].name, "EG Env2 Key"); knobConfigurations[42].cmdbyte = 0xAD;
  strcpy(knobConfigurations[43].name, "VCA EG Mode"); knobConfigurations[43].cmdbyte = 0xAF;
  strcpy(knobConfigurations[44].name, "PMW1 Width"); knobConfigurations[44].cmdbyte = 0xB0;
  strcpy(knobConfigurations[45].name, "PWM1 Env"); knobConfigurations[45].cmdbyte = 0xB1;
  strcpy(knobConfigurations[46].name, "PMW1 LFO"); knobConfigurations[46].cmdbyte = 0xB2;
  strcpy(knobConfigurations[47].name, "PWM2 Width"); knobConfigurations[47].cmdbyte = 0xB3;
  strcpy(knobConfigurations[48].name, "PWM2 Env"); knobConfigurations[48].cmdbyte = 0xB4;
  strcpy(knobConfigurations[49].name, "PWM2 LFO"); knobConfigurations[49].cmdbyte = 0xB5;
  strcpy(knobConfigurations[50].name, "PWM Dyna"); knobConfigurations[50].cmdbyte = 0xC4;
  strcpy(knobConfigurations[51].name, "LFO Sync"); knobConfigurations[51].cmdbyte = 0xC6;
  strcpy(knobConfigurations[52].name, "PWM Mode"); knobConfigurations[52].cmdbyte = 0xCC;
  strcpy(knobConfigurations[53].name, "DCO1 Range"); knobConfigurations[53].cmdbyte = 0x80;
  strcpy(knobConfigurations[54].name, "DCO1 Waveform"); knobConfigurations[54].cmdbyte = 0x81;
  strcpy(knobConfigurations[55].name, "DCO1 Tune"); knobConfigurations[55].cmdbyte = 0x82;
  knobConfigurations[0].typecode = TYPE_CODE_RANGE; knobConfigurations[1].typecode = TYPE_CODE_WAVE_FORM;
  knobConfigurations[2].typecode = TYPE_CODE_OCTAVE; knobConfigurations[3].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[4].typecode = TYPE_CODE_0_TO_10; knobConfigurations[5].typecode = TYPE_CODE_RANGE;
  knobConfigurations[6].typecode = TYPE_CODE_WAVE_FORM; knobConfigurations[7].typecode = TYPE_CODE_3_2_1_OFF;
  knobConfigurations[8].typecode = TYPE_CODE_OCTAVE; knobConfigurations[9].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[10].typecode = TYPE_CODE_0_TO_10; knobConfigurations[11].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[12].typecode = TYPE_CODE_3_2_1_OFF; knobConfigurations[13].typecode = TYPE_CODE_MODE;
  knobConfigurations[14].typecode = TYPE_CODE_0_TO_10; knobConfigurations[15].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[16].typecode = TYPE_CODE_0_TO_10; knobConfigurations[17].typecode = TYPE_CODE_3_2_1_OFF;
  knobConfigurations[18].typecode = TYPE_CODE_MODE; knobConfigurations[19].typecode = TYPE_CODE_3_2_1_OFF;
  knobConfigurations[20].typecode = TYPE_CODE_0_TO_10; knobConfigurations[21].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[22].typecode = TYPE_CODE_0_TO_10; knobConfigurations[23].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[24].typecode = TYPE_CODE_0_TO_10; knobConfigurations[25].typecode = TYPE_CODE_3_2_1_OFF;
  knobConfigurations[26].typecode = TYPE_CODE_MODE; knobConfigurations[27].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[28].typecode = TYPE_CODE_3_2_1_OFF; knobConfigurations[29].typecode = TYPE_CODE_2_1_OFF;
  knobConfigurations[30].typecode = TYPE_CODE_LFO_WAVE_FORM; knobConfigurations[31].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[32].typecode = TYPE_CODE_0_TO_10; knobConfigurations[33].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[34].typecode = TYPE_CODE_0_TO_10; knobConfigurations[35].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[36].typecode = TYPE_CODE_0_TO_10; knobConfigurations[37].typecode = TYPE_CODE_3_2_1_OFF;
  knobConfigurations[38].typecode = TYPE_CODE_0_TO_10; knobConfigurations[39].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[40].typecode = TYPE_CODE_0_TO_10; knobConfigurations[41].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[42].typecode = TYPE_CODE_3_2_1_OFF; knobConfigurations[43].typecode = TYPE_CODE_ENV2_GATE;
  knobConfigurations[44].typecode = TYPE_CODE_3_2_1_OFF; knobConfigurations[45].typecode = TYPE_CODE_ENV2_GATE;
  knobConfigurations[46].typecode = TYPE_CODE_0_TO_10; knobConfigurations[47].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[48].typecode = TYPE_CODE_0_TO_10; knobConfigurations[49].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[50].typecode = TYPE_CODE_0_TO_10; knobConfigurations[51].typecode = TYPE_CODE_0_TO_10;
  knobConfigurations[52].typecode = TYPE_CODE_0_TO_10; knobConfigurations[53].typecode = TYPE_CODE_RANGE;
  knobConfigurations[54].typecode = TYPE_CODE_WAVE_FORM; knobConfigurations[55].typecode = TYPE_CODE_OCTAVE;
  saveKnobs();
  */


  log("Initialized.");
}


/* .----------------------------------------------------------------------------------------------------.
   |  AsciiToEncoder() routine that translates an ASCII value to the appropriate encoder value          |
   '----------------------------------------------------------------------------------------------------' */
int AsciiToEncoder(char c) {
  int v = ((((int)c) - 65) * 4) + 132;
  return v;
}


/* .----------------------------------------------------------------------------------------------------.
   |  handleControlStatus() loop - gathers up control values and set status state machine based on them |
   '----------------------------------------------------------------------------------------------------' */
void handleControlStatus() {
  gatherControlSettings(); // sweep all the controls, store their state in a buffer

  if(systemMode == MODE_RUNNING) {
    for(uint i = 0; i < NUM_KNOBS; i++) {
      if(knobValueChanged(i) && millis() > 10000) {
        sendParameter(knobConfigurations[i].cmdbyte, knobValueAt(i));
      }
    }
  }
  
  if(systemMode == MODE_CONFIG) {  // if we're in Config mode
    if(systemSubMode == SUBMODE_1) {  // and we're in the knob selection screen
      if(!bPrevEncoderBtn && bEncoderBtn) {  // and the user clicked on encoder button
        bPrevEncoderBtn = bEncoderBtn;  // set the encoder previous state to the current state so we don't loop
        systemSubMode = SUBMODE_2;     // set the sub mode to the knob configuration screen
        configKnobID = getActiveKnob(NUM_KNOBS);   // this is the knob they clicked on
        memcpy(&lastKnobConfig, &knobConfigurations[configKnobID], sizeof(lastKnobConfig)); // make a backup of this knob's settings in case they click cancel
        return;
      }
    }
  }

  if(systemMode == MODE_CONFIG) {  // if we're in Config mode
    if(systemSubMode == SUBMODE_2_CMD) {  // and we were in the knob configuration screen, but now are adjusting the CMD byte
      if(!bPrevEncoderBtn && bEncoderBtn) {  // and the user clicked the encoder button
        systemSubMode = SUBMODE_2;  // return back to the knob configuration screen
        bPrevEncoderBtn = bEncoderBtn; // set the encoder previous state to the current state so we don't loop
        return;
      }
    }
  }

  if(systemMode == MODE_CONFIG) {   // if we're in Config mode
    if(systemSubMode == SUBMODE_2) {   // and we're in the knob configuration screen
      if(!bPrevEncoderBtn && bEncoderBtn) {  // and the user clicked the encoder knob
        if(getActiveKnob(5)  == KNOB_CONFIG_CANCEL_HIGHLIGHTED) { // and they had the cancel icon highlighted
            memcpy(&knobConfigurations[configKnobID], &lastKnobConfig, sizeof(lastKnobConfig)); // restore its knob's configuration to the save value (cancel)
            systemSubMode = SUBMODE_1; // return back to the knob selection screen
            bPrevEncoderBtn = bEncoderBtn; // set the encoder previous state to the current state so we don't loop
            encoderKnob.write(0); // reset encoder
            lastEncoderPosition = 0; // reset encoder
            return;
        }
        else
        if(getActiveKnob(5) == KNOB_CONFIG_OKAY_HIGHLIGHTED) {   // the had the OK button highlighted
            systemSubMode = SUBMODE_1; // return to the knob selection screen
            bPrevEncoderBtn = bEncoderBtn; // set the encoder previous state to the current state so we don't loop
            saveKnobs(); // write the knob buffer back to EEProm (probably should update this so it only writes what changed)
            encoderKnob.write(0); // reset encoder
            lastEncoderPosition = 0; // reset encoder
            return;
        }
        else
        if(getActiveKnob(5) ==  KNOB_CONFIG_CMD_HIGHLIGHTED) {   // had the CMD Byte spinner highlighted
          systemSubMode = SUBMODE_2_CMD; // go into Knob config CMD Byte update modal window
          bPrevEncoderBtn = bEncoderBtn; // set the encoder previous state to the current state so we don't loop
          return;
        }
        else
        if(getActiveKnob(5) == KNOB_CONFIG_NAME_HIGHLIGHTED) { // had the NAME box highlighted
          textCursorPos = 0;
          encoderKnob.write(AsciiToEncoder(knobConfigurations[configKnobID].name[textCursorPos])); // set encoder to the correct value to keep character whatever it currently is
          lastEncoderPosition = 0;
          systemSubMode = SUBMODE_2_NAME; // go into the knob config NAME update modal window
          bPrevEncoderBtn = bEncoderBtn; // set the encoder previous state to current state so we don't loop
          return;
        }
        else
        if(getActiveKnob(5) == KNOB_CONFIG_TYPECMD_HIGHLIGHTED) { // had the Type spinner highlighted
          systemSubMode = SUBMODE_2_TYPE; // go into the knob config type update modal window
          bPrevEncoderBtn = bEncoderBtn; // set the encoder previous state to current state so we don't loop
          textCursorPos = knobConfigurations[configKnobID].typecode;
          return;
        }
      }
    }
  }

  if(systemMode == MODE_CONFIG) { // we're in CONFIG mode
    if(systemSubMode == SUBMODE_2_TYPE) { // we're inside the TYPE modal window
      if(!bPrevEncoderBtn && bEncoderBtn) {  // the user depressed the encoder button
          systemSubMode = SUBMODE_2; // go back to the knob config screen
          bPrevEncoderBtn = bEncoderBtn; // set the encoder previous state to current state so we don't loop
          return;
      }
    }
  }

  if(systemMode == MODE_CONFIG) {  // if we're inside Config mode
    if(systemSubMode == SUBMODE_2_TYPE) {  // and we're inside the Type model dialog
      knobConfigurations[configKnobID].typecode = getActiveKnob(NUM_TYPE_CODES);  // set the typecode according to the encoder
      textCursorPos = knobConfigurations[configKnobID].typecode;
      return;
    }
  }

  if(systemMode == MODE_CONFIG) { // if we're inside Config mode
    if(systemSubMode == SUBMODE_2_CMD) { // and we're inside the CMD byte modal dialog
      knobConfigurations[configKnobID].cmdbyte = getActiveKnob(256); // set the command byte to a value between 0x00 - 0xFF based on encoder position
      return;
    }
  }

  if(buttonStates[0]) { // first button pushed
    if(systemMode == MODE_CONFIG) { // if we're inside Config mode
      if(systemSubMode == SUBMODE_2_NAME) { // we're in the name modal
        if(textCursorPos > 0)  // we can still go left
          textCursorPos--;      // go left
        else 
          textCursorPos = 14; // wrap around
        encoderKnob.write(AsciiToEncoder(knobConfigurations[configKnobID].name[textCursorPos])); // set the encoder to be match the next letter so it doesn't change it arbitrarily
        lastEncoderPosition = 0;
        delay(250);
        return;
      }
    }
  }

  if(buttonStates[1]) {
    if(systemMode == MODE_CONFIG) {  // if we're inside Config mode
      if(systemSubMode == SUBMODE_2_NAME) { // we're in the name modal
        if(textCursorPos < 14) // we can still go right
          textCursorPos++;     // go right 
        else
          textCursorPos = 0;   // wrap around
        encoderKnob.write(AsciiToEncoder(knobConfigurations[configKnobID].name[textCursorPos]));
        lastEncoderPosition = 0;
        delay(250);
        return;
      }
    }
  }

  if(systemMode == MODE_CONFIG) { // we're in CONFIG mode
    if(systemSubMode == SUBMODE_2_NAME) { // we're inside the NAME modal window
      if(!bPrevEncoderBtn && bEncoderBtn) {  // the user depressed the encoder button
          systemSubMode = SUBMODE_2; // go back to the knob config screen
          bPrevEncoderBtn = bEncoderBtn; // set the encoder previous state to current state so we don't loop
          return;
      }
    }
  }

  if(systemMode == MODE_CONFIG) { // we're in CONFIG mode
    if(systemSubMode == SUBMODE_2_CMD) { // we're inside the CMD byte modal window
      if(!bPrevEncoderBtn && bEncoderBtn) {  // the user depressed the encoder button
          systemSubMode = SUBMODE_2; // go back to the knob config screen
          bPrevEncoderBtn = bEncoderBtn; // set the encoder previous state to current state so we don't loop
          return;
      }
    }
  }

  if(systemMode == MODE_CONFIG) { // if we're inside Config mode
    if(systemSubMode == SUBMODE_2_NAME) { // and we're in the naming modal dialog
      knobConfigurations[configKnobID].name[textCursorPos] = getActiveKnob(94) + 32;  // set the current text position of the name to whatever the encoder sets it to
      return;
    }
  }



  if(buttonStates[0] && buttonStates[1]) {  // if top two buttons depressed simultaneously
    if(systemMode == MODE_TEST) { // if we're in TEST mode
      systemMode = MODE_CONFIG;  // change to CONFIG mode
      systemSubMode = SUBMODE_1; // set the CONFIG screen to the knob selection screen
      delay(500); // pause half a second
    }
    else if (systemMode == MODE_CONFIG) { // if we're in CONFIG mode
      systemMode = MODE_RUNNING; // change to RUNNING mode
      delay(500); // pause half a second
    }
    else if (systemMode == MODE_RUNNING) { // if we're in RUN mode
      systemMode = MODE_TEST; // switch to TEST mode
      delay(500); // pause half a second
    }
  }
}


/* .---------------------------------------------------------------------------------.
   |  handleDisplays() loop - loops through appropriate display screen based on mode |
   '---------------------------------------------------------------------------------' */
void handleDisplays() {
  if(systemMode == MODE_TEST) {
    drawTestScreen();
    setLEDs(ledPattern);
  }
  else if (systemMode == MODE_CONFIG) {
    drawConfigScreen();
    setLEDs(0b00000000);
  }
  else { // RUNNING MODE
    drawRunningScreen();
    setLEDs(0b00000000);
  } 
  delay(10);
}

/* .------------------------------------------------------------.
   |  Main loop                                                 |
   '------------------------------------------------------------' */
void loop() {
  handleControlStatus();
  handleDisplays();
}