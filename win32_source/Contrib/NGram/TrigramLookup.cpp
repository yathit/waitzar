/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "TrigramLookup.h"


using std::wstring;
using std::string;
using std::vector;
using std::map;



//TODO: Make our binary format more uniform, so that we only need to branch when storing data (not when loading it).
//      We might consider using JSON for this; not sure how it'll work on Linux.
TrigramLookup::TrigramLookup(const string& modelBufferOrFile, bool stringIsBuffer)
{
	//Get the actual, wide-character stream
	wstring buffer = stringIsBuffer ? waitzar::mbs2wcs(modelBufferOrFile) : waitzar::readUTF8File(modelBufferOrFile);

	//Now, read it into a json object
	Json::Value fileRoot;
	{
		Json::Reader reader;
		if (!reader.parse(waitzar::wcs2mbs(buffer), fileRoot))
			throw std::runtime_error("Can't parse TrigramLookup model file/stream!");
		if (!fileRoot.isObject())
			throw std::runtime_error("Can't parse TrigramLookup model: JSON root is not an object!");
	}

	//REQUIRED: Word list
	auto rootKeys = fileRoot.getMemberNames();
	if (std::find(rootKeys.begin(), rootKeys.end(), "words")==rootKeys.end())
		throw std::runtime_error("Can't parse TrigramLookup model: no \"words\" entry.");
	Json::Value dictObj = fileRoot["words"];
	if (!dictObj.isArray())
		throw std::runtime_error("Can't parse TrigramLookup model: \"words\" is not an array.");
	for (auto it=dictObj.begin(); it!=dictObj.end(); it++) {
		if (!it->isString())
			throw std::runtime_error("Can't parse TrigramLookup model: \"words\" contains a non-string entry.");
		wstring word = waitzar::mbs2wcs(it->asString());
		words.push_back(word);
		//TEMP
		//std::cout <<waitzar::escape_wstr(word) <<std::endl;
	}

	//REQUIRED: Lookup table
	if (std::find(rootKeys.begin(), rootKeys.end(), "lookup")==rootKeys.end())
		throw std::runtime_error("Can't parse TrigramLookup model: no \"lookup\" entry.");
	Json::Value lookupObj = fileRoot["lookup"];
	if (!lookupObj.isObject())
		throw std::runtime_error("Can't parse TrigramLookup model: \"lookup\" is not an object.");
	buildLookupRecursively(lookupObj, lookup);

	//OPTIONAL: n-grams prefix lookups
	if (std::find(rootKeys.begin(), rootKeys.end(), "ngrams")!=rootKeys.end()) {
		Json::Value ngramObj = fileRoot["ngrams"];
		if (!ngramObj.isObject())
			throw std::runtime_error("Can't parse TrigramLookup model: \"ngrams\" is not an object.");
		auto romanKeys = ngramObj.getMemberNames();
		for (auto romanIt=romanKeys.begin(); romanIt!=romanKeys.end(); romanIt++) {
			if (romanIt->empty())
				throw std::runtime_error("Can't parse TrigramLookup model: \"ngrams\" contains an empty romanized key.");
			Json::Value matchesObj = ngramObj[*romanIt];
			if (!matchesObj.isObject())
				throw std::runtime_error("Can't parse TrigramLookup model: \"ngrams\" contains a non-object ngram.");
			auto ngramKeys = matchesObj.getMemberNames();
			for (auto mmIt=ngramKeys.begin(); mmIt!=ngramKeys.end(); mmIt++) {
				if (mmIt->empty())
					throw std::runtime_error("Can't parse TrigramLookup model: \"ngrams\" contains an empty ngram key.");
				Json::Value mmResultObj = matchesObj[*mmIt];
				if (!mmResultObj.isArray())
					throw std::runtime_error("Can't parse TrigramLookup model: \"ngrams\" contains a non-array ngram value.");
				vector<unsigned int> reorder;
				for (auto it=mmResultObj.begin(); it!=mmResultObj.end(); mmResultObj++) {
					if (!it->isUint())
						throw std::runtime_error("Can't parse TrigramLookup model: \"ngrams\" referenes a non-integral reordering.");
					reorder.push_back(it->asUInt());
				}

				//Add it
				ngrams[*romanIt][waitzar::mbs2wcs(*mmIt)] = reorder;
			}
		}
	}

	//OPTIONAL: Last-chance recovery regexes
	if (std::find(rootKeys.begin(), rootKeys.end(), "lastchance")!=rootKeys.end()) {
		Json::Value lcObj = fileRoot["lastchance"];
		if (!lcObj.isArray())
			throw std::runtime_error("Can't parse TrigramLookup model: \"lastchance\" is not an array.");
		for (auto it=lcObj.begin(); it!=lcObj.end(); it++) {
			if (!it->isString())
				throw std::runtime_error("Can't parse TrigramLookup model: \"lastchance\" contains a non-string entry.");
			wstring recover = waitzar::mbs2wcs(it->asString());
			lastChanceRegexes.push_back(recover);
		}
	}

	//OPTIONAL: Pat-sint shortcuts
	if (std::find(rootKeys.begin(), rootKeys.end(), "shortcuts")!=rootKeys.end()) {
		Json::Value shortcutObj = fileRoot["shortcuts"];
		if (!shortcutObj.isObject())
			throw std::runtime_error("Can't parse TrigramLookup model: \"shortcuts\" is not an object.");
		auto baseKeys = shortcutObj.getMemberNames();
		for (auto baseIt=baseKeys.begin(); baseIt!=baseKeys.end(); baseIt++) {
			if (baseIt->empty())
				throw std::runtime_error("Can't parse TrigramLookup model: \"shortcuts\" contains an empty base key.");
			Json::Value matchesObj = shortcutObj[*baseIt];
			if (!matchesObj.isObject())
				throw std::runtime_error("Can't parse TrigramLookup model: \"shortcuts\" contains a non-object base matcher.");
			auto stackedKeys = matchesObj.getMemberNames();
			for (auto stackedIt=stackedKeys.begin(); stackedIt!=stackedKeys.end(); stackedIt++) {
				if (stackedIt->empty())
					throw std::runtime_error("Can't parse TrigramLookup model: \"shortcuts\" contains an empty stacked key.");
				Json::Value mmResultObj = matchesObj[*stackedIt];
				if (!mmResultObj.isString())
					throw std::runtime_error("Can't parse TrigramLookup model: \"shortcuts\" contains a non-string result value.");

				//Add it
				shortcuts[waitzar::mbs2wcs(*baseIt)][waitzar::mbs2wcs(*stackedIt)] = waitzar::mbs2wcs(mmResultObj->asString());
			}
		}
	}
}


