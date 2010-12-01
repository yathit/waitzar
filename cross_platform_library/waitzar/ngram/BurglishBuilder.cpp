/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "BurglishBuilder.h"


//Import needed stl
using std::string;
using std::wstringstream;
using std::wstring;
using std::vector;
using std::pair;
using std::map;
using std::set;


namespace waitzar
{


/**
 * Empty constructor.
 */
BurglishBuilder::BurglishBuilder() {}
BurglishBuilder::~BurglishBuilder() {}

//Static initializer:
json_spirit::wmObject BurglishBuilder::onsetPairs;
json_spirit::wmObject BurglishBuilder::rhymePairs;
json_spirit::wmObject BurglishBuilder::specialWords;
std::vector<std::wstring> BurglishBuilder::savedDigitIDs;
void BurglishBuilder::InitStatic()
{
	//Create, read, save our onsets and rhymes.
	json_spirit::wmValue onsetRoot;
	json_spirit::wmValue rhymeRoot;
	json_spirit::wmValue specialRoot;

	if (!(json_spirit::read(BURGLISH_ONSETS, onsetRoot) && 	json_spirit::read(BURGLISH_RHYMES, rhymeRoot) && json_spirit::read(BURGLISH_SPECIALS, specialRoot)))
		throw std::runtime_error("BURGLISH_ONSETS, BURGLISH_RHYMES, or BURGLISH_SPECIALS contains parse errors --this should never happen in release mode.");

	onsetPairs = onsetRoot.get_value<json_spirit::wmObject>();
	rhymePairs = rhymeRoot.get_value<json_spirit::wmObject>();
	specialWords = specialRoot.get_value<json_spirit::wmObject>();

	//Saved digits; these are never cleared.
	savedDigitIDs.push_back(L"\u1040");
	savedDigitIDs.push_back(L"\u1041");
	savedDigitIDs.push_back(L"\u1042");
	savedDigitIDs.push_back(L"\u1043");
	savedDigitIDs.push_back(L"\u1044");
	savedDigitIDs.push_back(L"\u1045");
	savedDigitIDs.push_back(L"\u1046");
	savedDigitIDs.push_back(L"\u1047");
	savedDigitIDs.push_back(L"\u1048");
	savedDigitIDs.push_back(L"\u1049");
}


bool BurglishBuilder::IsVowel(wchar_t letter)
{
	switch (letter) {
		case L'a':
		case L'e':
		case L'i':
		case L'o':
		case L'u':
			return true;
	}
	return false;
}


bool BurglishBuilder::IsValid(const wstring& word)
{
	//Warning: empty strings
	if (word.empty())
		return true; //Let's say.... valid.

	//Check for invalid pairwise combinations
	for (size_t i=0; i<word.size()-1; i++) {
		//We build the regex into a switch statement, handling this manually.
		switch (word[i]) {
			case L'\u100D':  case L'\u100B':  case L'\u100C':  case L'\u1023':
				switch (word[i+1]) {
					case L'\u1087':  case L'\u103D':  case L'\u102F':  case L'\u1030':
					case L'\u1088':  case L'\u1089':  case L'\u103C':  case L'\u108A':
						return false;
				}
				break;
			case L'\u1020':
				switch (word[i+1]) {
					case L'\u103C':  case L'\u108A':
						return false;
				}
				break;
			case L'\u1060':  case L'\u1061':  case L'\u1062':  case L'\u1063':  case L'\u1064':  case L'\u1065':
			case L'\u1066':  case L'\u1067':  case L'\u1068':  case L'\u1069':  case L'\u106A':  case L'\u106B':
			case L'\u106C':  case L'\u106D':  case L'\u106E':  case L'\u106F':  case L'\u1070':  case L'\u1071':
			case L'\u1072':  case L'\u1073':  case L'\u1074':  case L'\u1075':  case L'\u1076':  case L'\u1077':
			case L'\u1078':  case L'\u1079':  case L'\u107A':  case L'\u107B':  case L'\u107C':  case L'\u1092':
				switch (word[i+1]) {
					case L'\u1087':  case L'\u103D':  case L'\u1088':  case L'\u1089':
					case L'\u103C':  case L'\u108A':
						return false;
				}
				break;
		}
	}

	//Otherwise, it passed
	return true;
}



std::wstring BurglishBuilder::PatSintCombine(const std::wstring& base, const std::wstring& stacked)
{
	//Is there enough to go on?
	if (base.length()<2) //Need consonant + asat
		return L"";

	//Valid if the previous word has U+103A and not anything else stacked (U+1039)
	//Valid if the stacked word has 1039
	size_t aIndex = base.rfind(L"\u103A");
	if (aIndex==wstring::npos || base.find(L"\u1039")!=wstring::npos || stacked.find(L"\u1039")==wstring::npos)
		return L"";

	//Step 1: Remove U+103A from the base.
	wstring empty = L"";
	wstring baseRep = base;
	baseRep.replace(aIndex, 1, empty);

	//Step 2: Remove kinzi from the stacked
	wstring kinzi = L"\u1004\u103A\u1039";
	size_t kIndex = stacked.rfind(L"\u1004\u103A\u1039");
	wstring stackRep = stacked;
	if (kIndex!=wstring::npos) {
		stackRep.replace(kIndex, kinzi.length(), empty);
	} else if (aIndex>0 && baseRep[aIndex-1]==L'\u1004') {
		//Special case: we keep kinzi if base is stacking "nga"
		//See if we've got U+1039 followed by at least one letter
		size_t asatID = stackRep.find(L'\u1039');
		if (asatID!=wstring::npos && asatID+1<stackRep.length()) {
			//Step 1: replace the base letter
			baseRep[aIndex-1] = stackRep[asatID+1];

			//Step 2: Re-work the stacked letter; remove the consonant and U+1039
			wstring postfix = (asatID+2<stackRep.length()) ? (stackRep.substr(asatID+2, stackRep.length()) : L"";
			stackRep = stackRep.substr(0, asatID) + postfix);
		} else {
			//Ditch kinzi
			kinzi = empty;
		}
	} else {
		//No kinzi
		kinzi = empty;
	}

	//Step 3: Combine (kinzi?) + base + stacked. Note that kinzi should appear to the left of the killed consonant, not the base consonant
	if (baseRep.size()>0 && aIndex>0 && aIndex-1<baseRep.size())
		return baseRep.substr(0, aIndex-1) + kinzi + baseRep.substr(aIndex-1, baseRep.length()) + stackRep;
	return L"";
}



void BurglishBuilder::addStandardWords(wstring roman, std::set<std::wstring>& resultsKeyset, std::vector< std::pair<std::wstring, int> >& resultSet, bool firstLetterUppercase, const std::wstring& prevWord, std::vector<std::wstring>& combinationSaveLocation)
{
	//Nothing to do?
	if (roman.empty())
		return;

	//Step 1: Duplicate vowels at the beginning of the word
	if (IsVowel(roman[0]))
		roman = wstring(1, roman[0]) + roman;

	//Step 2: Find the longest matching prefix.
	wstring prefix;
	wstring suffix;
	for (size_t i=1; i<=roman.size(); i++) {
		//Done?
		wstring candPrefix = roman.substr(0, i);
		if (onsetPairs.count(candPrefix)==0)
			break;

		//Update (suffix is "a" if prefix takes up the whole string)
		prefix = candPrefix;
		suffix = (i==roman.size() ? L"a" : roman.substr(i, roman.size()-i));
	}

	//Step 3: Null pair? If not, pull their values
	if (onsetPairs.count(prefix)==0 || rhymePairs.count(suffix)==0)
		return;
	wstring prefixStr = onsetPairs[prefix].get_value<wstring>();
	wstring suffixStr = rhymePairs[suffix].get_value<wstring>();

	//Step 4: For each prefix, for each suffix, get the combined word.
	// Prefixes & suffixes are just strings separated by pipe marks: ".....|.....|...."
	// Suffixes, in addition, have a "-" to show where the inserted prefix should go.
	wstringstream onset;
	for (size_t onsID=0; onsID<prefixStr.size(); onsID++) {
		//Append?
		if (prefixStr[onsID]!=L'|' && prefixStr[onsID]!=L'+')
			onset <<prefixStr[onsID];
		//Skip?
		if (prefixStr[onsID]!=L'|' && onsID<prefixStr.size()-1)
			continue;

		//Never do anything if the onset is empty
		if (onset.str().empty())
			continue;

		//Replace the current prefix with a pat-sint equivalent if the first letter is capital
		if (firstLetterUppercase) {
			//Stacking only occurs with single-letter onsets (exception: the ya medials)
			wstring oldPrefix = onset.str();
			wchar_t c = oldPrefix[0];
			if (oldPrefix.length()==1 || (oldPrefix.length()==2 && c!=L'\u1004' && (oldPrefix[1]==L'\u103B' || oldPrefix[1]==L'\u103C'))) {

				if (  (c>=L'\u1000' && c<=L'\u1008')
					||(c==L'\u100B' || c==L'\u100C' || c==L'\u101C')
					||(c>=L'\u100F' && c<=L'\u1019'))
				{
					//Stack it
					onset.str(L"");
					if (c==L'\u1004')
						onset <<L"\u1004\u103A\u1039";
					else
						onset <<L'\u1039' <<oldPrefix;
				}
			}
		}


		//Ok, we have our onset.
		wstringstream rhyme;
		for (size_t rhymeID=0; rhymeID<suffixStr.size(); rhymeID++) {
			//Append?
			if (suffixStr[rhymeID]==L'-')
				rhyme <<onset.str(); //Insert our onset
			else if (suffixStr[rhymeID]!=L'|' && suffixStr[rhymeID]!=L'+')
				rhyme <<suffixStr[rhymeID];
			//Skip?
			if (suffixStr[rhymeID]!=L'|' && rhymeID<suffixStr.size()-1)
				continue;

			//Again, skip empty strings
			if (rhyme.str().empty())
				continue;

			//We've built our onset + rhyme into rhyme directly.
			//Now, we just need to test for errors, etc.
			wstring word = waitzar::normalize_bgunicode(rhyme.str());
			if (IsValid(word) && resultsKeyset.count(word)==0 && !word.empty()) {
				//If this is a pat-sint word, we have to add an entry to the combination array
				int combID = -1;
				if (firstLetterUppercase) {
					wstring newCombine = PatSintCombine(prevWord, word);
					if (!newCombine.empty()) {
						combID = combinationSaveLocation.size();
						combinationSaveLocation.push_back(newCombine);
					}
				}
				resultSet.push_back(pair<wstring, int>(word, combID)); //This is the only place we can add pat-sint words
				resultsKeyset.insert(word);
			}

			//Finally: reset
			rhyme.str(L"");
		}
		//Finally: reset
		onset.str(L"");
	}
}



void BurglishBuilder::addNumerals(std::wstring roman, std::set<std::wstring>& resultsKeyset, std::vector< std::pair<std::wstring, int> >& resultSet)
{
	//Make sure it contains only numbers
	std::wstringstream literalStr;
	for (size_t i=0; i<roman.size(); i++) {
		if (!(roman[i]>='0' && roman[i]<='9'))
			return;

		wchar_t digit = (roman[i]-'0') + L'\u1040';
		literalStr <<digit;
	}

	//Add the number string itself
	if (resultsKeyset.count(literalStr.str())==0) {
		resultSet.push_back(pair<wstring, int>(literalStr.str(), -1));
		resultsKeyset.insert(literalStr.str());
	}

	//Add compound numbers, when possible
	//TODO: Does this work right for 100, etc? Right now, it's mostly copied from Burglish.
	if (roman.length() <= BURGLISH_TOTAL_COMBINERS+1) {
		//Loop through the string backwards.
		int index = 0;
		int lastDigit = 0;
		bool someFlag = true;
		std::wstring foundWord;
		for (wstring::reverse_iterator digit=roman.rbegin(); digit!=roman.rend(); digit++) {
			if (lastDigit>0)
				someFlag = false;
			lastDigit = *digit - '0';

			if (index==0 && lastDigit>0) {
				foundWord = BURGLISH_NUMBER_LITERALS[lastDigit-1];
			} else if (lastDigit>0) {
				const std::wstring* const BURGLISH_NUMBER_COMBINERS = someFlag ? BURGLISH_NUMBER_COMBINERS_SING : BURGLISH_NUMBER_COMBINERS_DOUB;
				foundWord = BURGLISH_NUMBER_LITERALS[lastDigit-1] + BURGLISH_NUMBER_COMBINERS[index-1] + foundWord;
			}
			index++;
		}

		//Add it.
		if (!foundWord.empty() && resultsKeyset.count(foundWord)==0) {
			resultSet.push_back(pair<wstring, int>(foundWord, -1));
			resultsKeyset.insert(foundWord);
		}
	}
}



void BurglishBuilder::addSpecialWords(std::wstring roman, std::set<std::wstring>& resultsKeyset, std::vector< std::pair<std::wstring, int> >& resultSet, std::wstringstream& parenStr)
{
	//Actually, we have to loop through each special word, to allow "prediction"
	wstring comma = L"";
	parenStr.str(L"");
	for (json_spirit::wmObject::iterator pair=specialWords.begin(); pair!=specialWords.end(); pair++) {
		wstring key = pair->first;
		if (key.find(roman)==0) { //If key.startsWith(roman)
			//Add to the paren str?
			if (key.length()>roman.length()) {
				parenStr <<comma+key;
				comma = L",";
			}

			//Append all values
			wstring specialStrs = pair->second.get_value<wstring>();
			wstringstream entry;
			for (size_t specID=0; specID<specialStrs.size(); specID++) {
				//Append?
				if (specialStrs[specID]!=L'|')
					entry <<specialStrs[specID];
				//Skip?
				if (specialStrs[specID]!=L'|' && specID<specialStrs.size()-1)
					continue;

				//Add it (no need to check validity)
				if (resultsKeyset.count(entry.str())==0) {
					resultSet.push_back(std::pair<wstring, int>(entry.str(), -1));
					resultsKeyset.insert(entry.str());
				}

				//Finally: reset
				entry.str(L"");
			}
		}
	}
}


void BurglishBuilder::expandCurrentWords(std::set<std::wstring>& resultsKeyset, std::vector< std::pair<std::wstring, int> >& resultSet)
{
	//Just a few simple substitutions
	//TODO: Double-check this after all words have been added.
	vector<wstring> newCandidates;
	wstringstream newWord;
	for (std::set<std::wstring>::const_iterator word=resultsKeyset.begin(); word!=resultsKeyset.end(); word++) {
		//Try to build up a new match
		newWord.str(L"");
		for (size_t i=0; i<word->size(); i++) {
			//Build the next bit
			bool consumed = false;
			if ((*word)[i]==L'\u1021') {
				//First pattern:
				// \u1021 \u102F --> \u1025
				if (i<word->size()-1 && (*word)[i+1]==L'\u102F') {
					newWord <<L'\u1025';
					i++;
					consumed = true;
				}

				//Second pattern:
				// \u1021 \u102D [^\u102F]! --> \u1023 (don't replace !-expression)
				if (!(i+2<word->size()) || ((*word)[i+1]==L'\u102D' && (*word)[i+2]!=L'\u102F')) {
					newWord <<L'\u1023';
					if (i+2<word->size())
						i+=2; //Don't consume.
					else
						consumed = true;
				}

				//Third pattern:
				// \u1021 \u1031 \u102C [^\u103A \u1037]! --> \u1029 (don't replace !-expression)
				if (!(i+3<word->size()) || ((*word)[i+1]==L'\u1031' && (*word)[i+2]==L'\u102C' && (*word)[i+3]!=L'\u103A' && (*word)[i+3]!=L'\u1037')) {
					newWord <<L'\u1029';
					if (i+3<word->size())
						i+=3; //Don't consume.
					else
						consumed = true;
				}
			}

			//Append
			if (!consumed && i<word->size())
				newWord <<(*word)[i];
		}

		//Is this word worth adding?
		if (newWord.str() != *word && IsValid(newWord.str()))
			newCandidates.push_back(newWord.str());
	}


	//Add each new candidate (this preserves iterators)
	for (vector<wstring>::iterator it=newCandidates.begin(); it!=newCandidates.end(); it++) {
		if (resultsKeyset.count(*it)==0) {
			resultSet.push_back(pair<wstring, int>(*it, -1));
			resultsKeyset.insert(*it);
		}
	}

}




//The core of the application: how do we select which word we want?
void BurglishBuilder::reGenerateWordlist(const std::wstring& prevWord)
{
	//Reset
	set<wstring> resultLookup;
	generatedWords.clear();
	parenStr.str(L"");

	if (!typedRomanStr.str().empty()) {
		//Perform all combinations
		addStandardWords(typedRomanStr.str(), resultLookup, generatedWords, typeBeginsWithUpper, prevWord, savedCombinationIDs);
		addSpecialWords(typedRomanStr.str(), resultLookup, generatedWords, parenStr);
		addNumerals(typedRomanStr.str(), resultLookup, generatedWords);

		//Expand the set of words with common substitutions
		expandCurrentWords(resultLookup, generatedWords);
	}
}


//Reset our lookup
void BurglishBuilder::reset(bool fullReset)
{
	typedRomanStr.str(L"");
	generatedWords.clear();

	//More...
	currSelectedPage = 0;
	currSelectedID = 0;
	typeBeginsWithUpper = false;
	parenStr.str(L"");

	if (fullReset) {
		savedWordIDs.clear();
		savedCombinationIDs.clear();
	}
}


//Add a new letter
bool BurglishBuilder::typeLetter(char letter, bool isUpper, const std::wstring& prevWord)
{
	//Save
	wstring oldRoman = typedRomanStr.str();
	wstring oldParen = parenStr.str();
	bool oldIsUpper = typeBeginsWithUpper;

	//Typing pat-sint?
	if (isUpper && typedRomanStr.str().length()==0)
		typeBeginsWithUpper = true;

	//Attempt
	typedRomanStr <<letter;
	reGenerateWordlist(prevWord);

	//Rollback?
	if (generatedWords.empty()) {
		typedRomanStr.str(L"");
		typedRomanStr <<oldRoman;
		parenStr.str(L"");
		parenStr <<oldParen;
		typeBeginsWithUpper = oldIsUpper;
		reGenerateWordlist(prevWord);
		return false;
	}

	//Success
	currSelectedID = 0; //Reset
	currSelectedPage = 0; //Reset
	return true;  //Flag success
}



std::wstring BurglishBuilder::getParenString() const
{
	return parenStr.str();
}



//Get all possible words. Requires IDs, though.
//IDs are numbered starting after those in savedWordIDs() and savedDigitIDs()
//Note that a word in savedWordIDs and generatedWords might have 1..N
//   possible IDs. This isn't really a problem, as the words only build up as sentences are typed.
std::vector<unsigned int> BurglishBuilder::getPossibleWords() const
{
	//TEMP: For now, the word's ID is just its index. (We might need to hack around this for 0..9)
	vector<unsigned int> res;
	while (res.size()<generatedWords.size())
		res.push_back(savedDigitIDs.size() + savedWordIDs.size() + savedCombinationIDs.size() + res.size());
	return res;
}

std::vector<int> BurglishBuilder::getWordCombinations() const
{
	vector<int> res;
	for (vector< pair<wstring, int> >::const_iterator it=generatedWords.begin(); it!=generatedWords.end(); it++)
		res.push_back((it->second==-1) ? -1 : (savedDigitIDs.size() + savedWordIDs.size()) + it->second);
	return res;
}


std::pair<std::wstring, int> BurglishBuilder::getWordPair(unsigned int id) const
{
	if (id<savedDigitIDs.size())
		return pair<wstring, int>(savedDigitIDs[id], -1);
	id -= savedDigitIDs.size();
	if (id<savedWordIDs.size())
		return pair<wstring, int>(savedWordIDs[id], -1);
	id -= savedWordIDs.size();
	if (id<savedCombinationIDs.size())
		return pair<wstring, int>(savedCombinationIDs[id], -1);
	id -= savedCombinationIDs.size();

	return generatedWords[id];
}


//Simple.
//NOTE: An id is in the savedDigitIDs/savedWordIDs arrays, or it is after those, in the list of candidates.
std::wstring BurglishBuilder::getWordString(unsigned int id) const
{
	return getWordPair(id).first;
}



//Turns the current myanmar word into a list of all possible spellings.
//The original word is the first item in the list.
//Does not check for validity.
vector<wstring> BurglishBuilder::reverseExpandWords(const wstring& myanmar)
{
	//Add the current word
	set<wstring> resSet;
	vector<wstring> res;
	resSet.insert(myanmar);
	res.push_back(myanmar);
	unsigned int MAX_NUM_CONSIDERED = 5; //Reasonable for most words


	//Prepare a list of "candidates", along with starting indices
	vector< pair<wstring, unsigned int> > candidates;
	candidates.push_back(pair<wstring, unsigned int>(myanmar, 0));

	//Process each canidate, dynamically updating as you go along.
	std::wstringstream prefix;
	while (!candidates.empty()) {
		//Get the current word to check.
		wstring currMM = candidates[0].first;
		unsigned int startAt = candidates[0].second;
		candidates.erase(candidates.begin());

		//Reset the prefix string
		prefix.str(L"");
		prefix <<currMM.substr(0, startAt);

		//Build a new word based on this. Keep the old word handy too.
		for (size_t i=startAt; i<currMM.size(); i++) {
			//Used to store the replacement string, so we can put our
			// "new candidate" logic all in one place.
			size_t matchIndex = wstring::npos;
			wstring matchRep = L"";

			//Build the next bit
			if (currMM[i]==L'\u1025') {
				//First pattern:
				// \u1025 --> \u1021 \u102F
				matchIndex = i;
				matchRep = L"\u1021\u102F";
			} else if (!(i+1<currMM.size()) || (currMM[i]==L'\u1023' && currMM[i+1]!=L'\u102F')) {
				//Second pattern:
				// \u1023 [^\u102F]! --> \u1021 \u102D (don't replace !-expression)
				matchIndex = i;
				matchRep = L"\u1021\u102D";
			} else if (!(i+1<currMM.size()) || (currMM[i]==L'\u1029' && currMM[i+1]!=L'\u103A' && currMM[i+1]!=L'\u1037')) {
				//Third pattern:
				// \u1029 [^\u103A \u1037]! --> \u1021 \u1031 \u102C (don't replace !-expression)
				matchIndex = i;
				matchRep = L"\u1021\u1031\u102C";
			}

			//React if matched
			if (matchIndex!=wstring::npos) {
				//If we matched once, add ONLY the new word to the list of new candidates.
				wstring suffix = currMM.substr(i+1, currMM.length());
				wstring toAddNew = prefix.str() + matchRep + suffix;
				if (!toAddNew.empty() && res.size()<MAX_NUM_CONSIDERED)
					candidates.push_back(pair<wstring, unsigned int>(toAddNew, toAddNew.length()-suffix.length()));
			}

			//Either way, continue
			prefix <<currMM[i];
		}

		//Add. The set preserves uniformity.
		wstring word = currMM;//currWord.str();
		if (resSet.find(word)==resSet.end()) {
			resSet.insert(word);
			res.push_back(word);
		}
	}

	return res;
}



//Match the given myanmar word to its romanisation. Returns "" if no match can be made
string BurglishBuilder::matchSpecialWord(const wstring& myanmar)
{
	for (json_spirit::wmObject::iterator pair=specialWords.begin(); pair!=specialWords.end(); pair++) {
		wstring roman = pair->first;

		//Have to generate the myanmar words individually
		wstring specialStrs = pair->second.get_value<wstring>();
		wstringstream entry;
		for (size_t specID=0; specID<specialStrs.size(); specID++) {
			//Append?
			if (specialStrs[specID]!=L'|')
				entry <<specialStrs[specID];
			//Skip?
			if (specialStrs[specID]!=L'|' && specID<specialStrs.size()-1)
				continue;

			//Boundary reached; check
			wstring matcher = entry.str();
			if (matcher.length()>0 && matcher[0]==L'-') {
				entry.str(L"");
				for (size_t x=0; x<matcher.length(); x++) {
					if (matcher[x]!=L'-')
						entry <<matcher[x];
				}
				matcher = entry.str();
			}
			if (myanmar == matcher)
				return waitzar::escape_wstr(roman, false);

			//Finally: reset
			entry.str(L"");
		}
	}

	//No matches
	return "";
}



//TODO: Since we cache onset extensions, why not cache all possible onsets as well?
//      This will greatly speed up this function.
bool BurglishBuilder::matchOnsetFirstLetter(wchar_t letter)
{
	return BURGLISH_ONSET_CONSONANTS.find(wstring(1, letter))!=wstring::npos;

	//TEMP
/*	wstringstream temp;
	set<wchar_t> tempSet;

	//Iterate through each pair.
	for (json_spirit::wmObject::iterator it=onsetPairs.begin(); it!=onsetPairs.end(); it++) {
		const wstring& myanmar = it->second.get_value<wstring>();
		for (size_t i=0; i<myanmar.size(); i++) {
			//Does it match?
			//if (myanmar[i] == letter)
			//	return true;
			if (tempSet.find(myanmar[i])==tempSet.end()) {
				tempSet.insert(myanmar[i]);
				temp <<myanmar[i];
			}

			//Else, fast-forward to the next pipe character, "|"
			while (i<myanmar.size() && myanmar[i]!=L'|')
				i++;
		}
	}

	wstring temp2 = temp.str();

	//No matches
	return false;*/
}



pair<string, bool> BurglishBuilder::matchOnset(const wstring& myanmar)
{
	//Iterate through each pair.
	string unprefRes;
	for (json_spirit::wmObject::iterator it=onsetPairs.begin(); it!=onsetPairs.end(); it++) {
		wstring roman = it->first;
		const wstring& myStrs = it->second.get_value<wstring>();
		wstringstream entry;
		bool isPref = myStrs.size()>0 && myStrs[myStrs.size()-1] == L'+';
		for (size_t itemID=0; itemID<myStrs.size(); itemID++) {
			//Append?
			if (myStrs[itemID]!=L'|' && myStrs[itemID]!=L'+')
				entry <<myStrs[itemID];
			//Skip?
			if (myStrs[itemID]!=L'|' && itemID<myStrs.size()-1)
				continue;

			//Boundary reached; check
			if (myanmar == entry.str() && !entry.str().empty()) {
				if (isPref)
					return pair<string, bool>(waitzar::escape_wstr(roman, false), true);
				else if (unprefRes.empty())
					unprefRes = waitzar::escape_wstr(roman, false);
			}


			//Finally: reset
			entry.str(L"");
		}
	}

	//No matches, or an "unpreferred" match
	return pair<string, bool>(unprefRes, false);
}


pair<string, bool> BurglishBuilder::matchRhyme(const wstring& myanmar)
{
	//Iterate through each pair.
	string unprefRes;
	for (json_spirit::wmObject::iterator it=rhymePairs.begin(); it!=rhymePairs.end(); it++) {
		wstring roman = it->first;
		const wstring& myStrs = it->second.get_value<wstring>();
		wstringstream entry;
		bool isPref = myStrs.size()>0 && myStrs[myStrs.size()-1] == L'+';
		for (size_t itemID=0; itemID<myStrs.size(); itemID++) {
			//Append?
			if (myStrs[itemID]!=L'|' && myStrs[itemID]!=L'+')
				entry <<myStrs[itemID];
			//Skip?
			if (myStrs[itemID]!=L'|' && itemID<myStrs.size()-1)
				continue;

			//Boundary reached; check
			if (myanmar == entry.str() && !entry.str().empty()) {
				if (isPref)
					return pair<string, bool>(waitzar::escape_wstr(roman, false), true);
				else if (unprefRes.empty())
					unprefRes = waitzar::escape_wstr(roman, false);
			}

			//Finally: reset
			entry.str(L"");
		}
	}

	//No matches, or an "unpreferred" result
	return pair<string, bool>(unprefRes, false);
}




//We return what would be the "first match" in Burglish.
// For multiple spellings, we just return the one that matches first.
pair<int, string> BurglishBuilder::reverseLookupWord(std::wstring word)
{
	//Were we given a valid word?
	if (word.empty())
		return pair<int, string>(-1, "");

	//Sort the string. NOTE: a blank '\0' is sometimes appended for no reason
	//TODO: Fix sortMyanmarString() and remove trailing zeroes, then remove this workaround.
	wstringstream newWordPost;
	wstring newWord = waitzar::sortMyanmarString(word);
	for (size_t i=0; i<newWord.size(); i++) {
		if (newWord[i]!=L'\0')
			newWordPost <<newWord[i];
	}
	word = newWordPost.str();

	//Prepare a result string; we'll generate the ID later.
	string roman = "";

	//If the word itself is invalid, we can ONLY match against the special words.
	//Best to get that out of the way here.
	if (!IsValid(word)) {
		//Look it up directly.
		roman = matchSpecialWord(word);
	} else {
		//Special case: All numbers
		roman = string(word.length(), 'X'); //Allocate enough space.
		for (size_t i=0; i<word.length(); i++) {
			//Break if our assumption fails
			if (word[i]<L'\u1040' || word[i]>L'\u1049') {
				roman = ""; //"Still nothing"
				break;
			}

			//Else, keep building.
			roman[i] = (char)((word[i]-L'\u1040')+'0');
		}

		//Only continue if we haven't found it yet
		if (roman.empty()) {
			//Have to consider the effect of expandCurrentWords
			// Since some of them (like "oo" and "u") don't require expanding, we form a list of
			// possible words and their candidate romanisations. We keep the original word at the top of the list.
			vector< pair<wstring, string> > my2rom;
			vector<wstring> altSpellings = reverseExpandWords(word);
			for (vector<wstring>::iterator it=altSpellings.begin(); it!=altSpellings.end(); it++) {
				if (IsValid(*it))
					my2rom.push_back(pair<wstring, string>(*it, ""));
			}

			//Now, match all special words
			for (size_t i=0; i<my2rom.size(); i++) {
				if (!my2rom[i].second.empty())
					continue;
				my2rom[i].second = matchSpecialWord(my2rom[i].first);
			}

			//Next, all that's left is the set of regular matches. We'll need to match each
			// candidate, of course, and also check for of the "extended" letters (e.g., ya-yit)
			// that Burglish also allows in onsets.
			for (size_t i=0; i<my2rom.size(); i++) {
				if (!my2rom[i].second.empty()) {
					//For efficiency, break if the first candidate has matched already.
					//Otherwise, just skip this entry (continue)
					if (i==0)
						break;
					continue;
				}

				//Step 1: find the consonant. It is likely to be
				//   one of the first few letters. Additionally, each
				//   onset begins with a consonant (since we're working in Unicode)
				//   So, we'll just scan the onset list each time.
				wstring my = my2rom[i].first;
				size_t onsetIndex = wstring::npos;
				for (size_t myID=0; myID<my.length(); myID++) {
					if (matchOnsetFirstLetter(my[myID])) {
						onsetIndex = myID;
						break;
					}
				}
				if (onsetIndex==wstring::npos)
					break;

				//Step 2: Now, we need to consider each onset, starting with the base consonant,
				//        and appending any of the allowed onset extensions after that.
				wstringstream currOnset;
				for (size_t myID=onsetIndex; myID<my.length(); myID++) {
					//Are we done?
					if (myID!=onsetIndex) {
						//Only allow a few medials/vowels after the consonant.
						if (BURGLISH_ONSET_EXTENSIONS.find(wstring(1,my[myID]))==wstring::npos)
							break;
					}

					//Append
					currOnset <<my[myID];

					//We need to find the most PREFERED match. Preference goes:
					// onset+rhyme, onset, rhyme, none
					vector<string> prefRoman(4, "");

					//Search
					pair<string,bool> onsRomanPr = matchOnset(currOnset.str());
					string onsRoman = onsRomanPr.first;
					if (!onsRoman.empty()) {
						//We have a candidate onset. Now, we have to search for the rhyme.
						wstring suffix = my.substr(0, myID) + L"-" + my.substr(myID+1, my.length());
						pair<string, bool> rhymRomanPr = matchRhyme(suffix);
						string rhymRoman = rhymRomanPr.first;
						if (!rhymRoman.empty()) {
							//We have a match! Add it to the appropriate place.
							string newRoman = onsRoman + rhymRoman;
							if (onsRomanPr.second && rhymRomanPr.second) {
								prefRoman[0] = newRoman;
								break; //Will always be the best
							} else if (onsRomanPr.second) {
								if (prefRoman[1].empty())
									prefRoman[1] = newRoman; //Keep searching
							} else if (rhymRomanPr.second) {
								if (prefRoman[2].empty())
									prefRoman[2] = newRoman; //Keep searching
							} else {
								if (prefRoman[3].empty())
									prefRoman[3] = newRoman; //Keep searching
							}
						}
					}

					//Save the best match
					for (size_t j=0; j<prefRoman.size(); j++) {
						my2rom[i].second = prefRoman[j];
						if (!prefRoman[j].empty())
							break;
					}
				}
			}

			//At this point, the correct roman string is the first non-empty one in my2roman
			for (size_t consID=0; consID<my2rom.size()&&roman.empty(); consID++) {
				roman = my2rom[consID].second;
			}
		}
	}

	//Return the right word.
	///////////////////////////////
	// TODO: Set ID properly for words with a romanisation!
	//  NOTE: Can we always return -1? That way, the HelpKeyboard will track it externally...
	///////////////////////////////
	int id = roman.empty() ? -1 : -1;
	return pair<int, string>(id, roman);
}


//Just cut a letter off the string and update our list.
bool BurglishBuilder::backspace(const std::wstring& prevWord)
{
	if (typedRomanStr.str().empty())
		return false;

	wstring newStr = typedRomanStr.str().substr(0, typedRomanStr.str().size()-1);
	typedRomanStr.str(L"");
	typedRomanStr <<newStr;

	reGenerateWordlist(prevWord);
	return true;
}


//Simple, copied.
bool BurglishBuilder::moveRight(int amt)
{
	//Any words?
	if (generatedWords.size()==0)
		return false;

	//Any change?
	int newAmt = currSelectedID + amt;
	if (newAmt >= (int)generatedWords.size())
		newAmt = (int)generatedWords.size()-1;
	else if (newAmt < 0)
		newAmt = 0;
	if (newAmt == currSelectedID)
		return false;

	//Do it!
	currSelectedID = newAmt;

	//Auto-page
	currSelectedPage = currSelectedID / 10;

	return true;
}



//Simple, copied.
bool BurglishBuilder::pageUp(bool up)
{
	//Check
	int newID = currSelectedPage + (up?-1:1);
	if (newID<0 || newID>=getNumberOfPages())
		return false;

	//Page
	currSelectedPage = newID;

	//Select first word on that page
	currSelectedID = currSelectedPage * 10;

	return true;
}



//Simple, copied.
std::pair<int, int> BurglishBuilder::typeSpace(int quickJumpID)
{
	//We're at a valid stopping point?
	if (generatedWords.size() == 0)
		return pair<int, int>(-1, -1);

	//Quick jump
	if (!setCurrSelected(quickJumpID))
		return pair<int, int>(-1, -1);

	//Get the selected word, add it to the prefix array
	//NOTE: We save the IDs of previously-typed words.
	unsigned int newWord = savedDigitIDs.size() + savedWordIDs.size();
	unsigned int adjustedID = savedDigitIDs.size() + savedWordIDs.size() + savedCombinationIDs.size() + currSelectedID;

	//Adjust to pat-sint?
	int psWord = -1;
	if (getWordCombinations()[quickJumpID]!=-1) {
		adjustedID = getWordCombinations()[quickJumpID];
		psWord = newWord;
	}

	//Remove unwanted characters
	//TODO: Somewhere else I need a "remove all" function....
	wstring candidateStr = getWordString(adjustedID);
	{
		wstringstream toSave;
		for (size_t i=0; i<candidateStr.size(); i++) {
			if (candidateStr[i] != '-')
				toSave <<candidateStr[i];
		}
		if (!toSave.str().empty())
			candidateStr = toSave.str();
	}

	//Append, save.
	savedWordIDs.push_back(candidateStr);
	savedCombinationIDs.clear();

	//Reset the model, return this word
	this->reset(false);
	return pair<int, int>(newWord, psWord);
}


//Simple, copied.
int BurglishBuilder::getCurrPage() const
{
	return currSelectedPage;
}


//Simple, copied.
int BurglishBuilder::getCurrSelectedID() const
{
	return currSelectedID;
}


//Simple, copied.
int BurglishBuilder::getNumberOfPages() const
{
	int numWords = generatedWords.size();
	if (numWords % 10 == 0)
		return numWords / 10;
	else
		return numWords / 10 + 1;
}


//Simple, copied
bool BurglishBuilder::setCurrSelected(int id)
{
	//Any words?
	if (generatedWords.size()==0)
		return false;

	//Fail silently if this isn't a valid id
	if (id >= (int)generatedWords.size())
		return false;
	else if (id < 0)
		return false;

	//Do it!
	currSelectedID = id;
	return true;
}




//Simple, copied.
unsigned short BurglishBuilder::getStopCharacter(bool isFull) const
{
	//TEMP -- put somewhere global later.
	unsigned short punctHalfStopUni = 0x104A;
	unsigned short punctFullStopUni = 0x104B;

	if (isFull)
		return punctFullStopUni;
	else
		return punctHalfStopUni;
}


//Convert 0..9 to an ID.
unsigned short BurglishBuilder::getSingleDigitID(unsigned short arabicNumeral)
{
	//Return the word. (Returning 0 is a somewhat unsightly alternative, but at least it won't crash.)
	if (arabicNumeral>=0 && arabicNumeral<=9)
		return arabicNumeral; //"Always on"
	return 0;
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
