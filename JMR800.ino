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

  preset[ 0].cmd = 0x80; preset[ 0].val = 80; // DCO1 Range
  preset[ 1].cmd = 0x81; preset[ 1].val = 112;// DCO1 Waveform
  preset[ 2].cmd = 0x82; preset[ 2].val = 64; // DCO1 Tune
  preset[ 3].cmd = 0x83; preset[ 3].val = 0;  // DCO1 LFO Depth
  preset[ 4].cmd = 0x84; preset[ 4].val = 0;  // DCO1 Env Depth
  preset[ 5].cmd = 0x85; preset[ 5].val = 80; // DCO2 Range
  preset[ 6].cmd = 0x86; preset[ 6].val = 112;// DCO2 Waveform
  preset[ 7].cmd = 0x87; preset[ 7].val = 0;  // DCO XMOD
  preset[ 8].cmd = 0x88; preset[ 8].val = 64; // DCO2 Tune
  preset[ 9].cmd = 0x89; preset[ 9].val = 58; // DCO2 Fine Tune
  preset[10].cmd = 0x8A; preset[10].val = 0; // DCO2 LFO Depth
  preset[11].cmd = 0x8B; preset[11].val = 0; // DCO2 Env Depth
  preset[12].cmd = 0x8F; preset[12].val = 0; // DCO Dynamics
  preset[13].cmd = 0x90; preset[13].val = 0; // DCO EG Mode
  preset[14].cmd = 0x91; preset[14].val = 127; // Mix DCO1
  preset[15].cmd = 0x92; preset[15].val = 127; // Mix DCO2
  preset[16].cmd = 0x93; preset[16].val = 127; // Mix Env
  preset[17].cmd = 0x94; preset[17].val = 0; // Mix Dynamics
  preset[18].cmd = 0x95; preset[18].val = 0; // Mix EG Mode
  preset[19].cmd = 0x96; preset[19].val = 0; // VCF HPF
  preset[20].cmd = 0x97; preset[20].val = 55;// VCF Freq
  preset[21].cmd = 0x98; preset[21].val = 0; // VCF Resonance
  preset[22].cmd = 0x99; preset[22].val = 0; // VCF LFO
  preset[23].cmd = 0x9A; preset[23].val = 107; // VCF Env
  preset[24].cmd = 0x9B; preset[24].val = 127; // VCF Key
  preset[25].cmd = 0x9C; preset[25].val = 127; // VCF Dynamics
  preset[26].cmd = 0x9D; preset[26].val = 127; // VCF EG Mode
  preset[27].cmd = 0x9E; preset[27].val = 100; // VCA Level
  preset[28].cmd = 0x9F; preset[28].val = 0; // VCA Dynamics
  preset[29].cmd = 0xA0; preset[29].val = 0; // Chorus
  preset[30].cmd = 0xA1; preset[30].val = 64; // LFO Waveform
  preset[31].cmd = 0xA2; preset[31].val = 0; // LFO Delay
  preset[32].cmd = 0xA3; preset[32].val = 103; // LFO Rate
  preset[33].cmd = 0xA4; preset[33].val = 84; // EG Env1 Attk
  preset[34].cmd = 0xA5; preset[34].val = 79; // EG Env1 Decay
  preset[35].cmd = 0xA6; preset[35].val = 78; // EG Env1 Sust
  preset[36].cmd = 0xA7; preset[36].val = 100; // EG Env1 Rel
  preset[37].cmd = 0xA8; preset[37].val = 0; // EG Env1 Key
  preset[38].cmd = 0xA9; preset[38].val = 0; // EG Env2 Attk
  preset[39].cmd = 0xAA; preset[39].val = 15; // EG Env2 Decay
  preset[40].cmd = 0xAB; preset[40].val = 75; // EG Env2 Sust
  preset[41].cmd = 0xAC; preset[41].val = 0; // EG Env2 Rel
  preset[42].cmd = 0xAD; preset[42].val = 0; // EG Env2 Key
  preset[43].cmd = 0xAF; preset[43].val = 0; // VCA EG Mode
  preset[44].cmd = 0xB0; preset[44].val = 0; // PMW1 Width
  preset[45].cmd = 0xB1; preset[45].val = 0; // PWM1 Env
  preset[46].cmd = 0xB2; preset[46].val = 0; // PMW1 LFO
  preset[47].cmd = 0xB3; preset[47].val = 0; // PWM2 Width
  preset[48].cmd = 0xB4; preset[48].val = 0; // PWM2 Env
  preset[49].cmd = 0xB5; preset[49].val = 0; // PWM2 LFO
  preset[50].cmd = 0xC4; preset[50].val = 0; // PWM Dyna
  preset[51].cmd = 0xC6; preset[51].val = 0; // LFO Sync
  preset[52].cmd = 0xCC; preset[52].val = 0; // PWM Mode
  preset[53].cmd = 0x80; preset[53].val = 80; // DCO1 Range
  preset[54].cmd = 0x81; preset[54].val = 112;// DCO1 Waveform
  preset[55].cmd = 0x82; preset[55].val = 64; // DCO1 Tune

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
  strcpy(knobConfigurations[50].name, "PWM Dyna"); knobConfigurations[50].cmdbyte = 0xCE;
  strcpy(knobConfigurations[51].name, "LFO Sync"); knobConfigurations[51].cmdbyte = 0xD0;
  strcpy(knobConfigurations[52].name, "PWM Mode"); knobConfigurations[52].cmdbyte = 0xD6;
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