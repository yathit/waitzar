/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "wz_utilities.h"


//////////////////////////////////////////////////////////////////////////////////
// Please note that the wz_utilities library is currently considered
//   to be UNSTABLE, and is not included in the Linux build by default.
// This is because most of its functionality is of direct benefit to the
//   Windows build, so we hope to test its correctness through the next
//   two Windows releases of WaitZar. 
// Since there is some universally-useful code here, we will eventually
//   include this in both releases, but we want to avoid bumping libwaitzar's
//   major revision for breakages caused by code that wasn't originally intended
//   for Linux to begin with.
//////////////////////////////////////////////////////////////////////////////////


//A hidden namespace for our "private" methods
namespace
{
	//Constant pseudo-letters
	#define ZG_DASH         0x2000
	#define ZG_KINZI        0x2001

	//Constant pseudo-letters (stacked)
	#define ZG_STACK_KA     0x3000
	#define ZG_STACK_KHA    0x3001
	#define ZG_STACK_GA     0x3002
	#define ZG_STACK_GHA    0x3003
	#define ZG_STACK_NGA    0x3004
	#define ZG_STACK_SA     0x3005
	#define ZG_STACK_SSA    0x3006
	#define ZG_STACK_ZA     0x3007
	#define ZG_STACK_ZHA    0x3008
	//Skip one...
	#define ZG_STACK_NYA    0x3009
	#define ZG_STACK_TTA    0x300A
	#define ZG_STACK_HTA1   0x300B
	#define ZG_STACK_DHA1   0x300C
	//Skip one...
	#define ZG_STACK_NHA    0x300D
	#define ZG_STACK_TA     0x300E
	#define ZG_STACK_HTA2   0x300F
	#define ZG_STACK_DDA    0x3010
	#define ZG_STACK_DHA2   0x3011
	#define ZG_STACK_NA     0x3012
	#define ZG_STACK_PA     0x3013
	#define ZG_STACK_PHA    0x3014
	#define ZG_STACK_VA     0x3015
	#define ZG_STACK_BA     0x3016
	#define ZG_STACK_MA     0x3017
	//Break sequence
	#define ZG_STACK_YA     0x3018
	#define ZG_STACK_LA     0x3019
	#define ZG_STACK_THA    0x301A
	#define ZG_STACK_A      0x301B
	//Special-purpose indented stacked letters
	#define ZG_STACK_SSA_INDENT     0x301C
	#define ZG_STACK_TA_INDENT      0x301D
	#define ZG_STACK_HTA2_INDENT    0x301E

	//Some complex letters
	#define ZG_COMPLEX_1            0x4000
	#define ZG_COMPLEX_NA           0x4001

	//Letters which share similar semantic functionality
	#define ZG_TALL_WITH_ASAT       0x4002
	#define ZG_DOTTED_CIRCLE_ABOVE  0x4003
	#define ZG_LEGGED_CIRCLE_BELOW  0x4004
	#define ZG_LEGS_BOTH_WAYS       0x4005
	#define ZG_LEGS_OF_THREE        0x4006
	#define ZG_KINZI_102D           0x4007
	#define ZG_KINZI_102E           0x4008
	#define ZG_KINZI_1036           0x4009

	//Some more substitures
	#define ZG_DOT_BELOW_SHIFT_1    0x400A
	#define ZG_DOT_BELOW_SHIFT_2    0x400B
	#define ZG_TALL_SINGLE_LEG      0x400C
	#define ZG_TALL_DOUBLE_LEG      0x400D
	#define ZG_YA_PIN_CUT           0x400E
	#define ZG_YA_PIN_SSA           0x400F
	#define ZG_YA_YIT_LONG          0x4010
	#define ZG_YA_YIT_HIGHCUT       0x4011
	#define ZG_YA_YIT_LONG_HIGHCUT  0x4012
	#define ZG_YA_YIT_LOWCUT        0x4013
	#define ZG_YA_YIT_LONG_LOWCUT   0x4014
	#define ZG_YA_YIT_BOTHCUT       0x4015
	#define ZG_YA_YIT_LONG_BOTHCUT  0x4016
	#define ZG_LEG_FWD_SMALL        0x4017


