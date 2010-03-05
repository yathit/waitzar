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
json_spirit::wmObject BurglishBuilder::onsetPairs;
json_spirit::wmObject BurglishBuilder::rhymePairs;
json_spirit::wmObject BurglishBuilder::specialWords;
std::vector<std::wstring> BurglishBuilder::savedDigitIDs;
void BurglishBuilder::InitStatic()
{
	//Create, read, save our onsets and rhymes.
	json_spirit::wmValue onsetRoot;
	json_spirit::wmValue rhymeRoot;
	json_spirit::wmValue specialRoot;

	if (!(json_spirit::read(BURGLISH_ONSETS, onsetRoot) && 	json_spirit::read(BURGLISH_RHYMES, rhymeRoot) && json_spirit::read(BURGLISH_SPECIALS, specialRoot)))
		throw std::exception("BURGLISH_ONSETS, BURGLISH_RHYMES, or BURGLISH_SPECIALS contains parse errors --this should never happen in release mode.");

	onsetPairs = onsetRoot.get_value<json_spirit::wmObject>();
	rhymePairs = rhymeRoot.get_value<json_spirit::wmObject>();
	specialWords = specialRoot.get_value<json_spirit::wmObject>();

	//Saved digits; these are never cleared.
	savedDigitIDs.push_back(L"\u1040");
	savedDigitIDs.push_back(L"\u1041");
	savedDigitIDs.push_back(L"\u1042");
	savedDigitIDs.push_back(L"\u1043");
	savedDigitIDs.push_back(L"\u1044");
	savedDigitIDs.push_back(L"\u1045");
	savedDigitIDs.push_back(L"\u1046");
	savedDigitIDs.push_back(L"\u1047");
	savedDigitIDs.push_back(L"\u1048");
	savedDigitIDs.push_back(L"\u1049");
}


bool BurglishBuilder::IsVowel(wchar_t letter)
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


bool BurglishBuilder::IsValid(const wstring& word)
{
	//Check for invalid pairwise combinations
	for (size_t i=0; i<word.size()-1; i++) {
		//We build the regex into a switch statement, handling this manually.
		switch (word[i]) {
			case L'\u100D':  case L'\u100B':  case L'\u100C':  case L'\u1023':
				switch (word[i+1]) {
					case L'\u1087':  case L'\u103D':  case L'\u102F':  case L'\u1030':  
					case L'\u1088':  case L'\u1089':  case L'\u103C':  case L'\u108A':
						return false;
				}
				break;
			case L'\u1020':
				switch (word[i+1]) {
					case L'\u103C':  case L'\u108A':
						return false;
				}
				break;
			case L'\u1060':  case L'\u1061':  case L'\u1062':  case L'\u1063':  case L'\u1064':  case L'\u1065':
			case L'\u1066':  case L'\u1067':  case L'\u1068':  case L'\u1069':  case L'\u106A':  case L'\u106B': 
			case L'\u106C':  case L'\u106D':  case L'\u106E':  case L'\u106F':  case L'\u1070':  case L'\u1071':
			case L'\u1072':  case L'\u1073':  case L'\u1074':  case L'\u1075':  case L'\u1076':  case L'\u1077':
			case L'\u1078':  case L'\u1079':  case L'\u107A':  case L'\u107B':  case L'\u107C':  case L'\u1092':
				switch (word[i+1]) {
					case L'\u1087':  case L'\u103D':  case L'\u1088':  case L'\u1089':  
					case L'\u103C':  case L'\u108A':
						return false;
				}
				break;
		}
	}

	//Otherwise, it passed
	return true;
}


