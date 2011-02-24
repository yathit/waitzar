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


namespace waitzar
{



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
	//for (size_t i=0; i<dictObj.size(); i++) {
		if (!(*it).isString())
			throw std::runtime_error("Can't parse TrigramLookup model: \"words\" contains a non-string entry.");
		wstring word = waitzar::mbs2wcs((*it).asString());
		words.push_back(word);
	}

	//REQUIRED: Lookup table
	if (std::find(rootKeys.begin(), rootKeys.end(), "lookup")==rootKeys.end())
		throw std::runtime_error("Can't parse TrigramLookup model: no \"lookup\" entry.");
	Json::Value lookupObj = fileRoot["lookup"];
	if (!lookupObj.isObject())
		throw std::runtime_error("Can't parse TrigramLookup model: \"lookup\" is not an object.");
	buildLookupRecursively("", lookupObj, lookup);

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
				for (auto it=mmResultObj.begin(); it!=mmResultObj.end(); it++) {
					if (!(*it).isIntegral())
						throw std::runtime_error("Can't parse TrigramLookup model: \"ngrams\" referenes a non-integral reordering.");
					reorder.push_back((*it).asUInt());
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
			if (!(*it).isString())
				throw std::runtime_error("Can't parse TrigramLookup model: \"lastchance\" contains a non-string entry.");
			wstring recover = waitzar::mbs2wcs((*it).asString());
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
				shortcuts[waitzar::mbs2wcs(*baseIt)][waitzar::mbs2wcs(*stackedIt)] = waitzar::mbs2wcs(mmResultObj.asString());
			}
		}
	}
}


void TrigramLookup::buildLookupRecursively(string roman, Json::Value& currObj, Nexus& currNode)
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
			for (auto wordID=nextObj.begin(); wordID!=nextObj.end(); wordID++) {
				if (!(*wordID).isIntegral())
					throw std::runtime_error("Can't parse TrigramLookup model: \"lookup\" contains a non-integral value.");
				currNode.matchedWords.push_back((*wordID).asUInt());

				//Update the reverse lookup
				wstring myanmar = words[(*wordID).asUInt()];
				if (revLookup.count(myanmar)==0)
					revLookup[myanmar] =  roman;
			}
		} else {
			//Append the key, and a blank node for it
			currNode.moveOn += key;
			currNode.moveTo.push_back(Nexus());

			//Recurse
			if (!nextObj.isObject())
				throw std::runtime_error("Can't parse TrigramLookup model: \"lookup\" contains a non-object entry.");
			buildLookupRecursively(roman+key, nextObj, currNode.moveTo[currNode.moveTo.size()-1]);
		}
	}



}



//TODO: We might consider using a map<int, int> to lookup word IDs. Have to check the space requirements.
bool TrigramLookup::addRomanizationToModel(const string& roman, const wstring& myanmar, bool errorOnDuplicates)
{
	//Step 1: Do we need to add it? (Also, retrieve its ID)
	size_t currWordID = 0;
	for (; currWordID<words.size(); currWordID++) {
		if (words[currWordID] == myanmar)
			break;
	}
	if (currWordID==words.size())
		words.push_back(myanmar);

	//Update the reverse lookup?
	if (revLookup.count(myanmar)==0)
		revLookup[myanmar] =  roman;

	//Step 2: Update the nexus path to this romanization
	Nexus* currNode = &lookup;
	for (auto ch=roman.begin(); ch!=roman.end(); ch++) {
		//Add a path if needed
		if (currNode->getMoveID(*ch)==-1) {
			currNode->moveOn = currNode->moveOn + string(1, *ch);
			currNode->moveTo.push_back(Nexus());
		}

		//Advance
		currNode = &currNode->moveTo[currNode->getMoveID(*ch)];
	}

	//Step 3: Add this word's ID
	if (std::find(currNode->matchedWords.begin(), currNode->matchedWords.end(), currWordID)==currNode->matchedWords.end())
		currNode->matchedWords.push_back(currWordID);

	return true;

}


bool TrigramLookup::addShortcut(const wstring& baseWord, const wstring& toStack, const wstring& resultStacked)
{
	//Add/Get
	shortcuts[baseWord][toStack] = resultStacked;

	return true;
}



bool TrigramLookup::continueLookup(const string& roman)
{
	//Empty string?
	if (roman.empty())
		return false;

	//Apply to each letter
	for (auto ch=roman.begin(); ch!=roman.end(); ch++) {
		//Does an entry exist?
		int id = currLookup->getMoveID(*ch);
		if (id==-1)
			return false;  //TODO: Try our "lastchance" shortcuts; make sure "typedRoman" is updated correctly!

		//Append
		typedRoman += string(1, *ch);

		//Jump
		currLookup = &currLookup->moveTo[id];
	}

	//TODO:
	//We might update our "matched" words here, just to make it easier to quickly return an array.
	//But, actually, "updateTrigrams" would change this anway....

	//Success
	return true;
}


/**
 * Given a "fromNexus", find the link leading out on character "jumpChar" and take it.
 *   Returns -1 if no such nexus could be found.
 */
/*int WordBuilder::jumpToNexus(int fromNexus, char jumpChar) const
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
}*/



/**
 * Based on the current nexus, what letters are valid moves, and what words
 *   should we present to the user?
 */
void TrigramLookup::resolveWords()
{
	//TODO: We might "resolve" on a trigram, or keep a "dirty" bit and resolve, etc.
	//      In other words, ther's a lot to consider.


	//Init
	/*parenStr.clear();

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

	this->currSelectedAbsoluteID = firstRegularWordIndex; //Start at relative ID "0"*/
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

