/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "BurglishBuilder.h"


//Import needed stl
using std::string;
using std::wstringstream;
using std::wstring;
using std::vector;
using std::pair;
using std::map;
using std::set;


namespace waitzar 
{


/**
 * Empty constructor. 
 */
BurglishBuilder::BurglishBuilder() {}
BurglishBuilder::~BurglishBuilder() {}

//Static initializer:
json_spirit::wValue BurglishBuilder::onsetRoot;
json_spirit::wValue BurglishBuilder::rhymeRoot;
void BurglishBuilder::InitStatic()
{
	json_spirit::read_or_throw(BURGLISH_ONSETS, onsetRoot);
	json_spirit::read_or_throw(BURGLISH_RHYMES, rhymeRoot);
}


bool BurglishBuilder::IsVowel(wchar_t letter) const
{
	switch (letter) {
		case L'a':
		case L'e':
		case L'i':
		case L'o':
		case L'u':
			return true;
	}
	return false;
}


bool BurglishBuilder::IsValid(const wstring& word) const
{
	//Check for invalid pairwise combinations
	for (int i=0; i<word.size()-1; i++) {
		//We build the regex into a switch statement, handling this manually.
		switch (word[i]) {
			case L'\u100D':  case L'\u100B':  case L'\u100C':  case L'\u1023':
				switch (word[i+1]) {
					case L'\u1087':  L'\u103D': L'\u102F': L'\u1030': L'\u1088': L'\u1089': L'\u103C': L'\u108A':
						return false;
				}
				break;
			case L'\u1020':
				switch (word[i+1]) {
					case L'\u103C': L'\u108A':
						return false;
				}
				break;
			case L'\u1060':  case L'\u1061':  case L'\u1062':  case L'\u1063':  case L'\u1064':  case L'\u1065':
			case L'\u1066':  case L'\u1067':  case L'\u1068':  case L'\u1069':  case L'\u106A':  case L'\u106B': 
			case L'\u106C':  case L'\u106D':  case L'\u106E':  case L'\u106F':  case L'\u1070':  case L'\u1071':
			case L'\u1072':  case L'\u1073':  case L'\u1074':  case L'\u1075':  case L'\u1076':  case L'\u1077':
			case L'\u1078':  case L'\u1079':  case L'\u107A':  case L'\u107B':  case L'\u107C':  case L'\u1092':
				switch (word[i+1]) {
					case L'\u1087': L'\u103D': L'\u1088': L'\u1089': L'\u103C': L'\u108A':
						return false;
				}
				break;
		}
	}

	//Otherwise, it passed
	return true;
}


void BurglishBuilder::addStandardWords(wstring roman, set<wstring>& resultsList)
{
	//Nothing to do?
	if (roman.empty())
		return;

	///
	//TODO: We need to handle upper-case letters! This is how Burglish does pat-sint words.
	///

	//Step 1: Duplicate vowels at the beginning of the word
	if (IsVowel(roman[0]))
		roman = wstring(1, roman[0]) + roman;

	//Step 2: Find the longest matching prefix.
	wmObject onsPairs = onsetRoot.get_value<wmObject>();
	wmObject rhymePairs = rhymeRoot.get_value<wmObject>();
	wstring prefix;
	wstring suffix;
	for (int i=1; i<=roman.size(); i++) {
		//Done?
		wstring candPrefix = roman.substr(0, i);
		if (onsPairs.count(candPrefix)==0)
			break;
		
		//Update (suffix is "a" if prefix takes up the whole string)
		prefix = candPrefix;
		suffix = (i==roman.size() ? L"a" : roman.substr(i, roman.size()-i));
	}

	//Step 3: Null pair?
	if (onsPairs.count(prefix)==0 || rhymePairs.count(suffix)==0)
		return;

	//Step 4: For each prefix, for each suffix, get the combined word.
	// Prefixes & suffixes are just strings separated by pipe marks: ".....|.....|...."
	// Suffixes, in addition, have a "-" to show where the inserted prefix should go.
	wstringstream onset;
	for (int onsID=0; onsID<prefix.size(); onsID++) {
		//Append?
		if (prefix[onsID]!=L'|')
			onset <<prefix[onsID];
		//Skip?
		if (prefix[onsID]!=L'|' && onsID<prefix.size()-1)
			continue;

		//TODO: Somewhere in here, handle capital letters.

		//Ok, we have our onset.
		wstringstream rhyme;
		for (int rhymeID=0; rhymeID<suffix.size(); rhymeID++) {
			//Append?
			if (suffix[rhymeID]==L'-')
				rhyme <<onset.str(); //Insert our onset
			else if (suffix[rhymeID]!=L'|')
				rhyme <<suffix[rhymeID];
			//Skip?
			if (suffix[rhymeID]!=L'|' && rhymeID<suffix.size()-1)
				continue;

			//We've built our onset + rhyme into rhyme directly. 
			//Now, we just need to test for errors, etc.
			wstring word = rhyme.str();
			if (IsValid(word))
				resultsList[word];

			//Finally: reset
			rhyme.str(L"");
		}
		//Finally: reset
		onset.str(L"");
	}
}



//The core of the application: how do we select which word we want?
void BurglishBuilder::reGenerateWordlist()
{
	//Reset
	set<wstring> results;

	//For now, just do the general combinations
	addStandardWords(typedRomanStr, results);


	//Copy into our vector
	generatedWords.clear();
	generatedWords.insert(..........results....);
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
