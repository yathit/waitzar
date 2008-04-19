/*
 * Copyright 2007 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include ".\wordbuilder.h"


/**
 * Construction of a word builder requires three things. All of these are 2-D jagged arrays;
 *   the first entry in each row is the size of that row.
 * @param dictionary is an array of character sequences. For example,
 *     {"ka", "ko", "sa"} becomes {{1,0x1000}, {3,0x1000,0x102D,0x102F}, {2,0x1005,0x1032}}
 * @param nexus is an array of links. The first entry, of course, is the count. After that, 
 *     there are a series of integers, of the form 0xXXYY, where XX is the nexus to jump to
 *     if character YY is pressed. The whole structure is very much a flattened tree.
 *     Finally, if the character YY is "~", then the integer XX is actually an index into
 *     the "prefix" array.
 * @param prefix is an array of links, similar to nexus. However, it is much simpler. The first
 *     entry in each row is the the "pair count". The second entry is the "match count". The 
 *     "pair count" determines the number of pairs of entries that follow. The first item in each 
 *     pair is the "key", and is an id into the dictionary list. If that "key" is a prefix word 
 *     of the current word, then the "value" is an index into the prefix array to jump to.
 *     Following these pairs are a number of "matches", which are indices into the dictionary
 *     array; each match is a potential word.
 * NOTE: Hash tables make more sense for these data structures, but in my experience (and in a
 *       few profile runs) tiny hash tables offer virtually no performance improvement at a
 *       substantial increase in the memory footprint. So, deal with the C-style arrays.
 */
WordBuilder::WordBuilder (WORD **dictionary, int dictMaxID, int dictMaxSize, UINT32 **nexus, int nexusMaxID, int nexusMaxSize, UINT32 **prefix, int prefixMaxID, int prefixMaxSize)
{
	//Store for later
	this->dictionary = dictionary;
	this->nexus = nexus;
	this->prefix = prefix;

	this->dictMaxID = dictMaxID;
	this->dictMaxSize = dictMaxSize;

	this->nexusMaxID = nexusMaxID;
	this->nexusMaxSize = nexusMaxSize;

	this->prefixMaxID = prefixMaxID;
	this->prefixMaxSize = prefixMaxSize;

	//Start off
	this->reset(true);
}

WordBuilder::~WordBuilder(void)
{
	//NOTE: This function probably doesn't ever need to be implemented properly.
	//      The way it stands, all files are scanned and data loaded when the
	//      program first starts. This all remains in memory until the program
	//      exits, at which point the entire process terminates anyways.
	//      There is never more than one WordBuilder, dictionary, nexus, prefix array, etc.
	//      So, we don't really worry too much about memory (except brushes and windows and 
	//      those kinds of things.)
}


/**
 * Types a letter and advances the nexus pointer. Returns true if the letter was a valid move,
 *   false otherwise. Updates the available word/letter list appropriately.
 */
bool WordBuilder::typeLetter(char letter) 
{
	//Is this letter meaningful?
	int nextNexus = jumpToNexus(this->currNexus, letter);
	if (nextNexus == -1)
		return false;

	//Save the path to this point and continue
	pastNexus[pastNexusID++] = currNexus;
	currNexus = nextNexus;

	//Update the external state of the WordBuilder
	this->resolveWords();

	//Success
	return true;
}


BOOL WordBuilder::moveRight(int amt) {
	//Any words?
	if (possibleWords.size()==0)
		return FALSE;

	//Any change?
	int newAmt = currSelectedID + amt;
	if (newAmt >= (int)possibleWords.size())
		newAmt = (int)possibleWords.size()-1;
	else if (newAmt < 0)
		newAmt = 0;
	if (newAmt == currSelectedID)
		return FALSE;

	//Do it!
	currSelectedID = newAmt;
	return TRUE;
}


int WordBuilder::getCurrSelectedID() {
	return currSelectedID;
}


