/*
 * Copyright 2007 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "WordBuilder.h"


//I prefer to only shorthand STL components I use a lot, 
// rather than the whole namespace.
using std::string;
using std::wstringstream;
using std::wstring;
using std::vector;
using std::pair;
using std::map;


namespace waitzar 
{


/**
 * Empty constructor. Intended only to allow use of WordBuilder by value (not reference)
 */
WordBuilder::WordBuilder() {}


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
	unsigned short lastCommentedNumber;
	unsigned short mode = 0;

	//For global usage; should be faster
	wstring newWord;
	newWord.reserve(150);
	vector<unsigned int> newArr;
	newArr.reserve(150);


	while (currLineStart < model_buff_size) {
		//Step 3-A: left-trim whitespace
		while (model_buff[currLineStart]==' ' || model_buff[currLineStart]=='\t')
			currLineStart++;
		if (currLineStart >= model_buff_size) break; //NOTE: Re-write the loop later to fix this

		//Step 3-B: Deal with coments and empty lines
		if (model_buff[currLineStart] == '\n') {
			//Step 3-B-i: Skip comments
			currLineStart++;
			continue;
		} else if (model_buff[currLineStart] == '#') {
			//Step 3-B-ii: Handle comments (find the number which is commented out)
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
				while (model_buff[currLineStart] != '[')
					currLineStart++;
				currLineStart++;

				//Keep reading until the terminating bracket.
				//  Each "word" is of the form DD(-DD)*,
				newWord.clear();
				for(;;) {
					//Read a "pair", add this letter
					unsigned int currLetter = 0x1000;
					currLetter |= (toHex(model_buff[currLineStart++])<<4);
					currLetter |= (toHex(model_buff[currLineStart++]));
					newWord += (wchar_t)currLetter;

					//Continue?
					char nextChar = model_buff[currLineStart++];
					if (nextChar == ',' || nextChar == ']') {
						//Double check our digits
						if (dictionary.size() < 10) {
							if (newWord.length()!=1 || newWord[0] != 0x1040+dictionary.size()) {
								wstringstream msg;
								msg << "Model MUST begin with numbers 0 through 9 (e.g., 1040 through 1049) for reasons"
									<< " of parsimony.\nFound: [" <<(int)newWord[0]  <<"] at " <<newWord.length();
								mostRecentError = msg.str();
								return;
							}

							//Save the ID of this numeral
							//For now, this won't work for ASCII
							cachedNumerals.push_back(std::pair<bool, unsigned short>(true, dictionary.size()));
						}

						//Add this word to the dictionary (copy semantics)
						dictionary.push_back(newWord);
						newWord.clear();

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
				newArr.clear();
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
					newArr.push_back(((nextInt<<8) | (0xFF&nextChar)));

					//Continue?
					if (model_buff[currLineStart] == ',')
						currLineStart++;
				}

				//Push-back copies and shrinks the array.
				nexus.push_back(newArr);

				break;
			}
			case 3: //Prefixes (mapped)
			{
				//Skip until the first letter inside the bracket
				while (model_buff[currLineStart] != '{')
					currLineStart++;
				currLineStart++;

				//A new hashtable for this entry.
				newArr.clear();
				//Reserve a spot for our "halfway" marker
				newArr.push_back(0);
				while (model_buff[currLineStart] != '}') {
					//Read a hashed mapping: number
					unsigned int nextVal = 0;
					while (model_buff[currLineStart] != ':') {
						nextVal *= 10;
						nextVal += (model_buff[currLineStart++] - '0');
					}
					currLineStart++;

					//Store: key
					newArr.push_back(nextVal);

					//Read a hashed mapping: number
					nextVal = 0;
					while (model_buff[currLineStart] != ',' && model_buff[currLineStart] != '}') {
						nextVal *= 10;
						nextVal += (model_buff[currLineStart++] - '0');
					}
					//Store: val
					newArr.push_back(nextVal);

					//Continue
					if (model_buff[currLineStart] == ',')
						currLineStart++;
				}

				//Used to mark our "halfway" boundary.
				lastCommentedNumber = newArr.size()-1; //-1 for the first placeholder value

				//Skip until the first letter inside the square bracket
				while (model_buff[currLineStart] != '[')
					currLineStart++;
				currLineStart++;

				//Add a new vector for these
				while (model_buff[currLineStart] != ']') {
					//Read a hashed mapping: number
					unsigned int nextVal = 0;
					while (model_buff[currLineStart] != ',' && model_buff[currLineStart] != ']') {
						nextVal *= 10;
						nextVal += (model_buff[currLineStart++] - '0');
					}

					//Add it
					newArr.push_back(nextVal);

					//Continue
					if (model_buff[currLineStart] == ',')
						currLineStart++;
				}

				//Set the halfway marker to the number of PAIRS
				newArr[0] = lastCommentedNumber/2;

				//Copy and shrink & store
				prefix.push_back(newArr);

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
}




/**
 * Construction of a word builder requires three things:
 *   (NOTE: Please see the alternative constructor for a (usually preferred) helper to load from files.)
 * @param dictionary is a vector of wide character strings. For example:
 *     {"ka", "ko", "sa"} becomes {L"\u1000", L"\u1000\u102D\u102F", L"\u1005\u1032"}
 * @param nexus is a vector of links. Each integer following is stored as:
 *     0xYY...YYZZ (the number of Ys depends on the width of an integer on the current platform)
 *     Z is the index of the nexus to jump to if character Y is present, EXCEPT when Y is '~', in which case
 *     Z represents the index of the prefix to jump to for word resolution.
 * @param prefix is a vector of links, similar to nexus, followed by a vector of resolutions. The first entry
 *     in this vector is the number of link PAIRS in the first half of the array. So, if we have:
 *     prefix = [5, ..., ], prefix.size()==23
 *     ...then we know that 5 pairs (10 values) follow the "5", and that the remaining 23-1-10==12 values
 *     are resolutions.
 *     The first item in each pair is the "key", and is an id into the dictionary list. If that "key" is a 
 *     prefix word of the current word, then the "value" is an index into the prefix array to jump to.
 *     Each resolution following the pairs is an index into the dictionary array; each match is a potential word.
 *     The "lowest" prefix entry (the one referred to by '~' in the nexus list) MUST contain all possilbe matches;
 *     any subsequent prefix entries should merely present the reordered sequences and let the lowest entry provide
 *     "all the rest". See the resolveWords() function for an example of this in action.
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

	//Reserve space, to avoid a slow startup
	this->dictionary.reserve(2000);
	this->nexus.reserve(4000);
	this->prefix.reserve(3000);
}


/** 
 * All constructors pass through this function before returning.
 *   It's ok to call this twice, by mistake or design.
 */
void WordBuilder::initModel()
{
	//Start with our reverse lookup off by default
	revLookupOn = false;

	//Set the default encoding
	this->currEncoding = ENCODING_UNICODE;

	//Cache full/half stop values
	punctHalfStopUni = 0x104A;
	punctFullStopUni = 0x104B;
	punctHalfStopWinInnwa = 63;
	punctFullStopWinInnwa = 47;

	//Initialize our dictionaries to null entries
	if (winInnwaDictionary.size() != dictionary.size())
		winInnwaDictionary.assign(dictionary.size(), wstring());
	if (unicodeDictionary.size() != dictionary.size())
		unicodeDictionary.assign(dictionary.size(), wstring());

	//Avoid crashing.
	if (cachedNumerals.size()==0) {
		for (int i=0; i<10; i++)
			cachedNumerals.push_back(pair<bool, unsigned short>(false, 0));
	}

	//Start off
	this->reset(true);
}



ENCODING WordBuilder::getOutputEncoding() const
{
	return this->currEncoding;
}

void WordBuilder::setOutputEncoding(ENCODING encoding)
{
	this->currEncoding = encoding;
}


unsigned int WordBuilder::getTotalDefinedWords() const
{
	return dictionary.size();
}


bool WordBuilder::isAllowNonBurmese()
{
	return !this->restrictToMyanmar;
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
			mostRecentError = msg.str();
			return false;
		}

		//Add this nexus
		shortcuts[toStackNexusID][baseWordID] = resultStackedID;
	}

	return true;
}



