/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "wz_utilities.h"

using std::wstringstream;
using std::wstring;
using std::string;


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
	#define ZG_YA_PIN_SA            0x400F
	#define ZG_YA_YIT_LONG          0x4010
	#define ZG_YA_YIT_HIGHCUT       0x4011
	#define ZG_YA_YIT_LONG_HIGHCUT  0x4012
	#define ZG_YA_YIT_LOWCUT        0x4013
	#define ZG_YA_YIT_LONG_LOWCUT   0x4014
	#define ZG_YA_YIT_BOTHCUT       0x4015
	#define ZG_YA_YIT_LONG_BOTHCUT  0x4016
	#define ZG_LEG_FWD_SMALL        0x4017
	#define ZG_NA_CUT               0x4018
	#define ZG_YA_CUT               0x4019
	#define ZG_NYA_CUT              0x401A
	#define ZG_O_CUT                0x401B


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
	//FILE *wzUtilLogFile = NULL;

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
		bool blacklisted;

		Rule(int type, wchar_t atLetter, __int64 matchFlags, wchar_t *matchAdditional, wchar_t replace) {
			this->type = type;
			this->at_letter = atLetter;
			this->match_flags = matchFlags;
			this->match_additional = matchAdditional;
			this->replace = replace;
			this->blacklisted = false;
		}
	};
	std::vector<Rule*> matchRules;
	std::vector<wchar_t*> reorderPairs;


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
	bool atStoppingPoint(const wstring &uniString, size_t id, size_t length)
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
			case ZG_YA_PIN_SA:
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
			case ZG_NA_CUT:
			case ZG_YA_CUT:
				return S3_CONSONANT_NARROW;
			case 0x1008:
			case 0x1009:
			case 0x100A:
			case ZG_NYA_CUT:
			case 0x100B:
			case 0x100C:
			case 0x100D:
			case 0x1020:
			case 0x1025:
			case ZG_O_CUT:
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
				} else if (uniLetter>=0x100F && uniLetter<=0x1019) {
					return (uniLetter-0x100F)+ZG_STACK_NHA;
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
			case ZG_DOT_BELOW_SHIFT_2:
				return 0x1095;
			case ZG_TALL_SINGLE_LEG:
				return 0x1033;
			case ZG_TALL_DOUBLE_LEG:
				return 0x1034;
			case ZG_YA_PIN_CUT:
				return 0x107D;
			case ZG_YA_PIN_SA:
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
			case ZG_NA_CUT:
				return 0x108F;
			case ZG_YA_CUT:
				return 0x1090;
			case ZG_NYA_CUT:
				return 0x106B;
			case ZG_O_CUT:
				return 0x106A;
			default:
				return uniLetter; //Assume it's correct.
		}
	}


} //End of hidden namespace



namespace waitzar 
{


//TODO: This will sometimes append "\0" to a string for no apparent reason. 
//      Try: U+1000 U+1037
//      Need to fix eventually...
std::wstring sortMyanmarString(const std::wstring &uniString)
{
	//Count array for use with counting sort
	//We store a count, but we also need separate strings for the types.
	wstring res;
	int rhyme_flags[ID_TOTAL];
	wchar_t rhyme_vals[ID_TOTAL];
	size_t len = uniString.length();
	wstring vow_above_buffer;
	wstring vow_below_buffer;
	wstring vow_ar_buffer;
	for (int res=0; res<ID_TOTAL; res++) {
		rhyme_flags[res] = 0;
		rhyme_vals[res] = 0x0000;
	}
	
	//Scan each letter
	//size_t destI = 0;    //Allows us to eliminate duplicate letters
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
						res += letter;
					}
					rhyme_flags[x] = 0;
					rhyme_vals[x] = 0x0000;
				}
				vow_above_buffer.clear();
				vow_below_buffer.clear();
				vow_ar_buffer.clear();
				//vow_above_index = 0;
				//vow_below_index = 0;
				//vow_ar_index = 0;
			}

			//Increment if this is asat or virama
			res += uniString[i++];
			while (i<len && (uniString[i]==0x103A||uniString[i]==0x1039||shouldRestartCount(uniString[i])))
				 res += uniString[i++];

			//Don't sort until after this point
			prevStop = i;
			continue;
		}

		//Count, if possible. Else, copy in
		int rhymeID = getRhymeID(uniString[i]);
		rhyme_flags[rhymeID] += 1;
		rhyme_vals[rhymeID] = uniString[i];
		if (rhymeID == ID_VOW_ABOVE)
			vow_above_buffer.push_back(uniString[i]);
		else if (rhymeID == ID_VOW_BELOW)
			vow_below_buffer.push_back(uniString[i]);
		else if (rhymeID == ID_VOW_A)
			vow_ar_buffer.push_back(uniString[i]);

		//Standard increment
		i++;
	}

	return res;
}