//Returns true if the window is still visible.
bool WordBuilder::backspace()
{
	currNexus = pastNexus[--pastNexusID];
	this->resolveWords();

	if (pastNexusID == 0)
		return false;
	return true;
}


void WordBuilder::setCurrSelected(int id)
{
	//Any words?
	if (possibleWords.size()==0)
		return;

	//Fail silently if this isn't a valid id
	if (id >= (int)possibleWords.size())
		return;
	else if (id < 0)
		return;

	//Do it!
	currSelectedID = id;
}


//Returns the selected ID and a boolean
std::pair<BOOL, UINT32> WordBuilder::typeSpace(int quickJumpID) 
{
	//Return val
	std::pair<BOOL, UINT32> result(FALSE, 0);

	//We're at a valid stopping point?
	if (this->getPossibleWords().size() == 0)
		return result;

	//Quick jump?
	if (quickJumpID > -1)
		this->setCurrSelected(quickJumpID);

	//Get the selected word, add it to the prefix array
	UINT32 newWord = this->getPossibleWords()[this->currSelectedID];
	addPrefix(newWord);

	//Reset the model, return this word
	this->reset(false);

	result.first = TRUE;
	result.second = newWord;
	return result;
}


/**
 * Reset the builder to its zero-state
 * @param fullReset If true, remove all prefixes in memory. (Called when Alt+Shift is detected.)
 */
void WordBuilder::reset(bool fullReset)
{
	//Partial reset
	this->currNexus = 0;
	this->pastNexusID = 0;
	this->currSelectedID = -1;
	this->possibleChars.clear();
	this->possibleWords.clear();

	//Full reset: remove all prefixes
	if (fullReset)
		this->trigramCount = 0;
}


/**
 * Given a "fromNexus", find the link leading out on character "jumpChar" and take it.
 *   Returns -1 if no such nexus could be found.
 */
int WordBuilder::jumpToNexus(int fromNexus, char jumpChar) 
{
	for (UINT32 i=0; i<this->nexus[fromNexus][0]; i++) {
		if ( (this->nexus[fromNexus][i+1]&0xFF) == jumpChar )
			return (this->nexus[fromNexus][i+1]>>8);
	}
	return -1;
}


int WordBuilder::jumpToPrefix(int fromPrefix, int jumpID)
{
	for (UINT32 i=0; i<this->prefix[fromPrefix][0]; i++) {
		if ( this->prefix[fromPrefix][i*2+2] == jumpID )
			return this->prefix[fromPrefix][i*2+3];
	}
	return -1;
}


/**
 * Based on the current nexus, what letters are valid moves, and what words
 *   should we present to the user?
 */
void WordBuilder::resolveWords(void) 
{
	//What possible characters are available after this point?
	int lowestPrefix = -1;
	this->possibleChars.clear();
	for (UINT32 i=0; i<this->nexus[currNexus][0]; i++) {
		char currChar = (this->nexus[currNexus][i+1]&0xFF);
		if (currChar == '~')
			lowestPrefix = (this->nexus[currNexus][i+1]>>8);
		else
			this->possibleChars.push_back(currChar);
	}

	//What words are possible given this point?
	possibleWords.clear();
	this->currSelectedID = -1;
	if (lowestPrefix == -1)
		return;

	//Hop to the most informative and least informative prefixes
	int highPrefix = lowestPrefix;
	for (UINT32 i=0; i<trigramCount; i++) {
		int newHigh = jumpToPrefix(highPrefix, trigram[i]);
		if (newHigh==-1)
			break;
		highPrefix = newHigh;
	}

	//First, put all high-informative entries into the resultant vector
	//  Then, add any remaining low-information entries
	for (UINT32 i=0; i<prefix[highPrefix][1]; i++) {
		possibleWords.push_back(prefix[highPrefix][2+prefix[highPrefix][0]*2+i]);
	}
	if (highPrefix != lowestPrefix) {
		for (UINT32 i=0; i<prefix[lowestPrefix][1]; i++) {
			int val = prefix[lowestPrefix][2+prefix[lowestPrefix][0]*2+i];
			if (!vectorContains(possibleWords, val))
				possibleWords.push_back(val);
		}
	}

	this->currSelectedID = 0;
}