unsigned short WordBuilder::getStopCharacter(bool isFull) const
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


//Convert 0..9 to an ID.
unsigned short WordBuilder::getSingleDigitID(unsigned short arabicNumeral)
{
	//Return the word. (Returning 0 is a somewhat unsightly alternative, but at least it won't crash.)
	if (arabicNumeral>=0 && arabicNumeral<=9)
		return cachedNumerals[arabicNumeral].second;
	return 0;
}



/**
 * Types a letter and advances the nexus pointer. Returns true if the letter was a valid move,
 *   false otherwise. Updates the available word/letter list appropriately.
 */
bool WordBuilder::typeLetter(char letter, bool isUpper)
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
	int newAmt = currSelectedAbsoluteID + amt;
	if (newAmt >= (int)possibleWords.size())
		newAmt = (int)possibleWords.size()-1;
	else if (newAmt < 0)
		newAmt = 0;
	if (newAmt == currSelectedAbsoluteID)
		return false;

	//Do it!
	currSelectedAbsoluteID = newAmt;

	//Auto-page
	currSelectedPage = currSelectedAbsoluteID / 10;

	return true;
}


//Returns the RELATIVE id. If negative, this represents a pat-sint word (theoretically, could be <-1, but
//   for now we limit this).
int WordBuilder::getCurrSelectedID() const {
	return currSelectedAbsoluteID - firstRegularWordIndex;
}