	//Constants for our counting sort algorithm
	#define ID_MED_Y        0
	#define ID_MED_R        1
	#define ID_MED_W        2
	#define ID_MED_H        3
	#define ID_VOW_E        4
	#define ID_VOW_ABOVE    5
	#define ID_VOW_BELOW    6
	#define ID_VOW_A        7
	#define ID_ANUSVARA     8
	#define ID_DOW_BELOW    9
	#define ID_VISARGA     10
	#define ID_TOTAL       11

	//Bitflags for our step-three algorithm
	#define S3_TOTAL_FLAGS               42
	#define S3_VISARGA                   0x20000000000
	#define S3_LEG_SINGLE                0x10000000000
	#define S3_LEG_SINGLE_TALL            0x8000000000
	#define S3_LEG_DOUBLE                 0x4000000000
	#define S3_LEG_DOUBLE_TALL            0x2000000000
	#define S3_TWO_LEGS                   0x1000000000
	#define S3_THREE_LEGS                  0x800000000
	#define S3_MED_YA_PIN                  0x400000000
	#define S3_MED_YA_PIN_SHORT            0x200000000
	#define S3_MED_YA_PIN_SSA_STACK        0x100000000
	#define S3_MED_YA_YIT                   0x80000000
	#define S3_MED_YA_YIT_LONG              0x40000000
	#define S3_MED_YA_YIT_HIGH_CUT          0x20000000
	#define S3_MED_YA_YIT_LONG_HIGH_CUT     0x10000000
	#define S3_MED_YA_YIT_LOW_CUT            0x8000000
	#define S3_MED_YA_YIT_LONG_LOW_CUT       0x4000000
	#define S3_MED_YA_YIT_BOTH_CUT           0x2000000
	#define S3_MED_YA_YIT_LONG_BOTH_CUT      0x1000000
	#define S3_STACKED						  0x800000 //(includes indented)
	#define S3_CIRC_BELOW                     0x400000
	#define S3_CIRC_BELOW_W_LEG               0x200000
	#define S3_LEG_FWD                        0x100000
	#define S3_LEG_FWD_SMALL                   0x80000
	#define S3_DOT_BELOW                       0x40000
	#define S3_DOT_BELOW_2                     0x20000
	#define S3_DOT_BELOW_3                     0x10000
	#define S3_VOW_AR_TALL                      0x8000
	#define S3_VOW_AR_TALL_ASAT                 0x4000
	#define S3_VOW_AR_SHORT                     0x2000
	#define S3_DOT_ABOVE                        0x1000
	#define S3_CIRCLE_ABOVE                      0x800
	#define S3_CIRCLE_DOT_ABOVE                  0x400
	#define S3_CIRCLE_CROSSED_ABOVE              0x200
	#define S3_KINZI                             0x100
	#define S3_KINZI_DOT                          0x80
	#define S3_KINZI_CIRC                         0x40
	#define S3_KINZI_CIRC_CROSSED                 0x20
	#define S3_MED_SLANT_ABOVE                    0x10
	#define S3_VOW_A                               0x8
	#define S3_CONSONANT_WIDE                      0x4
	#define S3_CONSONANT_NARROW                    0x2
	#define S3_CONSONANT_OTHER                     0x1
	#define S3_OTHER                                 0


	//Bitflags for our zawgyi conversion algorithm
	#define BF_CONSONANT    2048
	#define BF_STACKER      1024
	#define BF_DOT_LOW       512
    #define BF_DOT_OVER      256
	#define BF_VOW_AR        128
	#define BF_LEG_NORM       64
	#define BF_VOW_OVER       32
    #define BF_VOW_A          16
	#define BF_LEG_REV         8
	#define BF_CIRC_LOW        4
	#define BF_MED_YA          2
	#define BF_ASAT            1
	#define BF_OTHER           0

	//Match into this array
	const unsigned int matchFlags[] = {0xBDE, 0x801, 0x803, 0x887, 0x80F, 0x89E, 0x93F, 0xB7F, 0x8FF, 0x9FF, 0x800};


	//Useful global vars
	wchar_t zawgyiStr[100];

