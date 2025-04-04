const char s0[]  = "DCO1 Range";
const char s1[]  = "DCO1 Wave";
const char s2[]  = "DCO1 Tune";
const char s3[]  = "DCO1 LFO";
const char s4[]   = "DCO1 EG";
const char s5[]   = "DCO2 Range";
const char s6[]   = "DCO2 Wave";
const char s7[]   = "XMOD";
const char s8[]   = "DCO2 Tune";
const char s9[]   = "DCO2 Fine Tune";
const char s10[]   = "DCO2 LFO";
const char s11[]   = "DCO2 EG";
const char s12[]   = "";
const char s13[]   = "";
const char s14[]   = "";
const char s15[]   = "DCO Dynamics";
const char s16[]   = "DCO EG Mode";
const char s17[]   = "DCO1 Level";
const char s18[]   = "DCO2 Level";
const char s19[]   = "Mix EG";
const char s20[]   = "Mix Dynamics";
const char s21[]   = "Mix EG Mode";
const char s22[]   = "VCF HPF";
const char s23[]   = "VCF Freq";
const char s24[]   = "VCF Resonance";
const char s25[]   = "VCF LFO";
const char s26[]   = "VCF EG";
const char s27[]   = "VCF Key";
const char s28[]   = "VCF Dynamics";
const char s29[]   = "VCF EG Mode";
const char s30[]   = "VCA Level";
const char s31[]   = "VCA Dynamics";
const char s32[]   = "Chorus";
const char s33[]   = "LFO Wave";
const char s34[]   = "LFO Delay";
const char s35[]   = "LFO Rate";
const char s36[]   = "EG1 Attack";
const char s37[]   = "EG1 Decay";
const char s38[]   = "EG1 Sustain";
const char s39[]   = "EG1 Release";
const char s40[]   = "EG1 Key Follow";
const char s41[]   = "EG2 Attack";
const char s42[]   = "EG2 Decay";
const char s43[]   = "EG2 Sustain";
const char s44[]   = "EG2 Release";
const char s45[]   = "EG2 Key Follow";
const char s46[]   = "";
const char s47[]   = "VCA EG Mode";


const char* const jx8p_param_names[] = {
   s0,  s1,  s2,  s3,  s4,  s5,  s6,  s7,  s8,  s9,
  s10, s11, s12, s13, s14, s15, s16, s17, s18, s19,
  s20, s21, s22, s23, s24, s25, s26, s27, s28, s29,
  s30, s31, s32, s33, s34, s35, s36, s37, s38, s39,
  s40, s41, s42, s43, s44, s45, s46, s47
};

#define PARAM_START 0x80
#define NUM_PARAMS 48