bool WordBuilder::hasPatSintWord() const {
	return firstRegularWordIndex > 0;
}

bool WordBuilder::isRedHilite(int selectionID, unsigned int wordID, const std::wstring& prevSentenceWord) const {
	//Return true for all pat-sint words. This requires just a simple index check.
	return selectionID < (int)this->getFirstWordIndex();
}

	
int WordBuilder::getNumberOfPages() const
{
	int numWords = getPossibleWords().size();
	if (numWords % 10 == 0)
		return numWords / 10;
	else
		return numWords / 10 + 1;
}

int WordBuilder::getCurrPage() const
{
	return currSelectedPage;
}

bool WordBuilder::pageUp(bool up)
{
	//Check
	int newID = currSelectedPage + (up?-1:1);
	if (newID<0 || newID>=getNumberOfPages())
		return false;

	//Page
	currSelectedPage = newID;

	//Select first word on that page
	currSelectedAbsoluteID = currSelectedPage * 10;

	return true;
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
	int absoluteID = id + firstRegularWordIndex;
	if (absoluteID >= (int)possibleWords.size())
		return;
	else if (absoluteID < 0)
		return;

	//Do it!
	currSelectedAbsoluteID = absoluteID;
}


//Returns the selected ID and a boolean
pair<bool, unsigned int> WordBuilder::typeSpace(int quickJumpID, bool useQuickJump)
{
	//We're at a valid stopping point?
	if (this->getPossibleWords().size() == 0)
		return pair<bool, unsigned int>(false, 0);

	//Quick jump?
	int quickJumpAbsID = quickJumpID + firstRegularWordIndex;
	if (quickJumpAbsID > -1)
		this->setCurrSelected(quickJumpID);
	if (currSelectedAbsoluteID!=quickJumpAbsID && quickJumpAbsID!=-1)
		return pair<bool, unsigned int>(false, 0); //No effect

	//Get the selected word, add it to the prefix array
	unsigned int newWord = this->getPossibleWords()[this->currSelectedAbsoluteID];
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
	this->currSelectedPage = 0;
	this->pastNexus.clear();
	this->currSelectedAbsoluteID = -1;
	this->possibleChars.clear();
	this->possibleWords.clear();
	this->parenStr.clear();

	//Full reset: remove all prefixes
	if (fullReset)
		this->trigrams.clear();
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
		if ( this->prefix[fromPrefix][i*2+1] == jumpID )
			return this->prefix[fromPrefix][i*2+2];
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
	if (shortcuts.count(currNexus)>0 && trigrams.size()>0) {
		if (shortcuts[currNexus].count(trigrams[0])>0) {
			possibleWords.insert(possibleWords.begin(), shortcuts[currNexus][trigrams[0]]);
			firstRegularWordIndex++;
		}
	}

	this->currSelectedAbsoluteID = firstRegularWordIndex; //Start at relative ID "0"
}


bool WordBuilder::vectorContains(const vector<unsigned int> &vec, unsigned int val) const
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
	this->trigrams.insert(trigrams.begin(), latestPrefix);

	//Trim
	while (this->trigrams.size()>3)
		this->trigrams.pop_back();
}


vector<char> WordBuilder::getPossibleChars() const
{
	return this->possibleChars;
}

unsigned int WordBuilder::getFirstWordIndex() const
{
	return this->firstRegularWordIndex;
}

vector<unsigned int> WordBuilder::getPossibleWords() const
{
	return this->possibleWords;
}

