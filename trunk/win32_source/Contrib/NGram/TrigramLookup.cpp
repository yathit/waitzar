/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "TrigramLookup.h"


using std::string;
using std::map;



//TODO: Make our binary format more uniform, so that we only need to branch when storing data (not when loading it).
//      We might consider using JSON for this; not sure how it'll work on Linux.
TrigramLookup::TrigramLookup(const string& modelBufferOrFile, bool stringIsBuffer)
{
	//Init a few parameters
	currNexus = 0;
	lastTypedLetter = '\0';

	//Read the file into a buffer
	std::string buffer = stringIsBuffer ? modelBufferOrFile : waitzar::ReadBinaryFile(modelBufferOrFile);

	//Loop variables
	unsigned short lastCommentedNumber;
	unsigned short mode = 0;
	std::wstringstream currWord;
	vector<unsigned int> newArr;

	//Process the buffer
	for (size_t currLineStart=0; currLineStart < buffer.size();) {
		//Step 3-A: left-trim whitespace
		while (buffer[currLineStart]==' ' || buffer[currLineStart]=='\t')
			currLineStart++;
		if (currLineStart >= buffer.size()) break; //NOTE: Re-write the loop later to fix this

		//Step 3-B: Deal with coments and empty lines
		if (buffer[currLineStart] == '\n') {
			//Step 3-B-i: Skip comments
			currLineStart++;
			continue;
		} else if (buffer[currLineStart] == '#') {
			//Step 3-B-ii: Handle comments (find the number which is commented out)
			mode++;
			lastCommentedNumber = 0;
			for (;;) {
				char curr = buffer[currLineStart++];
				if (curr == '\n')
					break;
				else if (curr >= '0' && curr <= '9') {
					lastCommentedNumber *= 10;
					lastCommentedNumber += (curr-'0');
				}
			}

			//Step 3-B-iii: use this number to initialize our data structures
			//Avoid un-necessary resizing by reserving space in our vector.
			switch (mode) {
				case 1: //Words
					dictionary.reserve(lastCommentedNumber);
					break;
				case 2: //Nexi
					nexus.reserve(lastCommentedNumber);
					break;
				case 3: //Prefixes
					prefix.reserve(lastCommentedNumber);
					break;
			}
			continue;
		}

		//Step 3-C: Act differently based on the mode we're in
		switch (mode) {
			case 1: //Words
			{
				//Skip until the first number inside the bracket
				while (buffer[currLineStart] != '[')
					currLineStart++;
				currLineStart++;

				//Keep reading until the terminating bracket.
				//  Each "word" is of the form DD(-DD)*,
				currWord.str(L"");
				for(;;) {
					//Read a "pair", add this letter
					unsigned int currLetter = 0x1000;
					currLetter |= (toHex(model_buff[currLineStart++])<<4);
					currLetter |= (toHex(model_buff[currLineStart++]));
					newWord << (wchar_t)currLetter;

					//Continue?
					char nextChar = buffer[currLineStart++];
					if (nextChar == ',' || nextChar == ']') {
						//Add this word to the dictionary (copy semantics)
						dictionary.push_back(newWord.str());
						newWord.str(L"");

						//Continue?
						if (nextChar == ']')
							break;
					}
				}

				break;
			}
			case 2: //Mappings (nexi)
			{
				//Skip until the first letter inside the bracket
				while (buffer[currLineStart] != '{')
					currLineStart++;
				currLineStart++;

				//A new hashtable for this entry.
				newArr.clear();
				while (buffer[currLineStart] != '}') {
					//Read a hashed mapping: character
					int nextInt = 0;
					char nextChar = 0;
					while (buffer[currLineStart] != ':')
						nextChar = buffer[currLineStart++];
					currLineStart++;

					//Read a hashed mapping: number
					while (buffer[currLineStart] != ',' && buffer[currLineStart] != '}') {
						nextInt *= 10;
						nextInt += (buffer[currLineStart++] - '0');
					}

					//Add that entry to the hash
					newArr.push_back(((nextInt<<8) | (0xFF&nextChar)));

					//Continue?
					if (buffer[currLineStart] == ',')
						currLineStart++;
				}

				//Push-back copies and shrinks the array.
				nexus.push_back(newArr);

				break;
			}
			case 3: //Prefixes (mapped)
			{
				//Skip until the first letter inside the bracket
				while (buffer[currLineStart] != '{')
					currLineStart++;
				currLineStart++;

				//A new hashtable for this entry.
				newArr.clear();
				//Reserve a spot for our "halfway" marker
				newArr.push_back(0);
				while (buffer[currLineStart] != '}') {
					//Read a hashed mapping: number
					unsigned int nextVal = 0;
					while (buffer[currLineStart] != ':') {
						nextVal *= 10;
						nextVal += (buffer[currLineStart++] - '0');
					}
					currLineStart++;

					//Store: key
					newArr.push_back(nextVal);

					//Read a hashed mapping: number
					nextVal = 0;
					while (buffer[currLineStart] != ',' && buffer[currLineStart] != '}') {
						nextVal *= 10;
						nextVal += (buffer[currLineStart++] - '0');
					}
					//Store: val
					newArr.push_back(nextVal);

					//Continue
					if (buffer[currLineStart] == ',')
						currLineStart++;
				}

				//Used to mark our "halfway" boundary.
				lastCommentedNumber = newArr.size()-1; //-1 for the first placeholder value

				//Skip until the first letter inside the square bracket
				while (buffer[currLineStart] != '[')
					currLineStart++;
				currLineStart++;

				//Add a new vector for these
				while (buffer[currLineStart] != ']') {
					//Read a hashed mapping: number
					unsigned int nextVal = 0;
					while (buffer[currLineStart] != ',' && buffer[currLineStart] != ']') {
						nextVal *= 10;
						nextVal += (model_buff[currLineStart++] - '0');
					}

					//Add it
					newArr.push_back(nextVal);

					//Continue
					if (buffer[currLineStart] == ',')
						currLineStart++;
				}

				//Set the halfway marker to the number of PAIRS
				newArr[0] = lastCommentedNumber/2;

				//Copy and shrink & store
				prefix.push_back(newArr);

				break;
			}
			default:
				throw std::runtime_error("Too many comments in model file.");
		}

		//Right-trim
		while (buffer[currLineStart] != '\n')
			currLineStart++;
		currLineStart++;
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