void TrigramLookup::buildLookupRecursively(Json::Value& currObj, Nexus& currNode)
{
	auto keys = currObj.getMemberNames();
	for (auto it=keys.begin(); it!=keys.end(); it++) {
		//Enforce single-character key property
		string key = *it;
		if (key.size()!=1)
			throw std::runtime_error("Can't parse TrigramLookup model: \"lookup\" contains a key of length != 1");

		//React to key value
		Json::Value nextObj = currObj[key];
		if (key == "~") {
			//Append all current matches
			if (!nextObj.isArray())
				throw std::runtime_error("Can't parse TrigramLookup model: \"lookup\" contains a non-array value set.");
			for (auto wordID=nextObj.begin(); wordID!=nextObj.end(); nextObj++) {
				if (!wordID->isUInt())
					throw std::runtime_error("Can't parse TrigramLookup model: \"lookup\" contains a non-integral value.");
				currNode.matchedWords.push_back(nextObj.asUInt());
			}
		} else {
			//Append the key, and a blank node for it
			currNode.moveOn += key;
			moveTo.push_back(Node());

			//Recurse
			if (!nextObj.isObject())
				throw std::runtime_error("Can't parse TrigramLookup model: \"lookup\" contains a non-object entry.");
			buildLookupRecursively(nextObj, moveTo[moveTo.size()-1]);
		}
	}



}



