#ifndef GLOBAL_h
#define GLOBAL_h


#define NUM_KNOBS 56


// Knob configuration related defines and structures
#define KNOB_CONFIG_NAME_HIGHLIGHTED 0
#define KNOB_CONFIG_CMD_HIGHLIGHTED 1
#define KNOB_CONFIG_TYPECMD_HIGHLIGHTED 2
#define KNOB_CONFIG_OKAY_HIGHLIGHTED 3
#define KNOB_CONFIG_CANCEL_HIGHLIGHTED 4



// system modes
#define MODE_TEST 0
#define MODE_CONFIG 1
#define MODE_RUNNING 2
#define MODE_RAW_OUTPUT 3

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

#define SUBMODE_3 5


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

// knob configuration structure
typedef struct {
  char name[15];
  uint8_t cmdbyte;
  uint8_t typecode;
} knobConfig;

// array of knob configurations
knobConfig knobConfigurations[NUM_KNOBS];
knobConfig knobConfigurations_bkup[NUM_KNOBS];
knobConfig lastKnobConfig;

typedef struct {
  uint8_t cmd;
  uint8_t val;
}
twoBytes;

twoBytes preset[NUM_KNOBS];


#endif
