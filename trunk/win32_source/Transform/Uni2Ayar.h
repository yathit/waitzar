/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _TRANSFORM_UNI2AYAR
#define _TRANSFORM_UNI2AYAR

#include <sstream>
#include "Transform/Transformation.h"

/**
 * Enable the "Ayar" encoding
 */
class Uni2Ayar : public Transformation
{
private:
	bool IsConsonant(wchar_t letter) const {
		if (letter>=L'\u1000' && letter<=L'\u102A')
			return true;
		if (letter==L'\u103F')
			return true;
		if (letter>=L'\u1040' && letter<=L'\u1049')
			return true;
		if (letter==L'\u104E')
			return true;
		return false;
	}

public:
	//Convert
	void convertInPlace(std::wstring& src) const {
		//Temporary algorithm: just split and move kinzi + tha-way-htoe + ya-yit
		std::wstringstream res;
		std::wstringstream currSyllable;
		std::wstringstream currSyllablePrefix;
		size_t u1031count = 0;
		size_t u103Ccount = 0;
		for (size_t i=0; i<src.length(); i++) {
			//The next syllable starts at the first non-stacked non-killed consonant, or at a non-myanmar letter
			if (src[i]<L'\u1000' || src[i]>L'\u109F') {   //TODO: Replace with an "IsMyanmar()" function, to catch extra characters in Unicode 5.2.
				//Append all non-Myanmar letters and continue
				res <<std::wstring(u1031count, L'\u1031') <<std::wstring(u103Ccount, L'\u103C');
				res <<currSyllablePrefix.str() <<currSyllable.str();
				currSyllablePrefix.str(L"");
				currSyllable.str(L"");
				while (i<src.length() && (src[i]<L'\u1000' || src[i]>L'\u109F')) {
					res <<src[i++];
				}
				i--;
				u1031count = u103Ccount = 0;
				continue;
			}

			//Are we at the boundary of a new word?
			bool boundary = false;
			if (src[i]==L'\u1004' && i+2<src.length() && src[i+1]==L'\u103A' && src[i+2]==L'\u1039')
				boundary = true;
			else if (IsConsonant(src[i])) {
				boundary = true;
				if (i>0 && src[i-1]==L'\u1039')
					boundary = false; //Stacked
				else if (i+1<src.length() && src[i+1]==L'\u103A')
					boundary = false; //Killed
			}
			if (boundary) {
				res <<std::wstring(u1031count, L'\u1031') <<std::wstring(u103Ccount, L'\u103C');
				res <<currSyllablePrefix.str() <<currSyllable.str();
				currSyllablePrefix.str(L"");
				currSyllable.str(L"");
				u1031count = u103Ccount = 0;
			}

			//Now, collect as usual.
			if (src[i]==L'\u1004' && i+2<src.length() && src[i+1]==L'\u103A' && src[i+2]==L'\u1039') {
				currSyllable <<L"\u1004\u103A\u1039";
				i+=2;
			} else if (IsConsonant(src[i]) && currSyllablePrefix.str().empty()) //Only the first consonant is the prefix.
				currSyllablePrefix <<src[i];
			else if (src[i]==L'\u1031' || src[i]==L'\u103C') {
				//Save for later
				if (src[i]==L'\u1031')
					u1031count++;
				else if (src[i]==L'\u103C')
					u103Ccount++;

				/*if (i+1<src.length() && src[i]==L'\u103C' && src[i+1]==L'\u1031') {
					//If BOTH prefix letters are present, enforce Ayar's ordering
					res <<L"\u1031\u103C";
					i+=1;
				} else
					res <<src[i];*/
			} else
				currSyllable <<src[i];

			if (i==src.length()-1) {
				res <<std::wstring(u1031count, L'\u1031') <<std::wstring(u103Ccount, L'\u103C');
				res <<currSyllablePrefix.str() <<currSyllable.str();
				currSyllablePrefix.str(L"");
				currSyllable.str(L"");
				u1031count = u103Ccount = 0;
			}
		}

		src = res.str();
	}
};


#endif //_TRANSFORM_UNI2AYAR

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