//TODO: We might consider using a map<int, int> to lookup word IDs. Have to check the space requirements.
bool TrigramLookup::addRomanizationToModel(const string& roman, const wstring& myanmar, bool errorOnDuplicates)
{
	//Step 1: Do we need to add it?
	if (std::find(dictionary.begin(), dictionary.end(), myanmar)==dictionary.end())
		dictionary.push_back(myanmar);

	//Update the reverse lookup?
	//TODO:
	//if (revLookupOn) {
	//	revLookup.push_back(string());
	//	addReverseLookupItem(revLookup.size()-1, roman);
	//}

	//Step 2: Update the nexus path to this romanization
	size_t currNodeID = 0;
	for (size_t rmID=0; rmID<roman.length(); rmID++) {
		//Does a path exist from our current node to the next step?
		size_t nextNexusID;
		for (nextNexusID=0; nextNexusID<nexus[currNodeID].size(); nextNexusID++) {
			if (((nexus[currNodeID][nextNexusID])&0xFF) == roman[rmID]) {
				break;
			}
		}
		if (nextNexusID==nexus[currNodeID].size()) {
			//First step: make a blank nexus entry at the END of this list
			if (nexus.size() == std::numeric_limits<size_t>::max())
				throw std::runtime_error("Too many custom nexi in model file.");
			nexus.push_back(vector<unsigned int>());

			//Now, link to this from the current nexus list.
			nexus[currNodeID].push_back(((nexus.size()-1)<<8) | (0xFF&roman[rmID]));
		}

		currNodeID = ((nexus[currNodeID][nextNexusID])>>8);
	}


	//Final task: add (just the first) prefix entry.
	size_t currPrefixID;
	for (currPrefixID=0; currPrefixID<nexus[currNodeID].size(); currPrefixID++) {
		if (((nexus[currNodeID][currPrefixID])&0xFF) == '~') {
			break;
		}
	}
	if (currPrefixID == nexus[currNodeID].size()) {
		//We need to add a prefix entry
		if (prefix.size() == std::numeric_limits<size_t>::max())
			throw std::runtime_error("Too many custom prefixes in model file.");
		prefix.push_back(vector<unsigned int>(1, 0));

		//Now, point the nexus to this entry
		nexus[currNodeID].push_back((unsigned int) (((prefix.size()-1)<<8) | ('~')));
	}

	//Translate
	currPrefixID = (nexus[currNodeID][currPrefixID])>>8;

	//Does our prefix entry contain this dictionary word?
	vector<unsigned int>::iterator currWord = prefix[currPrefixID].begin();
	std::advance(currWord, prefix[currPrefixID][0]*2+1);
	for (; currWord!=prefix[currPrefixID].end(); currWord++) {
		if (*currWord == dictID) {
			if (!ignoreDuplicates) {
				wstringstream msg;
				msg << "Word is already in dictionary at ID: " << (*currWord);
				throw std::runtime_error(waitzar::escape_wstr(msg.str()));
			} else {
				return true;
			}
		}
	}

	//Ok, copy it over
	prefix[currPrefixID].push_back(dictID);

	return true;

}


