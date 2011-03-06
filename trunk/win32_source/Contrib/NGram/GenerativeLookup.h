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
#include <set>
#include <vector>
#include <stdexcept>

#include "NGram/wz_utilities.h"
#include "Json CPP/json.h"



namespace waitzar
{



/**
 * This class replaces BurglishBuilder with something more focused (it only deals with word generation, not selection)
 *    while at the same time being more generic (it can load models besides Burglish).
 */
class GenerativeLookup {
public:
	//Construct from an in-memory stream or a series of *.json files.
	GenerativeLookup(const std::string& onsetsJson, const std::string& rhymesJson, const std::string& specialsJson, bool isStream=false);

	//Adding words to the model
	//bool addRomanizationToModel(const std::string& roman, const std::wstring& myanmar, bool errorOnDuplicates=false);
	//bool addShortcut(const std::wstring& baseWord, const std::wstring& toStack, const std::wstring& resultStacked);

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
	bool moveLookupOnTrigram(const std::wstring& ultimate, const std::wstring& penultimate=L"", const std::wstring& antepenultimate=L"");
	bool moveLookupOnTrigram(const std::vector<std::wstring>& lastThreeWords) {
		return moveLookupOnTrigram(
			lastThreeWords.size()>0 ? lastThreeWords[0] : L"",
			lastThreeWords.size()>1 ? lastThreeWords[1] : L"",
			lastThreeWords.size()>2 ? lastThreeWords[2] : L""
		);
	}

	//Retrieving words
	const std::vector<std::wstring>& getMatchedWords() {
		rebuildCachedResults();
		return cachedMatchedWords;
	}
	size_t getMatchedDefaultIndex() {
		rebuildCachedResults();
		return cachedStartID;
	}
	const std::string& getMatchedParenString() {
		rebuildCachedResults();
		return cachedParenStr;
	}

	//Additional useful methods
	/*size_t getTotalDefinedNonShortcutWords() {
		return words.size();
	}*/
	std::string reverseLookupWord(const std::wstring& myanmar) ;/*{
		auto it = revLookup.find(myanmar);
		if (it!=revLookup.end())
			return it->second;
		return "";
	}*/


	//TODO:
	void reset() {
/*		currLookup = &lookup;
		actualLookup = NULL;
		currNgram = NULL;
		currShortcutLookup = NULL;
		cacheDirty = true;*/

		typedRoman = "";
	}



private:
	//Primary data, converted from JSON
	std::map<std::wstring, std::wstring> onsetPairs;
	std::map<std::wstring, std::wstring> rhymePairs;
	std::map<std::wstring, std::wstring> specialWords;

	//Build helper
	//void buildLookupRecursively(std::string roman, Json::Value& currObj, Nexus& currNode);

	//State of a search
	std::string typedRoman;
	/*Nexus* currLookup;
	Nexus* actualLookup;  //Where we left off for "shortcut" words, or NULL.
	std::vector<unsigned int>* currNgram;
	std::map<std::wstring, std::wstring>* currShortcutLookup;*/


	//Cached results
	std::string cachedParenStr;
	std::vector<std::wstring> cachedMatchedWords;
	unsigned int cachedStartID;
	bool cacheDirty;


	//Internal functions
	void rebuildCachedResults();
	//std::string getAltString(const std::string& orig, const std::string& pattern);
	//Nexus* walkRomanizedString(const std::string& roman);
	//void resolvePatSint(const std::wstring& prevWord);

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

