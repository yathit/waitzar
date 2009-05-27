/*
 * Copyright 2007 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "WordBuilder.h"

namespace waitzar 
{



/**
 * Load a model given the Wait Zar binary model and a series of text files containing user additions.
 * @param modelFile is "Myanmar.model"
 * @param userWordsFiles contains several "mywords.txt"-style files.
 * If an exact match (roman+myanmar) is encountered in userWordsFiles[n+1] that was already in userWordsFiles[n], the 
 * newest entry is ignored.
 */	
 WordBuilder::WordBuilder (const char* modelFile, std::vector<std::string> userWordsFiles)
{
	//Load the model
	loadModel(modelFile, userWordsFiles);

	//Initialize it
	initModel();
}
	

/**
 * Load a model given the Wait Zar binary model and a text file containing user additions.
 * @param modelFile is "Myanmar.model"
 * @param userWordsFile is "mywords.txt"
 * If userWordsFile doesn't exist, it is ignored. If modelFile doesn't exist, it
 *  causes unpredictable behavior.
 */
WordBuilder::WordBuilder (const char* modelFilePath, const char* userWordsFilePath)
{
	//Load the model
	std::vector<std::string> oneFile;
	oneFile.push_back(userWordsFilePath);
	loadModel(modelFilePath, oneFile);

	//Initialize it
	initModel();
}


void WordBuilder::loadModel(const char* modelFilePath, vector<string> userWordsFilePaths) 
{
	//Assume the user wants Burmese words only.
	this->restrictToMyanmar = true;

	//Step one: open the model file (ASCII)
	FILE* modelFile = fopen(modelFilePath, "r");
	if (modelFile == NULL) {
		printf("Cannot create WordBuilder; model file does not exist.\n    %s\n", modelFilePath);
		return;
	}

	//Step two: figure out its length and convert it to a character array.
	fseek (modelFile, 0, SEEK_END);
	long modelFileSize = ftell(modelFile);
	rewind(modelFile);
	char * model_buff = new char[modelFileSize];
	size_t model_buff_size = fread(model_buff, 1, modelFileSize, modelFile);
	fclose(modelFile);



	//Delegate to another loading function
	loadModel(model_buff, model_buff_size, !this->restrictToMyanmar);



	//Reclaim memory
	delete [] model_buff;

	//Now, load the user's custom words (optional)
	for (size_t i=0; i<userWordsFilePaths.size(); i++) {
	  FILE* userFile = fopen(userWordsFilePaths[i].c_str(), "rb");
	  if (userFile == NULL) {
	    continue;
	  }

	  //Get file size
	  fseek (userFile, 0, SEEK_END);
	  long fileSize = ftell(userFile);
	  rewind(userFile);

	  //Read it all into an array, close the file.
	  char * buffer = new char[fileSize]; // (char*) malloc(sizeof(char)*fileSize);
	  size_t buff_size = fread(buffer, 1, fileSize, userFile);
	  fclose(userFile);
	  if (buff_size==0) {
	    return; //Empty file.
	  }

	  //Finally, convert this array to unicode
	  wchar_t * uniBuffer = new wchar_t[buff_size];
	  
	  //old:
	  //size_t numUniChars = MultiByteToWideChar(CP_UTF8, 0, buffer, (int)buff_size, NULL, 0);
	  //new:
	  size_t numUniChars = mymbstowcs(NULL, buffer, buff_size);
	  if (buff_size==numUniChars) {
	    wprintf(L"Warning! Conversion to wide-character string of mywords.txt probably failed...\n");
	    return;
	  }

	  uniBuffer = new wchar_t[numUniChars]; // (wchar_t*) malloc(sizeof(wchar_t)*numUniChars);
	  if (mymbstowcs(uniBuffer, buffer, buff_size)==0) {
	    printf("mywords.txt contains invalid UTF-8 characters.\n\nWait Zar will still function properly; however, your custom dictionary will be ignored.");
	    return;
	  }
	  delete [] buffer;

	  //Skip the BOM, if it exists
	  size_t currPosition = 0;
	  if (uniBuffer[currPosition] == 0xFEFF)
	    currPosition++;
	  else if (uniBuffer[currPosition] == 0xFFFE) {
	    printf("mywords.txt appears to be backwards. You should fix the Unicode encoding using Notepad or another Windows-based text utility.\n\nWait Zar will still function properly; however, your custom dictionary will be ignored.");
	    return;
	  }

	  //Read each line
	  wchar_t* name = new wchar_t[100];
	  char* value = new char[100];
	  while (currPosition<numUniChars) {
	    //Get the name/value pair using our nifty template function....
		  readLine(uniBuffer, currPosition, numUniChars, true, true, false, !this->restrictToMyanmar, true, false, false, false, name, value);

	    //Make sure both name and value are non-empty
	    if (strlen(value)==0 || wcslen(name)==0)
	      continue;
	    
	    //Add this romanization
	    if (!this->addRomanization(name, value, true)) {
	      printf("Error adding Romanisation");
	    }
	  }

	  delete [] uniBuffer;
	  delete [] name;
	  delete [] value;
      }
}