bool TrigramLookup::addShortcut(const wstring& baseWord, const wstring& toStack, const wstring& resultStacked)
{
	//Make sure all 3 words exist in the dictionary
	// NOTE: Technically, toStack need not be in the dictionary. However, we need to know
	//       how to spell it, at least, which would mean a kludge if the word wasn't catalogued.
	unsigned int baseWordID = getWordID(baseWord);
	unsigned int toStackID = getWordID(toStack);
	unsigned int resultStackedID = getWordID(resultStacked);
	unsigned int invalidID = dictionary.size();
	if (baseWordID==invalidID || toStackID==invalidID || resultStackedID==invalidID) {
		wstringstream msg;
		msg <<"pre/post/curr word does not exist in the dictionary (" <<baseWordID
			<<", " <<toStackID <<", " <<resultStackedID <<" : " <<invalidID <<")";
		throw std::runtime_error(waitzar::escape_wstr(msg.str()));
	}

	//For now (assuming no collisions, which is a bit broad of us) we need to say that from
	//  any given nexus that has toStackID in the base set, we set an "if,then" clause;
	//  namely, if baseWordID is the previous trigram entry, then resultStacked is the word of choice.
	//There aren't many pat-sint words, so an ordered list of some sort (or, I guess, a map) should do the
	// trick.
	for (unsigned int toStackNexusID=0; toStackNexusID<nexus.size(); toStackNexusID++) {
		//Is this nexus ID valid?
		vector<unsigned int> &thisNexus = nexus[toStackNexusID];
		unsigned int prefixID = 0;
		bool considerThisNexus = false;
		for (unsigned int x=0; x<thisNexus.size(); x++) {
			//Is this the resolving letter?
			char letter = (char)(0xFF&thisNexus[x]);
			if (letter!='~')
				continue;

			//Ok, save it
			prefixID = (thisNexus[x]>>8);
			considerThisNexus = true;
			break;
		}
		if (considerThisNexus) {
			//Now, test this prefix to see if its base pair contains our word in question.
			considerThisNexus = false;
			vector<unsigned int>::iterator prefWord = prefix[prefixID].begin();
			std::advance(prefWord, prefix[prefixID][0]*2+1);
			for (; prefWord!=prefix[prefixID].end(); prefWord++) {
				if (*prefWord == toStackID) {
					considerThisNexus = true;
					break;
				}
			}
		}
		if (!considerThisNexus)
			continue;


		//A new entry should be added by using the [] syntax
		if (shortcuts[toStackNexusID].count(baseWordID)>0) {
			//Some word's already claimed this nexus.
			wstringstream msg;
			msg <<"Nexus & prefix already in use: " <<toStackNexusID <<" " <<baseWordID;
			throw std::runtime_error(waitzar::escape_wstr(msg.str()));
		}

		//Add this nexus
		shortcuts[toStackNexusID][baseWordID] = resultStackedID;
	}

	return true;
}



bool TrigramLookup::continueLookup(const string& roman)
{
	//Empty string?
	if (roman.empty())
		return false;

	//Is this letter meaningful?
	int nextNexus = jumpToNexus(this->currNexus, letter);
	if (nextNexus == -1) {
		//There's a special case: if we are evaluating "g", it might be a shortcut for "aung"
		if (letter!='g') {
			return false;
		} else {
			//Start at "aung" if we haven't already typed "a"
			string test = (lastTypedLetter=='a') ? "ung" : "aung";

			//Ok, can we get ALL the way there?
			nextNexus = currNexus;
			for (size_t i=0; i<test.size(); i++) {
				nextNexus = jumpToNexus(nextNexus, test[i]);
				if (nextNexus==-1)
					break;
			}

			//Did it work?
			if (nextNexus==-1)
				return false;
		}
	}

	//Save the path to this point and continue
	//pastNexus.push_back(currNexus);
	currNexus = nextNexus;

	//Update the external state of the WordBuilder
	this->resolveWords();
	lastTypedLetter = roman[roman.length-1];

	//Success
	return true;
}


/**
 * Given a "fromNexus", find the link leading out on character "jumpChar" and take it.
 *   Returns -1 if no such nexus could be found.
 */
int WordBuilder::jumpToNexus(int fromNexus, char jumpChar) const
{
	for (unsigned int i=0; i<this->nexus[fromNexus].size(); i++) {
		if ( (this->nexus[fromNexus][i]&0xFF) == jumpChar )
			return (this->nexus[fromNexus][i]>>8);
	}
	return -1;
}


