/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "LetterInputMethod.h"

using std::vector;
using std::pair;
using std::string;
using std::wstring;



LetterInputMethod::LetterInputMethod()
{
}

LetterInputMethod::~LetterInputMethod()
{
}




void LetterInputMethod::handleEsc()
{
	if (this->isHelpInput()) {
		//Flag for removal.
		this->providingHelpFor = NULL;
	} else {
		//Cancle the current sentence if not in help mode
		typedSentenceStr.str(L"");
	}
}



void LetterInputMethod::handleBackspace(VirtKey& vkey)
{
	//Get the string, make a "new" version.
	wstring oldString = this->isHelpInput() ? typedCandidateStr.str() : typedSentenceStr.str();
	wstring newStr;

	//Special cases: backspace on a stacked letter deletes the stacker, and deleting on Kinzi deletes Kinzi
	size_t len = oldString.length();

	//Case 1: kinzi only
	if (oldString==L"\u1004\u103A\u1039")
		newStr = L"";

	//Case 2: kinzi with a stacked letter after
	else if (len>=4 && oldString[len-4]==L'\u1004' && oldString[len-3]==L'\u103A' && oldString[len-2]==L'\u1039' && canStack(oldString[len-1], 0x0000))
		newStr = oldString.substr(0, len-4) + wstring(1, oldString[len-1]);

	//Case 3: stacker with stacked letter after (stackers by themselves will be eliminated by the default
	//        backspace beavior
	else if (len>=2 && oldString[len-2]==L'\u1039' && canStack(oldString[len-1], 0x0000))
		newStr = oldString.substr(0, len-2) + wstring(1, oldString[len-1]);

	//Default behavior: cut off the last letter
	else
		newStr = !oldString.empty() ? oldString.substr(0, oldString.length()-1) : L"";

	//Delete a letter (performs differently in help mode)
	if (this->isHelpInput()) {
		//If help mode, delete a letter but don't hide the window
		typedCandidateStr.str(L"");
		typedCandidateStr <<newStr;

		updateRomanHelpString();
		viewChanged = true;
	} else {
		// Otherwise, delete a letter from the sentece, and hide if nothing left
		typedSentenceStr.str(L"");
		typedSentenceStr <<newStr;
		viewChanged = true;
	}
}

void LetterInputMethod::handleDelete()
{
	//Delete the letter in front of you (encoding-wise, not visibly)
	//TODO: ADD LATER
}


void LetterInputMethod::handleLeftRight(bool isRight, bool loopToZero)
{
	//Move the letter cursor one to the right/left
	//TODO: ADD LATER
}

void LetterInputMethod::handleTab()
{
	//Move right.
	handleLeftRight(true, false);
}

void LetterInputMethod::handleUpDown(bool isDown)
{
	//For now, nothing.
}

//Ignore numCode, typeBurmeseNumbers
void LetterInputMethod::handleNumber(VirtKey& vkey, bool typeBurmeseNumbers)
{
	this->handleKeyPress(vkey);
}

void LetterInputMethod::handleStop(bool isFull, VirtKey& vkey)
{
	//Perform the normal "enter" routine, unless we're being used as a help keyboard
	if (isHelpInput())
		handleKeyPress(vkey);
	else {
		handleKeyPress(vkey);
		//typedStopChar = isFull ? L'\u104B' : L'\u104A'; //TODO: Make this based on encoding...
		//requestToTypeSentence = true;
	}
}

std::pair<int, std::string> LetterInputMethod::lookupWord(std::wstring typedWord)
{
	throw std::runtime_error("Error: Cannot perform \"lookup word\" in a letter-based model.");
}

void LetterInputMethod::typeHelpWord(std::string roman, std::wstring myanmar, int currStrDictID)
{
	throw std::runtime_error("Error: Cannot perform \"type help word\" in a letter-based model.");
}

void LetterInputMethod::handleCommit(bool strongCommit)
{
	//If we are in help mode, add the word we chose to the dictionary, and flag 
	//   it so that it can be cleared later when the entire sentence is selected.
	//If in normal mode, commit the entire sentence. 
	//TODO: The "model" stuff only works on the "providingHelpFor" window... we need to move
	//      some functions around (like word lookup, etc.). 
	if (!this->isHelpInput()) {
		//Just commit the current sentence.
		if (!typedSentenceStr.str().empty()) 
			requestToTypeSentence = true;
	} else {
		//We need to first convert this string to the target Roman method's encoding.
		bool noEncChange = (uni2Romanenc->toEncoding==myenc2Uni->fromEncoding);
		wstring myanmar = typedCandidateStr.str();
		if (!noEncChange) {
			myenc2Uni->convertInPlace(myanmar);
			uni2Romanenc->convertInPlace(myanmar);
		}

		//Get its romanization, if it exists.
		//NOTE: The "roman" entry is only used for the memory list, so we don't
		//      have to check if wordData.first==-1 (fixes Burglish).
		std::pair<int, std::string> wordData = providingHelpFor->lookupWord(myanmar);
		int currStrDictID = wordData.first;
		string revWord = (!wordData.second.empty()) ? wordData.second : "<no entry>";

		//Add it to the memory list, dictionary, and current sentence.
		providingHelpFor->typeHelpWord(revWord, myanmar, currStrDictID);

		//Flag for removal
		this->providingHelpFor = NULL;
	}
}



