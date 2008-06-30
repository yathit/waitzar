/*
 * Copyright 2007 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "WordBuilder.h"




/**
 * Load a model given the Wait Zar binary model and a text file containing user additions.
 * @param modelFile is "Myanmar.model"
 * @param userWordsFile is "mywords.txt"
 * If userWordsFile doesn't exist, it is ignored. If modelFile doesn't exist, it 
 *  causes unpredictable behavior.
 */
WordBuilder::WordBuilder (char* modelFilePath, char* userWordsFilePath)
{
	//Step zero: prepare jagged arrays (and bookkeeping data related to them)
	unsigned short **dictionary;
	unsigned int **nexus;
	unsigned int **prefix;
	int dictMaxID;
	int dictMaxSize;
	int nexusMaxID;
	int nexusMaxSize;
	int prefixMaxID;
	int prefixMaxSize;
	
	//Step one: open the model file (ASCII)
	FILE* modelFile = fopen(modelFilePath, "r");
	if (modelFile == NULL) {
		printf("Cannot create WordBuilder; model file does not exist.\n    %s\n", modelFile);
		return;
	}
	
	//Step two: figure out its length and convert it to a character array.
	fseek (modelFile, 0, SEEK_END);
	long modelFileSize = ftell(modelFile);
	rewind(modelFile);
	char * model_buff = (char*) malloc(sizeof(char)*modelFileSize);
	size_t model_buff_size = fread(model_buff, 1, modelFileSize, modelFile);
	fclose(modelFile);
	
	//Step three: Read each line
	size_t currLineStart = 0;
	unsigned short count;
	unsigned short lastCommentedNumber;
	unsigned int currDictionaryID;
	unsigned short newWordSz;
	unsigned short mode = 0;
	unsigned int newWord[1000];
	char currLetter[] = "1000";
	int numberCheck = 0; //Special...
	while (currLineStart < model_buff_size) {
		//Step 3-A: left-trim spaces
		while (model_buff[currLineStart] == ' ')
			currLineStart++;
		
		//Step 3-B: Deal with coments and empty lines
		if (model_buff[currLineStart] == '\n') {
			//Step 3-B-i: Skip comments
			currLineStart++;
			continue;
		} else if (model_buff[currLineStart] == '#') {
			//Step 3-B-ii: Handle comments (find the number which is commented out)
			count = 0;
			mode++;
			lastCommentedNumber = 0;
			for (;;) {
				char curr = model_buff[currLineStart++];
				if (curr == '\n')
					break;
				else if (curr >= '0' && curr <= '9') {
					lastCommentedNumber *= 10;
					lastCommentedNumber += (curr-'0');
				}
			}
			
			//Step 3-B-iii: use this number to initialize our data structures
			switch (mode) {
				case 1: //Words
					//Initialize our dictionary
					dictMaxID = lastCommentedNumber;
					dictMaxSize = (dictMaxID*3)/2;
					dictionary = (unsigned short **)malloc(dictMaxSize * sizeof(unsigned short *));
					currDictionaryID = 0;
					break;
				case 2: //Nexi
					//Initialize our nexus list
					nexusMaxID = lastCommentedNumber;
					nexusMaxSize = (nexusMaxID*3)/2;
					nexus = (unsigned int **)malloc(nexusMaxSize * sizeof(unsigned int *));
					currDictionaryID = 0;
					break;
				case 3: //Prefixes
					//Initialize our prefixes list
					prefixMaxID = lastCommentedNumber;
					prefixMaxSize = (prefixMaxID*3)/2;
					prefix = (unsigned int **)malloc(prefixMaxSize * sizeof(unsigned int *));
					currDictionaryID = 0;
					break;
			}
			continue;
		}
		
		//Step 3-C: Act differently based on the mode we're in
		switch (mode) {
			case 1: //Words
			{
				//Skip until the first number inside the bracket
				while (model_buff[currLineStart] != '[')
					currLineStart++;
				currLineStart++;

				//Keep reading until the terminating bracket.
				//  Each "word" is of the form DD(-DD)*,
				newWordSz = 0;
				for(;;) {
					//Read a "pair"
					currLetter[2] = model_buff[currLineStart++];
					currLetter[3] = model_buff[currLineStart++];

					//Translate/Add this letter
					newWord[newWordSz++] = (unsigned short)strtol(currLetter, NULL, 16);

					//Continue?
					char nextChar = model_buff[currLineStart++];
					if (nextChar == ',' || nextChar == ']') {
						//Double check
						if (numberCheck<10) {
							if (newWordSz!=1 || newWord[0]!=0x1040+numberCheck) {
								printf("Model MUST begin with numbers 0 through 9 (e.g., 1040 through 1049) for reasons of parsimony.\nFound: [%x] at %i", newWord[0], newWordSz);
								return;
							}
							numberCheck++;
						}

						//Finangle & add this word
						dictionary[currDictionaryID] = (unsigned short *)malloc((newWordSz+1) * sizeof(unsigned short));
						dictionary[currDictionaryID][0] = (unsigned short)newWordSz;
						for (int i=0; i<newWordSz; i++) {
							dictionary[currDictionaryID][i+1] = newWord[i];
						}
						currDictionaryID++;

						//Continue?
						if (nextChar == ']')
							break;
						else
							newWordSz = 0;
					}
				}
				break;
			}
			case 2: //Mappings (nexi)
			{
				//Skip until the first letter inside the bracket
				while (model_buff[currLineStart] != '{')
					currLineStart++;
				currLineStart++;

				//A new hashtable for this entry.
				newWordSz=0;
				while (model_buff[currLineStart] != '}') {
					//Read a hashed mapping: character
					int nextInt = 0;
					char nextChar = 0;
					while (model_buff[currLineStart] != ':')
						nextChar = model_buff[currLineStart++];
					currLineStart++;

					//Read a hashed mapping: number
					while (model_buff[currLineStart] != ',' && model_buff[currLineStart] != '}') {
						nextInt *= 10;
						nextInt += (model_buff[currLineStart++] - '0');
					}

					//Add that entry to the hash
					newWord[newWordSz++] = ((nextInt<<8) | (0xFF&nextChar));

					//Continue?
					if (model_buff[currLineStart] == ',')
						currLineStart++;
				}

				//Add this entry to the current vector collection
				nexus[currDictionaryID] = (unsigned int *)malloc((newWordSz+1) * sizeof(unsigned int));
				nexus[currDictionaryID][0] = (unsigned int)newWordSz;
				for (int i=0; i<newWordSz; i++) {
					nexus[currDictionaryID][i+1] = newWord[i];
				}
				currDictionaryID++;

				break;
			}
			case 3: //Prefixes (mapped)
			{
				//Skip until the first letter inside the bracket
				while (model_buff[currLineStart] != '{')
					currLineStart++;
				currLineStart++;

				//A new hashtable for this entry.
				newWordSz = 0;
				int nextVal;
				while (model_buff[currLineStart] != '}') {
					//Read a hashed mapping: number
					nextVal = 0;
					while (model_buff[currLineStart] != ':') {
						nextVal *= 10;
						nextVal += (model_buff[currLineStart++] - '0');
					}
					currLineStart++;

					//Store: key
					newWord[newWordSz++] = nextVal;

					//Read a hashed mapping: number
					nextVal = 0;
					while (model_buff[currLineStart] != ',' && model_buff[currLineStart] != '}') {
						nextVal *= 10;
						nextVal += (model_buff[currLineStart++] - '0');
					}
					//Store: val
					newWord[newWordSz++] = nextVal;

					//Continue
					if (model_buff[currLineStart] == ',')
						currLineStart++;
				}

				//Used to mark our "halfway" boundary.
				lastCommentedNumber = newWordSz;

				//Skip until the first letter inside the square bracket
				while (model_buff[currLineStart] != '[')
					currLineStart++;
				currLineStart++;

				//Add a new vector for these
				while (model_buff[currLineStart] != ']') {
					//Read a hashed mapping: number
					nextVal = 0;
					while (model_buff[currLineStart] != ',' && model_buff[currLineStart] != ']') {
						nextVal *= 10;
						nextVal += (model_buff[currLineStart++] - '0');
					}

					//Add it
					newWord[newWordSz++] = nextVal;

					//Continue
					if (model_buff[currLineStart] == ',')
						currLineStart++;
				}

				//Add this entry to the current vector collection
				prefix[currDictionaryID] = (unsigned int *)malloc((newWordSz+2) * sizeof(unsigned int));
				prefix[currDictionaryID][0] = (unsigned int)lastCommentedNumber/2;
				prefix[currDictionaryID][1] = (unsigned int)(newWordSz - lastCommentedNumber);
				for (int i=0; i<lastCommentedNumber; i++) {
					prefix[currDictionaryID][i+2] = newWord[i];
				}
				for (unsigned int i=0; i<prefix[currDictionaryID][1]; i++) {
					prefix[currDictionaryID][i+lastCommentedNumber+2] = newWord[i+lastCommentedNumber];
				}
				currDictionaryID++;

				break;
			}
			default:
				printf("Too many comments.");
				return;
		}

		//Right-trim
		while (model_buff[currLineStart] != '\n')
			currLineStart++;
		currLineStart++;
	}
	
	//Initialize the model, reclaim memory
	init(dictionary, dictMaxID, dictMaxSize, nexus, nexusMaxID, nexusMaxSize, prefix, prefixMaxID, prefixMaxSize);
	delete [] model_buff;
	
	
	///Now, load the user's custom words (later)
}




