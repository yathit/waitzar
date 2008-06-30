/*
 * Copyright 2007 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _WORDBUILDER
#define _WORDBUILDER

//Necessary libraries
#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include "fontconv.h"

//If defined, we are running on Linux
//#define __STDC_ISO_10646__  200104L

//Windows and Linux have different "Unicode-aware" methods
#ifndef __STDC_ISO_10646__
#define copystr(a, b)      wcscpy((a), (b))
#define catstr(a, b)       wcscat((a), (b))
#define lenstr(a)          wcslen((a))
#define printstr(a, b, c)  swprintf((a), 150, (b), (c))
#define compstr(a, b)      wcscmp((a), (b))
//#include <windows.h>
#endif
#ifdef __STDC_ISO_10646__
#define copystr(a, b)      strcpy((a), (b))
#define catstr(a, b)       strcat((a), (b))
#define lenstr(a)          strlen((a))
#define printstr(a, b, c)  sprintf((a), (b), (c))
#define compstr(a, b)      strcmp((a), (b))
#endif

//Useful constants
#define ENCODING_UNICODE 1
#define ENCODING_ZAWGYI 2
#define ENCODING_WININNWA 3

/**
 * Used for converting a string of roman letters into a list of potential Burmese unsigned shorts.
 *  This class is intended for use in a Linux environment. Note that:
 *    "Before you start using UTF-8 under Linux make sure the distribution has glibc 2.2 
 *     and XFree86 4.0 or newer versions. Earlier versions lack UTF-8 locale support and ISO10646-1 X11 fonts." ibm.com
 */
class WordBuilder
{
public:
	WordBuilder (wchar_t* modelFile, wchar_t* userWordsFile);
	WordBuilder (unsigned short **dictionary, int dictMaxID, int dictMaxSize, unsigned int **nexus, int nexusMaxID, int nexusMaxSize, unsigned int **prefix, int prefixMaxID, int prefixMaxSize);
	~WordBuilder(void);

	//State-changing functions. Use these to respond to keypresses.
	bool typeLetter(char letter);
	std::pair<bool, unsigned int> typeSpace(int quickJumpID);
	bool backspace();
	void reset(bool fullReset);
	bool moveRight(int amt);
	
	//Information on the model's state
	int getCurrSelectedID();
	std::vector<char> getPossibleChars(void);
	std::vector<unsigned int> getPossibleWords(void);
	void insertTrigram(unsigned short* trigram_ids, int num_used_trigrams);

	//Get information about a particular unsigned short given its ID
	std::vector<unsigned short> getWordKeyStrokes(unsigned int id);
	wchar_t* getWordString(unsigned int id);
	wchar_t* getParenString();

	//Some additional useful info --used when mult. fonts are enabled
	unsigned short getStopCharacter(bool isFull);

	//Re-order the model
	bool addRomanization(wchar_t* myanmar, char* roman);

	//In case of error
	wchar_t* getLastError();

	//Change the encoding
	void setOutputEncoding(unsigned int encoding);
	unsigned int getOutputEncoding();

private:
	//Essential static data
	unsigned short **dictionary;
	unsigned int **nexus;
	unsigned int **prefix;

	//Cached lookups
	unsigned short **winInnwaDictionary;
	unsigned short **unicodeDictionary;

	//Cached
	unsigned short punctHalfStopUni;
	unsigned short punctFullStopUni;
	unsigned short punctHalfStopWinInnwa;
	unsigned short punctFullStopWinInnwa;

	//Encoding of output text only
	unsigned int currEncoding;

	//Also, for expansion
	int dictMaxID;
	int dictMaxSize;
	int nexusMaxID;
	int nexusMaxSize;
	int prefixMaxID;
	int prefixMaxSize;

	//Tracking the current unsigned short
	unsigned int currNexus;
	unsigned int pastNexus[200];
	int pastNexusID;

	//Tracking previous unsigned shorts
	unsigned int trigram[3];
	unsigned int trigramCount;

	//Tracking user selection
	int currSelectedID;

	//Internal stuff
	std::vector<char> possibleChars;
	std::vector<unsigned int> possibleWords;
	std::vector<unsigned short> keystrokeVector;
	wchar_t currStr[200];

	//Extension: guessing the next bit
	wchar_t parenStr[100];

	//For error messages
	wchar_t mostRecentError[200];

	//Internal functions
	void resolveWords(void);
	int jumpToNexus(int fromNexus, char jumpChar);
	int jumpToPrefix(int fromPrefix, int jumpID);
	bool vectorContains(std::vector<unsigned int> vec, unsigned int val);
	void addPrefix(unsigned int latestPrefix);
	void setCurrSelected(int id);

};

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

