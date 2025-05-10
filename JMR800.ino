#include "global.h"
#include <stdarg.h>
#include "JX8P.h"
#include <EEPROM.h>
#include "hardware.h"
#include "display.h"


// counter used to send all knob values to synth at the same time 
uint parmCtr;

uint8_t cmdValue = 0;
uint8_t byteValue = 0;



uint8_t systemMode;
uint8_t systemSubMode;

uint8_t activeKnob = 0;





// sends next command from given preset
void sendNextPresetParmToSynth(uint8_t presetNumber, uint8_t parmIX) {
  if(parmIX >= NUM_KNOBS)
    return;
  sendParameter(preset[parmIX].cmd, preset[parmIX].val);
}


/* --------------------------------------------------------------
   |  Logging routine                                           |
   -------------------------------------------------------------- */
void log(const String& s) {
  Serial.println(s);
  Serial.flush();
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


  uint ctr=0;
        preset[ctr].cmd = 0x80; preset[ctr].val = 80; // DCO1 Range
  ctr++;preset[ctr].cmd = 0x81; preset[ctr].val = 112;// DCO1 Waveform
  ctr++;preset[ctr].cmd = 0x82; preset[ctr].val = 64; // DCO1 Tune
  ctr++;preset[ctr].cmd = 0x83; preset[ctr].val = 0;  // DCO1 LFO Depth
  ctr++;preset[ctr].cmd = 0x84; preset[ctr].val = 0;  // DCO1 Env Depth

  ctr++;preset[ctr].cmd = 0x85; preset[ctr].val = 80; // DCO2 Range
  ctr++;preset[ctr].cmd = 0x86; preset[ctr].val = 112;// DCO2 Waveform
  ctr++;preset[ctr].cmd = 0x87; preset[ctr].val = 0;  // DCO XMOD
  ctr++;preset[ctr].cmd = 0x88; preset[ctr].val = 64; // DCO2 Tune
  ctr++;preset[ctr].cmd = 0x89; preset[ctr].val = 58; // DCO2 Fine Tune
  ctr++;preset[ctr].cmd = 0x8A; preset[ctr].val = 0; // DCO2 LFO Depth
  ctr++;preset[ctr].cmd = 0x8B; preset[ctr].val = 0; // DCO2 Env Depth

  ctr++;preset[ctr].cmd = 0x8F; preset[ctr].val = 0; // DCO Dynamics
  ctr++;preset[ctr].cmd = 0x90; preset[ctr].val = 0; // DCO EG Mode

  ctr++;preset[ctr].cmd = 0x91; preset[ctr].val = 127; // Mix DCO1
  ctr++;preset[ctr].cmd = 0x92; preset[ctr].val = 127; // Mix DCO2
  ctr++;preset[ctr].cmd = 0x93; preset[ctr].val = 127; // Mix Env
  ctr++;preset[ctr].cmd = 0x94; preset[ctr].val = 0; // Mix Dynamics
  ctr++;preset[ctr].cmd = 0x95; preset[ctr].val = 0; // Mix EG Mode

  ctr++;preset[ctr].cmd = 0x96; preset[ctr].val = 0; // VCF HPF
  ctr++;preset[ctr].cmd = 0x97; preset[ctr].val = 55;// VCF Freq
  ctr++;preset[ctr].cmd = 0x98; preset[ctr].val = 0; // VCF Resonance
  ctr++;preset[ctr].cmd = 0x99; preset[ctr].val = 0; // VCF LFO
  ctr++;preset[ctr].cmd = 0x9A; preset[ctr].val = 107; // VCF Env
  ctr++;preset[ctr].cmd = 0x9B; preset[ctr].val = 127; // VCF Key
  ctr++;preset[ctr].cmd = 0x9C; preset[ctr].val = 127; // VCF Dynamics
  ctr++;preset[ctr].cmd = 0x9D; preset[ctr].val = 127; // VCF EG Mode

  ctr++;preset[ctr].cmd = 0x9E; preset[ctr].val = 100; // VCA Level
  ctr++;preset[ctr].cmd = 0x9F; preset[ctr].val = 0; // VCA Dynamics
  ctr++;preset[ctr].cmd = 0xAF; preset[ctr].val = 0; // VCA EG Mode

  ctr++;preset[ctr].cmd = 0xA1; preset[ctr].val = 64; // LFO Waveform
  ctr++;preset[ctr].cmd = 0xA2; preset[ctr].val = 0; // LFO Delay
  ctr++;preset[ctr].cmd = 0xA3; preset[ctr].val = 103; // LFO Rate
  ctr++;preset[ctr].cmd = 0xC6; preset[ctr].val = 0; // LFO Sync

  ctr++;preset[ctr].cmd = 0xA4; preset[ctr].val = 84; // EG Env1 Attk
  ctr++;preset[ctr].cmd = 0xA5; preset[ctr].val = 79; // EG Env1 Decay
  ctr++;preset[ctr].cmd = 0xA6; preset[ctr].val = 78; // EG Env1 Sust
  ctr++;preset[ctr].cmd = 0xA7; preset[ctr].val = 100; // EG Env1 Rel
  ctr++;preset[ctr].cmd = 0xA8; preset[ctr].val = 0; // EG Env1 Key

  ctr++;preset[ctr].cmd = 0xA9; preset[ctr].val = 0; // EG Env2 Attk
  ctr++;preset[ctr].cmd = 0xAA; preset[ctr].val = 15; // EG Env2 Decay
  ctr++;preset[ctr].cmd = 0xAB; preset[ctr].val = 75; // EG Env2 Sust
  ctr++;preset[ctr].cmd = 0xAC; preset[ctr].val = 0; // EG Env2 Rel
  ctr++;preset[ctr].cmd = 0xAD; preset[ctr].val = 0; // EG Env2 Key

  ctr++;preset[ctr].cmd = 0xB0; preset[ctr].val = 0; // PMW1 Width
  ctr++;preset[ctr].cmd = 0xB1; preset[ctr].val = 0; // PWM1 Env
  ctr++;preset[ctr].cmd = 0xB2; preset[ctr].val = 0; // PMW1 LFO
  ctr++;preset[ctr].cmd = 0xB3; preset[ctr].val = 0; // PWM2 Width
  ctr++;preset[ctr].cmd = 0xB4; preset[ctr].val = 0; // PWM2 Env
  ctr++;preset[ctr].cmd = 0xB5; preset[ctr].val = 0; // PWM2 LFO
  ctr++;preset[ctr].cmd = 0xC4; preset[ctr].val = 0; // PWM Dyna
  ctr++;preset[ctr].cmd = 0xCC; preset[ctr].val = 0; // PWM Mode
  ctr++;preset[ctr].cmd = 0xA0; preset[ctr].val = 0; // Chorus
  ctr++;preset[ctr].cmd = 0x00; preset[ctr].val = 0; // Unassigned
  ctr++;preset[ctr].cmd = 0x00; preset[ctr].val = 0; // Unassigned
  ctr++;preset[ctr].cmd = 0x00; preset[ctr].val = 0; // Unassigned

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
  ctr = 0;
  strcpy(knobConfigurations[ctr].name, "DCO1 Range");     knobConfigurations[ctr].cmdbyte = 0x80; knobConfigurations[ctr].typecode = TYPE_CODE_RANGE;         ctr++;
  strcpy(knobConfigurations[ctr].name, "DCO1 Waveform");  knobConfigurations[ctr].cmdbyte = 0x81; knobConfigurations[ctr].typecode = TYPE_CODE_WAVE_FORM;     ctr++;
  strcpy(knobConfigurations[ctr].name, "DCO1 Tune");      knobConfigurations[ctr].cmdbyte = 0x82; knobConfigurations[ctr].typecode = TYPE_CODE_OCTAVE;        ctr++;
  strcpy(knobConfigurations[ctr].name, "DCO1 LFO Depth"); knobConfigurations[ctr].cmdbyte = 0x83; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "DCO1 Env Depth"); knobConfigurations[ctr].cmdbyte = 0x84; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;

  strcpy(knobConfigurations[ctr].name, "DCO2 Range");     knobConfigurations[ctr].cmdbyte = 0x85; knobConfigurations[ctr].typecode = TYPE_CODE_RANGE;         ctr++;
  strcpy(knobConfigurations[ctr].name, "DCO2 Waveform");  knobConfigurations[ctr].cmdbyte = 0x86; knobConfigurations[ctr].typecode = TYPE_CODE_WAVE_FORM;     ctr++;
  strcpy(knobConfigurations[ctr].name, "DCO Crossmod");   knobConfigurations[ctr].cmdbyte = 0x87; knobConfigurations[ctr].typecode = TYPE_CODE_3_2_1_OFF;     ctr++;
  strcpy(knobConfigurations[ctr].name, "DCO2 Tune");      knobConfigurations[ctr].cmdbyte = 0x88; knobConfigurations[ctr].typecode = TYPE_CODE_OCTAVE;        ctr++;
  strcpy(knobConfigurations[ctr].name, "DCO2 Fine Tune"); knobConfigurations[ctr].cmdbyte = 0x89; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "DCO2 LFO Depth"); knobConfigurations[ctr].cmdbyte = 0x8A; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "DCO2 Env Depth"); knobConfigurations[ctr].cmdbyte = 0x8B; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;

  strcpy(knobConfigurations[ctr].name, "DCO Dynamics");   knobConfigurations[ctr].cmdbyte = 0x8F; knobConfigurations[ctr].typecode = TYPE_CODE_3_2_1_OFF;     ctr++;
  strcpy(knobConfigurations[ctr].name, "DCO EG Mode");    knobConfigurations[ctr].cmdbyte = 0x90; knobConfigurations[ctr].typecode = TYPE_CODE_MODE;          ctr++;

  strcpy(knobConfigurations[ctr].name, "Mix DCO1");       knobConfigurations[ctr].cmdbyte = 0x91; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "Mix DCO2");       knobConfigurations[ctr].cmdbyte = 0x92; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "Mix Env");        knobConfigurations[ctr].cmdbyte = 0x93; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "Mix EG Dynamic"); knobConfigurations[ctr].cmdbyte = 0x94; knobConfigurations[ctr].typecode = TYPE_CODE_3_2_1_OFF;     ctr++;
  strcpy(knobConfigurations[ctr].name, "Mix EG Mode");    knobConfigurations[ctr].cmdbyte = 0x95; knobConfigurations[ctr].typecode = TYPE_CODE_MODE;          ctr++;

  strcpy(knobConfigurations[ctr].name, "VCF HP Filter");  knobConfigurations[ctr].cmdbyte = 0x96; knobConfigurations[ctr].typecode = TYPE_CODE_3_2_1_OFF;     ctr++;
  strcpy(knobConfigurations[ctr].name, "VCF Cutoff Frq"); knobConfigurations[ctr].cmdbyte = 0x97; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "VCF Resonance");  knobConfigurations[ctr].cmdbyte = 0x98; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "VCF LFO");        knobConfigurations[ctr].cmdbyte = 0x99; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "VCF Env");        knobConfigurations[ctr].cmdbyte = 0x9A; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "VCF Key Follow"); knobConfigurations[ctr].cmdbyte = 0x9B; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "VCF EG Dynamic"); knobConfigurations[ctr].cmdbyte = 0x9C; knobConfigurations[ctr].typecode = TYPE_CODE_3_2_1_OFF;     ctr++;
  strcpy(knobConfigurations[ctr].name, "VCF EG Mode");    knobConfigurations[ctr].cmdbyte = 0x9D; knobConfigurations[ctr].typecode = TYPE_CODE_MODE;          ctr++;

  strcpy(knobConfigurations[ctr].name, "VCA Level");      knobConfigurations[ctr].cmdbyte = 0x9E; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "VCA EG Dynamic"); knobConfigurations[ctr].cmdbyte = 0x9F; knobConfigurations[ctr].typecode = TYPE_CODE_3_2_1_OFF;     ctr++;
  strcpy(knobConfigurations[ctr].name, "VCA EG Mode");    knobConfigurations[ctr].cmdbyte = 0xAF; knobConfigurations[ctr].typecode = TYPE_CODE_ENV2_GATE;     ctr++;

  strcpy(knobConfigurations[ctr].name, "LFO Waveform");   knobConfigurations[ctr].cmdbyte = 0xA1; knobConfigurations[ctr].typecode = TYPE_CODE_LFO_WAVE_FORM; ctr++;
  strcpy(knobConfigurations[ctr].name, "LFO Delay");      knobConfigurations[ctr].cmdbyte = 0xA2; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "LFO Rate");       knobConfigurations[ctr].cmdbyte = 0xA3; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "LFO Sync");       knobConfigurations[ctr].cmdbyte = 0xD0; knobConfigurations[ctr].typecode = TYPE_CODE_2_1_OFF;       ctr++;
  
  strcpy(knobConfigurations[ctr].name, "EG Env1 Attk");   knobConfigurations[ctr].cmdbyte = 0xA4; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "EG Env1 Decay");  knobConfigurations[ctr].cmdbyte = 0xA5; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "EG Env1 Sust");   knobConfigurations[ctr].cmdbyte = 0xA6; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "EG Env1 Rel");    knobConfigurations[ctr].cmdbyte = 0xA7; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "EG Env1 Key");    knobConfigurations[ctr].cmdbyte = 0xA8; knobConfigurations[ctr].typecode = TYPE_CODE_3_2_1_OFF;     ctr++;
  strcpy(knobConfigurations[ctr].name, "EG Env2 Attk");   knobConfigurations[ctr].cmdbyte = 0xA9; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "EG Env2 Decay");  knobConfigurations[ctr].cmdbyte = 0xAA; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "EG Env2 Sust");   knobConfigurations[ctr].cmdbyte = 0xAB; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "EG Env2 Rel");    knobConfigurations[ctr].cmdbyte = 0xAC; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "EG Env2 Key");    knobConfigurations[ctr].cmdbyte = 0xAD; knobConfigurations[ctr].typecode = TYPE_CODE_3_2_1_OFF;     ctr++;
  
  strcpy(knobConfigurations[ctr].name, "PWM Mode");       knobConfigurations[ctr].cmdbyte = 0xD6; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "PMW1 Width");     knobConfigurations[ctr].cmdbyte = 0xB0; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "PWM1 Env");       knobConfigurations[ctr].cmdbyte = 0xB1; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "PMW1 LFO");       knobConfigurations[ctr].cmdbyte = 0xB2; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "PWM2 Width");     knobConfigurations[ctr].cmdbyte = 0xB3; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "PWM2 Env");       knobConfigurations[ctr].cmdbyte = 0xB4; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "PWM2 LFO");       knobConfigurations[ctr].cmdbyte = 0xB5; knobConfigurations[ctr].typecode = TYPE_CODE_0_TO_10;       ctr++;
  strcpy(knobConfigurations[ctr].name, "PWM Dyna");       knobConfigurations[ctr].cmdbyte = 0xCE; knobConfigurations[ctr].typecode = TYPE_CODE_3_2_1_OFF;     ctr++;
  
  strcpy(knobConfigurations[ctr].name, "Chorus");         knobConfigurations[ctr].cmdbyte = 0xA0; ctr++;

  strcpy(knobConfigurations[ctr].name, "Unassigned");     knobConfigurations[ctr].cmdbyte = 0x00; ctr++;
  strcpy(knobConfigurations[ctr].name, "Unassigned");     knobConfigurations[ctr].cmdbyte = 0x00; ctr++;
  strcpy(knobConfigurations[ctr].name, "Unassigned");     knobConfigurations[ctr].cmdbyte = 0x00; ctr++;
  saveKnobs();
  */

  log("Initialized.");
}