/**
 * Construction of a word builder requires three things. All of these are 2-D jagged arrays;
 *   the first entry in each row is the size of that row.
 * NOTE: Please see the alternative constructor for a (usually preferred) helper to load from files.
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
WordBuilder::WordBuilder (unsigned short **dictionary, int dictMaxID, int dictMaxSize, unsigned int **nexus, int nexusMaxID, int nexusMaxSize, unsigned int **prefix, int prefixMaxID, int prefixMaxSize)
{
        //Initialize the model
	init(dictionary, dictMaxID, dictMaxSize, nexus, nexusMaxID, nexusMaxSize, prefix, prefixMaxID, prefixMaxSize);	
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


void WordBuilder::init (unsigned short **dictionary, int dictMaxID, int dictMaxSize, unsigned int **nexus, int nexusMaxID, int nexusMaxSize, unsigned int **prefix, int prefixMaxID, int prefixMaxSize) 
{
	/*printf("dictionary: %i\n", dictMaxID);
	printf("nexus: %i\n", nexusMaxID);
	printf("prefix: %i\n", prefixMaxID);*/
	
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

	//Default:
	this->currEncoding = ENCODING_UNICODE;

	//Cache
	punctHalfStopUni = 0x104A;
	punctFullStopUni = 0x104B;
	punctHalfStopWinInnwa = 63;
	punctFullStopWinInnwa = 47;
	copystr(mostRecentError, L"");

	//Init dictionaries
	winInnwaDictionary = (unsigned short **)malloc(dictMaxSize * sizeof(unsigned short *));
	unicodeDictionary = (unsigned short **)malloc(dictMaxSize * sizeof(unsigned short *));
	for (int i=0; i<dictMaxSize; i++) {
		winInnwaDictionary[i] = (unsigned short *)malloc((1) * sizeof(unsigned short));
		unicodeDictionary[i] = (unsigned short *)malloc((1) * sizeof(unsigned short));
		winInnwaDictionary[i][0] = 0;
		unicodeDictionary[i][0] = 0;
	}

	//Start off
	this->reset(true);
}


