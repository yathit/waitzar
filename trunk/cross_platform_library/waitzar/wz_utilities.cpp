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




	wchar_t zawgyiLetter(wchar_t uniLetter)
	{
		switch (uniLetter)
		{
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
			case ZG_DASH:
				return 0x002D;
			case ZG_KINZI:
				return 0x1064;
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
	//We use a bit array (kind of) to avoid duplicates
	wchar_t rhyme_flags[ID_TOTAL];
	for (int res=0; res<ID_TOTAL; res++)
		rhyme_flags[res] = 0x0000;

	//Scan each letter
	size_t len = wcslen(uniString); 
	size_t destI = 0;    //Allows us to eliminate duplicate letters
	size_t prevStop = 0; //What was our last-processed letter
	for (size_t i=0; i<=len;) { //We count up to the trailing 0x0
		//Does this letter restart our algorithm?
		if (atStoppingPoint(uniString, i, len) || shouldRestartCount(uniString[i]) || getRhymeID(uniString[i])==-1) {
			//Now that we've counted, sort
			if (i!=prevStop) {
				for (int x=0; x<ID_TOTAL; x++) {
					//Add and restart
					if (rhyme_flags[x]!=0x0000)
						uniString[destI++] = rhyme_flags[x];
					rhyme_flags[x] = 0x0000;
				}
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
		rhyme_flags[getRhymeID(uniString[i])] = uniString[i];

		//Standard increment
		i++;
	}

	//Done. Append an extra zero, in case our new string is too short.
	uniString[destI] = 0x0000;
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
		if (!passed)
			zawgyiStr[destID++] = ZG_DASH;
		zawgyiStr[destID++] = currLetter;

		//Increment
		prevLetter = currLetter;
		prevType = currType;
	}
	zawgyiStr[destID] = 0x0000;

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
