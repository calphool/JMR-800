const char* jx8p_param_names[256] = {
  // 0x00–0x7F: undefined, fill with ""
  "", // 0x00
  "", // 0x01
  "", // 0x02
  "", // 0x03
  "", // 0x04
  "", // 0x05
  "", // 0x06
  "", // 0x07
  "", // 0x08
  "", // 0x09
  "", // 0x0A
  "", // 0x0B
  "", // 0x0C
  "", // 0x0D
  "", // 0x0E
  "", // 0x0F
  "", // 0x10
  "", // 0x11
  "", // 0x12
  "", // 0x13
  "", // 0x14
  "", // 0x15
  "", // 0x16
  "", // 0x17
  "", // 0x18
  "", // 0x19
  "", // 0x1A
  "", // 0x1B
  "", // 0x1C
  "", // 0x1D
  "", // 0x1E
  "", // 0x1F
  "", // 0x20
  "", // 0x21
  "", // 0x22
  "", // 0x23
  "", // 0x24
  "", // 0x25
  "", // 0x26
  "", // 0x27
  "", // 0x28
  "", // 0x29
  "", // 0x2A
  "", // 0x2B
  "", // 0x2C
  "", // 0x2D
  "", // 0x2E
  "", // 0x2F
  "", // 0x30
  "", // 0x31
  "", // 0x32
  "", // 0x33
  "", // 0x34
  "", // 0x35
  "", // 0x36
  "", // 0x37
  "", // 0x38
  "", // 0x39
  "", // 0x3A
  "", // 0x3B
  "", // 0x3C
  "", // 0x3D
  "", // 0x3E
  "", // 0x3F
  "", // 0x40
  "", // 0x41
  "", // 0x42
  "", // 0x43
  "", // 0x44
  "", // 0x45
  "", // 0x46
  "", // 0x47
  "", // 0x48
  "", // 0x49
  "", // 0x4A
  "", // 0x4B
  "", // 0x4C
  "", // 0x4D
  "", // 0x4E
  "", // 0x4F
  "", // 0x50
  "", // 0x51
  "", // 0x52
  "", // 0x53
  "", // 0x54
  "", // 0x55
  "", // 0x56
  "", // 0x57
  "", // 0x58
  "", // 0x59
  "", // 0x5A
  "", // 0x5B
  "", // 0x5C
  "", // 0x5D
  "", // 0x5E
  "", // 0x5F
  "", // 0x60
  "", // 0x61
  "", // 0x62
  "", // 0x63
  "", // 0x64
  "", // 0x65
  "", // 0x66
  "", // 0x67
  "", // 0x68
  "", // 0x69
  "", // 0x6A
  "", // 0x6B
  "", // 0x6C
  "", // 0x6D
  "", // 0x6E
  "", // 0x6F
  "", // 0x70
  "", // 0x71
  "", // 0x72
  "", // 0x73
  "", // 0x74
  "", // 0x75
  "", // 0x76
  "", // 0x77
  "", // 0x78
  "", // 0x79
  "", // 0x7A
  "", // 0x7B
  "", // 0x7C
  "", // 0x7D
  "", // 0x7E
  "", // 0x7F

  // 0x80–0xAF: defined parameters
  "11 DCO1 Range",        // 0x80
  "12 DCO1 Waveform",     // 0x81
  "13 DCO1 Tune",         // 0x82
  "14 DCO1 LFO Depth",    // 0x83
  "15 DCO1 Env Depth",    // 0x84
  "21 DCO2 Range",        // 0x85
  "22 DCO2 Waveform",     // 0x86
  "31 DCO XMOD",          // 0x87
  "23 DCO2 Tune",         // 0x88
  "32 DCO2 Fine Tune",    // 0x89
  "24 DCO2 LFO Depth",    // 0x8A
  "25 DCO2 Env Depth",    // 0x8B
  "",                     // 0x8C
  "",                     // 0x8D
  "",                     // 0x8E
  "33 DCO Dynamics",      // 0x8F
  "34 DCO EG Mode",       // 0x90
  "41 Mix DCO1",          // 0x91
  "42 Mix DCO2",          // 0x92
  "43 Mix Env",           // 0x93
  "44 Mix Dynamics",      // 0x94
  "45 Mix EG Mode",       // 0x95
  "51 VCF HPF",           // 0x96
  "52 VCF Freq",          // 0x97
  "53 VCF Resonance",     // 0x98
  "54 VCF LFO",           // 0x99
  "55 VCF Env",           // 0x9A
  "56 VCF Key",           // 0x9B
  "57 VCF Dynamics",      // 0x9C
  "58 VCF EG Mode",       // 0x9D
  "61 VCA Level",         // 0x9E
  "63 VCA Dynamics",      // 0x9F
  "64 Chorus",            // 0xA0
  "71 LFO Waveform",      // 0xA1
  "72 LFO Delay",         // 0xA2
  "73 LFO Rate",          // 0xA3
  "81 EG Env1 Attk",      // 0xA4
  "82 EG Env1 Decay",     // 0xA5
  "83 EG Env1 Sust",      // 0xA6
  "84 EG Env1 Rel",       // 0xA7
  "85 EG Env1 Key",       // 0xA8
  "91 EG Env2 Attk",      // 0xA9
  "92 EG Env2 Decay",     // 0xAA
  "93 EG Env2 Sust",      // 0xAB
  "94 EG Env2 Rel",       // 0xAC
  "95 EG Env2 Key",       // 0xAD
  "",                     // 0xAE
  "62 VCA EG Mode",       // 0xAF
  "16 PMW1 Width",        // 0xB0
  "17 PWM1 Env",          // 0xB1
  "18 PMW1 LFO",          // 0xB2
  "26 PWM2 Width",        // 0xB3
  "28 PWM2 Env",          // 0xB4
  "27 PWM2 LFO",          // 0xB5
  "", // 0xB6
  "", // 0xB7
  "", // 0xB8
  "", // 0xB9
  "", // 0xBA
  "", // 0xBB
  "", // 0xBC
  "", // 0xBD
  "", // 0xBE
  "", // 0xBF
  "", // 0xC0
  "", // 0xC1
  "", // 0xC2
  "", // 0xC3
  "", // 0xC4
  "", // 0xC5
  "", // 0xC6
  "", // 0xC7
  "", // 0xC8
  "", // 0xC9
  "", // 0xCA
  "", // 0xCB
  "", // 0xCC
  "", // 0xCD
  "35 PWM Dyna",       // 0xCE
  "", // 0xCF
  "74 LFO Sync",       // 0xD0
  "", // 0xD1
  "", // 0xD2
  "", // 0xD3
  "", // 0xD4
  "", // 0xD5
  "36 PWM Mode",       // 0xD6
  "", // 0xD7
  "", // 0xD8
  "", // 0xD9
  "", // 0xDA
  "", // 0xDB
  "", // 0xDC
  "", // 0xDD
  "", // 0xDE
  "", // 0xDF
  "", // 0xE0
  "ROM TONE?",         // 0xE1  - Write command maybe?
  "", // 0xE2
  "", // 0xE3
  "", // 0xE4
  "", // 0xE5
  "", // 0xE6
  "", // 0xE7
  "", // 0xE8
  "", // 0xE9
  "", // 0xEA
  "", // 0xEB
  "", // 0xEC
  "", // 0xED
  "", // 0xEE
  "", // 0xEF
  "", // 0xF0
  "", // 0xF1
  "", // 0xF2
  "", // 0xF3
  "", // 0xF4
  "", // 0xF5
  "", // 0xF6
  "", // 0xF7
  "", // 0xF8
  "", // 0xF9
  "", // 0xFA
  "", // 0xFB
  "", // 0xFC
  "", // 0xFD
  "",  // 0xFE
  ""   // 0xFF
};


#define PARAM_TYPES_COUNT 11
#define NUMERIC_PARAM 0
#define FOUR_VALUE_PARAM 1
#define WAVE_PARAM 2
#define RANGE_PARAM 3
#define XMOD_PARAM 4
#define EG_MODE_PARAM 5
#define TUNE_PARAM 6
#define FINE_PARAM 7
#define VCA_EG_MODE 8
#define LFO_WAVE 9
#define CHORUS 10


typedef struct {
  char* name;
  uint8_t type;
}
parameter;