WordBuilder::WordBuilder(char *model_buff, size_t model_buff_size, bool allowAnyChar)
{
	//Load the model
	loadModel(model_buff, model_buff_size, allowAnyChar);

	//Initialize it
	initModel();
}


void WordBuilder::loadModel(char *model_buff, size_t model_buff_size, bool allowAnyChar)
{
	//The user has passed in the option to use any character or only myanmar ones
	this->restrictToMyanmar = !allowAnyChar;

	//Step three: Read each line
	size_t currLineStart = 0;
	unsigned short count;
	unsigned short lastCommentedNumber;
	unsigned short mode = 0;
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
					//Avoid un-necessary resizing by reserving space in our vector.
					dictionary.reserve(lastCommentedNumber);

					break;
				case 2: //Nexi
					//Avoid un-necessary resizing by reserving space in our vector.
					nexus.reserve(lastCommentedNumber);

					break;
				case 3: //Prefixes
					//Avoid un-necessary resizing by reserving space in our vector.
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
				while (model_buff[currLineStart] != '[')
					currLineStart++;
				currLineStart++;

				//Keep reading until the terminating bracket.
				//  Each "word" is of the form DD(-DD)*,
				//newWordSz = 0;
				//wstring newWord;
				wchar_t newWordCstr[150];
				size_t newWordPos = 0;
				for(;;) {
					//Read a "pair"
					currLetter[2] = model_buff[currLineStart++];
					currLetter[3] = model_buff[currLineStart++];

					//Translate/Add this letter
					wchar_t nextLetter = (wchar_t)strtol(currLetter, NULL, 16);
					newWordCstr[newWordPos++] = nextLetter;

					//Continue?
					char nextChar = model_buff[currLineStart++];
					if (nextChar == ',' || nextChar == ']') {
						//Double check
						if (numberCheck<10) {
							if (newWordPos!=1 || newWordCstr[0]!=0x1040+numberCheck) {
								printf("Model MUST begin with numbers 0 through 9 (e.g., 1040 through 1049) for reasons of parsimony.\nFound: [%x] at %i", newWordCstr[0], newWordPos);
								return;
							}
							numberCheck++;
						}

						//Add this word to the dictionary (copy semantics)
						newWordCstr[newWordPos++] = 0x0000;
						dictionary.push_back(newWordCstr);
						newWordPos = 0;

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
				while (model_buff[currLineStart] != '{')
					currLineStart++;
				currLineStart++;

				//A new hashtable for this entry.
				nexus.push_back(vector<unsigned int>());
				vector<unsigned int> &newWord = nexus[nexus.size()-1];
				//newWord.reserve(150);
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
					newWord.push_back(((nextInt<<8) | (0xFF&nextChar)));

					//Continue?
					if (model_buff[currLineStart] == ',')
						currLineStart++;
				}

				break;
			}
			case 3: //Prefixes (mapped)
			{
				//Skip until the first letter inside the bracket
				while (model_buff[currLineStart] != '{')
					currLineStart++;
				currLineStart++;

				//A new hashtable for this entry.
				//newWordSz = 0;
				prefix.push_back(vector<unsigned int>());
				vector<unsigned int> &newWord = prefix[prefix.size()-1];
				//newWord.reserve(150);
				//Reserve a spot for our "halfway" marker
				newWord.push_back(0);
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
					newWord.push_back(nextVal);

					//Read a hashed mapping: number
					nextVal = 0;
					while (model_buff[currLineStart] != ',' && model_buff[currLineStart] != '}') {
						nextVal *= 10;
						nextVal += (model_buff[currLineStart++] - '0');
					}
					//Store: val
					newWord.push_back(nextVal);

					//Continue
					if (model_buff[currLineStart] == ',')
						currLineStart++;
				}

				//Used to mark our "halfway" boundary.
				lastCommentedNumber = newWord.size();

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
					newWord.push_back(nextVal);

					//Continue
					if (model_buff[currLineStart] == ',')
						currLineStart++;
				}

				//Set the halfway marker
				newWord[0] = lastCommentedNumber/2;

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

	//Initialize the model
	//init(dictionary, dictMaxID, dictMaxSize, nexus, nexusMaxID, nexusMaxSize, prefix, prefixMaxID, prefixMaxSize);
}