void BurglishBuilder::addStandardWords(wstring roman, set<wstring>& resultsList, bool firstLetterUppercase)
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
	wstring prefix;
	wstring suffix;
	for (size_t i=1; i<=roman.size(); i++) {
		//Done?
		wstring candPrefix = roman.substr(0, i);
		if (onsetPairs.count(candPrefix)==0)
			break;
		
		//Update (suffix is "a" if prefix takes up the whole string)
		prefix = candPrefix;
		suffix = (i==roman.size() ? L"a" : roman.substr(i, roman.size()-i));
	}

	//Step 3: Null pair? If not, pull their values
	if (onsetPairs.count(prefix)==0 || rhymePairs.count(suffix)==0)
		return;
	wstring prefixStr = onsetPairs[prefix].get_value<wstring>();
	wstring suffixStr = rhymePairs[suffix].get_value<wstring>();

	//Step 4: For each prefix, for each suffix, get the combined word.
	// Prefixes & suffixes are just strings separated by pipe marks: ".....|.....|...."
	// Suffixes, in addition, have a "-" to show where the inserted prefix should go.
	wstringstream onset;
	for (size_t onsID=0; onsID<prefixStr.size(); onsID++) {
		//Append?
		if (prefixStr[onsID]!=L'|')
			onset <<prefixStr[onsID];
		//Skip?
		if (prefixStr[onsID]!=L'|' && onsID<prefixStr.size()-1)
			continue;

		//Replace the current prefix with a pat-sint equivalent if the first letter is capital
		if (firstLetterUppercase) {
			wstring oldPrefix = onset.str();
			if (oldPrefix.length()==1) {
				wchar_t c = oldPrefix[0];
				if (  (c>=L'\u1000' && c<=L'\u1008')
					||(c==L'\u100B' || c==L'\u100C' || c==L'\u101C')
					||(c>=L'\u100F' && c<=L'\u1009')) 
				{
					//Stack it
					onset.str(L"");
					onset <<L'\u1039' <<oldPrefix;
				}
			}
		}


		//Ok, we have our onset.
		wstringstream rhyme;
		for (size_t rhymeID=0; rhymeID<suffixStr.size(); rhymeID++) {
			//Append?
			if (suffixStr[rhymeID]==L'-')
				rhyme <<onset.str(); //Insert our onset
			else if (suffixStr[rhymeID]!=L'|')
				rhyme <<suffixStr[rhymeID];
			//Skip?
			if (suffixStr[rhymeID]!=L'|' && rhymeID<suffixStr.size()-1)
				continue;

			//We've built our onset + rhyme into rhyme directly. 
			//Now, we just need to test for errors, etc.
			wstring word = rhyme.str();
			if (IsValid(word))
				resultsList.insert(word);

			//Finally: reset
			rhyme.str(L"");
		}
		//Finally: reset
		onset.str(L"");
	}
}


void BurglishBuilder::addSpecialWords(std::wstring roman, std::set<std::wstring>& resultsList)
{
	if (specialWords.count(roman)>0) {
		wstring specialStrs = specialWords[roman].get_value<wstring>();
		wstringstream entry;
		for (size_t specID=0; specID<specialStrs.size(); specID++) {
			//Append?
			if (specialStrs[specID]!=L'|')
				entry <<specialStrs[specID];
			//Skip?
			if (specialStrs[specID]!=L'|' && specID<specialStrs.size()-1)
				continue;

			//Add it (no need to check validity)
			resultsList.insert(entry.str());

			//Finally: reset
			entry.str(L"");
		}
	}
}