int WordBuilder::jumpToPrefix(int fromPrefix, int jumpID) const
{
	size_t numPrefixPairs = this->prefix[fromPrefix][0];
	for (size_t i=0; i<numPrefixPairs; i++) {
		if ( (int)this->prefix[fromPrefix][i*2+1] == jumpID )
			return this->prefix[fromPrefix][i*2+2];
	}
	return -1;
}



/**
 * Based on the current nexus, what letters are valid moves, and what words
 *   should we present to the user?
 */
void TrigramLookup::resolveWords()
{
	//Init
	parenStr.clear();

	//If there are no words possible, can we jump to a point that doesn't diverge?
	int speculativeNexusID = currNexus;
	while ((nexus[speculativeNexusID].size()==1) && ((nexus[speculativeNexusID][0]&0xFF)!='~')) {
		//Append this to our string
		parenStr += (wchar_t)(nexus[speculativeNexusID][0]&0xFF);

		//Move on this
		speculativeNexusID = ((nexus[speculativeNexusID][0])>>8);
	}
	if (nexus[speculativeNexusID].size()==1 && (nexus[speculativeNexusID][0]&0xFF)=='~') {
		//ok
	} else {
		//Reset
		speculativeNexusID = currNexus;
		parenStr.clear();
	}

	//What possible characters are available after this point?
	int lowestPrefix = -1;
	possibleChars.clear();
	for (unsigned int i=0; i<this->nexus[speculativeNexusID].size(); i++) {
		char currChar = (this->nexus[speculativeNexusID][i]&0xFF);
		if (currChar == '~')
			lowestPrefix = (this->nexus[speculativeNexusID][i]>>8);
		else
			this->possibleChars.push_back(currChar);
	}

	//What words are possible given this point?
	possibleWords.clear();
	wordCombinations.clear();
	firstRegularWordIndex = 0;
	this->currSelectedAbsoluteID = -1;
	if (lowestPrefix == -1)
		return;

	//Hop to the most informative and least informative prefixes
	int highPrefix = lowestPrefix;
	for (unsigned int i=0; i<trigrams.size(); i++) {
		int newHigh = jumpToPrefix(highPrefix, trigrams[i]);
		if (newHigh==-1)
			break;
		highPrefix = newHigh;
	}

	//First, put all high-informative entries into the resultant vector
	//  Then, add any remaining low-information entries
	vector<unsigned int>::iterator possWord = prefix[highPrefix].begin();
	std::advance(possWord, prefix[highPrefix][0]*2+1);
	for (; possWord!=prefix[highPrefix].end(); possWord++) {
		possibleWords.push_back(*possWord);
		wordCombinations.push_back(-1);
	}
	if (highPrefix != lowestPrefix) {
		possWord = prefix[lowestPrefix].begin();
		std::advance(possWord, prefix[lowestPrefix][0]*2+1);
		for (; possWord!=prefix[lowestPrefix].end(); possWord++) {
			if (!vectorContains(possibleWords, *possWord)) {
				possibleWords.push_back(*possWord);
				wordCombinations.push_back(-1);
			}
		}
	}

	//Finally, check if this nexus and the previously-typed word lines up; if so, we have a "post" match
	if (shortcuts.count(currNexus)>0 && trigrams.size()>0) {
		if (shortcuts[currNexus].count(trigrams[0])>0) {
			possibleWords.insert(possibleWords.begin(), shortcuts[currNexus][trigrams[0]]);
			wordCombinations.insert(wordCombinations.begin(), shortcuts[currNexus][trigrams[0]]); //PS resolves to simply this
			firstRegularWordIndex++;
		}
	}

	this->currSelectedAbsoluteID = firstRegularWordIndex; //Start at relative ID "0"
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