/**
 * The trigram_ids is organized as [0,1,2], where "0" is the most recently-typed word.
 * In the event that no previous words are available (e.g., beginning of a sentence)
 *  send an empty vector (size==0). If a unigram/bigram is available, send one or two-element vectors
 */
void WordBuilder::insertTrigram(const vector<unsigned int> &trigrams)
{
	//Assign
	this->trigrams = trigrams;

	//Limit to 3 elements
	while (this->trigrams.size()>3)
		this->trigrams.pop_back();
}


wstring WordBuilder::getWordKeyStrokes(unsigned int id)
{
	return this->getWordKeyStrokes(id, this->getOutputEncoding());
}


wstring WordBuilder::getWordKeyStrokes(unsigned int id, unsigned int encoding)
{
	//Can we skip the whole process?
	if (!this->restrictToMyanmar)
		return getWordString(id);

	//Get a reference to our dictionary value... it looks a bit odd, but changing this
	//  WILL change the word itself.
	wstring &myWord = (encoding==ENCODING_WININNWA) ? winInnwaDictionary[id] : (encoding==ENCODING_UNICODE) ? unicodeDictionary[id] : dictionary[id];
	
	//Determine our font... use Soe Min's names
	int destFont = Zawgyi_One;
	if (encoding==ENCODING_WININNWA) {
		destFont = WinInnwa;
	} else if (encoding==ENCODING_UNICODE) {
		destFont = Myanmar3;
	}

	//Does this word exist in the dictionary? If not, add it
	if (myWord.empty()) {
		//First, convert
		wchar_t srcStr[200];
		wchar_t destStr[200];
		wstring wordStr = this->getWordString(id);
		if (wordStr.size() >= 200)
			throw std::exception("String too big in WordBuilder");
		wcscpy(srcStr, wordStr.c_str());
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
		myWord = wstring(destStr);
	}

	//Return-by-value should copy the vector
	return myWord;
}


/**
 * Get the remaining letters to type to arrive at the guessed word (if any)
 */
wstring WordBuilder::getParenString() const
{
	return this->parenStr;
}



/**
 * Get the actual characters in a string (by ID).
 */
wstring WordBuilder::getWordString(unsigned int id) const
{
	return this->dictionary[id];
}


std::pair<int, std::string> WordBuilder::reverseLookupWord(std::wstring word)
{
	//Init
	std::pair<int, std::string> res;
	res.first = -1;

	//Get the ID
	for (unsigned int i=0; i<this->getTotalDefinedWords(); i++) {
		//Does the word match?
		wstring currWord = this->getWordString(i);
		if (currWord == word) {
			res.first = i;
			break;
		}
	}

	//Get the romanization 
	res.second = this->reverseLookupWord(res.first);  //Will be empty if word id is -1

	return res;
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
	while (!currNexi.empty()) {
		//Deal with all nexi on this level.
		for (size_t i=0; i<currNexi.size(); i++) {
			const vector<unsigned int> &thisNexus = nexus[currNexi[i]];
			for (unsigned int x=0; x<thisNexus.size(); x++) {
				int jmpToID = thisNexus[x]>>8;
				char letter = (char)(0xFF&thisNexus[x]);
				if (letter != '~') {
					nexiStrings[jmpToID] += nexiStrings[currNexi[i]];
					nexiStrings[jmpToID] += letter;

					nextNexi.push_back(jmpToID);
				}
			}
		}

		//Prepare for the next level
		currNexi = nextNexi;
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
				addReverseLookupItem(*currWord, nexiStrings[i]);
			}
		}
	}

	nexiStrings.clear();
	revLookupOn = true;
}


//Add a word, avoid duplicate strings
void WordBuilder::addReverseLookupItem(int wordID, const std::string &roman)
{
	//NOTE: Searching pre-existing entries is slow (in debug mode)! We might consider a hash-table
	//       instead, or something like a set.
	/*for (size_t x=0; x<revLookup.size(); x++) {
		if (revLookup[x] == roman) {
			//For now, copy by value. However, this isolates changes so that an 
			//  optimization is easy in the future if we want it.
			revLookup[wordID] = revLookup[x];
			return;
		}
	}*/

	//Store by value
	revLookup[wordID] = roman;
}



string WordBuilder::reverseLookupWord(unsigned int dictID)
{
	if (!revLookupOn)
		buildReverseLookup();

	if (dictID<0 || dictID>=revLookup.size())
		return string();

	return revLookup[dictID];
}