//TODO: We should remove the BOM here; it's just a nuisance elsewhere.
wstring readUTF8File(const string& path) 
{
	//Open the file, read-only, binary.
	FILE* userFile = fopen(path.c_str(), "rb");
	if (userFile == NULL)
		throw std::runtime_error(std::string("File doesn't exist: " + path).c_str()); //File doesn't exist

	//Get file size
	fseek (userFile, 0, SEEK_END);
	long fileSize = ftell(userFile);
	rewind(userFile);

	//Read that file as a sequence of bytes
	char* buffer = new char[fileSize];
	size_t buff_size = fread(buffer, 1, fileSize, userFile);
	fclose(userFile);
	if (buff_size==0)
		return L""; //Empty file

	//Finally, convert this array to unicode
	wchar_t * uniBuffer = new wchar_t[buff_size];
	size_t numUniChars = mymbstowcs(NULL, buffer, buff_size);
	/*if (buff_size==numUniChars) {
		std::stringstream msg;
		msg <<"Error reading file. Buffer size: " <<buff_size << " , numUniChars: " <<numUniChars;
		throw std::runtime_error(msg.str().c_str()); //Conversion probably failed.
	}*/ //Not true! What about an ascii-only file?
	uniBuffer = new wchar_t[numUniChars]; // (wchar_t*) malloc(sizeof(wchar_t)*numUniChars);
	if (mymbstowcs(uniBuffer, buffer, buff_size)==0)
		throw std::runtime_error("Invalid UTF-8 characters in file."); //Invalid UTF-8 characters

	//Done, clean up resources
	wstring res = wstring(uniBuffer, numUniChars);
	delete [] buffer;
	delete [] uniBuffer;

	//And return
	return res;
}



