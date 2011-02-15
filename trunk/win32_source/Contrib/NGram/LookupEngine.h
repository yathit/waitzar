/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once

#include <string>
#include <vector>


//Base abstract class for typing a letter and generating words.
class LookupEngine {
public:
	//Primary functionality
	virtual bool typeLetter(char letter, bool isUpper, const std::wstring& prevWord) = 0;
	virtual void reset(bool fullReset) = 0;
	virtual std::wstring getParenString() const = 0;

	//Secondary functionality
	virtual void insertTrigram(const std::vector<unsigned int> &trigrams) = 0;
	virtual bool canTypeShortcut() const = 0;
	virtual unsigned int getFirstWordIndex() const = 0;

	//Requires hacking (mostly b/c WordBuilder assumes word IDs)
	//TODO: Replace these!
	virtual std::vector<unsigned int> getPossibleWords() const = 0;
	virtual std::vector<int> getWordCombinations() const = 0; //Tied to getPossibleWords
	virtual std::wstring getWordString(unsigned int id) const = 0;
	virtual std::pair<int, std::string> reverseLookupWord(std::wstring word) = 0;
	virtual unsigned short getSingleDigitID(unsigned short arabicNumeral) = 0;

	//Seems to be, in large part, copied.
	//TODO: Extract methods to the superclass that apply to both.
	virtual bool backspace(const std::wstring& prevWord) = 0;
	virtual bool moveRight(int amt) = 0;
	virtual bool pageUp(bool up) = 0;
	virtual std::pair<int, int> typeSpace(int quickJumpID) = 0; //ID of res. word, ID of comb. of this word
	virtual int getCurrPage() const = 0;
	virtual int getCurrSelectedID() const = 0;
	virtual int getNumberOfPages() const = 0;
	virtual unsigned short getStopCharacter(bool isFull) const = 0;
};






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