wstring WordBuilder::getLastError() const
{
	return mostRecentError;
}

bool WordBuilder::isInError() const
{
	return !mostRecentError.empty();
}


bool WordBuilder::addRomanization(const wstring &myanmar, const string &roman)
{
  return this->addRomanization(myanmar, roman, false);
}

//returns dictionary.size()
unsigned int WordBuilder::getWordID(const wstring &wordStr) const
{
	for (size_t canID=0; canID<dictionary.size(); canID++) {
		if (wordStr == dictionary[canID])
			return canID;
	}

	//Not found
	return dictionary.size();
}

bool WordBuilder::addRomanization(const wstring &myanmar, const string &roman, bool ignoreDuplicates)
{
	//First task: find the word; add it if necessary
	int dictID = getWordID(myanmar);
	if (dictID==dictionary.size()) {
		//Need to add... we DO have a limit, though.
		if (dictionary.size() == std::numeric_limits<size_t>::max()) {
			mostRecentError = L"Too many custom words!";
			return false;
		}
		dictionary.push_back(myanmar);
		unicodeDictionary.push_back(wstring());
		winInnwaDictionary.push_back(wstring());

		//Is this a special word (number)?
		//For now, this won't work for ASCII
		if (myanmar.size()==1 && myanmar[0]>=L'\u1040' && myanmar[0]<=L'\u1049' && !cachedNumerals[myanmar[0]-L'\u1040'].first)
			cachedNumerals[myanmar[0]-L'\u1040'] = std::pair<bool, unsigned short>(true, dictID);
	}


	//Update the reverse lookup?
	if (revLookupOn) {
		revLookup.push_back(string());
		addReverseLookupItem(revLookup.size()-1, roman);
	}


	//Next task: add the romanized mappings
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
			if (nexus.size() == std::numeric_limits<size_t>::max()) {
				mostRecentError = L"Too many custom nexi!";
				return false;
			}
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
		if (prefix.size() == std::numeric_limits<size_t>::max()) {
			mostRecentError = L"Too many custom prefixes!";
			return false;
		}
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
				mostRecentError = msg.str();
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


//Print our model, to check if it's the same as the one we input
void WordBuilder::debugOut(FILE *out)
{
	//Words
	fprintf(out, "#DEFINE:%i", this->dictionary.size());
	for (size_t i=0; i<dictionary.size(); i++) {
		//Split every 50 words
		if (i%50==0) {
			fprintf(out, "\n%i[", i);
		}

		//Print this word
		for (size_t x=0; x<dictionary[i].length(); x++) {
			if (x!=0)
				fprintf(out, "-");
			fprintf(out, "%02X", dictionary[i][x]-0x1000);
		}

		//Print either a comma or the closing bracket
		if ((i+1)%50==0 || (i+1)==dictionary.size())
			fprintf(out, "]");
		else
			fprintf(out, ",");
	}

	//Nexi
	fprintf(out, "\n#MAP:%i", this->nexus.size());
	for (size_t i=0; i<nexus.size(); i++) {
		fprintf(out, "\n{");

		//Print this mapping
		for (size_t x=0; x<nexus[i].size(); x++) {
			if (x!=0)
				fprintf(out, ",");

			unsigned int val = nexus[i][x];
			fprintf(out, "%c:%i", (val&0xFF), (val>>8));
		}

		fprintf(out, "}");
	}

	//Prefixes
	fprintf(out, "\n#SEGMENT%i", this->prefix.size());
	for (size_t i=0; i<prefix.size(); i++) {
		fprintf(out, "\n{");

		//First half: further mappings
		for (size_t x=0; x<prefix[i][0]; x++) {
			if (x!=0)
				fprintf(out, ",");

			fprintf(out, "%i:%i", prefix[i][x*2+1], prefix[i][x*2+2]);
		}

		fprintf(out, "} [");

		vector<unsigned int>::iterator prefWord = prefix[i].begin();
		std::advance(prefWord, prefix[i][0]*2+1);
		bool comma = false;
		for (; prefWord!=prefix[i].end(); prefWord++) {
			if (comma)
				fprintf(out, ",");
			comma = true;

			fprintf(out, "%i", *prefWord);
		}

		fprintf(out, "]");
	}

	fprintf(out, "\n");
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
