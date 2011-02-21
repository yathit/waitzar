/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once


#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <stdexcept>

#include "NGram/wz_utilities.h"
#include "Json CPP/json.h"



namespace waitzar
{

//Helper class
class Nexus {
	std::string moveOn;
	std::vector<Node> moveTo;

	std::vector<unsigned int> matchedWords;
};



/**
 * This class replaces WordBuilder with something more focussed: it only deals with
 *    loading our binary model file and looking up candidates. Selection of these
 *    candidates is handled separately.
 */
class TrigramLookup {
public:
	//Construct from an in-memory stream or a *.model file.
	TrigramLookup(const std::string& modelBufferOrFile, bool stringIsBuffer=false);

	//Adding words to the model
	bool addRomanizationToModel(const std::string& roman, const std::wstring& myanmar, bool errorOnDuplicates=false);
	bool addShortcut(const std::wstring& baseWord, const std::wstring& toStack, const std::wstring& resultStacked);

	//Moving within the model
	bool continueLookup(const std::string& roman);
	bool startLookup(const std::string& roman) {
		reset();
		return continueLookup(roman);
	}
	/*bool backspaceLookup() {
		if (currRoman.empty())
			return false;
		return startLookup(currRoman.substr(0, currRoman.size()-1));
	}*/
	bool moveLookupOnTrigram(const std::wstring& ultimate, const std::wstring& penultimate, const std::wstring& antepenultimate);
	bool moveLookupOnTrigram(const std::vector<std::wstring>& lastThreeWords) {
		return moveLookupOnTrigram(
			lastThreeWords.size()>0 ? lastThreeWords[0] : L"",
			lastThreeWords.size()>1 ? lastThreeWords[1] : L"",
			lastThreeWords.size()>2 ? lastThreeWords[2] : L""
		);
	}

	//Retrieving words
	const std::vector<std::wstring>& getMatchedWords();
	size_t getMatchedDefaultIndex();
	const std::string& getMatchedParenString() {
		return parenStr;
	}

	//Additional useful methods
	size_t getTotalDefinedNonShortcutWords();
	const std::string& reverseLookupWord(const std::wstring& myanmar);



private:
	//Primary data, converted from JSON
	std::vector<std::wstring> words;
	Nexus lookup;
	std::vector<std::wstring> lastChanceRegexes;
	std::map<std::string, std::map<std::wstring, std::vector<std::unsigned int>>> ngrams;
	std::map<std::wstring, std::map<std::wstring, std::wstring>> shortcuts;

	//Build helper
	static void buildLookupRecursively(Json::Value& currObj, Nexus& currNode);


	//State of a search
	std::string parenStr;
	unsigned int currNexus;
	wchar_t lastTypedLetter;

	//Internal functions
	void resolveWords(void);
	int jumpToNexus(int fromNexus, char jumpChar) const;
	int jumpToPrefix(int fromPrefix, int jumpID) const;


};





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