/* .----------------------------------------------------------------------------------------------------.
   |  sendParameterToSynth() code to figure out what to send based on knob type                         |
   '----------------------------------------------------------------------------------------------------' */
void sendParameterToSynth(uint i) {
    /* code to ignore noise and jitter */
    uint kv = knobValueAt(i);

    if(knobConfigurations[i].typecode == TYPE_CODE_0_TO_10 || knobConfigurations[i].typecode == TYPE_CODE_OCTAVE) {
      sendParameter(knobConfigurations[i].cmdbyte, knobValueAt(i));
      return;
    }

    if(knobConfigurations[i].typecode == TYPE_CODE_LFO_WAVE_FORM || knobConfigurations[i].typecode == TYPE_CODE_2_1_OFF) {
      if(kv < 32)
        sendParameter(knobConfigurations[i].cmdbyte, 16); // Random      , Off      
      else
      if(kv < 64)
        sendParameter(knobConfigurations[i].cmdbyte, 48); // Square Wave , 1
      else
        sendParameter(knobConfigurations[i].cmdbyte, 96); // Triangle    , 2      

      return;
    }

    if(knobConfigurations[i].typecode == TYPE_CODE_ENV2_GATE) {
      if(kv < 64)
        sendParameter(knobConfigurations[i].cmdbyte, 32); // Gate
      else
        sendParameter(knobConfigurations[i].cmdbyte, 96); // Normal

      return;
    }
  
    if(knobConfigurations[i].typecode == TYPE_CODE_RANGE || knobConfigurations[i].typecode == TYPE_CODE_WAVE_FORM || 
       knobConfigurations[i].typecode == TYPE_CODE_3_2_1_OFF || knobConfigurations[i].typecode == TYPE_CODE_MODE) {
        if(kv < 32) {
          sendParameter(knobConfigurations[i].cmdbyte, 16); // range 16', Noise, OFF, Env-2-Inverted
          return;
        }
        else
        if(kv < 64) {
          sendParameter(knobConfigurations[i].cmdbyte, 48); // range 8', Sawtooth, 1, Env-2-Normal
          return;
        }
        else
        if(kv < 96) {
          sendParameter(knobConfigurations[i].cmdbyte, 80); // range 4', Pulse, 2, Env-1-Inverted
          return;
        }

        sendParameter(knobConfigurations[i].cmdbyte, 112); // range 2', Square, 3, Env-1-Normal
        return;
    }

}

