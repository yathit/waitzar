/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _TRANSFORM_AYAR2UNI
#define _TRANSFORM_AYAR2UNI

#include <sstream>
#include "Transform/Transformation.h"

/**
 * Enable the "Ayar" encoding
 */
class Ayar2Uni : public Transformation
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
		for (size_t i=0; i<src.length(); i++) {
			//The next syllable starts at the first non-stacked non-killed consonant, or at tha-way-htoe or ya-yit
			if (src[i]<L'\u1000' || src[i]>L'\u109F') {  //TODO: We need to extract this out to "IsMyanmar()" to include Unicode 5.2 letters.
				//Append all non-Myanmar letters and continue
				res <<currSyllablePrefix.str() <<currSyllable.str();
				currSyllablePrefix.str(L"");
				currSyllable.str(L"");
				while (i<src.length() && (src[i]<L'\u1000' || src[i]>L'\u109F')) {
					res <<src[i++];
				}
				i--;
				continue;
			}

			//Are we at the boundary of a new word?
			bool boundary = false;
			if (src[i]==L'\u1031' || src[i]==L'\u103C')
				boundary = true;
			else if (IsConsonant(src[i])) {
				boundary = true;
				if (src[i]==L'\u1004' && i+2<src.length() && src[i+1]==L'\u103A' && src[i+2]==L'\u1039')
					boundary = false; //Kinzi
				if (i>0 && src[i-1]==L'\u1039')
					boundary = false; //Stacked
				else if (i+1<src.length() && src[i+1]==L'\u103A')
					boundary = false; //Killed
				else if (i>0 && (src[i-1]==L'\u1031' || src[i-1]==L'\u103C'))
					boundary = false; //The word has actually already started.
			}
			if (boundary) {
				res <<currSyllablePrefix.str() <<currSyllable.str();
				currSyllablePrefix.str(L"");
				currSyllable.str(L"");
			}

			//Now, collect as usual.
			if (src[i]==L'\u1004' && i+2<src.length() && src[i+1]==L'\u103A' && src[i+2]==L'\u1039') {
				res <<L"\u1004\u103A\u1039";
				i+=2;
			} else if (IsConsonant(src[i]) && currSyllablePrefix.str().empty()) {
				//Only the first consonant is the prefix.
				currSyllablePrefix <<src[i];
			} else {
				//Enforce ordering of 103C 1031 in Unicode
				if (i+1<src.length() && src[i]==L'\u1031' && src[i+1]==L'\u103C') {
					currSyllable <<L"\u103C\u1031";
					i+=1;
				} else
					currSyllable <<src[i];
			}

			if (i==src.length()-1) {
				res <<currSyllablePrefix.str() <<currSyllable.str();
				currSyllablePrefix.str(L"");
				currSyllable.str(L"");
			}
		}

		src = res.str();
	}

};


#endif //_TRANSFORM_AYAR2UNI

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

