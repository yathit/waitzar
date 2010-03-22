/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _KEYMAGIC_INPUT_METHOD
#define _KEYMAGIC_INPUT_METHOD

#include "MyWin32Window.h"
#include "Input/LetterInputMethod.h"


//NOTE TO SELF: KMRT_STRING values which appear in sequence COMBINE to form one string
// E.g.   $temp = 'abc' + 'def' 
// ...must be represented as  $temp = 'abcdef' 
// ...for array indexing to function properly.
enum RULE_TYPE {
	KMRT_UNKNOWN,    //Initializer
	KMRT_STRING,     //Anything which just matches a string (includes single characters, UXXXX)
	KMRT_WILDCARD,   //The wildcard character *
	KMRT_VARIABLE,   //Any "normal" variable like $test
	KMRT_MATCHVAR,   //Any backreference like $1
	KMRT_SWITCH,     //Any switch like ('my_switch')
};

struct Rule {
	RULE_TYPE type;
	std::wstring str;
	int val;

	Rule(RULE_TYPE type1, const std::wstring& str1, int val1) {
		type = type1;
		str = str1;
		val = val1;
	}
};


class KeyMagicInputMethod : public LetterInputMethod {

public:
	KeyMagicInputMethod();
	virtual ~KeyMagicInputMethod();

	//Key functionality
	void loadRulesFile(const std::wstring& datastream);
	std::wstring applyRules(const std::wstring& input);

private:
	//Data
	std::map< std::wstring, std::vector<Rule> > variables;
	std::vector< std::pair< std::vector<Rule>, std::vector<Rule> > > replacements;

	//Helpers
	int hexVal(wchar_t letter);
	Rule parseRule(const std::wstring& ruleStr);
	void addSingleRule(const std::vector<Rule>& rules, size_t rhsStart, bool isVariable);



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

