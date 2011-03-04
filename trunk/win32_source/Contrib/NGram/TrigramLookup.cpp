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

//Add a word from a non-model file.
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


//TODO: We might merge this with "continueLookup" somehow.
Nexus* TrigramLookup::walkRomanizedString(const std::string& roman)
{
	if (roman.empty())
		return false;

	Nexus* retNexus = &lookup;
	for (auto ch=roman.begin(); ch!=roman.end(); ch++) {
		//Does an entry exist?
		int id = retNexus->getMoveID(*ch);
		if (id==-1)
			return NULL;

		//Jump to that entry
		retNexus = &retNexus->moveTo[id];
	}

	//Done
	return retNexus;
}


//Apply a pattern.
string TrigramLookup::getAltString(const string& orig, const string& pattern)
{
	//Avoid weird behavior.
	if (orig.empty() || pattern.empty())
		return "";

	//Now, break the regex apart. (TODO: We can cache this in the "init" phase, and throw an exception on error)
	size_t eqID = pattern.find(L'=');
	if (eqID==string::npos || eqID==0 || eqID==pattern.size()-1)
		return "";
	string lhs = pattern.substr(0, eqID);
	string rhs = pattern.substr(eqID+1, pattern.size());

	//Now, attempt to apply it.
	// We match from right-to-left starting at the end of the string. Each character consumes
	//   a single character in the original string, except "?" which makes the next character
	//   optional (in the greedy sense of the word).
	int dotID = orig.size()-1; //dotID can go negative, we just can't match anything at that point.
	for (auto it=lhs.rbegin(); it!=lhs.rend(); it++) {
		bool optional = false;
		if (*it == '?') {
			optional = true;
			it++;
			if (it==lhs.rend() || *it == '?')
				return ""; //Can't have '??' or trailing '?'
		}

		if (dotID>=0 && orig[dotID] == *it) {
			dotID--;
		} else if (!optional)
			return ""; //No match.
	}

	//Should never happen, but...
	if (dotID<-1)
		return "";

	//Done! Now substitute
	string prefix = orig.substr(0, dotID+1);
	return prefix + rhs;
}



bool TrigramLookup::continueLookup(const string& roman)
{
	//Empty string?
	if (roman.empty())
		return false;

	//Where were we _exactly_ ?
	if (actualLookup!=NULL) {
		currLookup = actualLookup;
		actualLookup = NULL;
	}

	//Apply to each letter
	bool found = true;
	for (auto ch=roman.begin(); ch!=roman.end(); ch++) {
		//Does an entry exist?
		int id = currLookup->getMoveID(*ch);
		Nexus* nextNexus = NULL;
		if (id!=-1)
			nextNexus = &currLookup->moveTo[id];
		else {
			//Last-chance matches.
			for (auto it=lastChanceRegexes.begin(); it!=lastChanceRegexes.end() && nextNexus==NULL; it++) {
				string altString = getAltString(typedRoman+string(1, *ch), waitzar::escape_wstr(*it));
				nextNexus = walkRomanizedString(altString);
			}
		}

		//Can't move?
		if (nextNexus==NULL) {
			found = false;
			break;
		}

		//Jump
		currLookup = nextNexus;

		//Append
		typedRoman += string(1, *ch);
	}

	//Reset trigrams, cache
	currNgram = NULL;
	cacheDirty = true;

	//Perform a "skip ahead"
	//Paren string
	cachedParenStr = "(";
	Nexus* parenLookup = currLookup;
	while (parenLookup->moveOn.size()==1) {
		cachedParenStr += parenLookup->moveOn;
		parenLookup = &parenLookup->moveTo[0];
	}
	if (parenLookup->moveOn.empty() && !parenLookup->matchedWords.empty()) {
		cachedParenStr += ")";
		actualLookup = currLookup;
		currLookup = parenLookup;
	} else {
		cachedParenStr = "";
	}

	//Success
	return found;
}



bool TrigramLookup::moveLookupOnTrigram(const std::wstring& ultimate, const std::wstring& penultimate, const std::wstring& antepenultimate)
{
	//Jump
	auto pairs = ngrams.find(typedRoman);
	if (pairs==ngrams.end())
		return false;

	//Make a list of candidates to try
	vector<wstring> attempts;
	if (!ultimate.empty()) {
		if (!penultimate.empty()) {
			if (!antepenultimate.empty())
				attempts.push_back(ultimate+L"/"+penultimate+L"/"+antepenultimate);
			attempts.push_back(ultimate+L"/"+penultimate);
		}
		attempts.push_back(ultimate);
	}

	//Try each candidate
	for (auto it=attempts.begin(); it!=attempts.end(); it++) {
		auto match = pairs->second.find(*it);
		if (match!=pairs->second.end()) {
			currNgram = &(match->second);
			return true;
		}
	}

	//Nothing matched.
	return false;
}



/**
 * Based on the current nexus, what letters are valid moves, and what words
 *   should we present to the user?
 */
void TrigramLookup::rebuildCachedResults()
{
	//Only once
	if (!cacheDirty)
		return;

	//Matched words
	vector<wstring> shortcutWords;

	//TODO: Matched Words & Start ID need to be aware of shortcuts
	//      I think we can just make 2 arrays (1 p.s., the other regular, and just add them)

	//Add prefixes
	cachedMatchedWords.clear();
	std::set<unsigned int> normalWordIDs;
	if (currNgram!=NULL) {
		for (auto it=currNgram->begin(); it!=currNgram->end(); it++) {
			cachedMatchedWords.push_back(words[*it]);
			normalWordIDs.insert(*it);
		}
	}

	//Add words
	for (auto it=currLookup->matchedWords.begin(); it!=currLookup->matchedWords.end(); it++) {
		if (normalWordIDs.count(*it)>0)
			continue;
		cachedMatchedWords.push_back(words[*it]);
	}

	//Matched+Shortcuts, StartID
	cachedMatchedWords.insert(cachedMatchedWords.begin(), shortcutWords.begin(), shortcutWords.end());
	cachedStartID = shortcutWords.size();

	//Done
	cacheDirty = false;
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

