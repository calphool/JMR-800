#include <Wire.h>
#include <ADC.h>
#include <Encoder.h>
#include <vector>



#ifndef HARDWARE_h
#define HARDWARE_h


extern void log(const String& s);

ADC adc;

// encoder position
long lastEncoderPosition = 0;



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


// buffer for values from ADC, and shadow copy for detecting changes
uint16_t AnalogValues[16][NUM_MUXES];
uint16_t oldAnalogValues[16][NUM_MUXES];



// array of pins for mux enable
uint enablePins[NUM_MUXES] = { U2_ENABLE_PIN, U3_ENABLE_PIN, U4_ENABLE_PIN, U6_ENABLE_PIN };

// array of pins for MUX addresses
uint MuxAddressPins[4] = { S0_PIN, S1_PIN, S2_PIN, S3_PIN };

// array of analog ADC input pins
uint analogInPins[NUM_MUXES] = { U2_ANALOG_IN_PIN, U3_ANALOG_IN_PIN, U4_ANALOG_IN_PIN , U6_ANALOG_IN_PIN};


// timer value for monitoring frequency of potentiometer scans
static unsigned long lastPotScanTime = 0;

// button lights shift register pins
#define SHIFT_REG_SER 32
#define SHIFT_REG_SRCLK 31
#define SHIFT_REG_RCLK 30

// Encoder pins
#define ENCODER_CLK_PIN 13
#define ENCODER_DT_PIN 14
#define ENCODER_SW_PIN 15

Encoder encoderKnob(ENCODER_CLK_PIN, ENCODER_DT_PIN);

// vector for de-duplicating messages sent to synthesizer
struct SentCommand {
  uint8_t paramID;
  uint8_t value;
  unsigned long timestamp;
};

std::vector<SentCommand> recentCommands;
const unsigned long COMMAND_TIMEOUT = 250;  // milliseconds

// button statuses
static bool bEncoderBtn = false;
static bool bPrevEncoderBtn = false;

// used for bit banging PG800 serial protocol
volatile int bitIndex = -1;
volatile uint16_t sendBuffer = 0;

// button state variables
uint8_t buttonStates[NUM_BUTTONS] = {0}; // Current button state (0 or 1)
uint8_t prevButtonStates[NUM_BUTTONS] = {0}; // Previous button state


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
        if(iGatherCtr % 10 == 0) // store a copy of the current AnalogValues() periodically
            oldAnalogValues[addr][muxCtr] = AnalogValues[addr][muxCtr];
      }
      digitalWrite(enablePins[muxCtr], HIGH); // disable this mux
      delayMicroseconds(5);
  }
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
  unsigned long now = millis();

  // Remove expired entries
  recentCommands.erase(
    std::remove_if(recentCommands.begin(), recentCommands.end(),
      [now](const SentCommand& cmd) {
        return now - cmd.timestamp > COMMAND_TIMEOUT;
      }),
    recentCommands.end()
  );

  // Check for duplicate
  for (const auto& cmd : recentCommands) {
    if (cmd.paramID == paramID && cmd.value == value) {
      return;  // Duplicate within timeout window; do not resend
    }
  }

  // Record this command
  recentCommands.push_back({paramID, value, now});

  while (bitIndex != -1 && (millis() - now < 500) );  // Wait until previous transfer is complete
  if(millis() - now >= 500) {
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
   |  gatherControlSettings -- handles button depresses and sets |
   |  LED status                                                 |
   --------------------------------------------------------------- */
void gatherControlSettings() {
  gatherPotentiometerValues();


  for(int i = 0; i < NUM_BUTTONS; i++) {
    prevButtonStates[i] = buttonStates[i];
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonStates[i] = !digitalRead(buttonPins[i]); // Active low
    /*
    if (buttonStates[i] && !prevButtonStates[i]) {
      uint8_t shift = (3 - i) * 2; // Maps button 0 to bits 6-7, 1 to 4-5, etc.
      uint8_t mask = 0b11 << shift;
      uint8_t current = (ledPattern & mask) >> shift;
      uint8_t next = cycleLedState(current);

      ledPattern = (ledPattern & ~mask) | (next << shift);
    }
    */
  }

  bPrevEncoderBtn = bEncoderBtn;
  bEncoderBtn = !digitalRead(ENCODER_SW_PIN);
  updateEncoder();
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
    knobIX = knobXformer[knobIX];  // Transform this index (necessary because of wiring) 
    uint mux = knobIX >> 4;
    uint mux_ix = knobIX & 0x0F;

    uint16_t currentValue = AnalogValues[mux_ix][mux];
    uint16_t oldValue = oldAnalogValues[mux_ix][mux];

    // Apply hysteresis (ignore small changes)
    int delta = (int)currentValue - (int)oldValue;
    if (abs(delta) < 4)  // Adjust the threshold as needed (3-10 is typical)
        return false;

    // Update the stored value if the change is significant
    oldAnalogValues[mux_ix][mux] = currentValue;
    return true;
}


uint knobValueAt(uint knobIX) {
  knobIX = knobXformer[knobIX];              // transform this index (necessary because of wiring) 
  uint mux = knobIX >> 4;
  uint mux_ix = knobIX & 0x0F;

  return AnalogValues[mux_ix][mux] >> 1; // 0 - 127 returned value
}


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

/* .----------------------------------------------------------------------------------------------------.
   |  AsciiToEncoder() routine that translates an ASCII value to the appropriate encoder value          |
   '----------------------------------------------------------------------------------------------------' */
int AsciiToEncoder(char c) {
  int v = ((((int)c) - 65) * 4) + 132;
  return v;
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




#endif