	//A fun struct
    #define RULE_MODIFY   1
	#define RULE_COMBINE  2
	#define RULE_ORDER    3
	struct Rule {
		int type;
		wchar_t at_letter;
		__int64 match_flags;
		wchar_t *match_additional;
		wchar_t replace;

		Rule(int type, wchar_t atLetter, __int64 matchFlags, wchar_t *matchAdditional, wchar_t replace) {
			this->type = type;
			this->at_letter = atLetter;
			this->match_flags = matchFlags;
			this->match_additional = matchAdditional;
			this->replace = replace;
		}
	};
	std::vector<Rule> matchRules;
	


	bool isMyanmar(wchar_t letter)
	{
		return (letter>=0x1000 && letter<=0x1021)
			|| (letter>=0x1023 && letter<=0x1032 && letter!=0x1028)
			|| (letter>=0x1036 && letter<=0x104F);
	}

	bool isConsonant(wchar_t letter) 
	{
		return (letter>=0x1000 && letter<=0x1021)
			|| (letter>=0x1023 && letter<=0x1027)
			|| (letter==0x103F);
	}

	bool isDigit(wchar_t letter)
	{
		return (letter>=0x1040 && letter<=0x1049);
	}

	bool isPunctuation(wchar_t letter)
	{
		return (letter==0x104A || letter==0x104B);
	}

	bool shouldRestartCount(wchar_t letter)
	{
		return isConsonant(letter) || letter==0x1029 || letter==0x102A || isDigit(letter) || isPunctuation(letter) || !isMyanmar(letter);
	}

	//There are several other stopping conditions besides a stopping character
	//For example, the last character in a string triggers a stop.
	//uniString[i+1]==0x103A catches "vowell_a" followed by "asat". This might be hackish; not sure.
	bool atStoppingPoint(wchar_t* uniString, size_t id, size_t length)
	{
		return id==length || (isMyanmar(uniString[id])&&uniString[id+1]==0x103A) || uniString[id]==0x103A || uniString[id]==0x1039;
	}

	int getRhymeID(wchar_t letter)
	{
		switch (letter)
		{
			case 0x103B:
				return ID_MED_Y;
			case 0x103C:
				return ID_MED_R;
			case 0x103D:
				return ID_MED_W;
			case 0x103E:
				return ID_MED_H;
			case 0x1031:
				return ID_VOW_E;
			case 0x102D:
			case 0x102E:
			case 0x1032:
				return ID_VOW_ABOVE;
			case 0x102F:
			case 0x1030:
				return ID_VOW_BELOW;
			case 0x102B:
			case 0x102C:
				return ID_VOW_A;
			case 0x1036:
				return ID_ANUSVARA;
			case 0x1037:
				return ID_DOW_BELOW;
			case 0x10338:
				return ID_VISARGA;
			default:
				return -1;
		}
	}


	int getBitflag(wchar_t uniLetter)
	{
		switch (uniLetter)
		{
			case 0x1039:
				return BF_STACKER;
			case 0x1037:
				return BF_DOT_LOW;
			case 0x1036:
				return BF_DOT_OVER;
			case 0x102B:
			case 0x102C:
				return BF_VOW_AR;
			case 0x102F:
			case 0x1030:
				return BF_LEG_NORM;
			case 0x102D:
			case 0x102E:
			case 0x1032:
				return BF_VOW_OVER;
			case 0x1031:
				return BF_VOW_A;
			case 0x103E:
				return BF_LEG_REV;
			case 0x103D:
				return BF_CIRC_LOW;
			case 0x103B:
			case 0x103C:
				return BF_MED_YA;
			case 0x103A:
				return BF_ASAT;
			case 0x1025:
			case 0x1027:
			case 0x1029:
			case 0x103F:
			case ZG_DASH:
				return BF_CONSONANT;
			default:
				if (uniLetter>=0x1000 && uniLetter<=0x1021)
					return BF_CONSONANT;
				return BF_OTHER;
		}
	}