wstring renderAsZawgyi(const wstring &uniString)
{
	//Temp:
	if (uniString.empty())
		return uniString;

	//For now, just wrap a low-level data structure.
	//  I want to re-write the entire algorithm to use
	//  bitflags, so for now we'll just preserve the STL interface.
	wchar_t zawgyiStr[1000];

	//Perform conversion
	//Step 1: Determine which finals won't likely combine; add
	// dashes beneath them.
	wchar_t prevLetter = 0x0000;
	wchar_t currLetter;
	int prevType = BF_OTHER;
	int currType;
	size_t destID = 0;
	size_t length = uniString.length();
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

		//Special case: U+1008
		if (currLetter==0x1008) {
			zawgyiStr[destID++] = L'\u1005';
			currLetter = L'\u103B';
		}

		//Append it, and a dash if necessary
		//Don't dash our stack letter; it's not necessary
		/*if (!passed && currType!=BF_STACKER) 
			zawgyiStr[destID++] = ZG_DASH;*/
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
		//Add initial rules; do this manually for now
		//1-7
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u102F', 0x7FFE00000, L"\u1009\u1025\u100A", ZG_TALL_SINGLE_LEG));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u1030', 0x7FFE00000, L"\u1009\u1025\u100A", ZG_TALL_DOUBLE_LEG));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u102F', 0x180000, NULL, ZG_LEGS_BOTH_WAYS));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u1030', 0x180000, NULL, ZG_LEGS_OF_THREE));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u1037', 0x1580018C000, L"\u1014", ZG_DOT_BELOW_SHIFT_1));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u1037', 0xA700E00000, L"\u101B", ZG_DOT_BELOW_SHIFT_2));
		matchRules.push_back(new Rule(RULE_MODIFY, ZG_DOT_BELOW_SHIFT_1, 0xA700E00000, L"\u101B", ZG_DOT_BELOW_SHIFT_2));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u103A', 0x8000, NULL, ZG_TALL_WITH_ASAT));

		//8-14
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u1036', 0x800, NULL, ZG_DOTTED_CIRCLE_ABOVE));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u1036', 0x100, NULL, ZG_KINZI_1036));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u102D', 0x100, NULL, ZG_KINZI_102D));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u102E', 0x100, NULL, ZG_KINZI_102E));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u102E', 0, L"\u1025", L'\u1026'));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u103E', 0xFF000000, L"\u1020\u100A", ZG_LEG_FWD_SMALL));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u103E', 0x400000, NULL, ZG_LEGGED_CIRCLE_BELOW));
		matchRules.push_back(new Rule(RULE_COMBINE, ZG_LEG_FWD_SMALL, 0x400000, NULL, ZG_LEGGED_CIRCLE_BELOW));
		matchRules.push_back(new Rule(RULE_ORDER, L'\u1036', 0xFF000000, NULL, 0x0000));

		//15-20
		matchRules.push_back(new Rule(RULE_ORDER, L'\u103C', 0x7, NULL, 0x0000));
		matchRules.push_back(new Rule(RULE_ORDER, L'\u1031', 0x7, NULL, 0x0000));
		matchRules.push_back(new Rule(RULE_ORDER, L'\u1031', 0xFF000000, NULL, 0x0000));
		matchRules.push_back(new Rule(RULE_COMBINE, ZG_STACK_SA, 0x600000000, NULL, ZG_YA_PIN_SA));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u103C', 0x4, NULL, ZG_YA_YIT_LONG));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u103B', 0x100E00000, NULL, ZG_YA_PIN_CUT));

		//21-30
		matchRules.push_back(new Rule(RULE_MODIFY, ZG_STACK_SSA, 0x2, NULL, ZG_STACK_SSA_INDENT));
		matchRules.push_back(new Rule(RULE_MODIFY, ZG_STACK_TA, 0x2, NULL, ZG_STACK_TA_INDENT));
		matchRules.push_back(new Rule(RULE_MODIFY, ZG_STACK_HTA2, 0x2, NULL, ZG_STACK_HTA2_INDENT));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u1014', 0x15F00F80000, NULL, ZG_NA_CUT));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u1009', 0x15800800000, L"\u103A", L'\u1025'));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u101B', 0x1800000000, NULL, ZG_YA_CUT));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u100A', 0x4000600000, NULL, ZG_NYA_CUT));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u1025', 0x800000, NULL, ZG_O_CUT));
		matchRules.push_back(new Rule(RULE_MODIFY, ZG_DOT_BELOW_SHIFT_1, 0x2000, NULL, L'\u1037'));
		matchRules.push_back(new Rule(RULE_MODIFY, ZG_DOT_BELOW_SHIFT_2, 0x2000, NULL, L'\u1037'));
	}


	//We maintain a series of offsets for the most recent match. This is used to speed up the process of 
	// pattern matching. We only track the first occurrance, from left-to-right, of the given flag.
	//We scan from left-to-right, then apply rules from right-to-left breaking after each consonant.
	// A "minor break" occurs after each killed consonant, which is not tracked. "CONSONANT" always refers
	// to the main consonant. 
	int firstOccurrence[S3_TOTAL_FLAGS];
	__int64 currMatchFlags = 0;
	for (size_t i=0; i<S3_TOTAL_FLAGS; i++)
		firstOccurrence[i] = -1;
	length = wcslen(zawgyiStr);
	size_t prevConsonant = 0;
	int kinziCascade = 0;
	for (size_t i=0; i<=length; i++) {
		//Get properties on this letter
		wchar_t currLetter = zawgyiStr[i];
		__int64 currFlag = getStage3BitFlags(currLetter);
		int currFlagID = getStage3ID(currFlag);

		//Are we at a stopping point?
		if (isConsonant(currLetter)|| i==length) {
			//First, scan for and fix "tall leg"s
			for (size_t x=i-1; x>=prevConsonant&&x<length; x--) {
				if (zawgyiStr[x]==0x102F || zawgyiStr[x]==0x1030) {
					Rule *r = matchRules[zawgyiStr[x]-0x102F];
					bool matches = ((r->match_flags&currMatchFlags)!=0);
					if (!matches) {
						for (size_t sID=0; sID<wcslen(r->match_additional) && !matches; sID++) {
							for (size_t zID=prevConsonant; zID<i && !matches; zID++) {
								if (zawgyiStr[zID]==r->match_additional[sID]) {
									matches = true;
								}
							}
						}
					}

					if (matches) {
						int currID = getStage3ID(getStage3BitFlags(zawgyiStr[x]));
						int replacementID = getStage3ID(getStage3BitFlags(r->replace));
						if (currID!=-1) {
							currMatchFlags ^= getStage3BitFlags(zawgyiStr[x]);
							firstOccurrence[currID] = -1;
						}
						if (replacementID != -1) {
							currMatchFlags |= getStage3BitFlags(r->replace);
							firstOccurrence[replacementID] = x;
						}
						zawgyiStr[x] = r->replace;
					}
				}
			}

			//Apply our filters, from right-to-left
			for (size_t x=i-1; x>=prevConsonant&&x<length; x--) {
				bool resetRules = false;
				for (size_t ruleID=2; ruleID<matchRules.size(); ruleID++) {
					if (resetRules) {
						ruleID = 2;
						resetRules = false;
					}

					//Unfortunately, we have to apply ALL filters.
					Rule *r = matchRules[ruleID];
					if (r->at_letter!=zawgyiStr[x])
						continue; 

					//First, match the input
					__int64 matchRes = r->match_flags&currMatchFlags;
					size_t matchLoc = -1;
					if (matchRes==0) {
						bool foundAdditional = false;
						if (r->match_additional!=NULL) {
							for (size_t sID=0; sID<wcslen(r->match_additional) && !foundAdditional; sID++) {
								for (size_t zID=prevConsonant; zID<i && !foundAdditional; zID++) {
									if (zawgyiStr[zID]==r->match_additional[sID]) {
										matchLoc = zID;
										foundAdditional = true;
									}
								}
							}
						}
						if (!foundAdditional)
							continue;
					} else {
						//Where did it match?
						matchLoc = getStage3ID(matchRes);
						if (matchLoc!=-1)
							matchLoc = firstOccurrence[matchLoc];
					}

					//Then, apply the rule. Make sure to keep our index array up-to-date
					//Note that protocol specifies that we DON'T re-scan for the next occurrence of a medial
					//  after modifying or combining it.
					int matchResID = getStage3ID(matchRes);
					int replacementID = getStage3ID(getStage3BitFlags(r->replace));
					int currID = getStage3ID(getStage3BitFlags(zawgyiStr[x]));
					bool checkMissingRules = false;
					switch (r->type) {
						case RULE_MODIFY:
							if (currID!=-1) {
								currMatchFlags ^= getStage3BitFlags(zawgyiStr[x]);
								firstOccurrence[currID] = -1;
							}
							if (replacementID != -1) {
								currMatchFlags |= getStage3BitFlags(r->replace);
								firstOccurrence[replacementID] = x;
							}
							zawgyiStr[x] = r->replace;

							//We now have to re-scan old rules
							checkMissingRules = true;

							break;
						case RULE_ORDER:
						{ //With c+j+c+j, the second syllable doesn't trigger a match at all (Must be incrementing wrongly)
							if (matchLoc==-1)
								break; //Our rules shouldn't have this problem.
							if (x<matchLoc)
								break; //Don't shift right
							if (r->blacklisted)
								break; //Avoid cycles
							wchar_t prevLetter = zawgyiStr[x];
							for (size_t repID=matchLoc; repID<=x; repID++) {
								int prevID = getStage3ID(getStage3BitFlags(prevLetter));
								if (prevID!=-1)
									firstOccurrence[prevID] = repID;
								wchar_t cached = zawgyiStr[repID];
								zawgyiStr[repID] = prevLetter;
								prevLetter = cached;
							}

							//We actually have to apply rules from the beginning, unfortunately. However,
							// we prevent an infinite cycle by blacklisting this rule until the next 
							// consonant occurs.
							r->blacklisted = true;
							resetRules = true;

							break;
						}
						case RULE_COMBINE:
							if (matchLoc==-1)
								break; //Shouldn't exist
							if (matchResID!=-1) {
								currMatchFlags ^= matchRes;
								firstOccurrence[matchResID] = -1;
							}
							if (currID!=-1) {
								currMatchFlags ^= getStage3BitFlags(zawgyiStr[x]);
								firstOccurrence[currID] = -1;
							}
							if (replacementID != -1) {
								currMatchFlags |= getStage3BitFlags(r->replace);
								firstOccurrence[replacementID] = x;
							}
							zawgyiStr[matchLoc] = 0x0000;
							zawgyiStr[x] = r->replace;

							//We now have to re-scan old rules
							checkMissingRules = true;


							break;
					}


					//Double-check for missing rules
					if (checkMissingRules && /*TEMP*/false/*ENDTEMP*/  /*Logger::isLogging('L')*/) {
						for (size_t prevRule=2; prevRule < ruleID; prevRule++) {
							Rule *r = matchRules[prevRule];
							if (r->at_letter==zawgyiStr[x] && ((r->match_flags&currMatchFlags)!=0) && !r->blacklisted) {
								matchLoc = -1;
								matchLoc = getStage3ID(r->match_flags&currMatchFlags);
								if (r->type==RULE_ORDER && x<matchLoc)
									continue;

								//Special cases added as we detect them:
								//Dot below
								if (matchRules[ruleID]->at_letter==ZG_DOT_BELOW_SHIFT_1 || matchRules[ruleID]->at_letter==ZG_DOT_BELOW_SHIFT_2)
									continue;

								/*TODO: Re-enable logging
								wstringstream line;
								line <<L"WARNING: Rule " <<prevRule <<L" was skipped after matching rule "  <<ruleID;
								Logger::writeLogLine('L', line.str());*/
							}
						}
					}



					//Make sure our cached data is up-to-date
					//No! Bad Seth!
					/*currLetter = zawgyiStr[x];
					currFlag = getStage3BitFlags(currLetter);
					currFlagID = getStage3ID(currFlag);*/
				}
			}

			//Final special cases 
			int yaYitID = firstOccurrence[getStage3ID(S3_MED_YA_YIT)];
			bool yaLong = false;
			if (yaYitID==-1) {
				yaYitID = firstOccurrence[getStage3ID(S3_MED_YA_YIT_LONG)];
				yaLong = true;
			}
			if (yaYitID != -1) {
				bool cutTop = false;
				bool cutBottom = false;
				if ((currMatchFlags&0xFF0)!=0)
					cutTop = true;
				if ((currMatchFlags&0x100E00000)!=0)
					cutBottom = true;
				wchar_t yaFinal = 'X';
				if (yaLong) {
					if (cutTop) {
						if(cutBottom)
							yaFinal = ZG_YA_YIT_LONG_BOTHCUT;
						else
							yaFinal = ZG_YA_YIT_LONG_HIGHCUT;
					} else {
						if(cutBottom)
							yaFinal = ZG_YA_YIT_LONG_LOWCUT;
						else
							yaFinal = ZG_YA_YIT_LONG;
					}
				} else {
					if (cutTop) {
						if(cutBottom)
							yaFinal = ZG_YA_YIT_BOTHCUT;
						else
							yaFinal = ZG_YA_YIT_HIGHCUT;
					} else {
						if(cutBottom)
							yaFinal = ZG_YA_YIT_LOWCUT;
						else
							yaFinal = L'\u103C';
					}
				}
				zawgyiStr[yaYitID] = yaFinal; 
			}


			//Is this a soft-stop or a hard stop?
			bool softStop = (i<length && zawgyiStr[i+1]==0x103A);
			currMatchFlags &= (S3_CONSONANT_NARROW|S3_CONSONANT_OTHER|S3_CONSONANT_WIDE);
			for (int x=0; x<S3_TOTAL_FLAGS; x++) {
				if (softStop && (x==S3_CONSONANT_NARROW || x==S3_CONSONANT_WIDE || x==S3_CONSONANT_OTHER))
					continue;
				firstOccurrence[x] = -1;
			}
			if (!softStop) {
				//Special case: re-order "kinzi + consonant"
				if (i>0 && kinziCascade==0 && zawgyiStr[i-1]==ZG_KINZI && zawgyiStr[i]!=0x0000) {
					zawgyiStr[i-1] = zawgyiStr[i];
					zawgyiStr[i] = ZG_KINZI;
					i--;
					kinziCascade = 2;
				}

				//Reset our black-list.
				for (size_t rID=0; rID<matchRules.size(); rID++)
					matchRules[rID]->blacklisted = false;

				if (currFlagID!=-1) {
					firstOccurrence[currFlagID] = i;
					currMatchFlags = currFlag;
				} else 
					currMatchFlags = 0;

				if (kinziCascade>0)
					kinziCascade--;
			}
			prevConsonant = i;
		} else {
			//Just track this letter's location
			if (currFlagID!=-1) {
				if (firstOccurrence[currFlagID]==-1)
					firstOccurrence[currFlagID] = i;
				currMatchFlags |= currFlag;
			}
		}
	}

	//Step 4: Convert each letter to its Zawgyi-equivalent
	//length = wcslen(zawgyiStr); //Keep embedded zeroes
	destID =0;
	for (size_t i=0; i<length; i++) {
		if (zawgyiStr[i]==0x0000)
			continue;
		zawgyiStr[destID++] = zawgyiLetter(zawgyiStr[i]);
	}
	zawgyiStr[destID++] = 0x0000;



	//Stage 5: Apply rules for re-ordering the Zawgyi text to fit our weird model.
	length = wcslen(zawgyiStr);
	if (reorderPairs.size()==0) {
		reorderPairs.push_back(L"\u102F\u102D");
		reorderPairs.push_back(L"\u103A\u102D");
		reorderPairs.push_back(L"\u103D\u102D");
		reorderPairs.push_back(L"\u1075\u102D");
		reorderPairs.push_back(L"\u102D\u1087");
		reorderPairs.push_back(L"\u103D\u102E");
		reorderPairs.push_back(L"\u103D\u103A");
		reorderPairs.push_back(L"\u1039\u103A");
		reorderPairs.push_back(L"\u1030\u102D");
		reorderPairs.push_back(L"\u1037\u1039");
		reorderPairs.push_back(L"\u1032\u1037");
		reorderPairs.push_back(L"\u1032\u1094");
		reorderPairs.push_back(L"\u1064\u1094");
		reorderPairs.push_back(L"\u102D\u1094");
		reorderPairs.push_back(L"\u102D\u1071");
		reorderPairs.push_back(L"\u1036\u1037");
		reorderPairs.push_back(L"\u1036\u1088");
		reorderPairs.push_back(L"\u1039\u1037");
		reorderPairs.push_back(L"\u102D\u1033");
		reorderPairs.push_back(L"\u103C\u1032");
		reorderPairs.push_back(L"\u103C\u102D");
		reorderPairs.push_back(L"\u103C\u102E");
		reorderPairs.push_back(L"\u1036\u102F");
		reorderPairs.push_back(L"\u1036\u1088");
		reorderPairs.push_back(L"\u1036\u103D");
		reorderPairs.push_back(L"\u1036\u103C");
		reorderPairs.push_back(L"\u103C\u107D");
		reorderPairs.push_back(L"\u1088\u102D");
		reorderPairs.push_back(L"\u1039\u103D");
		reorderPairs.push_back(L"\u108A\u107D");
		reorderPairs.push_back(L"\u103A\u1064");
		reorderPairs.push_back(L"\u1036\u1033");
	}
	for (size_t i=1; i<length; i++) {
		//Apply stage-2 rules
		for (size_t ruleID=0; ruleID<reorderPairs.size(); ruleID++) {
			wchar_t *rule = reorderPairs[ruleID];
			if (zawgyiStr[i]==rule[0] && zawgyiStr[i-1]==rule[1]) {
				zawgyiStr[i-1] = rule[0];
				zawgyiStr[i] = rule[1];
			}
		}

		//Apply stage 3 fixed rules
		if (i>1) {
			if (zawgyiStr[i-2]==0x1019 && zawgyiStr[i-1]==0x102C && zawgyiStr[i]==0x107B) {
				zawgyiStr[i-1]=0x107B;
				zawgyiStr[i]=0x102C;
			}
			if (zawgyiStr[i-2]==0x103A && zawgyiStr[i-1]==0x102D && zawgyiStr[i]==0x1033) {
				zawgyiStr[i-1]=0x1033;
				zawgyiStr[i]=0x102D;
			}
			if (zawgyiStr[i-2]==0x103C && zawgyiStr[i-1]==0x1033 && zawgyiStr[i]==0x102D) {
				zawgyiStr[i-1]=0x102D;
				zawgyiStr[i]=0x1033;
			}
			if (zawgyiStr[i-2]==0x103A && zawgyiStr[i-1]==0x1033 && zawgyiStr[i]==0x1036) {
				zawgyiStr[i-1]=0x1036;
				zawgyiStr[i]=0x1033;
			}
			if (zawgyiStr[i-2]==0x103A && zawgyiStr[i-1]==0x108B && zawgyiStr[i]==0x1033) {
				zawgyiStr[i-1]=0x1033;
				zawgyiStr[i]=0x108B;
			}

			//This one's a little different
			if (zawgyiStr[i]==0x1036 && zawgyiStr[i-2]==0x103C && zawgyiStr[i-1]==0x107D) {
				zawgyiStr[i-2]=0x1036;
				zawgyiStr[i-1]=0x103C;
				zawgyiStr[i]=0x107D;
			}
		}

		//Apply stage 4 multi-rules
		if (i>2) {
			if (zawgyiStr[i-3]==0x1019 &&
					(   (zawgyiStr[i-2]==0x107B && zawgyiStr[i-1]==0x1037 && zawgyiStr[i]==0x102C)
					  ||(zawgyiStr[i-2]==0x102C && zawgyiStr[i-1]==0x107B && zawgyiStr[i]==0x1037)
					  ||(zawgyiStr[i-2]==0x102C && zawgyiStr[i-1]==0x1037 && zawgyiStr[i]==0x107B)
					  ||(zawgyiStr[i-2]==0x1037 && zawgyiStr[i-1]==0x107B && zawgyiStr[i]==0x102C)
					)) {
				zawgyiStr[i-2]=0x107B;
				zawgyiStr[i-1]=0x102C;
				zawgyiStr[i]=0x1037;
			}
			if (zawgyiStr[i-3]==0x107E && zawgyiStr[i-1]==0x1033 && zawgyiStr[i]==0x1036) {
				zawgyiStr[i-1]=0x1036;
				zawgyiStr[i]=0x1033;
			}
		}
	}


	return wstring(zawgyiStr);
}



