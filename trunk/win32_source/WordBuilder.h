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