unsigned int WordBuilder::getOutputEncoding()
{
	return this->currEncoding;
}

void WordBuilder::setOutputEncoding(unsigned int encoding)
{
	this->currEncoding = encoding;
}



unsigned short WordBuilder::getStopCharacter(bool isFull) 
{
	if (isFull) {
		if (currEncoding==ENCODING_WININNWA)
			return punctFullStopWinInnwa;
		else
			return punctFullStopUni;
	} else {
		if (currEncoding==ENCODING_WININNWA)
			return punctHalfStopWinInnwa;
		else
			return punctHalfStopUni;
	}
}



/**
 * Types a letter and advances the nexus pointer. Returns true if the letter was a valid move,
 *   false otherwise. Updates the available word/letter list appropriately.
 */
bool WordBuilder::typeLetter(char letter) 
{
	//Is this letter meaningful?
	int nextNexus = jumpToNexus(this->currNexus, letter);
	if (nextNexus == -1) {
		//There's a special case: if we are evaluating "g", it might be a shortcut for "aung"
		if (letter!='g') {
			return false;
		} else {
			//Start at "aung" if we haven't already typed "a"
			char test[5];
			strcpy(test, "aung");
			if (pastNexusID==1 && jumpToNexus(pastNexus[pastNexusID-1], 'a')==currNexus) {
				strcpy(test, "ung");
			}
			size_t stLen = strlen(test);

			//Ok, can wet get ALL the way there?
			nextNexus = currNexus;
			for (size_t i=0; i<stLen; i++) {
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
	pastNexus[pastNexusID++] = currNexus;
	currNexus = nextNexus;

	//Update the external state of the WordBuilder
	this->resolveWords();

	//Success
	return true;
}


bool WordBuilder::moveRight(int amt) {
	//Any words?
	if (possibleWords.size()==0)
		return false;

	//Any change?
	int newAmt = currSelectedID + amt;
	if (newAmt >= (int)possibleWords.size())
		newAmt = (int)possibleWords.size()-1;
	else if (newAmt < 0)
		newAmt = 0;
	if (newAmt == currSelectedID)
		return false;

	//Do it!
	currSelectedID = newAmt;
	return true;
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
std::pair<bool, unsigned int> WordBuilder::typeSpace(int quickJumpID) 
{
	//Return val
	std::pair<bool, unsigned int> result(false, 0);

	//We're at a valid stopping point?
	if (this->getPossibleWords().size() == 0)
		return result;

	//Quick jump?
	if (quickJumpID > -1)
		this->setCurrSelected(quickJumpID);
	if (currSelectedID!=quickJumpID && quickJumpID!=-1)
		return result; //No effect

	//Get the selected word, add it to the prefix array
	unsigned int newWord = this->getPossibleWords()[this->currSelectedID];
	addPrefix(newWord);

	//Reset the model, return this word
	this->reset(false);

	result.first = true;
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
	for (unsigned int i=0; i<this->nexus[fromNexus][0]; i++) {
		if ( (this->nexus[fromNexus][i+1]&0xFF) == jumpChar )
			return (this->nexus[fromNexus][i+1]>>8);
	}
	return -1;
}


int WordBuilder::jumpToPrefix(int fromPrefix, int jumpID)
{
	for (unsigned int i=0; i<this->prefix[fromPrefix][0]; i++) {
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
	//Init
	copystr(parenStr, L"");
	int pStrOffset = 0;

	//If there are no words possible, can we jump to a point that doesn't diverge?
	int speculativeNexusID = currNexus;
	while (nexus[speculativeNexusID][0]==1 && ((nexus[speculativeNexusID][1])&0xFF)!='~') {
		//Append this to our string
		parenStr[pStrOffset++] = (nexus[speculativeNexusID][1]&0xFF);

		//Move on this
		speculativeNexusID = ((nexus[speculativeNexusID][1])>>8);
	}
	if (nexus[speculativeNexusID][0]==1 && (nexus[speculativeNexusID][1]&0xFF)=='~') {
		//Finalize our string
		parenStr[pStrOffset] = '\0';
	} else {
		//Reset
		speculativeNexusID = currNexus;
		copystr(parenStr, L"");
	}

	//What possible characters are available after this point?
	int lowestPrefix = -1;
	this->possibleChars.clear();
	for (unsigned int i=0; i<this->nexus[speculativeNexusID][0]; i++) {
		char currChar = (this->nexus[speculativeNexusID][i+1]&0xFF);
		if (currChar == '~')
			lowestPrefix = (this->nexus[speculativeNexusID][i+1]>>8);
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
	for (unsigned int i=0; i<trigramCount; i++) {
		int newHigh = jumpToPrefix(highPrefix, trigram[i]);
		if (newHigh==-1)
			break;
		highPrefix = newHigh;
	}

	//First, put all high-informative entries into the resultant vector
	//  Then, add any remaining low-information entries
	for (unsigned int i=0; i<prefix[highPrefix][1]; i++) {
		possibleWords.push_back(prefix[highPrefix][2+prefix[highPrefix][0]*2+i]);
	}
	if (highPrefix != lowestPrefix) {
		for (unsigned int i=0; i<prefix[lowestPrefix][1]; i++) {
			int val = prefix[lowestPrefix][2+prefix[lowestPrefix][0]*2+i];
			if (!vectorContains(possibleWords, val))
				possibleWords.push_back(val);
		}
	}

	this->currSelectedID = 0;
}


bool WordBuilder::vectorContains(std::vector<unsigned int> vec, unsigned int val)
{
	for (size_t i=0; i<vec.size(); i++) {
		if (vec[i] == val)
			return true;
	}
	return false;
}


void WordBuilder::addPrefix(unsigned int latestPrefix)
{
	//Latest prefixes go in the FRONT
	trigram[2] = trigram[1];
	trigram[1] = trigram[0];
	trigram[0] = latestPrefix;

	if (trigramCount<3)
		trigramCount++;
}


std::vector<char> WordBuilder::getPossibleChars(void)
{
	return this->possibleChars;
}

std::vector<unsigned int> WordBuilder::getPossibleWords(void)
{
	return this->possibleWords;
}

/**
 * The trigram_ids is organized as [0,1,2], where "0" is the most recently-typed word.
 * In the event that no previous words are available (e.g., beginning of a sentence)
 *  set num_used_trigrams to 0. If a unigram/bigram is available, set it to 1 or 2.
 */
void WordBuilder::insertTrigram(unsigned short* trigram_ids, int num_used_trigrams)
{
	trigramCount = num_used_trigrams;
	for (size_t i=0; i<trigramCount; i++) {
		this->trigram[i] = trigram_ids[i];
	}
}

std::vector<unsigned short> WordBuilder::getWordKeyStrokes(unsigned int id) 
{
	//Determine our dictionary
	unsigned short** myDict = dictionary;
	int destFont = Zawgyi_One;
	if (currEncoding==ENCODING_WININNWA) {
		myDict = winInnwaDictionary;
		destFont = WinInnwa;
	} else if (currEncoding==ENCODING_UNICODE) {
		myDict = unicodeDictionary;
		destFont = Myanmar3;
	}

	//Does this word exist in the dictionary? If not, add it
	if (currEncoding!=ENCODING_ZAWGYI && myDict[id][0] == 0) {
		//First, convert
		wchar_t* srcStr = this->getWordString(id);
		wchar_t destStr[200];
		copystr(destStr, L"");
		convertFont(destStr, srcStr, Zawgyi_One, destFont);

		//TEMP: Special cases
		if (currEncoding==ENCODING_WININNWA && compstr(srcStr, L"\u1009\u102C\u1025\u1039")==0) {
			copystr(destStr, L"123");
			destStr[0] = 211;
			destStr[1] = 79;
			destStr[2] = 102;
		} else if (currEncoding==ENCODING_UNICODE && compstr(srcStr, L"\u1031\u101A\u102C\u1000\u1039\u103A\u102C\u1038")==0) {
			copystr(destStr, L"\u101A\u1031\u102C\u1000\u103A\u103B\u102C\u1038");
		} else if (currEncoding==ENCODING_WININNWA && compstr(srcStr, L"\u1015\u102B\u1094")==0) {
			copystr(destStr, L"123");
			destStr[0] = 121;
			destStr[1] = 103;
			destStr[2] = 104;
		}

		//Now, add a new entry
		size_t stLen = lenstr(destStr);
		unsigned short * newEncoding = (unsigned short *)malloc((stLen+1) * sizeof(unsigned short));
		for (size_t i=0; i<stLen; i++) {
			newEncoding[i+1] = destStr[i];
		}
		newEncoding[0] = (int)stLen;
		free(myDict[id]);
		myDict[id] = newEncoding;
	}

	//Set return vector appropriately
	this->keystrokeVector.clear();
	unsigned short size = myDict[id][0];
	for (int i=0; i<size; i++) 
		this->keystrokeVector.push_back(myDict[id][i+1]);

	return this->keystrokeVector;
}


/**
 * Get the remaining letters to type to arrive at the guessed word (if any)
 */
wchar_t* WordBuilder::getParenString()
{
	return this->parenStr;
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
wchar_t* WordBuilder::getWordString(unsigned int id)
{
	copystr(currStr, L"");
	wchar_t temp[50];

	unsigned short size = this->dictionary[id][0];
	for (int i=0; i<size; i++)  {
		printstr(temp, L"%c", this->dictionary[id][i+1]);
		catstr(this->currStr, temp);
	}

	return this->currStr;
}


wchar_t* WordBuilder::getLastError() 
{
	return mostRecentError;
}


bool WordBuilder::addRomanization(wchar_t* myanmar, char* roman) 
{
	//First task: find the word; add it if necessary
	int dictID;
	size_t mmLen = lenstr(myanmar);
	for (dictID=0; dictID<dictMaxID; dictID++) {
		if (mmLen != dictionary[dictID][0])
			continue;

		bool found = true;
		for (int i=0; i<dictionary[dictID][0]; i++) {
			if (dictionary[dictID][i+1] != (unsigned short)myanmar[i]) {
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
		if (dictMaxID == dictMaxSize) {
			copystr(mostRecentError, L"Too many custom words!");
			return false;
		}

		dictionary[dictMaxID] = (unsigned short *)malloc((mmLen+1) * sizeof(unsigned short));
		dictionary[dictMaxID][0] = (int)mmLen;
		for (size_t i=0; i<mmLen; i++) {
			dictionary[dictMaxID][i+1] = (unsigned short)myanmar[i];
		}
		dictMaxID++;
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
				copystr(mostRecentError, L"Too many custom nexi!");
				return false;
			}
			nexus[nexusMaxID] = (unsigned int *)malloc((1) * sizeof(unsigned int));
			nexus[nexusMaxID][0] = 0;

			//Next: copy all old entries into a new array
			int newSizeNex = nexus[currNodeID][0]+2;
			unsigned int * newCurrent = (unsigned int *)malloc((newSizeNex) * sizeof(unsigned int));
			for (size_t i=0; i<nexus[currNodeID][0]+1; i++) {
				newCurrent[i] = nexus[currNodeID][i];
			}
			newCurrent[0] = nexus[currNodeID][0]+1;
			free(nexus[currNodeID]);
			nexus[currNodeID] = newCurrent;
			
			//Finally: add a new entry linking to the nexus we just created
			nexus[currNodeID][newSizeNex-1] = (nexusMaxID<<8) | (0xFF&roman[rmID]);
			nexusMaxID++;
		}

		currNodeID = ((nexus[currNodeID][nextNexusID+1])>>8);
	}


	//Final task: add (just the first) prefix entry.
	size_t currPrefixID;
	for (currPrefixID=0; currPrefixID<nexus[currNodeID][0]; currPrefixID++) {
		if (((nexus[currNodeID][currPrefixID+1])&0xFF) == '~') {
			break;
		}
	}
	if (currPrefixID == nexus[currNodeID][0]) {
		//We need to add a prefix entry
		if (prefixMaxID == prefixMaxSize) {
			copystr(mostRecentError, L"Too many custom prefixes!");
			return false;
		}
		prefix[prefixMaxID] = (unsigned int *)malloc((2) * sizeof(unsigned int));
		prefix[prefixMaxID][0] = 0;
		prefix[prefixMaxID][1] = 0;

		//Next: copy all old entries into a new array
		int newSizeNex = nexus[currNodeID][0]+2;
		unsigned int * newCurrent = (unsigned int *)malloc((newSizeNex) * sizeof(unsigned int));
		for (size_t i=0; i<nexus[currNodeID][0]+1; i++) {
			newCurrent[i] = nexus[currNodeID][i];
		}
		newCurrent[0]++;
		free(nexus[currNodeID]);
		nexus[currNodeID] = newCurrent;
			
		//Finally: add a new entry linking to the nexus we just created
		nexus[currNodeID][newSizeNex-1] = (unsigned int) ((prefixMaxID<<8) | ('~'));
		prefixMaxID++;
	}

	//Translate
	currPrefixID = (nexus[currNodeID][currPrefixID+1])>>8;

	//Does our prefix entry contain this dictionary word?
	size_t wordStart = 2+prefix[currPrefixID][0]*2;
	for (size_t i=0; i<prefix[currPrefixID][1]; i++) {
		if (prefix[currPrefixID][wordStart + i] == dictID) {
			copystr(mostRecentError, L"Word is already in dictionary!");
			return false;
		}
	}

	//Ok, copy it over
	size_t oldSize = wordStart + prefix[currPrefixID][1];
	unsigned int * newPrefix = (unsigned int *)malloc((oldSize+1) * sizeof(unsigned int));
	for (size_t i=0; i<oldSize; i++) {
		newPrefix[i] = prefix[currPrefixID][i];
	}
	newPrefix[1]++;
	newPrefix[oldSize] = dictID;
	free(prefix[currPrefixID]);
	prefix[currPrefixID] = newPrefix;

	return true;
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