/**
 * Construction of a word builder requires three things. All of these are 2-D jagged arrays;
 *   the first entry in each row is the size of that row. Prefixes have two size entries.
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
WordBuilder::WordBuilder(const vector<wstring> &dictionary, const vector< vector<unsigned int> > &nexus, const vector< vector<unsigned int> > &prefix)
{
    //Load the model
	loadModel(dictionary, nexus, prefix);

	//Initialize it
	initModel();
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
	//Although, it should probably be virtual if WordBuilder is ever overloaded and stored in a vector...
}
void WordBuilder::loadModel(const vector<wstring> &dictionary, const vector< vector<unsigned int> > &nexus, const vector< vector<unsigned int> > &prefix)
{
	//Assume the user only wants Burmese words
	this->restrictToMyanmar = true;

	//Store for later. 
	// We copy values in manually for now, since I'm not good with the constructor post-scripting notation
	this->dictionary = dictionary;
	this->nexus = nexus;
	this->prefix = prefix;
}


/** 
 * All constructors pass through this function before returning.
 *   It's ok to call this twice, by mistake or design.
 */
void WordBuilder::initModel()
{
	//Start with our reverse lookup off by default
	revLookupOn = false;

	//Initialize our strings
	//wcscpy(parenStr, L"");
	//wcscpy(postStr, L"");
	//wcscpy(mostRecentError, L"");

	//Set the default encoding
	this->currEncoding = ENCODING_UNICODE;

	//Cache full/half stop values
	punctHalfStopUni = 0x104A;
	punctFullStopUni = 0x104B;
	punctHalfStopWinInnwa = 63;
	punctFullStopWinInnwa = 47;

	//Initialize our dictionaries to null entries
	winInnwaDictionary.assign(dictionary.size(), wstring());
	unicodeDictionary.assign(dictionary.size(), wstring());

	//Start off
	this->reset(true);
}



ENCODING WordBuilder::getOutputEncoding()
{
	return this->currEncoding;
}

void WordBuilder::setOutputEncoding(ENCODING encoding)
{
	this->currEncoding = encoding;
}


unsigned int WordBuilder::getTotalDefinedWords()
{
	return dictionary.size();
}