	__int64 getStage3BitFlags(wchar_t letter) 
	{
		switch (letter) {
			case 0x1038:
				return S3_VISARGA;
			case 0x102F:
				return S3_LEG_SINGLE;
			case ZG_TALL_SINGLE_LEG:
				return S3_LEG_SINGLE_TALL;
			case 0x1030:
				return S3_LEG_DOUBLE;
			case ZG_TALL_DOUBLE_LEG:
				return S3_LEG_DOUBLE_TALL;
			case ZG_LEGS_BOTH_WAYS:
				return S3_TWO_LEGS;
			case ZG_LEGS_OF_THREE:
				return S3_THREE_LEGS;
			case 0x103B:
				return S3_MED_YA_PIN;
			case ZG_YA_PIN_CUT:
				return S3_MED_YA_PIN_SHORT;
			case ZG_YA_PIN_SSA:
				return S3_MED_YA_PIN_SSA_STACK;
			case 0x103C:
				return S3_MED_YA_YIT;
			case ZG_YA_YIT_LONG:
				return S3_MED_YA_YIT_LONG;
			case ZG_YA_YIT_HIGHCUT:
				return S3_MED_YA_YIT_HIGH_CUT;
			case ZG_YA_YIT_LONG_HIGHCUT:
				return S3_MED_YA_YIT_LONG_HIGH_CUT;
			case ZG_YA_YIT_LOWCUT:
				return S3_MED_YA_YIT_LOW_CUT;
			case ZG_YA_YIT_LONG_LOWCUT:
				return S3_MED_YA_YIT_LONG_LOW_CUT;
			case ZG_YA_YIT_BOTHCUT:
				return S3_MED_YA_YIT_BOTH_CUT;
			case ZG_YA_YIT_LONG_BOTHCUT:
				return S3_MED_YA_YIT_LONG_BOTH_CUT;
			case 0x103D:
				return S3_CIRC_BELOW;
			case ZG_LEGGED_CIRCLE_BELOW:
				return S3_CIRC_BELOW_W_LEG;
			case 0x103E:
				return S3_LEG_FWD;
			case ZG_LEG_FWD_SMALL:
				return S3_LEG_FWD_SMALL;
			case 0x1037:
				return S3_DOT_BELOW;
			case ZG_DOT_BELOW_SHIFT_1:
				return S3_DOT_BELOW_2;
			case ZG_DOT_BELOW_SHIFT_2:
				return S3_DOT_BELOW_3;
			case 0x102B:
				return S3_VOW_AR_TALL;
			case ZG_TALL_WITH_ASAT:
				return S3_VOW_AR_TALL_ASAT;
			case 0x102C:
				return S3_VOW_AR_SHORT;
			case 0x1036:
				return S3_DOT_ABOVE;
			case 0x102D:
				return S3_CIRCLE_ABOVE;
			case ZG_DOTTED_CIRCLE_ABOVE:
				return S3_CIRCLE_DOT_ABOVE;
			case 0x102E:
				return S3_CIRCLE_CROSSED_ABOVE;
			case ZG_KINZI:
				return S3_KINZI;
			case ZG_KINZI_1036:
				return S3_KINZI_DOT;
			case ZG_KINZI_102D:
				return S3_KINZI_CIRC;
			case ZG_KINZI_102E:
				return S3_KINZI_CIRC_CROSSED;
			case 0x1032:
				return S3_MED_SLANT_ABOVE;
			case 0x1031:
				return S3_VOW_A;
			case 0x1000:
			case 0x1003:
			case 0x1006:
			case 0x100F:
			case 0x1010:
			case 0x1011:
			case 0x1018:
			case 0x101A:
			case 0x101C:
			case 0x101E:
			case 0x101F:
			case 0x1021:
			case 0x103F:
				return S3_CONSONANT_WIDE;
			case 0x1001:
			case 0x1002:
			case 0x1004:
			case 0x1005:
			case 0x1007:
			case 0x100E:
			case 0x1012:
			case 0x1013:
			case 0x1014:
			case 0x1015:
			case 0x1016:
			case 0x1017:
			case 0x1019:
			case 0x101B:
			case 0x101D:
			case 0x1027:
				return S3_CONSONANT_NARROW;
			case 0x1008:
			case 0x1009:
			case 0x100A:
			case 0x100B:
			case 0x100C:
			case 0x100D:
			case 0x1020:
			case 0x1025:
			case 0x1029:
				return S3_CONSONANT_OTHER;
			default:
				if (letter>=ZG_STACK_KA && letter<=ZG_STACK_HTA2_INDENT)
					return S3_STACKED;
				return S3_OTHER;
		}
	}