void BurglishBuilder::expandCurrentWords(std::set<std::wstring>& resultsList)
{
	//Just a few simple substitutions
	//TODO: Double-check this after all words have been added.
	wstringstream newWord;
	for (std::set<std::wstring>::const_iterator word=resultsList.begin(); word!=resultsList.end(); word++) {
		//Try to build up a new match
		newWord.str(L"");
		for (size_t i=0; i<word->size(); i++) {
			//Build the next bit
			bool consumed = false;
			if ((*word)[i]==L'\u1021') {
				//First pattern:
				// \u1021 \u102F --> \u1025
				if (i<word->size()-1 && (*word)[i+1]==L'\u102F') {
					newWord <<L'\u1025';
					i++;
					consumed = true;
				}

				//Second pattern:
				// \u1021 \u102D [^\u102F]! --> \u1023 (don't replace !-expression)
				if (i<word->size()-2 && (*word)[i+1]==L'\u102D' && (*word)[i+2]!=L'\u102F') {
					newWord <<L'\u1023';
					i+=2; //Don't consume.
				}

				//Third pattern:
				// \u1021 \u1031 \u102C [^\u103A \u1037]! --> \u1029 (don't replace !-expression)
				if (i<word->size()-3 && (*word)[i+1]==L'\u1031' && (*word)[i+2]==L'\u102C' && (*word)[i+3]!=L'\u103A' && (*word)[i+3]!=L'\u1037') {
					newWord <<L'\u1029';
					i+=3; //Don't consume.
				}
			}

			//Append
			if (!consumed)
				newWord <<(*word)[i];
		}

		//Is this word worth adding?
		if (newWord.str() != *word && IsValid(newWord.str()))
			resultsList.insert(newWord.str());
	}
}




//The core of the application: how do we select which word we want?
void BurglishBuilder::reGenerateWordlist()
{
	//Reset
	set<wstring> results;

	//For now, just do the general combinations
	addStandardWords(typedRomanStr.str(), results, typeBeginsWithUpper);
	addSpecialWords(typedRomanStr.str(), results);
	//TODO: Numbers, etc.

	//Expand the set of words with common substitutions
	expandCurrentWords(results);

	//Copy into our vector
	generatedWords.clear();
	generatedWords.insert(generatedWords.begin(), results.begin(), results.end());
}


//Reset our lookup
void BurglishBuilder::reset(bool fullReset)
{
	typedRomanStr.str(L"");
	generatedWords.clear();

	//More...
	currSelectedPage = 0;
	currSelectedID = 0;
	typeBeginsWithUpper = false;

	if (fullReset)
		savedWordIDs.clear();
}


//Add a new letter
bool BurglishBuilder::typeLetter(char letter, bool isUpper)
{
	//Save
	wstring oldRoman = typedRomanStr.str();
	bool oldIsUpper = typeBeginsWithUpper;

	//Typing pat-sint?
	if (isUpper && typedRomanStr.str().length()==0)
		typeBeginsWithUpper = true;

	//Attempt
	typedRomanStr <<letter;
	reGenerateWordlist();

	//Rollback?
	if (generatedWords.empty()) {
		typedRomanStr.str(L"");
		typedRomanStr <<oldRoman;
		typeBeginsWithUpper = oldIsUpper;
		reGenerateWordlist();
		return false;
	}

	//Success
	return true;
}


//Get all possible words. Requires IDs, though.
//IDs are numbered starting after those in savedWordIDs() and savedDigitIDs()
//Note that a word in savedWordIDs and generatedWords might have 1..N 
//   possible IDs. This isn't really a problem, as the words only build up as sentences are typed.
std::vector<unsigned int> BurglishBuilder::getPossibleWords() const
{
	//TEMP: For now, the word's ID is just its index. (We might need to hack around this for 0..9)
	vector<unsigned int> res;
	while (res.size()<generatedWords.size())
		res.push_back(savedDigitIDs.size() + savedWordIDs.size() + res.size());
	return res;
}


//Simple.
//NOTE: An id is in the savedDigitIDs/savedWordIDs arrays, or it is after those, in the list of candidates.
std::wstring BurglishBuilder::getWordString(unsigned int id) const
{
	if (id<savedDigitIDs.size())
		return savedDigitIDs[id];
	id -= savedDigitIDs.size();
	if (id<savedWordIDs.size())
		return savedWordIDs[id];
	id -= savedWordIDs.size();

	return generatedWords[id];
}


