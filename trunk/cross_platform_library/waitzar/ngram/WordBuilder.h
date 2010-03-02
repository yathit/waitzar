/*
 * Copyright 2007 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _WORDBUILDER
#define _WORDBUILDER

//Don't let Visual Studio warn us to use the _s functions
//#define _CRT_SECURE_NO_WARNINGS

//Ignore new swprintf semantics
//#define _CRT_NON_CONFORMING_SWPRINTFS

//Necessary libraries
#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> //strtol
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <limits>
#include "Burglish/fontconv.h"


namespace waitzar
{

//Useful constants
enum ENCODING {
	ENCODING_UNICODE=1,
	ENCODING_ZAWGYI,
	ENCODING_WININNWA
};



/**
 * Used for converting a string of roman letters into a list of potential Burmese unsigned shorts.
 *  This class is intended for use in a Linux environment. Note that:
 *    "Before you start using UTF-8 under Linux make sure the distribution has glibc 2.2
 *     and XFree86 4.0 or newer versions. Earlier versions lack UTF-8 locale support and ISO10646-1 X11 fonts." ibm.com
 */
class WordBuilder
{
public:
	WordBuilder(char * model_buff, size_t model_buff_size, bool allowAnyChar);
	WordBuilder(const char* modelFile, const char* userWordsFile);
	WordBuilder(const char* modelFile, std::vector<std::string> userWordsFiles);
	WordBuilder(const std::vector<std::wstring> &dictionary, const std::vector< std::vector<unsigned int> > &nexus, const std::vector< std::vector<unsigned int> > &prefix);
	WordBuilder();
	~WordBuilder(void);

	//State-changing functions. Use these to respond to keypresses.
	bool typeLetter(char letter);
	std::pair<bool, unsigned int> typeSpace(int quickJumpID, bool useQuickJump);
	bool backspace();
	void reset(bool fullReset);
	bool moveRight(int amt);

	//Information on the model's state
	int getCurrSelectedID() const; //Returns -1 if PS, etc.
	std::vector<char> getPossibleChars() const;
	std::vector<unsigned int> getPossibleWords() const;
	void insertTrigram(const std::vector<unsigned int> &trigrams);
	unsigned int getFirstWordIndex() const;
	int getNumberOfPages() const;
	int getCurrPage() const;
	bool pageUp(bool up);

	//Get information about a particular unsigned short given its ID
	std::wstring getWordKeyStrokes(unsigned int id);
	std::wstring getWordKeyStrokes(unsigned int id, unsigned int encoding);
	std::wstring getWordString(unsigned int id) const;
	std::wstring getParenString() const;
	bool hasPatSintWord() const; 

	//Some additional useful info
	unsigned short getStopCharacter(bool isFull) const;
	unsigned int getTotalDefinedWords() const;
	std::string reverseLookupWord(unsigned int dictID);
	std::pair<int, std::string> reverseLookupWord(std::wstring word);
	unsigned short getSingleDigitID(unsigned short arabicNumeral);
	bool addShortcut(const std::wstring &baseWord, const std::wstring &toStack, const std::wstring &resultStacked);
	bool isAllowNonBurmese();

	//Re-order the model
	bool addRomanization(const std::wstring &myanmar, const std::string &roman);
	bool addRomanization(const std::wstring &myanmar, const std::string &roman, bool ignoreDuplicates);

	//In case of error
	std::wstring getLastError() const;
	bool isInError() const;

	//Change the encoding
	void setOutputEncoding(ENCODING encoding);
	ENCODING getOutputEncoding() const;

	//For now
	void debugOut(FILE *out);

private:
	//Essential static data
	std::vector< std::wstring > dictionary;
	std::vector< std::vector<unsigned int> > nexus;
	std::vector< std::vector<unsigned int> > prefix;
	std::vector< std::string > revLookup;
	bool revLookupOn;

	//We could use a multimap of pairs, but I think a map of maps works better.
	// This is arranged as: nexus -> pre_word_id -> combined_word_id
	std::map<unsigned int, std::map<unsigned int, unsigned int> > shortcuts;

	//If true, filter words
	bool restrictToMyanmar;

	//Cached lookups
	std::vector<std::wstring> winInnwaDictionary;
	std::vector<std::wstring> unicodeDictionary;

	//Needed to avoid errors in some models (not WZ)
	std::vector< std::pair<bool, unsigned short> > cachedNumerals;

	//Cached
	unsigned short punctHalfStopUni;
	unsigned short punctFullStopUni;
	unsigned short punctHalfStopWinInnwa;
	unsigned short punctFullStopWinInnwa;

	//Encoding of output text only
	ENCODING currEncoding;

	//Tracking the current unsigned short
	unsigned int currNexus;
	std::vector<unsigned int> pastNexus;

	//Tracking previous unsigned shorts
	std::vector<unsigned int> trigrams;

	//Tracking user selection
	int currSelectedAbsoluteID; //0 to size, not -1, etc.
	int currSelectedPage;