	int getStage3ID(__int64 bitflags)
	{
		switch (bitflags) {
			case S3_VISARGA:
				return 41;
			case S3_LEG_SINGLE:
				return 40;
			case S3_LEG_SINGLE_TALL:
				return 39;
			case S3_LEG_DOUBLE:
				return 38;
			case S3_LEG_DOUBLE_TALL:
				return 37;
			case S3_TWO_LEGS:
				return 36;
			case S3_THREE_LEGS:
				return 35;
			case S3_MED_YA_PIN:
				return 34;
			case S3_MED_YA_PIN_SHORT:
				return 33;
			case S3_MED_YA_PIN_SSA_STACK:
				return 32;
			case S3_MED_YA_YIT:
				return 31;
			case S3_MED_YA_YIT_LONG:
				return 30;
			case S3_MED_YA_YIT_HIGH_CUT:
				return 29;
			case S3_MED_YA_YIT_LONG_HIGH_CUT:
				return 28;
			case S3_MED_YA_YIT_LOW_CUT:
				return 27;
			case S3_MED_YA_YIT_LONG_LOW_CUT:
				return 26;
			case S3_MED_YA_YIT_BOTH_CUT:
				return 25;
			case S3_MED_YA_YIT_LONG_BOTH_CUT:
				return 24;
			case S3_STACKED:
				return 23;
			case S3_CIRC_BELOW:
				return 22;
			case S3_CIRC_BELOW_W_LEG:
				return 21;
			case S3_LEG_FWD:
				return 20;
			case S3_LEG_FWD_SMALL:
				return 19;
			case S3_DOT_BELOW:
				return 18;
			case S3_DOT_BELOW_2:
				return 17;
			case S3_DOT_BELOW_3:
				return 16;
			case S3_VOW_AR_TALL:
				return 15;
			case S3_VOW_AR_TALL_ASAT:
				return 14;
			case S3_VOW_AR_SHORT:
				return 13;
			case S3_DOT_ABOVE:
				return 12;
			case S3_CIRCLE_ABOVE:
				return 11;
			case S3_CIRCLE_DOT_ABOVE:
				return 10;
			case S3_CIRCLE_CROSSED_ABOVE:
				return 9;
			case S3_KINZI:
				return 8;
			case S3_KINZI_DOT:
				return 7;
			case S3_KINZI_CIRC:
				return 6;
			case S3_KINZI_CIRC_CROSSED:
				return 5;
			case S3_MED_SLANT_ABOVE:
				return 4;
			case S3_VOW_A:
				return 3;
			case S3_CONSONANT_WIDE:
				return 2;
			case S3_CONSONANT_NARROW:
				return 1;
			case S3_CONSONANT_OTHER:
				return 0;
			default:
				return -1;
		}
	}




	unsigned int flag2id(unsigned int flag)
	{
		switch (flag)
		{
			case BF_STACKER:
				return 10;
			case BF_DOT_LOW:
				return 9;
			case BF_DOT_OVER:
				return 8;
			case BF_VOW_AR:
				return 7;
			case BF_LEG_NORM:
				return 6;
			case BF_VOW_OVER:
				return 5;
			case BF_VOW_A:
				return 4;
			case BF_LEG_REV:
				return 3;
			case BF_CIRC_LOW:
				return 2;
			case BF_MED_YA:
				return 1;
			case BF_ASAT:
				return 0;
			default:
				return -1;
		}
	}


	wchar_t getStackedVersion(wchar_t uniLetter)
	{
		switch (uniLetter) {
			case 0x101B:
				return ZG_STACK_YA;
			case 0x101C:
				return ZG_STACK_LA;
			case 0x101E:
				return ZG_STACK_THA;
			case 0x1021:
				return ZG_STACK_A;
			default:
				if (uniLetter>=0x1000 && uniLetter<=0x1008) {
					return (uniLetter-0x1000)+ZG_STACK_KA;
				} else if (uniLetter>=0x100A && uniLetter<=0x100D) {
					return (uniLetter-0x100A)+ZG_STACK_NYA;
				} else if (uniLetter>=0x100E && uniLetter<=0x1019) {
					return (uniLetter-0x100E)+ZG_STACK_NHA;
				}
		}
		return 0;
	}