std::string escape_wstr(const std::wstring& str)
{
	return escape_wstr(str, false);
}

std::string escape_wstr(const std::wstring& str, bool errOnUnicode)
{
	std::stringstream res;
	for (wstring::const_iterator c=str.begin(); c!=str.end(); c++) {
		if (*c < std::numeric_limits<char>::max())
			res << static_cast<char>(*c);
		else if (errOnUnicode)
			throw std::runtime_error("String contains unicode");
		else
			res << "\\u" << std::hex << *c << std::dec;
	}
	return res.str();
}


std::string wcs2mbs(const std::wstring& str)
{
	std::stringstream res;
	for (wstring::const_iterator c=str.begin(); c!=str.end(); c++) {
		if (*c >= 0x0000 && *c <= 0x007F)
			res << static_cast<char>(*c);
		else if (*c >= 0x0080 && *c <= 0x07FF) {
			char c1 = (((*c)>>6)&0x1F) | 0xC0;
			char c2 = ((*c)&0x3F) | 0x80;
			res <<c1 <<c2;
		} else if (*c >= 0x0800 && *c <= 0xFFFF) {
			char c1 = (((*c)>>12)&0xF) | 0xE0;
			char c2 = (((*c)>>6)&0x3F) | 0x80;
			char c3 = ((*c)&0x3F) | 0x80;
		} else 
			throw std::runtime_error("Unicode value out of range.");

	}
	return res.str();
}