bool WordBuilder::vectorContains(std::vector<UINT32> vec, UINT32 val)
{
	for (size_t i=0; i<vec.size(); i++) {
		if (vec[i] == val)
			return true;
	}
	return false;
}


void WordBuilder::addPrefix(UINT32 latestPrefix)
{
	//Latest prefixes go in the back
	if (trigramCount == 3) {
		trigram[0] = trigram[1];
		trigram[1] = trigram[2];
		trigram[2] = latestPrefix;
	} else
		trigram[trigramCount++] = latestPrefix;
}


std::vector<char> WordBuilder::getPossibleChars(void)
{
	return this->possibleChars;
}

std::vector<UINT32> WordBuilder::getPossibleWords(void)
{
	return this->possibleWords;
}

std::vector<WORD> WordBuilder::getWordKeyStrokes(UINT32 id) 
{
	this->keystrokeVector.clear();

	WORD size = this->dictionary[id][0];
	for (int i=0; i<size; i++) 
		this->keystrokeVector.push_back(this->dictionary[id][i+1]);

	return this->keystrokeVector;
}


/**
 * Get the actual characters in a string (by ID). Note that
 *  this returns a single (global) object, so if multiple strings are to be 
 *  dealt with, they must be copied into local variables -something like:
 *    TCHAR *temp1 = wb->getWordString(1);
 *    TCHAR *temp2 = wb->getWordString(2);
 *    TCHAR *temp3 = wb->getWordString(3);
 * ...will FAIL; temp1, temp2, and temp3 will ALL have the value of string 3.
 * Do something like this instead:
 *   TCHAR temp1[50]; 
 *   lstrcpy(temp1, wb->getWordString(1));
 *   TCHAR temp2[50]; 
 *   lstrcpy(temp2, wb->getWordString(2));
 *   ...etc.
 */ 
TCHAR* WordBuilder::getWordString(UINT32 id)
{
	lstrcpy(currStr, _T(""));
	TCHAR temp[50];

	WORD size = this->dictionary[id][0];
	for (int i=0; i<size; i++)  {
		wsprintf(temp, _T("%c"), this->dictionary[id][i+1]);
		lstrcat(this->currStr, temp);
	}

	return this->currStr;
}