	wchar_t zawgyiLetter(wchar_t uniLetter)
	{
		switch (uniLetter)
		{
			case 0x1039:
				return 0x005E; //Leftover stack letters as ^
			case 0x103A:
				return 0x1039;
			case 0x103B:
				return 0x103A;
			case 0x103C:
				return 0x103B;
			case 0x103D:
				return 0x103C;
			case 0x103E:
				return 0x103D;
			case 0x103F:
				return 0x1086;
			/*case :
				return 0x1092;*/ //Add later; this is actually a typing issue, not a rendering one.
			case ZG_DASH:
				return 0x002D;
			case ZG_KINZI:
				return 0x1064;
			case ZG_STACK_KA:
				return 0x1060;
			case ZG_STACK_KHA:
				return 0x1061;
			case ZG_STACK_GA:
				return 0x1062;
			case ZG_STACK_GHA:
				return 0x1063;
			case ZG_STACK_NGA:
				return 0x003F; //It appears Zawgyi cannot stack "NGA"
			case ZG_STACK_SA:
				return 0x1065;
			case ZG_STACK_SSA:
				return 0x1066;
			case ZG_STACK_SSA_INDENT:
				return 0x1067;
			case ZG_STACK_ZA:
				return 0x1068;
			case ZG_STACK_ZHA:
				return 0x1069;
			case ZG_STACK_NYA:
				return 0x003F; //Can't stack "NYA" either
			case ZG_STACK_TTA:
				return 0x106C;
			case ZG_STACK_HTA1:
				return 0x106D;
			case ZG_STACK_DHA1:
				return 0x003F; //Appears we can't stack this either.
			case ZG_STACK_NHA:
				return 0x1070;
			case ZG_STACK_TA:
				return 0x1071;
			case ZG_STACK_TA_INDENT:
				return 0x1072;
			case ZG_STACK_HTA2:
				return 0x1073;
			case ZG_STACK_HTA2_INDENT:
				return 0x1074;
			case ZG_STACK_DDA:
				return 0x1075;
			case ZG_STACK_DHA2:
				return 0x1076;
			case ZG_STACK_NA:
				return 0x1077;
			case ZG_STACK_PA:
				return 0x1078;
			case ZG_STACK_PHA:
				return 0x1079;
			case ZG_STACK_VA:
				return 0x107A;
			case ZG_STACK_BA:
				return 0x107B;
			case ZG_STACK_MA:
				return 0x107C;
			case ZG_STACK_YA:
				return 0x003F; //Can't stack "ya"
			case ZG_STACK_LA:
				return 0x1085;
			case ZG_STACK_THA:
				return 0x003F; //Can't stack "tha"
			case ZG_STACK_A:
				return 0x003F; //Can't stack "a"
			case ZG_COMPLEX_1:
				return 0x1092;
			case ZG_COMPLEX_NA:
				return 0x1091;
			case ZG_TALL_WITH_ASAT:
				return 0x105A;
			case ZG_DOTTED_CIRCLE_ABOVE:
				return 0x108E;
			case ZG_LEGGED_CIRCLE_BELOW:
				return 0x108A;
			case ZG_LEGS_BOTH_WAYS:
				return 0x1088;
			case ZG_LEGS_OF_THREE:
				return 0x1089;
			case ZG_KINZI_102D:
				return 0x108B;
			case ZG_KINZI_102E:
				return 0x108C;
			case ZG_KINZI_1036:
				return 0x108D;
			case ZG_DOT_BELOW_SHIFT_1:
				return 0x1094;
			case ZG_TALL_SINGLE_LEG:
				return 0x1033;
			case ZG_TALL_DOUBLE_LEG:
				return 0x1034;
			case ZG_YA_PIN_CUT:
				return 0x107D;
			case ZG_YA_PIN_SSA:
				return 0x1069;
			case ZG_YA_YIT_LONG:
				return 0x107E;
			case ZG_YA_YIT_HIGHCUT:
				return 0x107F;
			case ZG_YA_YIT_LONG_HIGHCUT:
				return 0x1080;
			case ZG_YA_YIT_LOWCUT:
				return 0x1081;
			case ZG_YA_YIT_LONG_LOWCUT:
				return 0x1082;
			case ZG_YA_YIT_BOTHCUT:
				return 0x1083;
			case ZG_YA_YIT_LONG_BOTHCUT:
				return 0x1084;
			case ZG_LEG_FWD_SMALL:
				return 0x1087;
			default:
				return uniLetter; //Assume it's correct.
		}
	}


} //End of hidden namespace



