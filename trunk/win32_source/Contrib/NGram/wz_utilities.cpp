/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "wz_utilities.h"

using std::vector;
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
	//Various constants
	enum {
		//Constant pseudo-letters
		ZG_DASH           = 0xE000,
	    ZG_KINZI,

		//Constant pseudo-letters (stacked)
		ZG_STACK_KA       = 0xE100,
		ZG_STACK_KHA,
		ZG_STACK_GA,
		ZG_STACK_GHA,
		ZG_STACK_NGA,
		ZG_STACK_SA,
		ZG_STACK_SSA,
		ZG_STACK_ZA,
		ZG_STACK_ZHA,
		//Skip one...
		ZG_STACK_NYA,
		ZG_STACK_TTA,
		ZG_STACK_HTA1,
		ZG_STACK_DHA1,
		ZG_STACK_EXTRA,
		ZG_STACK_NHA,
		ZG_STACK_TA,
		ZG_STACK_HTA2,
		ZG_STACK_DDA,
		ZG_STACK_DHA2,
		ZG_STACK_NA,
		ZG_STACK_PA,
		ZG_STACK_PHA,
		ZG_STACK_VA,
		ZG_STACK_BA,
		ZG_STACK_MA,
		//Break sequence
		ZG_STACK_YA,
		ZG_STACK_LA,
		ZG_STACK_THA,
		ZG_STACK_A,
		//Special-purpose indented stacked letters
		ZG_STACK_SSA_INDENT,
		ZG_STACK_TA_INDENT,
		ZG_STACK_HTA2_INDENT,

		//Some complex letters
		ZG_COMPLEX_1                  = 0xE200,
		ZG_COMPLEX_2,
		ZG_COMPLEX_3,
		ZG_COMPLEX_4,
		ZG_COMPLEX_5,
		ZG_COMPLEX_NA,

		//Letters which share similar semantic functionality
		ZG_TALL_WITH_ASAT,
		ZG_DOTTED_CIRCLE_ABOVE,
		ZG_LEGGED_CIRCLE_BELOW,
		ZG_LEGS_BOTH_WAYS,
		ZG_LEGS_OF_THREE,
		ZG_KINZI_102D,
		ZG_KINZI_102E,
		ZG_KINZI_1036,

		//Some more substitures
		ZG_DOT_BELOW_SHIFT_1,
		ZG_DOT_BELOW_SHIFT_2,
		ZG_TALL_SINGLE_LEG,
		ZG_TALL_DOUBLE_LEG,
		ZG_YA_PIN_CUT,
		ZG_YA_PIN_SA,
		ZG_YA_YIT_LONG,
		ZG_YA_YIT_HIGHCUT,
		ZG_YA_YIT_LONG_HIGHCUT,
		ZG_YA_YIT_LOWCUT,
		ZG_YA_YIT_LONG_LOWCUT,
		ZG_YA_YIT_BOTHCUT,
		ZG_YA_YIT_LONG_BOTHCUT,
		ZG_LEG_FWD_SMALL,
		ZG_NA_CUT,
		ZG_YA_CUT,
		ZG_NYA_CUT,
		ZG_O_CUT,
	};

	//Constants for our counting sort algorithm
	enum {
		ID_MED_Y          = 0,
		ID_MED_R,
		ID_MED_W,
		ID_MED_H,
		ID_VOW_E,
		ID_VOW_ABOVE,
		ID_VOW_BELOW,
		ID_VOW_A,
		ID_ANUSVARA,
		ID_DOW_BELOW,
		ID_VISARGA,
		ID_TOTAL
	};

	//Bitflags for our step-three algorithm
	const uint64_t S3_TOTAL_FLAGS                 = 42;
	const uint64_t S3_VISARGA                     = 0x20000000000;
	const uint64_t S3_LEG_SINGLE                  = 0x10000000000;
	const uint64_t S3_LEG_SINGLE_TALL             =  0x8000000000;
	const uint64_t S3_LEG_DOUBLE                  =  0x4000000000;
	const uint64_t S3_LEG_DOUBLE_TALL             =  0x2000000000;
	const uint64_t S3_TWO_LEGS                    =  0x1000000000;
	const uint64_t S3_THREE_LEGS                  =   0x800000000;
	const uint64_t S3_MED_YA_PIN                  =   0x400000000;
	const uint64_t S3_MED_YA_PIN_SHORT            =   0x200000000;
	const uint64_t S3_MED_YA_PIN_SSA_STACK        =   0x100000000;
	const uint64_t S3_MED_YA_YIT                  =    0x80000000;
	const uint64_t S3_MED_YA_YIT_LONG             =    0x40000000;
	const uint64_t S3_MED_YA_YIT_HIGH_CUT         =    0x20000000;
	const uint64_t S3_MED_YA_YIT_LONG_HIGH_CUT    =    0x10000000;
	const uint64_t S3_MED_YA_YIT_LOW_CUT          =     0x8000000;
	const uint64_t S3_MED_YA_YIT_LONG_LOW_CUT     =     0x4000000;
	const uint64_t S3_MED_YA_YIT_BOTH_CUT         =     0x2000000;
	const uint64_t S3_MED_YA_YIT_LONG_BOTH_CUT    =     0x1000000;
	const uint64_t S3_STACKED                     =      0x800000; //(includes indented)
	const uint64_t S3_CIRC_BELOW                  =      0x400000;
	const uint64_t S3_CIRC_BELOW_W_LEG            =      0x200000;
	const uint64_t S3_LEG_FWD                     =      0x100000;
	const uint64_t S3_LEG_FWD_SMALL               =       0x80000;
	const uint64_t S3_DOT_BELOW                   =       0x40000;
	const uint64_t S3_DOT_BELOW_2                 =       0x20000;
	const uint64_t S3_DOT_BELOW_3                 =       0x10000;
	const uint64_t S3_VOW_AR_TALL                 =        0x8000;
	const uint64_t S3_VOW_AR_TALL_ASAT            =        0x4000;
	const uint64_t S3_VOW_AR_SHORT                =        0x2000;
	const uint64_t S3_DOT_ABOVE                   =        0x1000;
	const uint64_t S3_CIRCLE_ABOVE                =         0x800;
	const uint64_t S3_CIRCLE_DOT_ABOVE            =         0x400;
	const uint64_t S3_CIRCLE_CROSSED_ABOVE        =         0x200;
	const uint64_t S3_KINZI                       =         0x100;
	const uint64_t S3_KINZI_DOT                   =          0x80;
	const uint64_t S3_KINZI_CIRC                  =          0x40;
	const uint64_t S3_KINZI_CIRC_CROSSED          =          0x20;
	const uint64_t S3_MED_SLANT_ABOVE             =          0x10;
	const uint64_t S3_VOW_A                       =           0x8;
	const uint64_t S3_CONSONANT_WIDE              =           0x4;
	const uint64_t S3_CONSONANT_NARROW            =           0x2;
	const uint64_t S3_CONSONANT_OTHER             =           0x1;
	const uint64_t S3_OTHER                       =           0x0;


	//Bitflags for our zawgyi conversion algorithm
	const uint16_t BF_CONSONANT    = 2048;
	const uint16_t BF_STACKER      = 1024;
	const uint16_t BF_DOT_LOW      =  512;
	const uint16_t BF_DOT_OVER     =  256;
	const uint16_t BF_VOW_AR       =  128;
	const uint16_t BF_LEG_NORM     =   64;
	const uint16_t BF_VOW_OVER     =   32;
	const uint16_t BF_VOW_A        =   16;
	const uint16_t BF_LEG_REV      =    8;
	const uint16_t BF_CIRC_LOW     =    4;
	const uint16_t BF_MED_YA       =    2;
	const uint16_t BF_ASAT         =    1;
	const uint16_t BF_OTHER        =    0;

	//Match into this array
	const unsigned int matchFlags[] = {0xBDE, 0x801, 0x803, 0x887, 0x80F, 0x89E, 0x93F, 0xB7F, 0x8FF, 0x9FF, 0x800};


	//Useful global vars
	//FILE *wzUtilLogFile = NULL;

	//A fun struct
	enum {
		RULE_MODIFY   = 1,
	    RULE_COMBINE,
		RULE_ORDER,
	};

	struct Rule {
		int type;
		wchar_t at_letter;
		uint64_t match_flags;
		wstring match_additional;
		wchar_t replace;
		bool blacklisted;

		Rule(int type, wchar_t atLetter, uint64_t matchFlags, const wstring& matchAdditional, wchar_t replace) {
			this->type = type;
			this->at_letter = atLetter;
			this->match_flags = matchFlags;
			this->match_additional = matchAdditional;
			this->replace = replace;
			this->blacklisted = false;
		}

		wstring matchFlagsBin() const {
			std::wstringstream res;
			for (uint64_t i=S3_VISARGA; i>0; i>>=1) {
				if (match_flags&i)
					res <<L"1";
				else
					res <<L"0";
			}
			return res.str();
		}

		wstring toString() const {
			std::wstringstream res;
			res <<L"Rule{";
			res <<((type==RULE_MODIFY)?L"MODIFY":(type==RULE_COMBINE)?L"COMBINE":(type==RULE_ORDER)?L"ORDER":L"<ERR>");
			res <<L", at[" <<at_letter <<"]";
			res <<L", match[" <<matchFlagsBin() <<L"]";
			if (!match_additional.empty())
				res <<L", additional[" <<match_additional <<L"]";
			res <<L", replace[" <<replace <<L"]";
			res <<L"}";
			return res.str();
		}
	};
	vector<Rule*> matchRules;
	vector<wstring> reorderPairs;


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
			|| (letter==0x103F)
			|| (letter==0x200B);
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
			case L'\u103B':
				return ID_MED_Y;
			case L'\u103C':
				return ID_MED_R;
			case L'\u103D':
				return ID_MED_W;
			case L'\u103E':
				return ID_MED_H;
			case L'\u1031':
				return ID_VOW_E;
			case L'\u102D':
			case L'\u102E':
			case L'\u1032':
				return ID_VOW_ABOVE;
			case L'\u102F':
			case L'\u1030':
				return ID_VOW_BELOW;
			case L'\u102B':
			case L'\u102C':
				return ID_VOW_A;
			case L'\u1036':
				return ID_ANUSVARA;
			case L'\u1037':
				return ID_DOW_BELOW;
			case L'\u1038':
				return ID_VISARGA;
			default:
				return -1;
		}
	}


	int getBitflag(wchar_t uniLetter)
	{
		switch (uniLetter)
		{
			case L'\u1039':
				return BF_STACKER;
			case L'\u1037':
				return BF_DOT_LOW;
			case L'\u1036':
				return BF_DOT_OVER;
			case L'\u102B':
			case L'\u102C':
				return BF_VOW_AR;
			case L'\u102F':
			case L'\u1030':
				return BF_LEG_NORM;
			case L'\u102D':
			case L'\u102E':
			case L'\u1032':
				return BF_VOW_OVER;
			case L'\u1031':
				return BF_VOW_A;
			case L'\u103E':
				return BF_LEG_REV;
			case L'\u103D':
				return BF_CIRC_LOW;
			case L'\u103B':
			case L'\u103C':
				return BF_MED_YA;
			case L'\u103A':
				return BF_ASAT;
			case L'\u1025':
			case L'\u1027':
			case L'\u1029':
			case L'\u103F':
			case ZG_DASH:
				return BF_CONSONANT;
			default:
				if (uniLetter>=L'\u1000' && uniLetter<=L'\u1021')
					return BF_CONSONANT;
				return BF_OTHER;
		}
	}



	uint64_t getStage3BitFlags(wchar_t letter)
	{
		switch (letter) {
			case L'\u1038':
				return S3_VISARGA;
			case L'\u102F':
				return S3_LEG_SINGLE;
			case ZG_TALL_SINGLE_LEG:
				return S3_LEG_SINGLE_TALL;
			case L'\u1030':
				return S3_LEG_DOUBLE;
			case ZG_TALL_DOUBLE_LEG:
				return S3_LEG_DOUBLE_TALL;
			case ZG_LEGS_BOTH_WAYS:
				return S3_TWO_LEGS;
			case ZG_LEGS_OF_THREE:
				return S3_THREE_LEGS;
			case L'\u103B':
				return S3_MED_YA_PIN;
			case ZG_YA_PIN_CUT:
				return S3_MED_YA_PIN_SHORT;
			case ZG_YA_PIN_SA:
				return S3_MED_YA_PIN_SSA_STACK;
			case L'\u103C':
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
			case L'\u103D':
				return S3_CIRC_BELOW;
			case ZG_LEGGED_CIRCLE_BELOW:
				return S3_CIRC_BELOW_W_LEG;
			case L'\u103E':
				return S3_LEG_FWD;
			case ZG_LEG_FWD_SMALL:
				return S3_LEG_FWD_SMALL;
			case L'\u1037':
				return S3_DOT_BELOW;
			case ZG_DOT_BELOW_SHIFT_1:
				return S3_DOT_BELOW_2;
			case ZG_DOT_BELOW_SHIFT_2:
				return S3_DOT_BELOW_3;
			case L'\u102B':
				return S3_VOW_AR_TALL;
			case ZG_TALL_WITH_ASAT:
				return S3_VOW_AR_TALL_ASAT;
			case L'\u102C':
				return S3_VOW_AR_SHORT;
			case L'\u1036':
				return S3_DOT_ABOVE;
			case L'\u102D':
				return S3_CIRCLE_ABOVE;
			case ZG_DOTTED_CIRCLE_ABOVE:
				return S3_CIRCLE_DOT_ABOVE;
			case L'\u102E':
				return S3_CIRCLE_CROSSED_ABOVE;
			case ZG_KINZI:
				return S3_KINZI;
			case ZG_KINZI_1036:
				return S3_KINZI_DOT;
			case ZG_KINZI_102D:
				return S3_KINZI_CIRC;
			case ZG_KINZI_102E:
				return S3_KINZI_CIRC_CROSSED;
			case L'\u1032':
				return S3_MED_SLANT_ABOVE;
			case L'\u1031':
				return S3_VOW_A;
			case L'\u1000':
			case L'\u1003':
			case L'\u1006':
			case L'\u100F':
			case L'\u1010':
			case L'\u1011':
			case L'\u1018':
			case L'\u101A':
			case L'\u101C':
			case L'\u101E':
			case L'\u101F':
			case L'\u1021':
			case L'\u103F':
				return S3_CONSONANT_WIDE;
			case L'\u1001':
			case L'\u1002':
			case L'\u1004':
			case L'\u1005':
			case L'\u1007':
			case L'\u100E':
			case L'\u1012':
			case L'\u1013':
			case L'\u1014':
			case L'\u1015':
			case L'\u1016':
			case L'\u1017':
			case L'\u1019':
			case L'\u101B':
			case L'\u101D':
			case L'\u1027':
			case ZG_NA_CUT:
			case ZG_YA_CUT:
				return S3_CONSONANT_NARROW;
			case L'\u1008':
			case L'\u1009':
			case L'\u100A':
			case ZG_NYA_CUT:
			case L'\u100B':
			case L'\u100C':
			case L'\u100D':
			case L'\u1020':
			case L'\u1025':
			case ZG_O_CUT:
			case L'\u1029':
				return S3_CONSONANT_OTHER;
			default:
				if (letter>=ZG_STACK_KA && letter<=ZG_STACK_HTA2_INDENT)
					return S3_STACKED;
				return S3_OTHER;
		}
	}


	int getStage3ID(uint64_t bitflags)
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
			case L'\u101B':
				return ZG_STACK_YA;
			case L'\u101C':
				return ZG_STACK_LA;
			case L'\u101E':
				return ZG_STACK_THA;
			case L'\u1021':
				return ZG_STACK_A;
			default:
				if (uniLetter>=L'\u1000' && uniLetter<=L'\u1008') {
					return (uniLetter-L'\u1000')+ZG_STACK_KA;
				} else if (uniLetter>=L'\u100A' && uniLetter<=L'\u100D') {
					return (uniLetter-L'\u100A')+ZG_STACK_NYA;
				} else if (uniLetter>=L'\u100F' && uniLetter<=L'\u1019') {
					return (uniLetter-L'\u100F')+ZG_STACK_NHA;
				} else if (uniLetter==L'\u100E') {
					return ZG_STACK_EXTRA; //Todo: we can merge this & the top 2 "if"s.
				}
		}
		return 0;
	}




	wchar_t zawgyiLetter(wchar_t uniLetter)
	{
		switch (uniLetter)
		{
			case L'\u1039':
				return L'\u005E'; //Leftover stack letters as ^
			case L'\u103A':
				return L'\u1039';
			case L'\u103B':
				return L'\u103A';
			case L'\u103C':
				return L'\u103B';
			case L'\u103D':
				return L'\u103C';
			case L'\u103E':
				return L'\u103D';
			case L'\u103F':
				return L'\u1086';
			/*case :
				return L'\u1092';*/ //Add later; this is actually a typing issue, not a rendering one.
			case ZG_DASH:
				return L'\u200B';
			case ZG_KINZI:
				return L'\u1064';
			case ZG_STACK_KA:
				return L'\u1060';
			case ZG_STACK_KHA:
				return L'\u1061';
			case ZG_STACK_GA:
				return L'\u1062';
			case ZG_STACK_GHA:
				return L'\u1063';
			case ZG_STACK_NGA:
				return L'\u003F'; //It appears Zawgyi cannot stack "NGA"
			case ZG_STACK_SA:
				return L'\u1065';
			case ZG_STACK_SSA:
				return L'\u1066';
			case ZG_STACK_SSA_INDENT:
				return L'\u1067';
			case ZG_STACK_ZA:
				return L'\u1068';
			case ZG_STACK_ZHA:
				return L'\u1069';
			case ZG_STACK_NYA:
				return L'\u003F'; //Can't stack "NYA" either
			case ZG_STACK_TTA:
				return L'\u106C';
			case ZG_STACK_HTA1:
				return L'\u106D';
			case ZG_STACK_DHA1:
				return L'\u003F'; //Appears we can't stack this either.
			case ZG_STACK_EXTRA:
				return L'\u003F'; //Appears we can't stack this either.
			case ZG_STACK_NHA:
				return L'\u1070';
			case ZG_STACK_TA:
				return L'\u1071';
			case ZG_STACK_TA_INDENT:
				return L'\u1072';
			case ZG_STACK_HTA2:
				return L'\u1073';
			case ZG_STACK_HTA2_INDENT:
				return L'\u1074';
			case ZG_STACK_DDA:
				return L'\u1075';
			case ZG_STACK_DHA2:
				return L'\u1076';
			case ZG_STACK_NA:
				return L'\u1077';
			case ZG_STACK_PA:
				return L'\u1078';
			case ZG_STACK_PHA:
				return L'\u1079';
			case ZG_STACK_VA:
				return L'\u107A';
			case ZG_STACK_BA:
				return L'\u107B';
			case ZG_STACK_MA:
				return L'\u107C';
			case ZG_STACK_YA:
				return L'\u003F'; //Can't stack "ya"
			case ZG_STACK_LA:
				return L'\u1085';
			case ZG_STACK_THA:
				return L'\u003F'; //Can't stack "tha"
			case ZG_STACK_A:
				return L'\u003F'; //Can't stack "a"
			case ZG_COMPLEX_1:
				return L'\u1092';
			case ZG_COMPLEX_2:
				return L'\u1097';
			case ZG_COMPLEX_3:
				return L'\u106E';
			case ZG_COMPLEX_4:
				return L'\u106F';
			case ZG_COMPLEX_5:
				return L'\u1096';
			case ZG_COMPLEX_NA:
				return L'\u1091';
			case ZG_TALL_WITH_ASAT:
				return L'\u105A';
			case ZG_DOTTED_CIRCLE_ABOVE:
				return L'\u108E';
			case ZG_LEGGED_CIRCLE_BELOW:
				return L'\u108A';
			case ZG_LEGS_BOTH_WAYS:
				return L'\u1088';
			case ZG_LEGS_OF_THREE:
				return L'\u1089';
			case ZG_KINZI_102D:
				return L'\u108B';
			case ZG_KINZI_102E:
				return L'\u108C';
			case ZG_KINZI_1036:
				return L'\u108D';
			case ZG_DOT_BELOW_SHIFT_1:
				return L'\u1094';
			case ZG_DOT_BELOW_SHIFT_2:
				return L'\u1095';
			case ZG_TALL_SINGLE_LEG:
				return L'\u1033';
			case ZG_TALL_DOUBLE_LEG:
				return L'\u1034';
			case ZG_YA_PIN_CUT:
				return L'\u107D';
			case ZG_YA_PIN_SA:
				return L'\u1069';
			case ZG_YA_YIT_LONG:
				return L'\u107E';
			case ZG_YA_YIT_HIGHCUT:
				return L'\u107F';
			case ZG_YA_YIT_LONG_HIGHCUT:
				return L'\u1080';
			case ZG_YA_YIT_LOWCUT:
				return L'\u1081';
			case ZG_YA_YIT_LONG_LOWCUT:
				return L'\u1082';
			case ZG_YA_YIT_BOTHCUT:
				return L'\u1083';
			case ZG_YA_YIT_LONG_BOTHCUT:
				return L'\u1084';
			case ZG_LEG_FWD_SMALL:
				return L'\u1087';
			case ZG_NA_CUT:
				return L'\u108F';
			case ZG_YA_CUT:
				return L'\u1090';
			case ZG_NYA_CUT:
				return L'\u106B';
			case ZG_O_CUT:
				return L'\u106A';
			default:
				return uniLetter; //Assume it's correct.
		}
	}



	//And finally, locale-driven nonsense with to_lower:
	template<class T>
	class ToLower {
	public:
		 ToLower(const std::locale& loc):loc(loc)
		 {
		 }
		 T operator()(const T& src) const
		 {
			  return std::tolower<T>(src, loc);
		 }
	protected:
		 const std::locale& loc;
	};

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