//False means nothing else to type
pair<wstring, bool> LetterInputMethod::appendTypedLetter(const std::wstring& prevStr, VirtKey& vkey)
{
	//Nothing to do? NOTE: We need to eventually abstract the mywin functionality from the helpKeyboard. 
	//    For now, though, it will cause no error (KeyMagic overrides this method.)
	char modAlpha = vkey.alphanum;
	if (modAlpha>='a' && modAlpha<='z' && vkey.modShift)
		modAlpha = (modAlpha-'a') + 'A';
	wstring nextBit = helpKeyboard->typeLetter(vkey.vkCode, modAlpha, vkey.modShift);
	if (nextBit.empty())
		return pair<wstring, bool>(prevStr, false);

	//Temp var
	wstring currStr = prevStr + nextBit;
	size_t len = currStr.length();

	//Special cases
	if (nextBit.length()==1 && nextBit[0]==L'\u1039') {
		//Combiner functions in reverse
		wchar_t prev2 = (len>2) ? currStr[len-3] : 0x0000;
		if (len>1 && canStack(currStr[len-2], prev2)) {
			currStr[len-1] = currStr[len-2];
			currStr[len-2] = nextBit[0];
		} else {
			currStr.erase(currStr.length()-1); //Not standard behavior, but let's avoid bad combinations.
		}
	} else if (nextBit == wstring(L"\u1004\u103A\u1039")) {
		//Kinzi can be typed after the consonant instead of before it.
		//For now, we only cover the general case of typing "kinzi" directly after a consonant
		wchar_t prev2 = (len>4) ? currStr[len-5] : 0x0000;
		if (len>3 && canStack(currStr[len-4], prev2)) {
			currStr[len-1] = currStr[len-4];
			currStr[len-4] = nextBit[0];
			currStr[len-3] = nextBit[1];
			currStr[len-2] = nextBit[2];
		}
	}

	return pair<wstring, bool>(currStr, true);
}



void LetterInputMethod::handleKeyPress(VirtKey& vkey)
{
	//Convert locale
	//TODO: Centralize this elsewhere
	vkey.stripLocale();

	//Delegate our combination algorithm
	wstring curr = typedSentenceStr.str();
	if (isHelpInput())
		curr = typedCandidateStr.str();
	pair<wstring, bool> next = appendTypedLetter(curr, vkey);

	//Did we change anything?
	if (next.second) {
		//Update
		if (isHelpInput()) {
			typedCandidateStr.str(L"");
			typedCandidateStr <<next.first;
		} else {
			typedSentenceStr.str(L"");
			typedSentenceStr <<next.first;
		}

		//Save a romanized string if in help mode
		if (this->isHelpInput())
			updateRomanHelpString();

		//Trigger view change.
		viewChanged = true;
	} else {
		//Un-Convert locale
		//TODO: Centralize this elsewhere
		vkey.considerLocale();

		//Check for system keys
		InputMethod::handleKeyPress(vkey);
	}
}



void LetterInputMethod::updateRomanHelpString()
{
	//We need to first convert this string to the target Roman method's encoding.
	bool noEncChange = (myenc2Uni->fromEncoding==uni2Romanenc->toEncoding);
	wstring myanmar = typedCandidateStr.str();
	if (!noEncChange) {
		myenc2Uni->convertInPlace(myanmar);
		uni2Romanenc->convertInPlace(myanmar);
	}

	//Check each romanisation
	typedRomanStr.str(L"");
	string roman = providingHelpFor->lookupWord(myanmar).second;
	if (!roman.empty())
		typedRomanStr <<L'(' <<roman.c_str() <<L')';
}




vector<wstring> LetterInputMethod::getTypedSentenceStrings()
{
	//Special case: don't overwrite the sentence string if we're just showing help.
	if (this->isHelpInput()) {
		//Easy
		bool noEncChange = (romanenc2Uni->fromEncoding==uni2Myenc->toEncoding);
		if (noEncChange)
			return providingHelpFor->getTypedSentenceStrings();

		//Major pain converting encodings, but it has to be done.
		vector<wstring> res;
		{
			vector<wstring> romanRes = providingHelpFor->getTypedSentenceStrings();
			for (vector<wstring>::iterator i=romanRes.begin(); i!=romanRes.end(); i++) {
				//Convert & add
				wstring candidate = *i;
				romanenc2Uni->convertInPlace(candidate);
				uni2Myenc->convertInPlace(candidate);
				res.push_back(candidate);
			}
		}

		return res;
	}

	vector<wstring> res;
	//res.push_back(waitzar::removeZWS(typedSentenceStr.str()));
	res.push_back(typedSentenceStr.str());
	res.push_back(L"");
	res.push_back(L"");
	//res.push_back(waitzar::removeZWS(typedSentenceStr.str()));
	res.push_back(typedSentenceStr.str());
	return res;
}


vector< pair<wstring, unsigned int> > LetterInputMethod::getTypedCandidateStrings()
{
	vector< pair<wstring, unsigned int> > res;
	//res.push_back(pair<wstring, unsigned int>(waitzar::removeZWS(typedCandidateStr.str()), 0));
	res.push_back(pair<wstring, unsigned int>(typedCandidateStr.str(), 0));
	return res;
}


void LetterInputMethod::appendToSentence(wchar_t letter, int id)
{
	typedSentenceStr <<letter;
}


void LetterInputMethod::reset(bool resetCandidates, bool resetRoman, bool resetSentence, bool performFullReset)
{
	//A "full" reset entails the others
	if (performFullReset) {
		resetCandidates = resetRoman = resetSentence = true;
		userDefinedWords.clear();
	}

	if (resetCandidates)
		typedCandidateStr.str(L"");
	if (resetRoman)
		typedRomanStr.str(L"");
	if (resetSentence)
		typedSentenceStr.str(L"");
}


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