//TODO: This is going to be a huge pain in Burglish, esp. considering multiple words.
//      For now, we return nothing.
pair<int, string> BurglishBuilder::reverseLookupWord(std::wstring word)
{
	return pair<int, string>(-1, "");
}


//Just cut a letter off the string and update our list.
bool BurglishBuilder::backspace()
{
	if (typedRomanStr.str().empty())
		return false;

	wstring newStr = typedRomanStr.str().substr(0, typedRomanStr.str().size()-1);
	typedRomanStr.str(L"");
	typedRomanStr <<newStr;

	reGenerateWordlist();
	return true;
}


//Simple, copied.
bool BurglishBuilder::moveRight(int amt)
{
	//Any words?
	if (generatedWords.size()==0)
		return false;

	//Any change?
	int newAmt = currSelectedID + amt;
	if (newAmt >= (int)generatedWords.size())
		newAmt = (int)generatedWords.size()-1;
	else if (newAmt < 0)
		newAmt = 0;
	if (newAmt == currSelectedID)
		return false;

	//Do it!
	currSelectedID = newAmt;

	//Auto-page
	currSelectedPage = currSelectedID / 10;

	return true;
}



//Simple, copied.
bool BurglishBuilder::pageUp(bool up)
{
	//Check
	int newID = currSelectedPage + (up?-1:1);
	if (newID<0 || newID>=getNumberOfPages())
		return false;

	//Page
	currSelectedPage = newID;

	//Select first word on that page
	currSelectedID = currSelectedPage * 10;

	return true;
}



//Simple, copied.
std::pair<bool, unsigned int> BurglishBuilder::typeSpace(int quickJumpID, bool useQuickJump)
{
	//We're at a valid stopping point?
	if (generatedWords.size() == 0)
		return pair<bool, unsigned int>(false, 0);

	//Quick jump?
	if (useQuickJump && quickJumpID!=-1)
		setCurrSelected(quickJumpID);

	//Get the selected word, add it to the prefix array
	//NOTE: We save the IDs of previously-typed words.
	unsigned int newWord = savedDigitIDs.size() + savedWordIDs.size();
	unsigned int adjustedID = savedDigitIDs.size() + savedWordIDs.size() + currSelectedID;
	savedWordIDs.push_back(getWordString(adjustedID));

	//Reset the model, return this word
	this->reset(false);
	return pair<bool, unsigned int>(true, newWord);
}


//Simple, copied.
int BurglishBuilder::getCurrPage() const
{
	return currSelectedPage;
}


//Simple, copied.
int BurglishBuilder::getCurrSelectedID() const
{
	return currSelectedID;
}


//Simple, copied.
int BurglishBuilder::getNumberOfPages() const
{
	int numWords = generatedWords.size();
	if (numWords % 10 == 0)
		return numWords / 10;
	else
		return numWords / 10 + 1;
}


//Simple, copied
void BurglishBuilder::setCurrSelected(int id)
{
	//Any words?
	if (generatedWords.size()==0)
		return;

	//Fail silently if this isn't a valid id
	if (id >= (int)generatedWords.size())
		return;
	else if (id < 0)
		return;

	//Do it!
	currSelectedID = id;
}




//Simple, copied.
unsigned short BurglishBuilder::getStopCharacter(bool isFull) const
{
	//TEMP -- put somewhere global later.
	unsigned short punctHalfStopUni = 0x104A;
	unsigned short punctFullStopUni = 0x104B;

	if (isFull)
		return punctFullStopUni;
	else
		return punctHalfStopUni;
}


//Convert 0..9 to an ID.
unsigned short BurglishBuilder::getSingleDigitID(unsigned short arabicNumeral)
{
	//Return the word. (Returning 0 is a somewhat unsightly alternative, but at least it won't crash.)
	if (arabicNumeral>=0 && arabicNumeral<=9)
		return arabicNumeral; //"Always on"
	return 0;
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