//TODO: Merge readUTF8File with this, and also clean up mymbstowcs
string ReadBinaryFile(const string& path)
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
		return ""; //Empty file

	//Done, clean up resources
	string res = string(buffer, fileSize);
	delete [] buffer;

	//And return
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

	const std::wstring tab = L"   ";
	Logger::writeLogLine('Z', tab + L"norm: {" + uniString + L"}");

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
		if (!passed && currType!=BF_STACKER)
			zawgyiStr[destID++] = ZG_DASH;
		zawgyiStr[destID++] = currLetter;

		//Increment
		prevLetter = currLetter;
		prevType = currType;
	}
	zawgyiStr[destID] = 0x0000;


	Logger::writeLogLine('Z', tab + L"dash: {" + wstring(zawgyiStr, destID) + L"}");


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
			//Note: We must make an active check for letters with no stacked Zawgyi representation (0x003F)
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
					} else if (stacked==ZG_STACK_TTA && zawgyiStr[oldDestID-2]==L'\u100B') {
						destID = oldDestID-2;
						currLetter = ZG_COMPLEX_2;
					} else if (stacked==ZG_STACK_DHA1 && zawgyiStr[oldDestID-2]==L'\u100F') {
						destID = oldDestID-2;
						currLetter = ZG_COMPLEX_NA;
					} else if (zawgyiStr[destID-2]==L'\u100D' && zawgyiStr[destID-1]==L'\u1039') {
						//There are a few letters without a rendering in Zawgyi that can stack specially.
						// So, the "if" block might look different for this one.
						if (zawgyiStr[destID]==L'\u100D') {
							destID -= 2;
							currLetter = ZG_COMPLEX_3;
						} else if (zawgyiStr[destID]==L'\u100E') {
							destID -= 2;
							currLetter = ZG_COMPLEX_4;
						}
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


	Logger::writeLogLine('Z', tab + L"stck: {" + wstring(zawgyiStr, destID) + L"}");


	//Step 3: Apply a series of specific rules
	if (matchRules.empty()) {
		//Add initial rules; do this manually for now
		//1-7
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u102F', 0x7FFE00000, L"\u1009\u1025\u100A", ZG_TALL_SINGLE_LEG));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u1030', 0x7FFE00000, L"\u1009\u1025\u100A", ZG_TALL_DOUBLE_LEG));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u102F', 0x180000, L"", ZG_LEGS_BOTH_WAYS));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u1030', 0x180000, L"", ZG_LEGS_OF_THREE));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u1037', 0x1580018C000, L"\u1014", ZG_DOT_BELOW_SHIFT_1));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u1037', 0xA700E00000, L"\u101B", ZG_DOT_BELOW_SHIFT_2));
		matchRules.push_back(new Rule(RULE_MODIFY, ZG_DOT_BELOW_SHIFT_1, 0xA700E00000, L"\u101B", ZG_DOT_BELOW_SHIFT_2));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u103A', 0x8000, L"", ZG_TALL_WITH_ASAT));

		//8
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u1036', 0x800, L"", ZG_DOTTED_CIRCLE_ABOVE));

		//A new rule! Combine "stacked TA" with "circle below"
		matchRules.push_back(new Rule(RULE_COMBINE, ZG_STACK_TA, 0x400000, L"", ZG_COMPLEX_5));

		//9-14
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u1036', 0x100, L"", ZG_KINZI_1036));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u102D', 0x100, L"", ZG_KINZI_102D));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u102E', 0x100, L"", ZG_KINZI_102E));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u102E', 0, L"\u1025", L'\u1026'));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u103E', 0xFF000000, L"\u1020\u100A", ZG_LEG_FWD_SMALL));
		matchRules.push_back(new Rule(RULE_COMBINE, L'\u103E', 0x400000, L"", ZG_LEGGED_CIRCLE_BELOW));
		matchRules.push_back(new Rule(RULE_COMBINE, ZG_LEG_FWD_SMALL, 0x400000, L"", ZG_LEGGED_CIRCLE_BELOW));
		matchRules.push_back(new Rule(RULE_ORDER, L'\u1036', 0xFF000000, L"", 0x0000));

		//15-20
		matchRules.push_back(new Rule(RULE_ORDER, L'\u103C', 0x7, L"", 0x0000));
		matchRules.push_back(new Rule(RULE_ORDER, L'\u1031', 0x7, L"", 0x0000));
		matchRules.push_back(new Rule(RULE_ORDER, L'\u1031', 0xFF000000, L"", 0x0000));
		matchRules.push_back(new Rule(RULE_COMBINE, ZG_STACK_SA, 0x600000000, L"", ZG_YA_PIN_SA));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u103C', 0x4, L"", ZG_YA_YIT_LONG));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u103B', 0x100E00000, L"", ZG_YA_PIN_CUT));

		//21-30
		matchRules.push_back(new Rule(RULE_MODIFY, ZG_STACK_SSA, 0x2, L"", ZG_STACK_SSA_INDENT));
		matchRules.push_back(new Rule(RULE_MODIFY, ZG_STACK_TA, 0x2, L"", ZG_STACK_TA_INDENT));
		matchRules.push_back(new Rule(RULE_MODIFY, ZG_STACK_HTA2, 0x2, L"", ZG_STACK_HTA2_INDENT));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u1014', 0x15F00F80000, L"", ZG_NA_CUT));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u1009', 0x15800800000, L"\u103A", L'\u1025'));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u101B', 0x1800000000, L"", ZG_YA_CUT));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u100A', 0x4000600000, L"", ZG_NYA_CUT));
		matchRules.push_back(new Rule(RULE_MODIFY, L'\u1025', 0x800000, L"", ZG_O_CUT));
		matchRules.push_back(new Rule(RULE_MODIFY, ZG_DOT_BELOW_SHIFT_1, 0x2000, L"", L'\u1037'));
		matchRules.push_back(new Rule(RULE_MODIFY, ZG_DOT_BELOW_SHIFT_2, 0x2000, L"", L'\u1037'));
	}

	Logger::writeLogLine('Z', tab + L"Begin Match");


	//We maintain a series of offsets for the most recent match. This is used to speed up the process of
	// pattern matching. We only track the first occurrance, from left-to-right, of the given flag.
	//We scan from left-to-right, then apply rules from right-to-left breaking after each consonant.
	// A "minor break" occurs after each killed consonant, which is not tracked. "CONSONANT" always refers
	// to the main consonant.
	int firstOccurrence[S3_TOTAL_FLAGS];
	uint64_t currMatchFlags = 0;
	for (size_t i=0; i<S3_TOTAL_FLAGS; i++)
		firstOccurrence[i] = -1;
	length = wcslen(zawgyiStr);
	size_t prevConsonant = 0;
	//int kinziCascade = 0;
	bool switchedKinziOnce = false;
	for (size_t i=0; i<=length; i++) {
		//Get properties on this letter
		wchar_t currLetter = zawgyiStr[i];
		uint64_t currFlag = getStage3BitFlags(currLetter);
		int currFlagID = getStage3ID(currFlag);

		//Are we at a stopping point?
		//NOTE: kinzi occurs before the consonant for Unicode
		if ((!switchedKinziOnce && currLetter==ZG_KINZI) || isConsonant(currLetter) || i==length) {
			//First, scan for and fix "tall leg"s
			for (size_t x=i-1; x>=prevConsonant&&x<length; x--) { //Note: checking x<length is a very weird way of handling overflow (it works, though)
				if (zawgyiStr[x]==0x102F || zawgyiStr[x]==0x1030) {
					Rule *r = matchRules[zawgyiStr[x]-0x102F];
					bool matches = ((r->match_flags&currMatchFlags)!=0);
					if (!matches) {
						for (size_t sID=0; sID<r->match_additional.length()&&!matches; sID++) {
							for (size_t zID=prevConsonant; zID<i && !matches; zID++) {
								if (zawgyiStr[zID]==r->match_additional[sID]) {
									matches = true;
								}
							}
						}
					}

					if (matches) {
						Logger::writeLogLine('Z', tab+tab + r->toString() + L"  (pre)");

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
			wstringstream lLine;
			if ((i-1)<length)
				lLine <<L"Dealing with syllable from " <<(i-1) <<" to " <<prevConsonant;
			Logger::writeLogLine('Z', tab+tab + lLine.str());
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
					uint64_t matchRes = r->match_flags&currMatchFlags;
					int matchLoc = -1;
					if (matchRes==0) {
						bool foundAdditional = false;
						if (!r->match_additional.empty()) {
							for (size_t sID=0; sID<r->match_additional.length()&&!foundAdditional; sID++) {
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

					Logger::writeLogLine('Z', tab+tab + r->toString());

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
							if ((int)x<matchLoc)
								break; //Don't shift right
							if (r->blacklisted)
								break; //Avoid cycles

							wstringstream logLine;
							logLine <<L"shift sequence[" <<matchLoc <<".." <<x <<"] right 1, wrap around";
							Logger::writeLogLine('Z', tab+tab+tab + logLine.str());

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
								if (r->type==RULE_ORDER && (int)x<matchLoc)
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
				//111111110000
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

				std::wstringstream logline;
				logline <<L"YA at [" <<yaYitID <<L"], cut? " <<cutTop <<"T  " <<cutBottom <<"B";
				Logger::writeLogLine('Z', tab+tab + logline.str());
			}


			//Is this a soft-stop or a hard stop?
			bool softStop = (i<length && zawgyiStr[i+1]==0x103A);
			currMatchFlags &= (S3_CONSONANT_NARROW|S3_CONSONANT_OTHER|S3_CONSONANT_WIDE);
			for (size_t x=0; x<S3_TOTAL_FLAGS; x++) {
				if (softStop && (x==S3_CONSONANT_NARROW || x==S3_CONSONANT_WIDE || x==S3_CONSONANT_OTHER))
					continue;
				firstOccurrence[x] = -1;
			}
			if (!softStop) {
				//Special case: re-order "kinzi + consonant" on "kinzi"
				if (i+1<length && isConsonant(zawgyiStr[i+1]) && zawgyiStr[i]==ZG_KINZI && !switchedKinziOnce) {
				//if (i>0 && kinziCascade==0 && zawgyiStr[i-1]==ZG_KINZI && zawgyiStr[i]!=0x0000) {
					zawgyiStr[i] = zawgyiStr[i+1];
					zawgyiStr[i+1] = ZG_KINZI;
					//i--;
					switchedKinziOnce = true;
				}

				//Reset our black-list.
				for (size_t rID=0; rID<matchRules.size(); rID++)
					matchRules[rID]->blacklisted = false;

				//Reeset letter & flags
				currLetter = zawgyiStr[i];
				currFlag = getStage3BitFlags(currLetter);
				currFlagID = getStage3ID(currFlag);

				//Propagate
				if (currFlagID!=-1) {
					firstOccurrence[currFlagID] = i;
					currMatchFlags = currFlag;
				} else
					currMatchFlags = 0;

				//if (kinziCascade>0)
				//	kinziCascade--;
			}
			prevConsonant = i;
		} else {
			//Just track this letter's location
			if (currFlagID!=-1) {
				if (firstOccurrence[currFlagID]==-1)
					firstOccurrence[currFlagID] = i;
				currMatchFlags |= currFlag;
			}

			//Allow kinzi to work again for the next letter
			if (currLetter==ZG_KINZI)
				switchedKinziOnce = false;

			//Fix some segmentation problems
			/*if (currLetter==L'\u200B') {
				//ZWS will _always_ reset everything
				prevConsonant = i;
				for (size_t itr=0; itr<S3_TOTAL_FLAGS; itr++)
					firstOccurrence[itr] = -1;
				currMatchFlags = 0;
			}*/
		}
	}

	Logger::writeLogLine('Z', tab + L"End Match");
	Logger::writeLogLine('Z', tab + L"mtch: {" + wstring(zawgyiStr, length) + L"}");

	//Step 4: Convert each letter to its Zawgyi-equivalent
	//length = wcslen(zawgyiStr); //Keep embedded zeroes
	destID =0;
	for (size_t i=0; i<length; i++) {
		if (zawgyiStr[i]==0x0000 || zawgyiStr[i]==ZG_DASH)
			continue;
		zawgyiStr[destID++] = zawgyiLetter(zawgyiStr[i]);
	}
	zawgyiStr[destID++] = 0x0000;


	Logger::writeLogLine('Z', tab + L"subs: {" + wstring(zawgyiStr, destID) + L"}");
	Logger::writeLogLine('Z', tab + L"Begin Re-Ordering");


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
			const wstring& rule = reorderPairs[ruleID];
			if (zawgyiStr[i]==rule[0] && zawgyiStr[i-1]==rule[1]) {
				Logger::writeLogLine('Z', tab+tab + L"Order{" + wstring(rule) + L"}");

				zawgyiStr[i-1] = rule[0];
				zawgyiStr[i] = rule[1];
			}
		}

		//Apply stage 3 fixed rules
		if (i>1) {
			if (zawgyiStr[i-2]==0x1019 && zawgyiStr[i-1]==0x102C && zawgyiStr[i]==0x107B) {
				Logger::writeLogLine('Z', tab+tab + L"Order{L3[1]}");

				zawgyiStr[i-1]=0x107B;
				zawgyiStr[i]=0x102C;
			}
			if (zawgyiStr[i-2]==0x103A && zawgyiStr[i-1]==0x102D && zawgyiStr[i]==0x1033) {
				Logger::writeLogLine('Z', tab+tab + L"Order{L3[2]}");

				zawgyiStr[i-1]=0x1033;
				zawgyiStr[i]=0x102D;
			}
			if (zawgyiStr[i-2]==0x103C && zawgyiStr[i-1]==0x1033 && zawgyiStr[i]==0x102D) {
				Logger::writeLogLine('Z', tab+tab + L"Order{L3[3]}");

				zawgyiStr[i-1]=0x102D;
				zawgyiStr[i]=0x1033;
			}
			if (zawgyiStr[i-2]==0x103A && zawgyiStr[i-1]==0x1033 && zawgyiStr[i]==0x1036) {
				Logger::writeLogLine('Z', tab+tab + L"Order{L3[4]}");

				zawgyiStr[i-1]=0x1036;
				zawgyiStr[i]=0x1033;
			}
			if (zawgyiStr[i-2]==0x103A && zawgyiStr[i-1]==0x108B && zawgyiStr[i]==0x1033) {
				Logger::writeLogLine('Z', tab+tab + L"Order{L3[5]}");

				zawgyiStr[i-1]=0x1033;
				zawgyiStr[i]=0x108B;
			}

			//This one's a little different
			if (zawgyiStr[i]==0x1036 && zawgyiStr[i-2]==0x103C && zawgyiStr[i-1]==0x107D) {
				Logger::writeLogLine('Z', tab+tab + L"Order{L3[6]}");

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
				Logger::writeLogLine('Z', tab+tab + L"Order{L4[1]}");

				zawgyiStr[i-2]=0x107B;
				zawgyiStr[i-1]=0x102C;
				zawgyiStr[i]=0x1037;
			}
			if (zawgyiStr[i-3]==0x107E && zawgyiStr[i-1]==0x1033 && zawgyiStr[i]==0x1036) {
				Logger::writeLogLine('Z', tab+tab + L"Order{L4[2]}");

				zawgyiStr[i-1]=0x1036;
				zawgyiStr[i]=0x1033;
			}
		}
	}


	Logger::writeLogLine('Z', tab + L"End Re-Ordering");


	return wstring(zawgyiStr);
}


std::string escape_wstr(const std::wstring& str, bool errOnUnicode)
{
	std::stringstream res;
	for (wstring::const_iterator c=str.begin(); c!=str.end(); c++) {
		//if (*c < std::numeric_limits<char>::max())
		if (*c < 0x7F) //TODO: This might not be correct..
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
			res <<c1 <<c2 <<c3;
		} else
			throw std::runtime_error("Unicode value out of range.");

	}
	return res.str();
}


std::wstring mbs2wcs(const std::string& src)
{
	std::wstringstream res;
	for (size_t i=0; i<src.size(); i++) {
		unsigned short curr = (src[i]&0xFF);
		if (((curr>>3)^0x1E)==0) {
			//We can't handle anything outside the BMP
			throw std::runtime_error("Error: mbs2wcs does not handle bytes outside the BMP");
		} else if (((curr>>4)^0xE)==0) {
			//Verify the next two bytes
			if (i>=src.length()-2 || (((src[i+1]&0xFF)>>6)^0x2)!=0 || (((src[i+2]&0xFF)>>6)^0x2)!=0)
				throw std::runtime_error("Error: 2-byte character error in UTF-8 file");

			//Combine all three bytes, check, increment
			wchar_t destVal = 0x0000 | ((curr&0xF)<<12) | ((src[i+1]&0x3F)<<6) | (src[i+2]&0x3F);
			if (destVal>=0x0800 && destVal<=0xFFFF) {
				i+=2;
			} else
				throw std::runtime_error("Error: 2-byte character error in UTF-8 file");

			//Set
			res <<destVal;
		} else if (((curr>>5)^0x6)==0) {
			//Verify the next byte
			if (i>=src.length()-1 || (((src[i+1]&0xFF)>>6)^0x2)!=0)
				throw std::runtime_error("Error: 1-byte character error in UTF-8 file");

			//Combine both bytes, check, increment
			wchar_t destVal = 0x0000 | ((curr&0x1F)<<6) | (src[i+1]&0x3F);
			if (destVal>=0x80 && destVal<=0x07FF) {
				i++;
			} else
				throw std::runtime_error("Error: 1-byte character error in UTF-8 file");

			//Set
			res <<destVal;
		} else if ((curr>>7)==0) {
			wchar_t destVal = 0x0000 | curr;

			//Set
			res <<destVal;
		} else {
			throw std::runtime_error("Error: Unknown sequence in UTF-8 file");
		}
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
		if (str[i]==L'\u1004' && i+2<str.size() && str[i+1]==L'\u103A' && str[i+2]==L'\u1039') {
			//Kinzi, skip
			res <<L"\u1004\u103A\u1039";
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



//Implementations of "glue"
string glue(string str1, string str2, string str3, string str4)
{
	std::stringstream msg;
	msg <<str1 <<str2 <<str3 <<str4;
	return msg.str();
}
string glue(wstring str1, wstring str2, wstring str3, wstring str4)
{
	return glue(escape_wstr(str1),escape_wstr(str2),escape_wstr(str3),escape_wstr(str4));
}



wstring purge_filename(const wstring& str)
{
	size_t lastSlash = str.rfind(L'\\');
	if (lastSlash!=wstring::npos && lastSlash!=str.size()-1)
		return str.substr(lastSlash+1, wstring::npos);
	return str;
}


wstring sanitize_id(const wstring& str)
{
	std::wstring res = str; //Copy out the "const"-ness.
	//res = std::wstring(res.begin(), std::remove_if(res.begin(), res.end(), is_id_delim<wchar_t>()));
	auto is_id_delim = [](wchar_t letter)->bool {
		  if ((letter==' ')||(letter=='\t')||(letter=='\n')||(letter=='-')||(letter=='_'))
		   return true; //Remove it
		  return false; //Don't remove it
	};
	res = std::wstring(res.begin(), std::remove_if(res.begin(), res.end(), is_id_delim));
	loc_to_lower(res); //Operates in-place.
	return res;
}



bool read_bool(const std::wstring& str)
{
	std::wstring test = str;
	loc_to_lower(test);
	if (test == L"yes" || test==L"true")
		return true;
	else if (test==L"no" || test==L"false")
		return false;
	else
		throw std::runtime_error(glue(std::wstring(L"Bad boolean value: \""), str, std::wstring(L"\"")).c_str());
}


int read_int(const std::wstring& str)
{
	//Read
	int resInt;
	std::wistringstream reader(str);
	reader >>resInt;

	//Problem?
	if (reader.fail()) {
		//TEMP
		throw std::runtime_error(glue(std::wstring(L"Bad integer value: \""), str, L"\"").c_str());
	}

	//Done
	return resInt;
}



//Locale-aware "toLower" converter
void loc_to_lower(wstring& str)
{
	std::locale loc(""); //Get native locale
	std::transform(str.begin(),str.end(),str.begin(),ToLower<wchar_t>(loc));
}



//Tokenize on a character
//Inelegant, but it does what I want it to.
vector<wstring> separate(wstring str, wchar_t delim)
{
	vector<wstring> tokens;
	std::wstringstream curr;
	for (size_t i=0; i<str.length(); i++) {
		if (str[i]!=delim)
			curr << str[i];
		if (str[i]==delim || i==str.length()-1) {
			tokens.push_back(curr.str());
			curr.str(L"");
		}
	}

	return tokens;
}


//Take an educated guess as to whether or not this is a file.
bool IsProbablyFile(const std::wstring& str)
{
	//First, get the right-most "."
	size_t lastDot = str.rfind(L'.');
	if (lastDot==wstring::npos)
		return false;

	//It's a file if there are between 1 and 4 characters after this dot
	int diff = str.size() - 1 - (int)lastDot;
	return (diff>=1 && diff<=4);
}


//Remove leading and trailing whitespace
wstring sanitize_value(const wstring& str, const std::wstring& filePath)
{
	//First, remove spurious spaces/tabs/newlines
	size_t firstLetter = str.find_first_not_of(L" \t\n");
	size_t lastLetter = str.find_last_not_of(L" \t\n");
	if (firstLetter==wstring::npos||lastLetter==wstring::npos)
		return L"";

	//Next, try to guess if this represents a file
	wstring res = str.substr(firstLetter, lastLetter-firstLetter+1);
	if (IsProbablyFile(res)) {
		//Replace all "/" with "\\"
		std::replace(res.begin(), res.end(), L'/', L'\\');

		//Ensure it references no sub-directories whatsoever
		if (res.find(L'\\')!=wstring::npos)
			throw std::runtime_error("Config files cannot reference files outside their own directories.");

		//Append the directory (and a "\\" if needed)
		res = filePath + (filePath[filePath.size()-1]==L'\\'?L"":L"\\") + res;
	}
	return res;
}


INPUT_TYPE read_input_type(const wstring& str)
{
	wstring key = sanitize_id(str);
	if (key==L"builtin")
		return INPUT_TYPE::BUILTIN;
	else if (key==L"keymagic")
		return INPUT_TYPE::KEYBOARD;
	else if (key==L"roman")
		return INPUT_TYPE::ROMAN;
	throw std::runtime_error(waitzar::glue(L"Unknown \"type\": ", str).c_str());
}

DISPLAY_TYPE read_display_type(const wstring& str)
{
	wstring key = sanitize_id(str);
	if (key==L"builtin")
		return DISPLAY_TYPE::BUILTIN;
	else if (key==L"png")
		return DISPLAY_TYPE::PNG;
	else if (key==L"ttf")
		return DISPLAY_TYPE::TTF;
	throw std::runtime_error(waitzar::glue(L"Unknown \"type\": ", str).c_str());
}


TRANSFORM_TYPE read_transform_type(const wstring& str)
{
	wstring key = sanitize_id(str);
	if (key==L"builtin")
		return TRANSFORM_TYPE::BUILTIN;
	else if (key==L"javascript")
		return TRANSFORM_TYPE::JAVASCRIPT;
	throw std::runtime_error(waitzar::glue(L"Unknown \"type\": ", str).c_str());
}


CONTROL_KEY_TYPE read_control_key_style(const wstring& str)
{
	wstring key = sanitize_id(str);
	if (key==L"chinese")
		return CONTROL_KEY_TYPE::CHINESE;
	else if (key==L"japanese")
		return CONTROL_KEY_TYPE::JAPANESE;
	throw std::runtime_error(waitzar::glue(L"Unknown \"control-key-style\": ", str).c_str());
}


string GetMD5Hash(const std::string& fileName) {
	//Some variables
	const size_t digest_size = 16;
	md5_state_t state;
	md5_byte_t digest[digest_size];

	//Get the file's binary data
	string data = waitzar::ReadBinaryFile(fileName);

	//Run the algorithm on the data
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)data.c_str(), data.size());
	md5_finish(&state, digest);

	std::stringstream md5Res;
	md5Res <<std::hex;
	for (size_t i=0; i<digest_size; i++) {
		md5Res <<((digest[i]<0x10)?"0":"") <<(unsigned int)(digest[i]);
	}
	return md5Res.str();
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