//Remove: 
//  space_p, comment_p('#'), U+FEFF (BOM)
std::wstring preparse_json(const std::wstring& str)
{
	std::wstringstream res;
	for (size_t i=0; i<str.length(); i++) {
		//If you're on a whitespace/BOM, keep skipping until you reach non-whitespace.
		while (i<str.length() && (iswspace(str[i]) || str[i]==L'\uFEFF' || str[i]==L'\uFFFE'))
			i++;
		if (i>=str.length())
			break;

		//If you're on a quote, read until the endquote (skip escapes) and append. (Then continue)
		//TODO: We can speed this up later, but for now it doesn't matter.
		if (str[i]==L'"') {
			size_t start = i++;
			while (i<str.length() && (str[i]!=L'"' || str[i-1]==L'\\')) //Skip \" too
				i++;
			res <<str.substr(start, i+1-start); //Note: even if this overruns the length of the string, it won't err.
			continue;
		}

		//If you're on a comment character, skip until the end of the line and continue
		if (str[i]==L'#') {
			while (i<str.length() && str[i]!=L'\n') //Skip until the end of the line; should skip \r too.
				i++;
			continue;
		}

		//Now, we know we're on a valid character. So keep reading until we encounter:
		//   A double quote, a comment marker, or whitespace (BOMs won't occur more than once)
		//   The end of the stream
		size_t start = i;
		while (i<str.length() && str[i]!=L'#' && !iswspace(str[i]))
			i++;
		res <<str.substr(start, i-start);
	}

	return res.str();
}