	//Code for loading the model specifically from a variety of options, and the initialization code
	void loadModel(char * model_buff, size_t model_buff_size, bool allowAnyChar);
    void loadModel(const char* modelFile, std::vector<std::string> userWordsFiles);
    void loadModel(const std::vector<std::wstring> &dictionary, const std::vector< std::vector<unsigned int> > &nexus, const std::vector< std::vector<unsigned int> > &prefix);
	void initModel();

	//Internal stuff
	std::vector<char> possibleChars;
	std::vector<unsigned int> possibleWords;
	size_t firstRegularWordIndex;

	//Extension: guessing the next bit
	std::wstring parenStr;
	//std::wstring postStr;

	//For error messages
	std::wstring mostRecentError;

	//Internal functions
	void resolveWords(void);
	int jumpToNexus(int fromNexus, char jumpChar) const;
	int jumpToPrefix(int fromPrefix, int jumpID) const;
	bool vectorContains(const std::vector<unsigned int> &vec, unsigned int val) const;
	void addPrefix(unsigned int latestPrefix);
	void setCurrSelected(int id);
	void buildReverseLookup();
	void addReverseLookupItem(int wordID, const std::string &roman);
	unsigned int getWordID(const std::wstring &wordStr) const;

	//Inline
	unsigned int toHex(char letter) const {
		switch(letter) {
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				return (letter-'0');
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				return (letter-'A')+10;
			default:
				return 0;
		}
	}

};



size_t mymbstowcs(wchar_t *dest, const char *src, size_t maxCount);




/**
 * Read a line from a file. Yay! First template function!
 *  Note that this function automatically converts all upper case to lower case before checking.
 * NOTE: Template implementations must be defined in the same file they are declared; hence, this must be in the .h file.
 *       This is because Visual Studio does not support "export" on templates; otherwise, we could separate it.
 *       http://msdn.microsoft.com/en-us/library/4w5cfxs1.aspx
 * @param stream - TCHAR* or char*
 * @param index,streamSize - current position, max pos
 * @param nameRet, valRet The return strings for name/value pairs. Should be big enough to hold the name/value strings
 */
template <class T, class S>
void readLine(T* stream, size_t &index, size_t streamSize, bool nameHasASCII, bool nameHasMyanmar, bool nameHasSymbols, bool nameHasAny, bool valueHasASCII, bool valueHasMyanmar, bool valueHasSymbols, bool valueHasAnything, T* nameRet, S* valRet)
{
	//Init --note: 0x0000 is necessary, see:
	//http://msdn.microsoft.com/en-us/library/ms776431(VS.85).aspx
	//....er, actually, it should probably be \u0000; I think 0x00 would work. But no matter, really.
	nameRet[0] = (T)0x0000;
	valRet[0] = (S)0x0000;

	//Left-trim
	while (stream[index] == ' ')
		index++;

	//Comment? Empty line? If so, skip...
	if (stream[index]=='#' || stream[index]=='\n') {
		while (stream[index] != '\n' && index<streamSize)
			index++;
		index++;
		return;
	}

	//Start reading "name" and "value"
	int name_pos = 0;
	int value_pos = 0;
	bool nameDone = false;
	bool hasASCII = nameHasASCII;
	bool hasMyanmar = nameHasMyanmar;
	bool hasSymbols = nameHasSymbols;
	T currChar;
	T prevCaseChar;
	while (index<streamSize) {
		if (stream[index] == '\n') {
			//Done
			index++;
			break;
		} else if (stream[index] == '=') {
			//Switch modes
			nameDone = true;
			hasASCII = valueHasASCII;
			hasMyanmar = valueHasMyanmar;
			hasSymbols = valueHasSymbols;
		} else if (stream[index]!=' ') {
			//Convert to lowercase
			currChar = (T)stream[index];
			prevCaseChar = currChar;
			if (currChar>='A' && currChar<='Z')
				currChar += ('a'-'A');

			//Check if it's valid
			if (
			   (hasASCII==true && currChar>='a' && currChar<='z') ||
			   (hasMyanmar==true && currChar>=(T)0x1000 && currChar<=(T)0x109F) ||
			   (hasSymbols==true && (currChar=='_' || currChar=='!' || currChar=='^' || currChar=='+' || currChar=='.' || currChar=='-')) ||
			   (nameDone==true && valueHasAnything==true) ||
			   (nameDone==false && nameHasAny==true)
			   ) {
				  //This test exists for hotkey configurations
				  if (hasSymbols==true)
				    currChar = prevCaseChar;

				  //Add it
				  if (nameDone==false)
					nameRet[name_pos++] = currChar;
				  else
					valRet[value_pos++] = (S)currChar;
			}
		}

		//Continue
		index++;
	}

	//Append & return
	nameRet[name_pos] = (T)0x0000;
	valRet[value_pos] = (S)0x0000;
}


} //End waitzar namespace



#endif //_WordBUILDER



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