//Given baseword + tostack = resultstacked, add this shortcut to our own internal storage
//return false in error
bool WordBuilder::addShortcut(const wstring &baseWord, const wstring &toStack, const wstring &resultStacked)
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
		mostRecentError = msg.str();
		return false;
	}

	//For now (assuming no collisions, which is a bit broad of us) we need to say that from
	//  any given nexus that has toStackID in the base set, we set an "if,then" clause;
	//  namely, if baseWordID is the previous trigram entry, then resultStacked is the word of choice.
	//There aren't many pat-sint words, so an ordered list of some sort (or, I guess, a map) should do the
	// trick.
	for (unsigned int toStackNexusID = 0; toStackNexusID<nexus.size(); toStackNexusID++) {
		//Is this nexus ID valid?
		vector<unsigned int> &thisNexus = nexus[toStackNexusID];
		int prefixID = 0;
		bool considerThisNexus = false;
		for (unsigned int x=0; x<thisNexus.size(); x++) {
			//Is this the resolving letter?
			char letter = (char)(0xFF&thisNexus[x]);
			if (letter!='~')
				continue;

			//Ok, save it
			prefixID = thisNexus[x]>>8;
			break;
		}
		if (prefixID!=0) {
			//Now, test this prefix to see if its base pair contains our word in question.
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
			mostRecentError = msg.str();
			return false;
		}

		//Add this nexus
		shortcuts[toStackNexusID][baseWordID] = resultStackedID;
	}

	return true;
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
			if (pastNexus.size()==1 && jumpToNexus(pastNexus[pastNexus.size()-1], 'a')==currNexus) {
				strcpy(test, "ung");
			}
			size_t stLen = strlen(test);

			//Ok, can we get ALL the way there?
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
	pastNexus.push_back(currNexus);
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
	if (pastNexus.empty())
		return false;

	currNexus = pastNexus[pastNexus.size()-1];
	pastNexus.pop_back();
	this->resolveWords();

	if (pastNexus.empty())
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
pair<bool, unsigned int> WordBuilder::typeSpace(int quickJumpID)
{
	//We're at a valid stopping point?
	if (this->getPossibleWords().size() == 0)
		return pair<bool, unsigned int>(false, 0);

	//Quick jump?
	if (quickJumpID > -1)
		this->setCurrSelected(quickJumpID);
	if (currSelectedID!=quickJumpID && quickJumpID!=-1)
		return pair<bool, unsigned int>(false, 0); //No effect

	//Get the selected word, add it to the prefix array
	unsigned int newWord = this->getPossibleWords()[this->currSelectedID];
	addPrefix(newWord);

	//Reset the model, return this word
	this->reset(false);
	return pair<bool, unsigned int>(true, newWord);
}


/**
 * Reset the builder to its zero-state
 * @param fullReset If true, remove all prefixes in memory. (Called when Alt+Shift is detected.)
 */
void WordBuilder::reset(bool fullReset)
{
	//Partial reset
	this->currNexus = 0;
	this->pastNexus.clear();
	this->currSelectedID = -1;
	this->possibleChars.clear();
	this->possibleWords.clear();
	this->parenStr.clear();
	this->postStr.clear();

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
	for (unsigned int i=0; i<this->nexus[fromNexus].size(); i++) {
		if ( (this->nexus[fromNexus][i]&0xFF) == jumpChar )
			return (this->nexus[fromNexus][i]>>8);
	}
	return -1;
}


int WordBuilder::jumpToPrefix(int fromPrefix, int jumpID)
{
	for (unsigned int i=0; i<this->prefix[fromPrefix].size(); i++) {
		if ( this->prefix[fromPrefix][i*2] == jumpID )
			return this->prefix[fromPrefix][i*2+1];
	}
	return -1;
}


/**
 * Based on the current nexus, what letters are valid moves, and what words
 *   should we present to the user?
 */
