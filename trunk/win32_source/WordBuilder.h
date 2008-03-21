/*
 * Copyright 2007 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once

#define _UNICODE
#define UNICODE

#include <windows.h>
#include <tchar.h>
#include <vector>

class WordBuilder
{
public:
	WordBuilder (WORD **dictionary, UINT32 **nexus, UINT32 **prefix);
	~WordBuilder(void);

	bool typeLetter(char letter);
	std::pair<BOOL, UINT32> typeSpace(int quickJumpID);
	bool backspace();
	void reset(bool fullReset);
	BOOL moveRight(int amt);
	int getCurrSelectedID();
	
	//Information on the model's state
	std::vector<char> getPossibleChars(void);
	std::vector<UINT32> getPossibleWords(void);

	//Translation
	std::vector<WORD> getWordKeyStrokes(UINT32 id);
	TCHAR* getWordString(UINT32 id);

private:
	//Essential static data
	WORD **dictionary;
	UINT32 **nexus;
	UINT32 **prefix;

	//Tracking the current word
	UINT32 currNexus;
	UINT32 pastNexus[200];
	int pastNexusID;

	//Tracking previous words
	UINT32 trigram[3];
	UINT32 trigramCount;

	//Tracking user selection
	int currSelectedID;

	//Internal stuff
	std::vector<char> possibleChars;
	std::vector<UINT32> possibleWords;
	std::vector<WORD> keystrokeVector;
	TCHAR currStr[200];

	//Internal functions
	void resolveWords(void);
	int jumpToNexus(int fromNexus, char jumpChar);
	int jumpToPrefix(int fromPrefix, int jumpID);
	bool vectorContains(std::vector<UINT32> vec, UINT32 val);
	void addPrefix(UINT32 latestPrefix);
	void setCurrSelected(int id);

};


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