//Removes some errors that typically occur in Burglish strings. A more solid normalization may be performed later.
//Return the same string if unknown characters are encountered.
//TODO: We also need to fix tall/short "ar".
std::wstring normalize_bgunicode(const std::wstring& str)
{
	//Unicode order goes something like this:
	//KINZI, CONSONANT, STACKED, ASAT, YA-PIN, YA-YIT, WA, HA, AY, "UPPER VOWEL", "LOWER VOWEL", AR, SLASH, DOT UP, DOT DOWN, ASAT, VIS
	//KINZI = \u1004\u103A\u1039
	//CONSONANT = [\u1000..\u102A, \u103F, \u104E]
	//STACKED = \u1039 [\u1000..\u1019 \u101C, \u101E, \u1020, \u1021]
	//...exclusively for Myanmar. Now... we just have to flag each item, and avoid adding it twice. If something unknown 
	//   if found, return the orig. word.
	//This function is very fragile; we'll have to replace it with something better eventually.
	//We can assume kinzi & stacked letters aren't abused. Also consonant.
	wstringstream res;
	bool flags[] = {false,false,false,false,false,false,false,false,false,false,false,false,false};
	size_t numFlags = 13;
	for (size_t i=0; i<str.size(); i++) {
		//First, skip stuff we don't care about
		if (str[i]==L'\u1004' && i+2<str.size() && str[i+1]==L'\u103A' && str[i+1]==L'\u1039') {
			//Kinzi, skip
			res <<str[i] <<str[i+1] <<str[i+2];
			i += 2;
			continue;
		} else if (str[i]==L'\u1039' && i+1<str.size()) {
			//Stacked letter, skip
			res <<str[i] <<str[i+1];
			i += 1;
			continue;
		} else if ((str[i]>=L'\u1000' && str[i]<=L'\u102A') || str[i]==L'\u103F' || str[i]==L'\u104E') {
			//Consonant, skip
			res <<str[i];

			//Also, reset our "flags" array so that multiple killed consants parse ok.
			for (size_t x=0; x<numFlags; x++)
				flags[x] = false;
			
			continue;
		}

		//Now, we're at some definite data. Skip duplicates, return early if we don't know this letter.
		int x=-1; //Use fallthrough to build up an array index, 0..12 (-1 is filtered by the default statement)
		switch (str[i]) {
			case L'\u103A':                   x++;
			case L'\u103B':                   x++;
			case L'\u103C':                   x++;
			case L'\u103D':                   x++;
			case L'\u103E':                   x++;
			case L'\u1031':                   x++;
			case L'\u102D': case L'\u102E':   x++;
			case L'\u102F': case L'\u1030':   x++;
			case L'\u102B': case L'\u102C':   x++;
			case L'\u1032':                   x++;
			case L'\u1036':                   x++;
			case L'\u1037':                   x++;
			case L'\u1038':                   x++;
				break;
			default:
				return str;
		}

		//Check our ID anyway
		if (x<0 || x>=(int)numFlags)
			throw std::runtime_error("normalize_bgunicode id error!");

		//Now, append the letter ONLY if this flag is false
		if (!flags[x]) {
			flags[x] = true;

			//Adjust one more letter (tall/short "AR")
			wchar_t toAppend = str[i];
			if (toAppend==L'\u102C') {
				//We need to search backwards like so:
				//Continue on [\u1039\u1000  -  \u1039\u1019  \u103E  \u102F   \u1030  \u103D \u1031 (but not \u103C)]
				//Stop on [\u1001 \u1002 \u1004 \u1012 \u1015 \u101D \u101D]
				//(We'll have to check this against the WZ wordlist eventually.).
				bool match = false;
				for (int si = ((int)i)-1;si>=0;si--) {
					//Simple letters
					if (str[si]==L'\u103E' || str[si]==L'\u102F' || str[si]==L'\u1030' || str[si]==L'\u103D' || str[si]==L'\u1031')
						continue;
					//Stacked letters
					if (str[si]>=L'\u1030' && str[si]<=L'\u1019') {
						if (si>=1 && str[si-1]==L'\u1039') {
							si--;
							continue;
						} else
							break;
					}
					//What we're searching for.
					if (str[si]==L'\u1001' || str[si]==L'\u1002' || str[si]==L'\u1004' || str[si]==L'\u1012' || str[si]==L'\u1015' || str[si]==L'\u101D' || str[si]==L'\u101D') {
						match = true;
						break;
					}
					//Else... (no match)
					break;
				}
				if (match)
					toAppend=L'\u102B';
			}
			res <<toAppend;
		}
	}


	//Every test passed
	return res.str();
}


//Add everything EXCEPT what's in the filterStr.
std::wstring removeZWS(const std::wstring& str, const std::wstring& filterStr)
{
	std::wstringstream res; 
	for (size_t i=0; i<str.length(); i++) {
		if (filterStr.find(str[i])==wstring::npos) 
			res <<str[i];
	}
	return res.str();
}


size_t count_letter(const std::wstring& str, wchar_t letter) 
{
	size_t res = 0;
	for (size_t i=0; i<str.length(); i++) {
		if (str[i]==letter)
			res++;
	}
	return res;
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
