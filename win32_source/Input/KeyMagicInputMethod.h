/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _KEYMAGIC_INPUT_METHOD
#define _KEYMAGIC_INPUT_METHOD

#include <stack>
#include "MyWin32Window.h"
#include "Input/LetterInputMethod.h"
#include "Input/keymagic_vkeys.h"
#include "Settings/ConfigManager.h"


//NOTE TO SELF: KMRT_STRING values which appear in sequence COMBINE to form one string
// E.g.   $temp = 'abc' + 'def' 
// ...must be represented as  $temp = 'abcdef' 
// ...for array indexing to function properly.
enum RULE_TYPE {
	KMRT_UNKNOWN,          //Initializer
	KMRT_STRING,           //Anything which just matches a string (includes single characters, UXXXX)
	KMRT_WILDCARD,         //The wildcard character *
	KMRT_VARIABLE,         //Any "normal" variable like $test
	KMRT_MATCHVAR,         //Any backreference like $1
	KMRT_SWITCH,           //Any switch like ('my_switch')
	KMRT_VARARRAY,         //Variable matches like $temp[1]
	KMRT_VARARRAY_SPECIAL, //Variable matches like $temp[*], $temp[^] (we store the char itself in the val value)
	KMRT_VARARRAY_BACKREF, //Variable matches like $temp[$1]
	KMRT_KEYCOMBINATION,   //Something like <VK_SHIFT & VK_T> (val contains the key, combined with modifiers)
};

struct Rule {
	RULE_TYPE type;
	std::wstring str;
	int val;
	int id; //Used for variables (and switches, later). 

	Rule(RULE_TYPE type1, const std::wstring& str1, int val1) {
		type = type1;
		str = str1;
		val = val1;
		id = -1;
	}
};


//String/int pairs
struct Group {
	std::wstring value;
	int group_match_id; //Groups count up from ONE
	Group() : value(L""), group_match_id(0) {}
	Group(const std::wstring& value1) : value(value1), group_match_id(0) {}
	Group(const std::wstring& value1, int id1) : value(value1), group_match_id(id1) {}
};


//A rule being matched and any relevant data (like the dot)
struct Matcher {
	std::vector<Rule>& rulestream; //Can't make this const or it won't let itself be copied easily.
	int dot; //0 means "before the first rule"
	int strDot; //Special dot for the string.
	Matcher(std::vector<Rule>& rules) : rulestream(rules), dot(0), strDot(0) {}

	Matcher operator=(const Matcher& m2) {
		rulestream = m2.rulestream;
		dot = m2.dot;
		strDot = m2.strDot;
		return *this;
	}

	void moveDot() {
		if (rulestream[dot].type==KMRT_STRING && strDot<(int)rulestream[dot].str.length()-1)
			strDot++;
		else {
			dot++;
			strDot = 0;
		}
	}
	int getDot() {
		return dot;
	}
	bool isDone() {
		return dot==rulestream.size();
	}
};


//Candidate matches.
struct Candidate {
private:
	//Matches we've already made
	std::vector<Group> matches;
	unsigned int currRootDot;

	//Matches put on hold while we recurse
	std::stack<Matcher> matchStack;

	//What switches will this candidate turn off?
	std::vector<unsigned int> switchesToOff;

public:
	//What to replace after
	std::vector<Rule>& replacementRules;

	//Dot IDS
	int dotStartID;
	int dotEndID;

	//Default constructor
	Candidate(std::vector<Rule> tempVec) : replacementRules(tempVec) {}

	//Init
	Candidate(std::pair< std::vector<Rule>, std::vector<Rule> >& rulePair, int dotStartID1) : replacementRules(rulePair.second) {
		matchStack.push(Matcher(rulePair.first));

		//Init array sizes
		for (unsigned int i=0; i<rulePair.first.size(); i++)
			matches.push_back(Group(L"", -1));
		currRootDot = 0;

		dotStartID = dotStartID1;
	}

	//For vector contexts
	Candidate operator=(const Candidate& c2) {
		matches = c2.matches;
		matchStack = c2.matchStack;
		switchesToOff = c2.switchesToOff;
		replacementRules = c2.replacementRules;
		dotStartID = c2.dotStartID;
		dotEndID = c2.dotEndID;
		currRootDot = c2.currRootDot;
		return *this;
	}