namespace waitzar 
{

void sortMyanmarString(wchar_t* uniString)
{
	//Count array for use with counting sort
	//We store a count, but we also need separate strings for the types.
	int rhyme_flags[ID_TOTAL];
	wchar_t rhyme_vals[ID_TOTAL];
	size_t len = wcslen(uniString); 
	wchar_t *vow_above_buffer = new wchar_t[len];
	wchar_t *vow_below_buffer = new wchar_t[len];
	wchar_t *vow_ar_buffer = new wchar_t[len];
	for (int res=0; res<ID_TOTAL; res++) {
		rhyme_flags[res] = 0;
		rhyme_vals[res] = 0x0000;
	}
	int vow_above_index = 0;
	int vow_below_index = 0;
	int vow_ar_index = 0;

	//Scan each letter
	size_t destI = 0;    //Allows us to eliminate duplicate letters
	size_t prevStop = 0; //What was our last-processed letter
	for (size_t i=0; i<=len;) { //We count up to the trailing 0x0
		//Does this letter restart our algorithm?
		if (atStoppingPoint(uniString, i, len) || shouldRestartCount(uniString[i]) || getRhymeID(uniString[i])==-1) {
			//Now that we've counted, sort
			if (i!=prevStop) {
				for (int x=0; x<ID_TOTAL; x++) {
					//Add and restart
					for (int r_i=0; r_i <rhyme_flags[x]; r_i++) {
						wchar_t letter = rhyme_vals[x];
						if (x==ID_VOW_ABOVE)
							letter = vow_above_buffer[r_i];
						else if (x==ID_VOW_BELOW)
							letter = vow_below_buffer[r_i];
						else if (x==ID_VOW_A)
							letter = vow_ar_buffer[r_i];
						uniString[destI++] = letter;
					}
					rhyme_flags[x] = 0;
					rhyme_vals[x] = 0x0000;
				}
				vow_above_index = 0;
				vow_below_index = 0;
				vow_ar_index = 0;
			}

			//Increment if this is asat or virama
			uniString[destI++] = uniString[i++];
			while (i<len && (uniString[i]==0x103A||uniString[i]==0x1039||shouldRestartCount(uniString[i])))
				 uniString[destI++] = uniString[i++];

			//Don't sort until after this point
			prevStop = i;
			continue;
		}

		//Count, if possible. Else, copy in
		int rhymeID = getRhymeID(uniString[i]);
		rhyme_flags[rhymeID] += 1;
		rhyme_vals[rhymeID] = uniString[i];
		if (rhymeID == ID_VOW_ABOVE)
			vow_above_buffer[vow_above_index++] = uniString[i];
		else if (rhymeID == ID_VOW_BELOW)
			vow_below_buffer[vow_below_index++] = uniString[i];
		else if (rhymeID == ID_VOW_A)
			vow_ar_buffer[vow_ar_index++] = uniString[i];

		//Standard increment
		i++;
	}

	//Done. Append an extra zero, in case our new string is too short.
	uniString[destI] = 0x0000;
	delete [] vow_above_buffer;
	delete [] vow_below_buffer;
	delete [] vow_ar_buffer;
}




wchar_t* renderAsZawgyi(wchar_t* uniString)
{
	//Perform conversion
	//Step 1: Determine which finals won't likely combine; add
	// dashes beneath them.
	wchar_t prevLetter = 0x0000;
	wchar_t currLetter;
	int prevType = BF_OTHER;
	int currType;
	size_t destID = 0;
	size_t length = wcslen(uniString);
	for (size_t i=0; i<length; i++) {
		//Get the current letter and type
		currLetter = uniString[i];
		currType = getBitflag(currLetter);

		//Match
		bool passed = true;
		if (currType!=BF_OTHER && currType!=BF_CONSONANT) {
			//Complex matching (simple ones will work with passed==true)
			if ((prevType&matchFlags[flag2id(currType)])==0) {
				//Handle kinzi
				if (currType==BF_STACKER && prevType==BF_ASAT && i>=2 && uniString[i-2]==0x1004) {
					//Store this two letters back
					destID-=2;

					//Kinzi might be considered a "stacker", but since nothing matches
					//  against stacker in phase 1, it's not necessary. So we'll call it "other"
					currLetter = ZG_KINZI;
					currType = BF_OTHER;
				} else 
					passed = false;
			} else {
				//Handle special cases
				if (currType==BF_ASAT && prevLetter==0x103C)
					passed = false;
				else if (currType==BF_STACKER && prevType==BF_CONSONANT) {
					if (prevLetter==0x1029 || prevLetter==0x103F)
						passed = false;
				}
			}
		}

		//Append it, and a dash if necessary
		//Don't dash our stack letter; it's not necessary
		if (!passed && currType!=BF_STACKER) 
			zawgyiStr[destID++] = ZG_DASH;
		zawgyiStr[destID++] = currLetter;

		//Increment
		prevLetter = currLetter;
		prevType = currType;
	}
	zawgyiStr[destID] = 0x0000;



	//Step 2: Stack letters. This will only reduce the string's length, so
	//  we can perform it in-place.
	destID = 0;
	length = wcslen(zawgyiStr);
	prevLetter = 0x0000;
	prevType = BF_OTHER;
	for (size_t i=0; i<length; i++) {
		//Special: skip "ghost letters" left from previous combines
		if (zawgyiStr[i]==0x0000)
			continue;

		//Get the current letter and type
		currLetter = zawgyiStr[i];
		currType = getBitflag(currLetter);

		//Should we stack this?
		if (prevType==BF_STACKER && currType==BF_CONSONANT) {
			//Note: We must make an active check for letters with no Zawgyi representation
			// (0x003F)
			wchar_t stacked = getStackedVersion(currLetter);
			if (stacked!=0) {
				//General case
				int oldDestID = destID;
				if (zawgyiLetter(stacked)!=0x003F) {
					destID--;
					currLetter = stacked;
				}

				//Special cases (stacked)
				if (oldDestID>1) {
					if (stacked==ZG_STACK_HTA1 && zawgyiStr[oldDestID-2]==L'\u100B') {
						destID = oldDestID-2;
						currLetter = ZG_COMPLEX_1;
					} else if (stacked==ZG_STACK_DHA1 && zawgyiStr[oldDestID-2]==L'\u100F') {
						destID = oldDestID-2;
						currLetter = ZG_COMPLEX_NA;
					}
				}
			} 
		}

		//Re-copy this letter
		zawgyiStr[destID++] = currLetter;

		//Increment
		prevType = currType;
		prevLetter = currLetter;
	}
	zawgyiStr[destID++] = 0x0000;


	//Step 3: Apply a series of specific rules
	if (matchRules.size()==0) {
		//Add initial rules
		matchRules.push_back(Rule(RULE_MODIFY, L'\u1037', 0x1580018C000, L"\u1014", ZG_DOT_BELOW_SHIFT_1));
	}
	//We maintain a series of offsets for the most recent match. This is used to speed up the process of 
	// pattern matching. We only track the first occurrance, from left-to-right, of the given flag.
	//We scan from left-to-right, then apply rules from right-to-left breaking after each consonant.
	// A "minor break" occurs after each killed consonant, which is not tracked. "CONSONANT" always refers
	// to the main consonant. 
	int firstOccurrence[S3_TOTAL_FLAGS];
	for (int i=0; i<S3_TOTAL_FLAGS; i++)
		firstOccurrence[i] = -1;
	length = wcslen(zawgyiStr);
	for (size_t i=0; i<length; i++) {
		//Scan until the next word.
	}



	//Final Step: Convert each letter to its Zawgyi-equivalent
	length = wcslen(zawgyiStr);
	for (size_t i=0; i<length; i++)
		zawgyiStr[i] = zawgyiLetter(zawgyiStr[i]);
	return zawgyiStr;
}





} //End waitzar namespace


/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
