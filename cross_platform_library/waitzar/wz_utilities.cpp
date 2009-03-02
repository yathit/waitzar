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
	//Useful local constants
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
	//uniString[i+1]==0x103A catches "vowell_a" followed by "asat". This might be hackish; not sure.
	bool atStoppingPoint(wchar_t* uniString, size_t id, size_t length)
	{
		return (id+1<length && uniString[id+1]==0x103A) || uniString[id]==0x103A || uniString[id]==0x1039;
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
	for (size_t i=0; i<len;) {
		//Does this letter restart our algorithm?
		if (shouldRestartCount(uniString[i]) || atStoppingPoint(uniString, i, len) || getRhymeID(uniString[i]==-1)) {
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

	//Done
	uniString[destI] = 0x0000;
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