	//Our "current match" is the top entry on the stack; this greatly simplifies memory management
	Matcher& currMatch() {
		if (matchStack.empty())
			throw std::exception("Error: cannot match on a rule which is already done");
		return matchStack.top();
	}

	//Useful stuff
	const Rule& getCurrRule() {
		if (currMatch().dot<(int)currMatch().rulestream.size())
			return currMatch().rulestream[currMatch().dot];
		throw std::exception("Bad programming: the dot has exceeded the length of the current rulestream.");
	}
	wchar_t getCurrStringRuleChar() {
		if (getCurrRule().type!=KMRT_STRING)
			throw std::exception("Can only call getCurrStringRuleChar() on a STRING type");
		return getCurrRule().str[currMatch().strDot];
	}
	void newCurr(std::vector<Rule>& rule) {
		matchStack.push(Matcher(rule));
	}
	void advance(const wstring& foundStr, int foundID) { //Called if a match is allowed
		if (isDone())
			return;

		//Move the dot
		currMatch().moveDot();

		//Save our match data
		matches[currRootDot].value += foundStr;
		matches[currRootDot].group_match_id = foundID;

		//Pop the stack
		if (currMatch().isDone()) {
			matchStack.pop();
			advance(L"", foundID); //We need to move the current match's dot by one, signifying that we've "matched" this variable.
		}

		//Update root dot?
		if (matchStack.size()==1)
			currRootDot = currMatch().getDot();
	}
	void advance(wchar_t foundChar, int foundID) {
		std::wstringstream str;
		str <<foundChar;
		advance(str.str(), foundID);
	}
	bool isDone() { //Did we finish matching?
		return matchStack.empty();
	}

	//For switches
	void queueSwitchOff(unsigned int id) {
		switchesToOff.push_back(id);
	}
	const std::vector<unsigned int>& getPendingSwitches() const {
		return switchesToOff;
	}

	//Return
	std::wstring getMatch(unsigned int id) const {
		return matches[id].value;
	}
	int getMatchID(unsigned int id) const {
		return matches[id].group_match_id;
	}
};


class KeyMagicInputMethod : public LetterInputMethod {

public:
	KeyMagicInputMethod();
	virtual ~KeyMagicInputMethod();

	//Key functionality
	void loadRulesFile(const std::string& rulesFilePath);
	std::wstring applyRules(const std::wstring& origInput, unsigned int vkeyCode);

	//Overrides of LetterInputMethod
	std::pair<std::wstring, bool> appendTypedLetter(const std::wstring& prevStr, wchar_t nextASCII, WPARAM nextKeycode, LPARAM lParam);
	virtual void handleBackspace(WPARAM wParam, LPARAM lParam);
	virtual void handleStop(bool isFull, WPARAM wParam, LPARAM lParam);

private:
	//Trace?
	static bool LOG_KEYMAGIC_TRACE;
	static std::string keyMagicLogFileName;
	static void clearLogFile(const std::string& fileName);
	static void writeLogLine(const std::string& fileName, const std::wstring& logLine); //We'll escape MM outselves

	//Data
	std::vector<bool> switches;
	std::vector< std::vector<Rule> > variables;
	std::vector< std::pair< std::vector<Rule>, std::vector<Rule> > > replacements;
	std::vector<std::wstring> debugRuleText; //NOTE: If we load rules from a cached file, this will contain empty strings.

	//Helpers
	int hexVal(wchar_t letter);
	Rule parseRule(const std::wstring& ruleStr);
	void addSingleRule(const std::wstring& fullRuleText, const std::vector<Rule>& rules, std::map< std::wstring, unsigned int>& varLookup, std::map< std::wstring, unsigned int>& switchLookup, size_t rhsStart, bool isVariable);
	std::vector<Rule> createRuleVector(const std::vector<Rule>& rules, const std::map< std::wstring, unsigned int>& varLookup, std::map< std::wstring, unsigned int>& switchLookup, size_t iStart, size_t iEnd, bool condenseStrings);
	Rule compressToSingleStringRule(const std::vector<Rule>& rules);
	std::pair<Candidate, bool> getCandidateMatch(std::pair< std::vector<Rule>, std::vector<Rule> >& rule, const std::wstring& input, unsigned int vkeyCode, bool& matchedOneVirtualKey);
	std::wstring applyMatch(const Candidate& result, bool& resetLoop, bool& breakLoop);



};




#endif //_KEYMAGIC_INPUT_METHOD

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