void WordBuilder::addRomanization(TCHAR* myanmar, char* roman) 
{
	//First task: find the word; add it if necessary
	int dictID;
	int mmLen = lstrlen(myanmar);
	for (dictID=0; dictID<dictMaxID; dictID++) {
		if (mmLen != dictionary[dictID][0])
			continue;

		bool found = true;
		for (int i=0; i<dictionary[dictID][0]; i++) {
			if (dictionary[dictID][i+1] != (WORD)myanmar[i]) {
				found = false;
				break;
			}
		}
		if (found) {
			break;
		}
	}
	if (dictID==dictMaxID) {
		//Need to add... we DO have a limit, though.
		if (dictID == dictMaxSize) {
			MessageBox(NULL, _T("Too many custom words!"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}

		dictMaxID++;
		dictionary[dictID] = (WORD *)malloc((mmLen+1) * sizeof(WORD));
		dictionary[dictID][0] = mmLen;
		for (int i=0; i<mmLen; i++) {
			dictionary[dictID][i+1] = (WORD)myanmar[i];
		}
	} 


	//Next task: add the romanized mappings
	size_t currNodeID = 0;
	size_t romanLen = strlen(roman);
	for (size_t rmID=0; rmID<romanLen; rmID++) {
		//Does a path exist from our current node to the next step?
		size_t nextNexusID;
		for (nextNexusID=0; nextNexusID<nexus[currNodeID][0]; nextNexusID++) {
			if (((nexus[currNodeID][nextNexusID+1])&0xFF) == roman[rmID]) {
				break;
			}
		}
		if (nextNexusID==nexus[currNodeID][0]) {
			//First step: make a blank nexus entry at the END of this list
			if (nexusMaxID == nexusMaxSize) {
				MessageBox(NULL, _T("Too many custom nexi!"), _T("Error"), MB_ICONERROR | MB_OK);
				return;
			}
			nexus[nexusMaxID] = (UINT32 *)malloc((1) * sizeof(UINT32));
			nexus[nexusMaxID][0] = 0;

			//Next: copy all old entries into a new array
			UINT32 * newCurrent = (UINT32 *)malloc((nexus[currNodeID][0]+1) * sizeof(UINT32));
			newCurrent[0] = nexus[currNodeID][0]+1;
			for (size_t i=0; i<nexus[currNodeID][0]; i++) {
				newCurrent[i+1] = nexus[currNodeID][i+1];
			}
			free(nexus[currNodeID]);
			nexus[currNodeID] = newCurrent;
			
			//Finally: add a new entry linking to the nexus we just created
			nexus[currNodeID][nexus[currNodeID][0]] = (nexusMaxID<<8) | (0xFF&roman[rmID]);
			nexusMaxID++;
		}

		currNodeID = ((nexus[currNodeID][nextNexusID+1])>>8);
	}


	//Final task: add (just the first) prefix entry.
	size_t currPrefixID;
	for (currPrefixID=0; currPrefixID<nexus[currNodeID][0]; currPrefixID++) {
		if (((nexus[currNodeID][currPrefixID])&0xFF) == '~') {
			break;
		}
	}
	if (currPrefixID == nexus[currNodeID][0]) {
		//We need to add a prefix entry
		if (prefixMaxID == prefixMaxSize) {
			MessageBox(NULL, _T("Too many custom prefixes!"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}
		prefix[prefixMaxID] = (UINT32 *)malloc((2) * sizeof(UINT32));
		prefix[prefixMaxID][0] = 0;
		prefix[prefixMaxID][1] = 0;

		//Next: copy all old entries into a new array
		UINT32 * newCurrent = (UINT32 *)malloc((nexus[currNodeID][0]+1) * sizeof(UINT32));
		newCurrent[0] = nexus[currNodeID][0]+1;
		for (size_t i=0; i<nexus[currNodeID][0]; i++) {
			newCurrent[i+1] = nexus[currNodeID][i+1];
		}
		free(nexus[currNodeID]);
		nexus[currNodeID] = newCurrent;
			
		//Finally: add a new entry linking to the nexus we just created
		currPrefixID = prefixMaxID;
		nexus[currNodeID][nexus[currNodeID][0]] = (UINT32) ((currPrefixID<<8) | ('~'));
		prefixMaxID++;
	}

	//Does our prefix entry contain this dictionary word?
	for (size_t i=0; i<prefix[currPrefixID][1]; i++) {
		if (prefix[currPrefixID][prefix[currPrefixID][0]*2+2+i] == dictID) {
			MessageBox(NULL, _T("Word is already in dictionary!"), _T("Error"), MB_ICONERROR | MB_OK);
			return;
		}
	}

	//Ok, copy it over
	size_t oldSize = prefix[currPrefixID][0]*2 + prefix[currPrefixID][1] + 2;
	UINT32 * newPrefix = (UINT32 *)malloc((oldSize+1) * sizeof(UINT32));
	newPrefix[0] = prefix[currPrefixID][0];
	newPrefix[1] = prefix[currPrefixID][1]+1;
	for (size_t i=2; i<oldSize; i++) {
		newPrefix[i] = prefix[currPrefixID][i];
	}
	newPrefix[oldSize] = dictID;
	free(prefix[currPrefixID]);
	prefix[currPrefixID] = newPrefix;
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