void WordBuilder::resolveWords()
{
	//Init
	parenStr.clear();
	postStr.clear();
	int pStrOffset = 0;

	//If there are no words possible, can we jump to a point that doesn't diverge?
	int speculativeNexusID = currNexus;
	while (nexus[speculativeNexusID].size()==1 && ((nexus[speculativeNexusID][0])&0xFF)!='~') {
		//Append this to our string
		parenStr[pStrOffset++] = (nexus[speculativeNexusID][0]&0xFF);

		//Move on this
		speculativeNexusID = ((nexus[speculativeNexusID][0])>>8);
	}
	if (nexus[speculativeNexusID].size()==1 && (nexus[speculativeNexusID][0]&0xFF)=='~') {
		//Finalize our string
		parenStr[pStrOffset] = 0x0000;
	} else {
		//Reset
		speculativeNexusID = currNexus;
		parenStr.clear();
	}

	//What possible characters are available after this point?
	int lowestPrefix = -1;
	this->possibleChars.clear();
	for (unsigned int i=0; i<this->nexus[speculativeNexusID].size(); i++) {
		char currChar = (this->nexus[speculativeNexusID][i]&0xFF);
		if (currChar == '~')
			lowestPrefix = (this->nexus[speculativeNexusID][i]>>8);
		else
			this->possibleChars.push_back(currChar);
	}

	//What words are possible given this point?
	possibleWords.clear();
	this->currSelectedID = -1;
	this->postStr[0] = 0x0000;
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
	vector<unsigned int>::iterator possWord = prefix[highPrefix].begin();
	std::advance(possWord, prefix[highPrefix][0]*2+1);
	for (; possWord!=prefix[highPrefix].end(); possWord++) {
		possibleWords.push_back(*possWord);
	}
	if (highPrefix != lowestPrefix) {
		possWord = prefix[lowestPrefix].begin();
		std::advance(possWord, prefix[lowestPrefix][0]*2+1);
		for (; possWord!=prefix[lowestPrefix].end(); possWord++) {
			if (!vectorContains(possibleWords, *possWord))
				possibleWords.push_back(*possWord);
		}
	}

	//Finally, check if this nexus and the previously-typed word lines up; if so, we have a "post" match
	if (shortcuts.count(currNexus)>0 && trigramCount>0) {
		if (shortcuts[currNexus].count(trigram[0])>0) {
			postStr = getWordString(shortcuts[currNexus][trigram[0]]);
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


wstring WordBuilder::getWordKeyStrokes(unsigned int id)
{
	return this->getWordKeyStrokes(id, this->getOutputEncoding());
}


wstring WordBuilder::getWordKeyStrokes(unsigned int id, unsigned int encoding)
{
	//Determine our dictionary
	vector<wstring> &myDict = dictionary;
	int destFont = Zawgyi_One;
	if (this->restrictToMyanmar) {
		if (encoding==ENCODING_WININNWA) {
			myDict = winInnwaDictionary;
			destFont = WinInnwa;
		} else if (encoding==ENCODING_UNICODE) {
			myDict = unicodeDictionary;
			destFont = Myanmar3;
		}
	}

	//Does this word exist in the dictionary? If not, add it
	if (encoding!=ENCODING_ZAWGYI && myDict[id].size() == 0) {
		//First, convert
		const wchar_t* srcStr = this->getWordString(id).c_str();
		wchar_t destStr[200];
		wcscpy(destStr, L"");
		convertFont(destStr, srcStr, Zawgyi_One, destFont);

		//TEMP: Special cases
		if (encoding==ENCODING_WININNWA && wcscmp(srcStr, L"\u1009\u102C\u1025\u1039")==0) {
			wcscpy(destStr, L"123");
			destStr[0] = 211;
			destStr[1] = 79;
			destStr[2] = 102;
		} else if (encoding==ENCODING_UNICODE && wcscmp(srcStr, L"\u1031\u101A\u102C\u1000\u1039\u103A\u102C\u1038")==0) {
			wcscpy(destStr, L"\u101A\u1031\u102C\u1000\u103A\u103B\u102C\u1038");
		} else if (encoding==ENCODING_WININNWA && wcscmp(srcStr, L"\u1015\u102B\u1094")==0) {
			wcscpy(destStr, L"123");
			destStr[0] = 121;
			destStr[1] = 103;
			destStr[2] = 104;
		} else if (encoding==ENCODING_UNICODE && wcscmp(srcStr, L"104E")==0) {
			//New encoding for "lakaung"
			wcscpy(destStr, L"1234");
			destStr[0] = 0x104E;
			destStr[1] = 0x1004;
			destStr[2] = 0x103A;
			destStr[3] = 0x1038;
		}

		//Now, add a new entry
		//size_t stLen = wcslen(destStr);
		//wstring newEncoding(destStr);
		//for (size_t i=0; i<stLen; i++) {
		//	newEncoding.push_back(destStr[i]);
			//wprintf(L"   test: %x   %x  %x\n", newEncoding[i+1], destStr[i], srcStr[i]);
		//}
		myDict[id] = wstring(destStr);
	}

	//Return-by-value should copy the vector
	return myDict[id];
}


/**
 * Get the remaining letters to type to arrive at the guessed word (if any)
 */
wstring WordBuilder::getParenString()
{
	return this->parenStr;
}


wstring WordBuilder::getPostString()
{
	return this->postStr;
}


unsigned int WordBuilder::getPostID()
{
	return this->getWordID(this->getPostString());
}

bool WordBuilder::hasPostStr()
{
	return this->postStr[0] != 0x0000;
}



/**
 * Get the actual characters in a string (by ID). Note that
 *  this returns a single (global) object, so if multiple strings are to be
 *  dealt with, they must be copied into local variables -something like:
 *    wchar_t *temp1 = wb->getWordString(1);
 *    wchar_t *temp2 = wb->getWordString(2);
 *    wchar_t *temp3 = wb->getWordString(3);
 * ...will FAIL; temp1, temp2, and temp3 will ALL have the value of string 3.
 * Do something like this instead:
 *   wchar_t temp1[50];
 *   lstrcpy(temp1, wb->getWordString(1));
 *   wchar_t temp2[50];
 *   lstrcpy(temp2, wb->getWordString(2));
 *   ...etc.
 */
wstring WordBuilder::getWordString(unsigned int id)
{
	this->currStr = this->dictionary[id];

	//Try with for_each later?

	//for (size_t i=0; i<this->dictionary[id].size(); i++)  {
	//	this->currStr[i] = this->dictionary[id][i];
	//}
	//this->currStr[this->dictionary[id].size()] = 0x0000; //Note: must be full-width 0000, lest the string not be properly terminated.

	return this->currStr;
}



//Allows us to look-up a word and get its romanisation
void WordBuilder::buildReverseLookup()
{
	//Allocate space
	revLookup.assign(dictionary.size(), string());

	//Now, we have to jump down out nexus list one-by-one. 
	// I'd like to avoid recursion with this one.
	vector<unsigned int> nextNexi;
	vector<unsigned int> currNexi;
	currNexi.push_back(0);

	//Initialize our list of nexi-strings-to-date
	vector<string> nexiStrings(nexus.size(), string());

	//Now, "walk" down our list of nexi, appending as we go.
	char letter = 0x0;
	while (!currNexi.empty()) {
		//Deal with all nexi on this level.
		for (size_t i=0; i<currNexi.size(); i++) {
			const vector<unsigned int> &thisNexus = nexus[currNexi[i]];
			for (unsigned int x=0; x<thisNexus.size(); x++) {
				int jmpToID = thisNexus[x]>>8;
				letter = (char)(0xFF&thisNexus[x]);
				if (letter != '~') {
					nexiStrings[jmpToID] += nexiStrings[currNexi[i]];
					nexiStrings[jmpToID] += letter;

					nextNexi.push_back(jmpToID);
				}
			}
		}

		//Prepare for the next level
		currNexi.clear();
		currNexi.insert(currNexi.end(), nextNexi.begin(), nextNexi.end());
		nextNexi.clear();
	}

	//Finally, take our allocated strings and assign them to dictionary values (or delete them)
	// Note that we will avoid duplicate strings to save space. 
	for (size_t i=0; i<nexus.size(); i++) {
		//Find out if this has a prefix entry
		int prefixID = -1;
		for (unsigned int x=0; x<nexus[i].size(); x++) {
			if ((char)(0xFF&nexus[i][x])=='~') {
				prefixID = nexus[i][x]>>8;
				break;
			}
		}

		//If there's no entries, just clear this string
		if (prefixID == -1) {
			nexiStrings[i] = "";
			continue;
		}

		//Else, reference this string in every prefix this refers to.
		//  Note that we only have to check prefix level zero, which by 
		//  definition contains every prefix.
		vector<unsigned int>::iterator currWord = prefix[prefixID].begin();
		std::advance(currWord, prefix[prefixID][0]*2+1);
		for (; currWord!=prefix[prefixID].end(); currWord++) {
			if (*currWord>=0 && *currWord<dictionary.size()) {
				revLookup[*currWord] = string();
				for (size_t q=0; q<nexiStrings[i].size(); q++)
					revLookup[*currWord].push_back(nexiStrings[i][q]);
			}
		}
	}

	nexiStrings.clear();
	revLookupOn = true;
}



string WordBuilder::reverseLookupWord(unsigned int dictID)
{
	if (!revLookupOn)
		buildReverseLookup();

	return revLookup.at(dictID);
}



wstring WordBuilder::getLastError()
{
	return mostRecentError;
}


bool WordBuilder::addRomanization(const wstring &myanmar, const string &roman)
{
  return this->addRomanization(myanmar, roman, false);
}

//returns dictionary.size()
unsigned int WordBuilder::getWordID(const wstring &wordStr)
{
	//size_t mmLen = wordStr.length();
	for (size_t canID=0; canID<dictionary.size(); canID++) {
		if (wordStr == dictionary[canID])
			return canID;

		//Easy check: different lengths
		/*if (mmLen != dictionary[canID].size())
			continue;

		//Complex check: different letters
		bool found = true;
		for (size_t i=0; i<dictionary[canID].size(); i++) {
			if (dictionary[canID][i] != (unsigned short)wordStr[i]) {
				found = false;
				break;
			}
		}

		//Does it match?
		if (found)
			return canID;*/
	}

	//Not found
	return dictionary.size();
}

bool WordBuilder::addRomanization(const wstring &myanmar, const string &roman, bool ignoreDuplicates)
{
	//First task: find the word; add it if necessary
	int dictID = getWordID(myanmar);
	size_t mmLen = myanmar.length();
	if (dictID==dictionary.size()) {
		//Need to add... we DO have a limit, though.
		if (dictionary.size() == std::numeric_limits<size_t>::max()) {
			mostRecentError = L"Too many custom words!";
			return false;
		}

		/*wstring newWord;
		for (size_t i=0; i<mmLen; i++) {
			newWord.push_back((unsigned short)myanmar[i]);
		}*/
		dictionary.push_back(myanmar);
	}


	//Update the reverse lookup?
	if (revLookupOn) {
		/*vector<char> newRoman;
		for (size_t q=0; q<strlen(roman); q++) 
			newRoman.push_back(roman[q]);*/
		revLookup.push_back(roman);
	}


	//Next task: add the romanized mappings
	size_t currNodeID = 0;
	size_t romanLen = roman.length();
	for (size_t rmID=0; rmID<romanLen; rmID++) {
		//Does a path exist from our current node to the next step?
		size_t nextNexusID;
		for (nextNexusID=0; nextNexusID<nexus[currNodeID].size(); nextNexusID++) {
			if (((nexus[currNodeID][nextNexusID])&0xFF) == roman[rmID]) {
				break;
			}
		}
		if (nextNexusID==nexus[currNodeID].size()) {
			//First step: make a blank nexus entry at the END of this list
			if (nexus.size() == std::numeric_limits<size_t>::max()) {
				mostRecentError = L"Too many custom nexi!";
				return false;
			}
			nexus.push_back(vector<unsigned int>());

			//Now, link to this from the current nexus list.
			nexus[currNodeID].push_back((nextNexusID<<8) | (0xFF&roman[rmID]));
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
		if (prefix.size() == std::numeric_limits<size_t>::max()) {
			mostRecentError = L"Too many custom prefixes!";
			return false;
		}
		prefix.push_back(vector<unsigned int>(1, 0));

		//Now, point the nexus to this entry
		nexus[currNodeID].push_back((unsigned int) ((currPrefixID<<8) | ('~')));
	}

	//Translate
	currPrefixID = (nexus[currNodeID][currPrefixID])>>8;

	//Does our prefix entry contain this dictionary word?
	vector<unsigned int>::iterator currWord = prefix[currPrefixID].begin();
	std::advance(currWord, prefix[currPrefixID][0]*2+1);
	for (; currWord!=prefix[currPrefixID].end(); currWord++) {
		if (*currWord == dictID) {
			if (!ignoreDuplicates) {
				mostRecentError = L"Word is already in dictionary at ID: " + *currWord;
			  return false;
			} else {
			  return true;
			}
		}
	}

	//Ok, copy it over
	prefix[currPrefixID].push_back(dictID);

	return true;
}




//Return 0: error
// This follows RFC 3629's recommendations, although it is not strictly compliant.
size_t mymbstowcs(wchar_t *dest, const char *src, size_t maxCount)
{
	size_t lenStr = maxCount;
	size_t destIndex = 0;
	if (lenStr==0) {
		lenStr = strlen(src);
	}
	for (unsigned int i=0; i<lenStr; i++) {
		unsigned short curr = (src[i]&0xFF);

		//Handle carefully to avoid the security risk...
		if (((curr>>3)^0x1E)==0) {
			//We can't handle anything outside the BMP
			return 0;
		} else if (((curr>>4)^0xE)==0) {
			//Verify the next two bytes
			if (i>=lenStr-2 || (((src[i+1]&0xFF)>>6)^0x2)!=0 || (((src[i+2]&0xFF)>>6)^0x2)!=0)
				return 0;

			//Combine all three bytes, check, increment
			wchar_t destVal = 0x0000 | ((curr&0xF)<<12) | ((src[i+1]&0x3F)<<6) | (src[i+2]&0x3F);
			if (destVal>=0x0800 && destVal<=0xFFFF) {
				destIndex++;
				i+=2;
			} else
				return 0;

			//Set
			if (dest!=NULL)
				dest[destIndex-1] = destVal;
		} else if (((curr>>5)^0x6)==0) {
			//Verify the next byte
			if (i>=lenStr-1 || (((src[i+1]&0xFF)>>6)^0x2)!=0)
				return 0;

			//Combine both bytes, check, increment
			wchar_t destVal = 0x0000 | ((curr&0x1F)<<6) | (src[i+1]&0x3F);
			if (destVal>=0x80 && destVal<=0x07FF) {
				destIndex++;
				i++;
			} else
				return 0;

			//Set
			if (dest!=NULL)
				dest[destIndex-1] = destVal;
		} else if ((curr>>7)==0) {
			wchar_t destVal = 0x0000 | curr;
			destIndex++;

			//Set
			if (dest!=NULL)
				dest[destIndex-1] = destVal;
		} else {
			return 0;
		}
	}

	return destIndex;
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
