/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _BURGLISH_BUILDER
#define _BURGLISH_BUILDER

#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <stdexcept>

#include "Input/burglish_data.h"
#include "Json CPP/value.h"
#include "Json CPP/reader.h"

namespace waitzar
{

/* 
 * This class is for templatized replacement of WordBuilder.
 */

class BurglishBuilder
{
public:
	//Basic
	BurglishBuilder();
	~BurglishBuilder();
	static void InitStatic();

	///////////////////////////////////////////////
	//Functionality expected in RomanInputMethod()
	///////////////////////////////////////////////
	
	//Key elements of BurglishBuilder
	bool typeLetter(char letter, bool isUpper, const std::wstring& prevWord);
	void reset(bool fullReset);
	std::wstring getParenString() const;

	//Requires hacking (mostly b/c WordBuilder assumes word IDs)
	std::vector<unsigned int> getPossibleWords() const;
	std::vector<int> getWordCombinations() const; //Tied to getPossibleWords
	std::wstring getWordString(unsigned int id) const;
	std::pair<int, std::string> reverseLookupWord(std::wstring word);
	unsigned short getSingleDigitID(unsigned short arabicNumeral);

	//Requires copying of WordBuilder code. (Unfortunate, but unavoidable).
	bool backspace(const std::wstring& prevWord);
	bool moveRight(int amt);
	bool pageUp(bool up);
	std::pair<int, int> typeSpace(int quickJumpID); //ID of res. word, ID of comb. of this word
	int getCurrPage() const;
	int getCurrSelectedID() const;
	int getNumberOfPages() const;
	unsigned short getStopCharacter(bool isFull) const;

	//Required by SentenceList
	void insertTrigram(const std::vector<unsigned int> &trigrams) { } //Burglish doesn't use trigrams.

	//Vestigial (performs no relevant work here)
	bool canTypeShortcut() const { return false; }
	unsigned int getFirstWordIndex() const { return 0; }



private:
	static bool IsVowel(wchar_t letter);
	static bool IsValid(const std::wstring& word);
	static void addStandardWords(std::wstring roman, std::set<std::wstring>& resultsKeyset, std::vector< std::pair<std::wstring, int> >& resultSet, bool firstLetterUppercase, const std::wstring& prevWord, std::vector<std::wstring>& combinationSaveLocation);
	static void addSpecialWords(std::wstring roman, std::set<std::wstring>& resultsKeyset, std::vector< std::pair<std::wstring, int> >& resultSet, std::wstringstream& parenStr);
	static void addNumerals(std::wstring roman, std::set<std::wstring>& resultsKeyset, std::vector< std::pair<std::wstring, int> >& resultSet);
	static void expandCurrentWords(std::set<std::wstring>& resultsKeyset, std::vector< std::pair<std::wstring, int> >& resultSet);

	//Looking backwards
	static std::vector<std::wstring> reverseExpandWords(const std::wstring& myanmar);
	static std::string matchSpecialWord(const std::wstring& myanmar);
	static bool matchOnsetFirstLetter(wchar_t letter);
	static std::pair<std::string, bool> matchOnset(const std::wstring& myanmar);
	static std::pair<std::string, bool> matchRhyme(const std::wstring& myanmar);
	

	//Helper
	std::pair<std::wstring, int> getWordPair(unsigned int id) const;

	void reGenerateWordlist(const std::wstring& prevWord);

	//Borrowed from WordBuilder
	bool setCurrSelected(int id);

private:
	/*static json_spirit::wmObject onsetPairs;
	static json_spirit::wmObject rhymePairs;
	static json_spirit::wmObject specialWords;*/
	static std::map<std::wstring, std::wstring> onsetPairs;
	static std::map<std::wstring, std::wstring> rhymePairs;
	static std::map<std::wstring, std::wstring> specialWords;

	static std::wstring PatSintCombine(const std::wstring& base, const std::wstring& stacked);

	//New candidate words are added like so:
	//   ID[X] = savedDigitIDs.size() + savedWordIDs.size() + savedCombinationIDs.size() + X
	//If a word has a combination, then the word it forms is stored as:
	//   ID[X] = savedDigitIDs.size() + savedWordIDs.size() + X
	//When "space" is pressed, the word is saved, and that new ID is returned.
	//   ID[X] = savedDigitIDs.size() + savedWordIDs.size() + 1
	//   (reset savedCombinationIDs)
	std::wstringstream parenStr;
	std::wstringstream typedRomanStr;
	static std::vector<std::wstring> savedDigitIDs; //0 through 9
	std::vector<std::wstring> savedWordIDs;
	std::vector<std::wstring> savedCombinationIDs;
	std::vector< std::pair<std::wstring, int> > generatedWords; //int refers to the id of its combination in savedCombinationIDs (or -1 if none)
	
	int currSelectedID;
	int currSelectedPage;
	bool typeBeginsWithUpper;

};

}//End of waitzar namespace


#endif //_BURGLISH_BUILDER



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