/* .----------------------------------------------------------------------------------------------------.
   |  handleControlStatus() loop - gathers up control values and set status state machine based on them |
   '----------------------------------------------------------------------------------------------------' */
void handleControlStatus() {
  gatherControlSettings(); // sweep all the controls, store their state in a buffer

  if(systemMode == MODE_RUNNING) {
    for(uint i = 0; i < NUM_KNOBS; i++) {
      if(knobValueChanged(i) && millis() > 12000) { // we start sending parameters after 10 seconds (allows knob leveling to settle)
        sendParameterToSynth(i);
        activeKnob = i;
      }
    }
  }

  if(systemMode == MODE_RAW_OUTPUT) {
    if(buttonStates[0] && !prevButtonStates[0]) {
        prevButtonStates[0] = buttonStates[0];
        cmdValue--;
        delay(100);
    }
    else
    if(buttonStates[1] && !prevButtonStates[1]) {
      prevButtonStates[1] = buttonStates[1];
      cmdValue++;
      delay(100);
    }
    else
    if(buttonStates[2] && !prevButtonStates[2]) {
      prevButtonStates[2] = buttonStates[2];
      byteValue--;
      delay(100);
    }
    else
    if(buttonStates[3] && !prevButtonStates[3]) {
      prevButtonStates[3] = buttonStates[3];
      byteValue++;
      delay(100);
    }
    if(!bPrevEncoderBtn && bEncoderBtn) {
        bPrevEncoderBtn = bEncoderBtn;  // set the encoder previous state to the current state so we don't loop
        sendParameter(cmdValue, byteValue);
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

  if(systemMode == MODE_RUNNING && buttonStates[1]) { // button 2 pressed while running, send preset
    systemSubMode = SUBMODE_3;
    parmCtr = 0;
    delay(500);
  }
  else
  if(systemMode == MODE_RUNNING && buttonStates[0]) { // button 1 pressed while running, send all current parameters
    systemSubMode = SUBMODE_2;
    parmCtr = 0;
    delay(500);
  }
  else 
  if(systemMode == MODE_RUNNING && systemSubMode == SUBMODE_2) {  // sync all knobs to synth
    parmCtr++;
    if(parmCtr < NUM_KNOBS)
      sendParameterToSynth(parmCtr);
    else
      systemSubMode = SUBMODE_1;
  }
  else
  if(systemMode == MODE_RUNNING && systemSubMode == SUBMODE_3) {  // send preset parameters
    parmCtr++;
    if(parmCtr < NUM_KNOBS) 
      sendNextPresetParmToSynth(0, parmCtr);
    else
      systemSubMode = SUBMODE_1;
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
      systemMode = MODE_RAW_OUTPUT; // switch to TEST mode
      delay(500); // pause half a second
    }
    else if (systemMode == MODE_RAW_OUTPUT) {
      systemMode = MODE_TEST;
      delay(500);
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
  else if (systemMode == MODE_RAW_OUTPUT) {
    drawRawOutputScreen();
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