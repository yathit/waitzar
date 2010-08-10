/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "KeyMagicInputMethod.h"

using std::vector;
using std::map;
using std::pair;
using std::string;
using std::wstring;
using std::ifstream;
using std::ofstream;
using std::ios;


//"True" provides a full trace of all KeyMagic rule matches.
//bool KeyMagicInputMethod::LOG_KEYMAGIC_TRACE = false;
//string KeyMagicInputMethod::keyMagicLogFileName = "wz_log_keymagic.txt";


//Logging -- Just shuffle off onto our logger for now.
void KeyMagicInputMethod::clearLogFile()
{
	Logger::resetLogFile('K');
}
void KeyMagicInputMethod::writeLogLine()
{
	Logger::writeLogLine('K');
}
void KeyMagicInputMethod::writeLogLine(const wstring& logLine)
{
	Logger::writeLogLine('K', logLine);
}




KeyMagicInputMethod::KeyMagicInputMethod()
{
	KeyMagicInputMethod::clearLogFile();
}

KeyMagicInputMethod::~KeyMagicInputMethod()
{
}


const std::wstring KeyMagicInputMethod::emptyStr = L"";
const wstring& KeyMagicInputMethod::getOption(const wstring& optName)
{
	if (options.count(optName)>0)
		return options[optName];
	return emptyStr;
}

vector< pair<wstring, wstring> > KeyMagicInputMethod::convertToRulePairs()
{
	vector< pair<wstring, wstring> > res;

	//Add a rule for each pair; enforce that they're string/string pairs.
	for (size_t i=0; i<replacements.size(); i++) {
		Rule lhs = compressToSingleStringRule(replacements[i].match);
		Rule rhs = compressToSingleStringRule(replacements[i].replace);
		if (lhs.type!=KMRT_STRING)
			throw std::exception("Error: LHS is not of type \"string\".");
		if (rhs.type!=KMRT_STRING)
			throw std::exception("Error: RHS is not of type \"string\".");
		res.push_back(pair<wstring, wstring>(lhs.str, rhs.str));
	}

	return res;
}


void KeyMagicInputMethod::loadRulesFile(const string& rulesFilePath, const string& binaryFilePath, bool disableCache, std::string (*fileMD5Function)(const std::string&))
{
	//The first thing we need to do is determine whether we're loading the source file (text) or a binary compiled cache
	// of this file. Then, just pass off the relevant data to whichever function performs the relevant loading.
	// Finally, we may choose to cache the resultant file.
	bool reloadSourceText = true;
	string actualMD5 = fileMD5Function(rulesFilePath);
	if (!disableCache) {
		//Does the binary file exist?
		WIN32_FILE_ATTRIBUTE_DATA InfoFile;
		std::wstringstream temp;
		temp <<binaryFilePath.c_str();
		if (GetFileAttributesEx(temp.str().c_str(), GetFileExInfoStandard, &InfoFile)==TRUE) {
			//Open the file, get the expected checksum
			std::string expectedMD5;
			{
				ifstream binFile;
				binFile.open(binaryFilePath.c_str(), ios::in | ios::binary);
				unsigned char buffer[19];
				if (binFile.read((char*)(&buffer[0]), 19)) {
					//Check the version
					if (buffer[0]==KEYMAGIC_BINARY_VERSION) {
						//Check the BOM
						if (buffer[1]==0xFE && buffer[2]==0xFF) {
							//Read the checksum
							//TODO: Clean up
							std::stringstream digit;
							digit <<std::hex;
							for (size_t i=0; i<16; i++) {
								digit <<(unsigned int)buffer[3+i];
								if (digit.str().size()==1)
									expectedMD5 += "0";
								expectedMD5 += digit.str();
								digit.str("");
							}
						}
					}
				}
				binFile.close();
			}

			//Compare with the actual checksum
			if (!expectedMD5.empty()) {
				//Get it
				/*CryptoPP::Weak::MD5 hash;
				CryptoPP::FileSource(rulesFilePath.c_str(), true, new
					CryptoPP::HashFilter(hash,new CryptoPP::HexEncoder(new CryptoPP::StringSink(actualMD5),false))); */

				//Check
				if (actualMD5.length()!=32 || expectedMD5.length()!=32)
					throw std::exception("Bad MD5 length in KeyMagic binary file (or source file)");
				if (actualMD5==expectedMD5)
					reloadSourceText = false;
			}
		}
	}

	//If we have to reload the text file,do that here.
	if (reloadSourceText)
		loadTextRulesFile(rulesFilePath);
	else
		loadBinaryRulesFile(binaryFilePath);


	//Now, we may save the text file back as a binary file
	if (!disableCache && reloadSourceText)
		saveBinaryRulesFile(binaryFilePath, actualMD5);
}


int KeyMagicInputMethod::readInt(unsigned char* buffer, size_t& currPos, size_t bufferSize)
{
	if (currPos+2>bufferSize)
		throw std::exception("Error: buffer overrun when reading KeyMagic file");

	int res = buffer[currPos++]<<8;
	res |= buffer[currPos++];
	if (res==0xFFFF)
		return -1;
	return res;
}

void KeyMagicInputMethod::loadBinaryRulesFile(const string& binaryFilePath)
{
	//Open the file
	ifstream binFile;
	binFile.open(binaryFilePath.c_str(), ios::in | ios::binary);

	//Get the size, rewind
	binFile.seekg(0, ios::end);
	std::streampos file_size = binFile.tellg();
	binFile.seekg(0, ios::beg);

	//Load the entire file at once to minimize file I/O
	unsigned char* buffer = new unsigned char [file_size];
	binFile.read((char*)(&buffer[0]), file_size);
	binFile.close();

	//Step 1: Read header
	size_t pos = 0;
	int version = buffer[pos++];
	if (version!=KEYMAGIC_BINARY_VERSION)
		throw std::exception("Error: invalid Key Magic binary file version");
	int bom = readInt(buffer, pos, file_size);
	if (bom!=0xFEFF)
		throw std::exception("Error: invalid Key Magic binary file BOM");
	pos += 16; //Skip checksum
	int numSwitches = readInt(buffer, pos, file_size);
	for (int i=0; i<numSwitches; i++)
		switches.push_back(false);
	int numVariables = readInt(buffer, pos, file_size);
	int numReplacements = readInt(buffer, pos, file_size);

	//Step 2: Read all rules/variables
	for (int relID=0; relID<numVariables+numReplacements; relID++) {
		//Get the actual ID
		bool isVar = relID<numVariables;
		int actID = isVar ? relID : relID-numVariables;

		//Read three integers regardless
		size_t numSwitches = readInt(buffer, pos, file_size);
		size_t numMatches = readInt(buffer, pos, file_size);
		size_t numReplacements = readInt(buffer, pos, file_size);
		if (isVar && (numSwitches!=0 || numMatches!=0))
			throw std::exception("Error: Variable contains switches and matches in KeyMagic binary file.");

		//Read all switches into a vector for later.
		vector<unsigned int> switches;
		for (size_t i=0; i<numSwitches; i++)
			switches.push_back(readInt(buffer, pos, file_size));

		//Read all matches into one vector, and replacements into another.
		vector<Rule> left;
		vector<Rule> right;
		for (size_t relRuleID=0; relRuleID<numMatches+numReplacements; relRuleID++) {
			//Get the ID
			bool isLHS = relRuleID<numMatches;

			//Read the rule
			Rule r(KMRT_UNKNOWN, L"?", 0);
			int rType = buffer[pos++];
			switch(rType) {
				case 0:   r.type = KMRT_STRING;     break;
				case 1:   r.type = KMRT_WILDCARD;   break;
				case 2:   r.type = KMRT_VARIABLE;   break;
				case 3:   r.type = KMRT_MATCHVAR;   break;
				case 4:   r.type = KMRT_SWITCH;     break;
				case 5:   r.type = KMRT_VARARRAY;           break;
				case 6:   r.type = KMRT_VARARRAY_SPECIAL;   break;
				case 7:   r.type = KMRT_VARARRAY_BACKREF;   break;
				case 8:   r.type = KMRT_KEYCOMBINATION;     break;
				default:   throw std::exception("Error: bad rule \"type\" in binary Key Magic file");
			}
			r.val = readInt(buffer, pos, file_size);
			r.id = readInt(buffer, pos, file_size);
			size_t strSize = readInt(buffer, pos, file_size);
			std::wstringstream strStr;
			for (size_t i=0; i<strSize; i++)
				strStr <<(wchar_t)readInt(buffer, pos, file_size);
			r.str = strStr.str();

			//Save the rule
			if (isLHS)
				left.push_back(r);
			else
				right.push_back(r);
		}
		
		//Now, save these rules/matches as either a variable or a replacement
		if (isVar)
			variables.push_back(right);
		else {
			RuleSet res;
			res.match = left;
			res.replace = right;
			res.requiredSwitches = switches;
			replacements.push_back(res);
		}
	}

	//Done
	delete [] buffer;
}


void KeyMagicInputMethod::writeInt(vector<unsigned char>& stream, int intVal)
{
	//Special case 1
	if (intVal==-1) {
		stream.push_back('\xFF');
		stream.push_back('\xFF');
		return;
	}

	//In bounds?
	if (intVal<0 || intVal>=0xFFFF) {
		std::stringstream msg;
		msg <<"Integer is too big or too small: " <<intVal;
		throw std::exception(msg.str().c_str());
	}

	//Write first half, write second half
	stream.push_back((intVal>>8)&0xFF);
	stream.push_back(intVal&0xFF);
}

//TODO: Save and load the "options" stored in comment headers.
void KeyMagicInputMethod::saveBinaryRulesFile(const string& binaryFilePath, const string& checksum)
{
	//Save the file into a vector first; this will allow us to 
	//  minimize file I/O to one call.
	vector<unsigned char> binStream;
	binStream.reserve(1024); //Avoid slow startup

	//Write the header
	binStream.push_back(KEYMAGIC_BINARY_VERSION);
	writeInt(binStream, 0xFEFF);
	if (checksum.size()!=32)
		throw std::exception((string("Invalid checksum: ") + checksum).c_str());
	for (size_t i=0; i<16; i++) {
		int nextInt = (hexVal(checksum[i*2])<<4) | hexVal(checksum[i*2+1]);
		binStream.push_back((unsigned char)(nextInt&0xFF));
	}
	writeInt(binStream, switches.size());
	writeInt(binStream, variables.size());
	writeInt(binStream, replacements.size());

	//Write each variable and replacement
	for (size_t relID=0; relID<variables.size()+replacements.size(); relID++) {
		bool isVar = relID<variables.size();
		int actID = isVar ? relID : relID-variables.size();

		//Write the number of switches, matches, and replacements
		if (isVar) {
			writeInt(binStream, 0);
			writeInt(binStream, 0);
			writeInt(binStream, variables[actID].size());
		} else {
			writeInt(binStream, replacements[actID].requiredSwitches.size());
			writeInt(binStream, replacements[actID].match.size());
			writeInt(binStream, replacements[actID].replace.size());
		}

		//Write switches (if this is a replacement)
		if (!isVar) {
			for (size_t i=0; i<replacements[actID].requiredSwitches.size(); i++)
				writeInt(binStream, replacements[actID].requiredSwitches[i]);
		}

		//Write LHS (if this is a replacement) and RHS
		size_t totalRules = isVar ? (variables[actID].size()) : (replacements[actID].match.size() + replacements[actID].replace.size());
		for (size_t ruleRelID=0; ruleRelID<totalRules; ruleRelID++) {
			//Actually get this rule.
			bool transpose = !isVar && ruleRelID>=replacements[actID].match.size(); //Transpose to RHS
			int ruleActID = !transpose ? ruleRelID : ruleRelID-replacements[actID].match.size();
			Rule& r = isVar ? variables[actID][ruleActID] : !transpose ? replacements[actID].match[ruleActID] : replacements[actID].replace[ruleActID];

			//Write this rule.
			try {
				switch (r.type) {
					case KMRT_STRING:     binStream.push_back(0);   break;
					case KMRT_WILDCARD:   binStream.push_back(1);   break;
					case KMRT_VARIABLE:   binStream.push_back(2);   break;
					case KMRT_MATCHVAR:   binStream.push_back(3);   break;
					case KMRT_SWITCH:     binStream.push_back(4);   break;
					case KMRT_VARARRAY:   binStream.push_back(5);   break;
					case KMRT_VARARRAY_SPECIAL:     binStream.push_back(6);   break;
					case KMRT_VARARRAY_BACKREF:     binStream.push_back(7);   break;
					case KMRT_KEYCOMBINATION:  binStream.push_back(8);        break;
				}
				writeInt(binStream, r.val);
				writeInt(binStream, r.id);
				if (r.type!=KMRT_STRING) {
					writeInt(binStream, 0);
				} else {
					//Trim "string" values where not necessary.
					writeInt(binStream, r.str.length());
					for (size_t i=0; i<r.str.length(); i++) 
						writeInt(binStream, r.str[i]);
				}
			} catch (std::exception ex) {
				std::stringstream msg;
				msg <<"Error writing rule file: \n";
				msg <<ex.what() <<std::endl;
				if (isVar)
					msg <<"On variable:" <<actID <<std::endl;
				else {
					msg <<"On rule:\n";
					msg <<waitzar::escape_wstr(replacements[actID].debugRuleText, false) <<std::endl;
				}
				throw std::exception(msg.str().c_str());
			}
		}
	}

	//Now, convert what we've written into a native array.
	ofstream binFile;
	binFile.open(binaryFilePath.c_str(), ios::out | ios::binary);
	unsigned char* binArray = new unsigned char[binStream.size()];
	std::copy(binStream.begin(), binStream.end(), binArray);
	binFile.write((char*)(&binArray[0]), binStream.size());
	delete [] binArray;
	binFile.close();

}


void KeyMagicInputMethod::loadTextRulesFile(const string& rulesFilePath)
{
	vector<wstring> lines;
	{
		//Step 1: Load the file, convert to wchar_t*, skip the BOM
		size_t i = 0;
		wstring datastream = waitzar::readUTF8File(rulesFilePath);
		if (datastream[i] == L'\uFEFF')
			i++;
		else if (datastream[i] == L'\uFFFE')
			throw std::exception("KeyMagicInputMethod rules file  appears to be encoded backwards.");


		//First, parse into an array of single "rules" (or variables), removing 
		//  comments and combining multiple lines, etc.
		size_t sz = datastream.size();
		std::wstringstream line;
		wchar_t lastChar = L'\0';
		bool firstComment = true;
		bool onlywhitespace = true;
		for (; i<sz; i++) {
			//Skip comments: C-style
			if (i+1<sz && datastream[i]==L'/' && datastream[i+1]==L'*') {
				//Also track commentBegin and commentEnd, which will be equal to all text inside the /*,*/
				i += 2;
				size_t commentBegin = i;
				size_t commentEnd = i;
				while (i<sz && (datastream[i]!=L'/' || datastream[i-1]!=L'*'))
					commentEnd = ++i;
				commentEnd -= 2;
				//i--; //We want to parse the final newline.
				
				//Parse options if this is the first comment. 
				// An option begins on a newline and begins with an @, following possible whitespace.
				if (firstComment) {
					std::wstringstream currOptKey;
					std::wstringstream currOptVal;
					std::wstringstream* currOpt = &currOptKey;
					bool newline = true;
					for (size_t x=commentBegin; x<=commentEnd; x++)  {
						//If we just started a new line, skip whitespace and look for a @. Otherwise,
						// skip this entire new line too.
						if (newline) {
							newline = false;

							//Skip
							while (x<=commentEnd && (datastream[x]==L' ' || datastream[x]==L'\t'))
								x++;

							//Done?
							if (x>commentEnd)
								break;

							//At-sign?
							if (datastream[x] == L'@')
								continue; //Good.

							//Not a comment
							while (x<=commentEnd && datastream[x]!=L'\n')
								x++;

							//Done?
							if (x>commentEnd)
								break;

							//Otherwise, just process the newline later.
						}

						//Append the letter. We strictly support only lowercase characters.
						wchar_t c = datastream[x]; 
						if (c>=L'A' && c<=L'Z')
							c = (c-L'A') + 'a';
						if (c>='a' && c<='z')
							(*currOpt) <<c;

						//Advance the option
						if (datastream[x] == L'=')
							currOpt = &currOptVal;

						//Append the option
						if (datastream[x] == L'\n' || x==commentEnd) {
							//Only matters if we have a key/value pair.
							if (!currOptKey.str().empty() && !currOptVal.str().empty()) {
								options[currOptKey.str()] = currOptVal.str();
								currOptKey.str(L"");
								currOptVal.str(L"");
							}

							//Reset the pointer & flag
							currOpt = &currOptKey;
							newline = true;
						}
					}
				}

				firstComment = false;
				continue;
			}
			//Skip comments: Cpp-style
			if (i+1<sz && datastream[i]==L'/' && datastream[i+1]==L'/') {
				i += 2;
				while (i<sz && datastream[i]!=L'\n')
					i++;
				i--; //We want to process the newline.

				firstComment = false;
				continue;
			}

			//For debugging
			wchar_t currC = datastream[i];

			//Skip \r
			if (datastream[i]==L'\r')
				continue;

			//Newline causes a line break 
			if (datastream[i]==L'\n') {
				//...unless preceeded by "\"
				if (lastChar==L'\\') {
					lastChar = L'\0';
					continue;
				}

				if (!line.str().empty() && !onlywhitespace)
					lines.push_back(line.str());
				line.str(L"");
				lastChar = L'\0';
				onlywhitespace = true;
				continue;
			}

			//Anything else is just appended and saved (except \\ in some cases, and ' ' and '\t' in others)
			lastChar = datastream[i];
			if (lastChar==L'\t') //Replace tab with space
				lastChar = L' ';
			if (lastChar==L'\\' && ((i+1<sz && datastream[i+1]==L'\n')||(i+2<sz && datastream[i+1]==L'\r' && datastream[i+2]==L'\n')))
				continue;
			if (lastChar==L' ' && i>0 && (datastream[i-1]==L' '||datastream[i-1]==L'\t')) //Remove multiple spaces
				continue;
			line <<lastChar;

			//Update our flag
			if (onlywhitespace && lastChar!=' ' && lastChar!='\t' && lastChar!='\r' && lastChar!='\n')
				onlywhitespace = false;

			//Last line?
			if (i==sz-1 && !line.str().empty() && !onlywhitespace)
				lines.push_back(line.str());
		}
	}

	//Now, turn the rules into an understandable set of data structures
	//We have:
	//   $variable = rule+
	//   rule+ => rule+
	map< wstring, unsigned int> tempVarLookup;
	map< wstring, unsigned int> tempSwitchLookup;
	std::wstringstream rule;
	wstring prevLine = L"(N/A)";
	for (size_t id=0; id<lines.size(); id++) {
		wstring line = lines[id];
		rule.str(L"");

		//Break the current line into a series of:
		// item + ...+ item (=|=>) item +...+ item
		//Then apply logic to each individual item
		int separator = 0; //1 for =, 2 for =>
		size_t sepIndex = 0;
		vector<Rule> allRules;
		wchar_t currQuoteChar = L'\0';
		for (size_t i=0; i<line.size(); i++) {
			//Separator? (Must NOT be inside a string sequence)
			if (line[i]==L'=' && currQuoteChar==L'\0') {
				//= or =>
				separator = 1;
				if (i+1<line.size() && line[i+1] == L'>') {
					separator = 2;
					i++;
				}
				sepIndex = allRules.size()+1;

				//Interpret
				//TODO: Put this in a central place.
				try {
					allRules.push_back(parseRule(rule.str()));
				} catch (std::exception ex) {
					std::wstringstream err;
					err <<"File: " <<rulesFilePath.c_str() <<"\n";
					err <<ex.what();
					err <<"\nRule:\n";
					err <<line;
					err <<"\nPrevious rule:\n";
					err <<prevLine;
					throw std::exception(waitzar::escape_wstr(err.str(), false).c_str());
				}
				//Reset
				rule.str(L"");

				continue;
			}

			//Whitespace?
			if (iswspace(line[i]))
				continue;

			//String?
			if (line[i]==L'"' || line[i]==L'\'') {
				if (currQuoteChar==L'\0')
					currQuoteChar = line[i];
				else if (line[i]==currQuoteChar){
					//It gets a little tricky here...
					//First, check if the previous character was a "\". If not, then we're good.
					//Next, check if the previous TWO letters were backslashes. If so, that was a literal 
					// backslash, and we're also ok. Otherwise, we're not.
					if (i==0 || line[i-1]!=L'\\')
						currQuoteChar = L'\0';
					else if (i>1 && line[i-1]==L'\\' && line[i-2]==L'\\')
						currQuoteChar = L'\0';
				}
			}

			//Append the letter?
			if (line[i]!=L'+' || currQuoteChar!=L'\0')
				rule <<line[i];

			//New rule?
			//NOTE: This won't add the rule if there are ignorable characters (e.g., whitespace) at the end of the line
			if ((line[i]==L'+' || i==line.size()-1) && !rule.str().empty() && currQuoteChar==L'\0') {
				//Interpret
				try {
					allRules.push_back(parseRule(rule.str()));
				} catch (std::exception ex) {
					std::wstringstream err;
					err <<"File: " <<rulesFilePath.c_str() <<"\n";
					err <<ex.what();
					err <<"\nRule:\n";
					err <<line;
					err <<"\nPrevious rule:\n";
					err <<prevLine;
					throw std::exception(waitzar::escape_wstr(err.str(), false).c_str());
				}

				//Reset
				rule.str(L"");
			}
		}


		//Any leftover rules? (To-do: centralize)
		if (!rule.str().empty()) {
			//Interpret
			try {
				allRules.push_back(parseRule(rule.str()));
			} catch (std::exception ex) {
				std::wstringstream err;
				err <<"File: " <<rulesFilePath.c_str() <<"\n";
				err <<ex.what();
				err <<"\nRule:\n";
				err <<line;
				err <<"\nPrevious rule:\n";
				err <<prevLine;
				throw std::exception(waitzar::escape_wstr(err.str(), false).c_str());
			}

			//Reset
			rule.str(L"");
		}


		//Interpret and add it
		try {
			//Check
			if (separator==0)
				throw std::exception("Error: Rule does not contain = or =>");

			//Try to add
			addSingleRule(line, allRules, tempVarLookup, tempSwitchLookup, sepIndex, separator==1);
		} catch (std::exception ex) {
			std::wstringstream err;
			err <<"File: " <<rulesFilePath.c_str() <<"\n";
			err <<ex.what();
			err <<"\nRule:\n";
			err <<line;
			err <<"\nPrevious rule:\n";
			err <<prevLine;
			throw std::exception(waitzar::escape_wstr(err.str(), false).c_str());
		}

		//Save previous line; it sometimes helps with error messages.
		prevLine = line;
	}
}


int KeyMagicInputMethod::hexVal(wchar_t letter)
{
	switch(letter) {
		case L'0':  case L'1':  case L'2':  case L'3':  case L'4':  case L'5':  case L'6':  case L'7':  case L'8':  case L'9':
			return letter-L'0';
		case L'a':  case L'b':  case L'c':  case L'd':  case L'e':  case L'f':
			return letter-L'a' + 10;
		default:
			return -1;
	}
}


Rule KeyMagicInputMethod::parseRule(const std::wstring& ruleStr)
{
	//Initialize a suitable structure for our results.
	Rule result = Rule(KMRT_UNKNOWN, L"", 0);

	try {
		//Detection of type is a bit ad-hoc for now. At least, we need SOME length of string
		if (ruleStr.empty())
			throw std::exception("Error: Cannot create a rule from an empty string");
		wstring ruleLowercase = ruleStr;
		waitzar::loc_to_lower(ruleLowercase);

		//KMRT_VARIABLE: Begins with a $, must be followed by [a-zA-Z0-9_]
		//KMRT_MATCHVAR: Same, but ONLY numerals
		//KMRT_VARARRAY: Ends with "[" + ([0-9]+) + "]"
		//KMRT_VARARRAY_SPECIAL: Ends with "[" + [\*\^] + "]"
		//KMRT_VARARRAY_BACKREF: Ends with "[" + "$" + ([0-9]+) + "]"
		if (ruleStr[0] == L'$') {
			//Validate
			int numberVal = 0;
			int bracesStartIndex = -1;
			for (size_t i=1; i<ruleStr.length(); i++) {
				//First part: letters or numbers only
				wchar_t c = ruleStr[i];
				if (c>=L'0'&&c<=L'9') {
					if (numberVal != -1) {
						numberVal *= 10;
						numberVal += (c - L'0');
					}
					continue;
				}
				if ((c>=L'a'&&c<=L'z') || (c>=L'A'&&c<=L'Z') || (c==L'_')) {
					numberVal = -1;
					continue;
				}

				//Switch?
				if (c==L'[') {
					if (numberVal!=-1)
						throw std::exception("Invalid variable: cannot subscript number constants");

					//Flag for further parsing
					bracesStartIndex = i;
					break;
				}

				//Else, error
				throw std::exception("Invalid variable letter: should be alphanumeric.");
			}

			//No further parsing?
			if (bracesStartIndex==-1) {
				//Save
				if (numberVal != -1) {
					result.str = ruleStr;
					result.type = KMRT_MATCHVAR;
					result.val = numberVal;
				} else {
					result.str = ruleStr;
					result.type = KMRT_VARIABLE;
				}
			} else {
				//It's a complex variable.
				if (bracesStartIndex==ruleStr.length()-1 || ruleStr[ruleStr.length()-1]!=L']')
					throw std::exception("Invalid variable: bracket isn't closed.");
				result.str = ruleStr.substr(0, bracesStartIndex);
				
				//Could be [*] or [^]; check these first
				if (bracesStartIndex+1==ruleStr.length()-2 && (ruleStr[bracesStartIndex+1]==L'^' || ruleStr[bracesStartIndex+1]==L'*')) {
					result.type = KMRT_VARARRAY_SPECIAL;
					result.val = ruleStr[bracesStartIndex+1];
				} else if (ruleStr[bracesStartIndex+1]==L'$') {
					//It's a variable reference
					int total = 0;
					for (size_t id=bracesStartIndex+2; id<ruleStr.length()-1; id++) {
						if (ruleStr[id]>=L'0' && ruleStr[id]<=L'9') {
							total *= 10;
							total += (ruleStr[id]-L'0');
						} else 
							throw std::exception("Invalid variable: bracket variable contains non-numeric ID characters.");
					}
					if (total==0)
						throw std::exception("Invalid variable: bracket variable has no ID.");

					//Save
					result.type = KMRT_VARARRAY_BACKREF;
					result.val = total;
				} else {
					//It's a simple ID reference
					int total = 0;
					for (size_t id=ruleStr[bracesStartIndex+1]; id<ruleStr.length()-1; id++) {
						if (ruleStr[id]>=L'0' && ruleStr[id]<=L'9') {
							total *= 10;
							total += (ruleStr[id]-L'0');
						} else 
							throw std::exception("Invalid variable: bracket id contains non-numeric ID characters.");
					}
					if (total==0)
						throw std::exception("Invalid variable: bracket id has no ID.");

					//Save
					result.type = KMRT_VARARRAY;
					result.val = total;
				}
			}
		}


		//KMRT_STRING: Enclosed with ' or "
		else if (ruleStr.size()>1 && ((ruleStr[0]==L'\''&&ruleStr[ruleStr.size()-1]==L'\'') || ((ruleStr[0]==L'"'&&ruleStr[ruleStr.size()-1]==L'"')))) {
			//Escaped strings should already have been taken care of (but not translated)
			std::wstringstream buff;
			for (size_t i=1; i<ruleStr.length()-1; i++) {
				//Normal letters
				if (ruleStr[i]!=L'\\') {
					buff <<ruleStr[i];
					continue;
				}

				//Escape sequences
				// Can be: \\, \", \uXXXX
				if (i+1<ruleStr.length()-1 && ruleStr[i+1]==L'\\') {
					buff <<L'\\';
					i++;
				} else if (i+1<ruleStr.length()-1 && (ruleStr[i+1]==L'"'||ruleStr[i+1]==L'\'')) {
					buff <<wstring(1, ruleStr[i+1]);
					i++;
				} else if (i+5<ruleStr.length()-1 && (ruleLowercase[i+1]==L'u') && hexVal(ruleLowercase[i+2])!=-1 && hexVal(ruleLowercase[i+3])!=-1 && hexVal(ruleLowercase[i+4])!=-1 && hexVal(ruleLowercase[i+5])!=-1) {
					int num = hexVal(ruleLowercase[i+2])*0x1000 + hexVal(ruleLowercase[i+3])*0x100 + hexVal(ruleLowercase[i+4])*0x10 + hexVal(ruleLowercase[i+5]);
					buff <<(wchar_t)num;
					i += 5;
				} else {
					std::wstringstream tempEsc;
					for (size_t eOff=0; eOff<=4; eOff++) {
						size_t eID = eOff + i;
						if (eID>=0 && eID<ruleStr.length())
							tempEsc <<wstring(1, ruleStr[eID]);
					}
					throw std::exception(waitzar::glue(L"Invalid escape sequence around: ", tempEsc.str()).c_str());
				}
			}
			result.type = KMRT_STRING;
			result.str = buff.str();
		}


		//KMRT_STRING: We translate NULL/null to empty strings
		else if (ruleLowercase == L"null") {
			result.type = KMRT_STRING;
			result.str = L"";
		}


		//KMRT_STRING: The VK_* keys can be handled as single-character strings
		else if (ruleStr.size()>3 && ruleStr[0]==L'V' && ruleStr[1]==L'K' && ruleStr[2]==L'_') {
			result.type = KMRT_STRING;
			result.val = -1;
			result.str = L"X";
			for (size_t id=0; !KeyMagicVKeys[id].keyName.empty(); id++) {
				if (KeyMagicVKeys[id].keyName == ruleStr) {
					result.val = KeyMagicVKeys[id].keyValue;
					result.str[0] = (wchar_t)result.val;
					break;
				}
			}
			if (result.val == -1)
				throw std::exception("Unknown VKEY specified");
		}


		//KMRT_STRING: Unicode letters are converted here
		else if (ruleStr.length()==5 && ruleLowercase[0]==L'u' && hexVal(ruleLowercase[1])!=-1 && hexVal(ruleLowercase[2])!=-1 && hexVal(ruleLowercase[3])!=-1 && hexVal(ruleLowercase[4])!=-1) {
			result.type = KMRT_STRING;
			result.str = L"X";

			//Convert
			result.val = hexVal(ruleLowercase[1])*0x1000 + hexVal(ruleLowercase[2])*0x100 + hexVal(ruleLowercase[3])*0x10 + hexVal(ruleLowercase[4]);
			result.str[0] = (wchar_t)result.val;
		}

		
		//KMRT_WILDCARD: The * wildcard
		else if (ruleStr == L"*") {
			result.type = KMRT_WILDCARD;
			result.str = L"*";
		}


		//KMRT_SWITCH: Enclosed in (....); switches are also named with quotes (single or double)
		else if (ruleStr.size()>1 && ruleStr[0]==L'(' && ruleStr[ruleStr.size()-1]==L')') {
			//Ensure proper quotation (and something inside
			if (ruleStr.size()>4 && ((ruleStr[1]==L'\''&&ruleStr[ruleStr.size()-2]==L'\'') || (ruleStr[1]==L'"'&&ruleStr[ruleStr.size()-2]==L'"'))) {
				result.type = KMRT_SWITCH;
				result.str = ruleStr.substr(2, ruleStr.length()-4);
			} else 
				throw std::exception("Bad 'switch' type rule");
		}
		

		//KMRT_KEYCOMBINATION: <VK_SHIFT & VK_T>
		else if (ruleStr.size()>2 && ruleStr[0]==L'<' && ruleStr[ruleStr.length()-1]==L'>') {
			//Read each value into an array
			vector<wstring> vkeys;
			std::wstringstream currKey;
			for (size_t id=0; id<ruleStr.length(); id++) {
				//Append?
				if ((ruleStr[id]>='A'&&ruleStr[id]<='Z') || (ruleStr[id]>='0'&&ruleStr[id]<='9') || ruleStr[id]==L'_')
					currKey <<ruleStr[id];

				//New key?
				if ((ruleStr[id]==L'&' || ruleStr[id]==L'>') && !ruleStr.empty()) {
					vkeys.push_back(currKey.str());
					currKey.str(L"");
				}
			}

			//Handle the final key
			result.type = KMRT_KEYCOMBINATION;
			result.val = -1;
			if (vkeys.empty())
				throw std::exception("Invalid VKEY: nothing specified");
			for (size_t id=0; !KeyMagicVKeys[id].keyName.empty(); id++) {
				if (KeyMagicVKeys[id].keyName == vkeys[vkeys.size()-1]) {
					result.val = KeyMagicVKeys[id].keyValue;
					break;
				}
			}
			if (result.val == -1)
				throw std::exception(waitzar::glue(L"Unknown VKEY specified \"", vkeys[vkeys.size()-1], L"\"").c_str());

			//Now, handle all modifiers
			for (size_t id=0; id<vkeys.size()-1; id++) {
				if (vkeys[id]==L"VK_SHIFT" || vkeys[id] == L"VK_LSHIFT" || vkeys[id] == L"VK_RSHIFT") {
					result.val |= KM_VKMOD_SHIFT;
				} else if (vkeys[id]==L"VK_CONTROL" || vkeys[id]==L"VK_CTRL" || vkeys[id] == L"VK_LCONTROL" || vkeys[id] == L"VK_RCONTROL" || vkeys[id] == L"VK_LCTRL" || vkeys[id] == L"VK_RCTRL") {
					result.val |= KM_VKMOD_CTRL;
				} else if (vkeys[id]==L"VK_ALT" || vkeys[id]==L"VK_MENU" || vkeys[id] == L"VK_LALT" || vkeys[id] == L"VK_RALT" || vkeys[id] == L"VK_LMENU" || vkeys[id] == L"VK_RMENU") {
					result.val |= KM_VKMOD_ALT;
				} else if (vkeys[id]==L"VK_CAPSLOCK" || vkeys[id]==L"VK_CAPITAL") {
					result.val |= KM_VKMOD_CAPS;
				} else {
					throw std::exception(waitzar::glue(L"Unknown VKEY specified as modifier \"", vkeys[id], L"\"").c_str());
				}
			}
		}

		//Done?
		if (result.type==KMRT_UNKNOWN)
			throw std::exception("Error: Unknown rule type");
	} catch (std::exception ex) {
		throw std::exception(waitzar::glue(ex.what(), "  \"", waitzar::escape_wstr(ruleStr, false), "\"").c_str());
	}

	return result;
}



//This function will likely be replaced by something more powerful later
Rule KeyMagicInputMethod::compressToSingleStringRule(const std::vector<Rule>& rules)
{
	//Init
	Rule res(KMRT_STRING, L"", 0);

	//Combine all strings, and all variables which point ONLY to strings
	for (std::vector<Rule>::const_iterator topRule = rules.begin(); topRule!=rules.end(); topRule++) {
		if (topRule->type==KMRT_STRING) {
			res.str += topRule->str;
		} else if (topRule->type==KMRT_VARARRAY) {
			if (variables[topRule->id].size()!=1 || variables[topRule->id][0].type!=KMRT_STRING)
				throw std::exception("Error: Variable depth too great for '^' or '*' reference as \"VARARRAY\"");
			res.str += variables[topRule->id][0].str[topRule->val];
		} else if (topRule->type==KMRT_VARIABLE) {
			if (variables[topRule->id].size()!=1 || variables[topRule->id][0].type!=KMRT_STRING)
				throw std::exception("Error: Variable depth too great for '^' or '*' reference as \"VARIABLE\"");
			res.str += variables[topRule->id][0].str;
		} else
			throw std::exception("Error: Variable accessed with '*' or '^', but points to non-obvious data structure.");
	}

	return res;
}




//Validate (and slightly transform) a set of rules given a start and an end index
//We assume that this is never called on the LHS of an assigment statement.
vector<Rule> KeyMagicInputMethod::createRuleVector(const vector<Rule>& rules, const map< wstring, unsigned int>& varLookup, map< wstring, unsigned int>& switchLookup, std::vector<unsigned int>& switchesUsed, size_t iStart, size_t iEnd, bool condenseStrings)
{
	//Behave differently depending on the rule type
	vector<Rule> res;
	for (size_t i=iStart; i<iEnd; i++) {
		Rule currRule = rules[i];
		switch (currRule.type) {
			case KMRT_UNKNOWN:
				throw std::exception("Error: A rule was discovered with type \"KMRT_UNKNOWN\"");

			case KMRT_STRING:
				//If a string is followed by another string, combine them.
				if (condenseStrings) {
					while (i+1<iEnd && rules[i+1].type==KMRT_STRING) {
						currRule.str += rules[i+1].str;
						i++;
					}
				}
				break;

			case KMRT_VARIABLE:
			case KMRT_VARARRAY:
			case KMRT_VARARRAY_SPECIAL:
			case KMRT_VARARRAY_BACKREF:
				//Make sure all variables exist; fill in their implicit ID
				//NOTE: We cannot assume variables before they are declared; this would allow for circular references.
				if (varLookup.count(currRule.str)==0)
					throw std::exception(waitzar::glue(L"Error: Previously undefined variable \"", currRule.str, L"\"").c_str());
				currRule.id = varLookup.find(currRule.str)->second;

				//Special check
				if (currRule.type==KMRT_VARARRAY_SPECIAL || currRule.type==KMRT_VARARRAY_BACKREF) {
					//For now, we need to treat these as arrays of strings. To avoid crashing at runtime, we
					//   attempt to conver them here. (We let the result fizzle)
					compressToSingleStringRule(variables[currRule.id]);
				}
				break;

			case KMRT_SWITCH:
				//Make sure the switch exists, and fill in its implicit ID
				if (switchLookup.count(currRule.str)==0) {
					switchLookup[currRule.str] = switches.size();
					switches.push_back(false);
				}
				currRule.id = switchLookup.find(currRule.str)->second;
				switchesUsed.push_back(currRule.id);
				break;
		}

		//Add all rules that consume input.
		if (condenseStrings || (currRule.type!=KMRT_SWITCH)) //Only keep the switch if we're on the RHS.
			res.push_back(currRule);
	}

	return res;
}




//Add a rule to our replacements/variables
void KeyMagicInputMethod::addSingleRule(const std::wstring& fullRuleText, const vector<Rule>& rules, map< wstring, unsigned int>& varLookup, map< wstring, unsigned int>& switchLookup, size_t rhsStart, bool isVariable)
{
	//
	// Step 1: Validate
	//

	//Simple checks 1,2: Variables are correct
	if (isVariable && rhsStart!=1)
		throw std::exception("Rule error: Variables cannot have multiple assignments left of the parentheses");
	if (isVariable && rules[0].type!=KMRT_VARIABLE)
		throw std::exception("Rule error: Assignment ($x = y) can only assign into a variable.");

	//LHS checks: no backreferences
	for (size_t i=0; i<rhsStart; i++) {
		if (rules[i].type==KMRT_MATCHVAR || rules[i].type==KMRT_VARARRAY_BACKREF)
			throw std::exception("Cannot have backreference ($1, $2, or $test[$1]) on the left of an expression");
	}

	//RHS checks: no ambiguous wildcards
	for (size_t i=rhsStart; i<rules.size(); i++) {
		if (rules[i].type==KMRT_WILDCARD || rules[i].type==KMRT_VARARRAY_SPECIAL)
			throw std::exception("Cannot have wildcards (*, $test[*], $test[^]) on the right of an expression");
	}

	//TODO: We might consider checking the number of backreferences against the $1...${n} values, but I don't 
	//      think it's necessary. We can also check this elsewhere.


	//
	// Step 2: Modify
	//

	//Compute the RHS string first (this also avoids circular variable references)
	std::vector<unsigned int> temp;
	std::vector<Rule> rhsVector = createRuleVector(rules, varLookup, switchLookup, temp, rhsStart, rules.size(), true);

	//LHS storage depends on if this is a variable or not
	if (isVariable) {
		//TODO: Allow switches in variables under some conditions
		if (!temp.empty())
			throw std::exception("Error: At the moment, Key Magic on WaitZar does not support switches inside of variables");

		//Make sure it's in the array ONLY once. Then add it.
		const wstring& candidate = rules[0].str;
		if (varLookup.count(candidate) != 0)
			throw std::exception(waitzar::glue(L"Error: Duplicate variable definition: \"", candidate, L"\"").c_str());
		varLookup[candidate] = variables.size();
		variables.push_back(rhsVector);
	} else {
		//Get a similar LHS vector, add it to our replacements list
		std::vector<unsigned int> switches;
		std::vector<Rule> lhsVector = createRuleVector(rules, varLookup, switchLookup, switches, 0, rhsStart, false); //We need to preserve groupings
		RuleSet rule;
		rule.match = lhsVector;
		rule.replace = rhsVector;
		rule.requiredSwitches = switches;
		rule.debugRuleText = fullRuleText;
		replacements.push_back(rule);
	}
}



//NOTE: This function will update matchedOneVirtualKey
pair<Candidate, bool> KeyMagicInputMethod::getCandidateMatch(RuleSet& rule, const wstring& input, unsigned int vkeyCode, bool& matchedOneVirtualKey)
{
	//Check all switches that this rule relies on:
	for (unsigned int i=0; i<rule.requiredSwitches.size(); i++) {
		//Switch: only if that switch is ON, and matching will turn that switch OFF later.
		if (!switches[rule.requiredSwitches[i]]) {
			//It won't work; a switch is off
			vector<Rule> temp;
			return pair<Candidate, bool>(Candidate(temp), false);
		}

	}

	//Skip entries that obviously will never match?
	//NOTE: This only checks TOP-level key_combination matches. If someone were to put
	//      a match inside a $var[*], we'd have to catch that later.
	//TODO: Cache this upon loading the rule.
	if (matchedOneVirtualKey) {
		for (size_t i=0; i<rule.match.size(); i++) {
			if (rule.match[i].type==KMRT_KEYCOMBINATION) {
				//Can't work; we've already matched a key combination. 
				vector<Rule> temp;
				return pair<Candidate, bool>(Candidate(temp), false);
			}
		}
	}

	//Get an estimate of rule size (useful later)
	//NOTE: This will always be <= the actual size of the match.
	//TODO: Cache and build this when the rule first loads.
	int estRuleSize = 0;
	for (size_t i=0; i<rule.match.size(); i++) {
		if (rule.match[i].type==KMRT_STRING)
			estRuleSize += rule.match[i].str.length();
		else if (rule.match[i].type!=KMRT_SWITCH)
			estRuleSize++;
	}

	vector<Candidate> candidates;
	for (size_t dot=0; dot<input.length(); dot++) {
		//Skip entries that are obviously too big to finish at the end of the string
		int lenLeft = input.length()-dot;

		//Add a new empty candidate
		if (estRuleSize <= lenLeft)
			candidates.push_back(Candidate(rule, dot));

		//Continue matching.
		//Note: For now, the candidates list won't expand while we move the dot. (It might shrink, however)
		//      Later, we will have to dynamically update i, but this should be fairly simple.
		for (vector<Candidate>::iterator curr=candidates.begin(); curr!=candidates.end();) {
			//Before attemping to "move the dot", we must expand any variables by adding new entries to the stack.
			//NOTE: We also need to handle switches, which shouldn't move the dot anyway
			bool allow = true;
			while (allow && curr->getCurrRule().type==KMRT_VARIABLE) {
				//Variable: Push to stack
				curr->newCurr(variables[curr->getCurrRule().id]);
			}


			//TODO: Our code for handling switches doesn't work properly.We need to look at the entire loop & re-do it.


			//"Moving the dot" is allowed under different circumstances, depending on the type of rule we're dealing with.
			if (allow) {
				switch(curr->getCurrRule().type) {
					//Wildcard: always move
					case KMRT_WILDCARD:
						curr->advance(input[dot], -1);
						break;

					//String: only if the current char matches
					case KMRT_STRING:
						if (curr->getCurrStringRuleChar() == input[dot])
							curr->advance(input[dot], -1);
						else
							allow = false;
						break;

					//Vararray: Only if that particular character matches
					//Silently fail to match if an invalid index is given.
					case KMRT_VARARRAY:
					{
						//TODO: Right now, this fails for anything except a string array (and anything simple)
						Rule toCheck  = compressToSingleStringRule(variables[curr->getCurrRule().id]);
						wstring str = toCheck.str;
						if (curr->getCurrRule().val>0 && curr->getCurrRule().val<=(int)str.length() && (str[curr->getCurrRule().val-1] == input[dot]))
							curr->advance(input[dot], -1);
						else
							allow = false;
						break;
					}

					//Match "any" or "not any"; silently fail on anything else.
					case KMRT_VARARRAY_SPECIAL:
						if (curr->getCurrRule().val!='*' && curr->getCurrRule().val!='^')
							allow = false;
						else {
							//TODO: Right now, this fails silently for anything except strings and simple variables.
							Rule toCheck  = compressToSingleStringRule(variables[curr->getCurrRule().id]);
							int foundID = -1;
							for (size_t i=0; i<toCheck.str.length(); i++) {
								//Does it match?
								if (toCheck.str[i] == input[dot]) {
									foundID = i;
									break;	
								}
							}

							//We allow if '*' and found, or '^' and not
							if (curr->getCurrRule().val=='*' && foundID==-1)
								allow = false;
							else if (curr->getCurrRule().val=='^' && foundID!=-1)
								allow = false;
							else
								curr->advance(input[dot], foundID);
						}
						break;

					//Key combinations only match if this is the LAST letter.
					//By definition, if one matches, it will replace the last letter (however, it won't 
					//   then cause a paradox, as key combinations will be diabled for remaining matches.
					case KMRT_KEYCOMBINATION:
						allow = false;
						if (!matchedOneVirtualKey && dot==input.length()-1) {
							//Move on the VKEY?
							if (curr->getCurrRule().val==vkeyCode) {
								curr->advance(input[dot], -1);
								if (curr->isDone()) {
									//Kind of hackish, but it works (for now).
									matchedOneVirtualKey = true;
									curr->dotEndID = input.length();
									return pair<Candidate, bool>(*curr, true);
								}
							}
						}

						break;

					//"Error" and "quasi-error" cases.
					case KMRT_VARIABLE:
						throw std::exception("Bad match: variables should be dealt with outside the main loop.");
					case KMRT_SWITCH:
						throw std::exception("Bad match: switches should be dealt with outside the main loop.");
					default:
						throw std::exception("Bad match: inavlid rule type.");
				}
			}

			//Did this rule pass or fail?
			bool eraseFlag = false;
			bool isDone = curr->isDone();
			if (!allow) {
				//Remove the current entry; decrement the pointer
				curr = candidates.erase(curr);
				eraseFlag = true;
				//continue;
			}

			//Did this match finish?
			if (allow && isDone) {
				//Are we also at the end of the string?
				if (dot==input.length()-1) {
					//Match successful
					curr->dotEndID = dot+1;
					return pair<Candidate, bool>(*curr, true);
				} else {
					//Match failed; too short
					curr = candidates.erase(curr);
					eraseFlag = true;
				}
			}

			//Did we just erase an element? If so, the pointer doesn't increment
			if (eraseFlag) 
				eraseFlag = false;
			else
				curr++;
		}
	}

	//No matches: past the end of the input string.
	vector<Rule> temp;
	return pair<Candidate, bool>(Candidate(temp), false);
}


//NOTE: resetLoop and breakLoop are changed by this function.
//NOTE: breakLoop takes precedence
wstring KeyMagicInputMethod::applyMatch(const Candidate& result, bool& resetLoop, bool& breakLoop)
{
	//Reset flags
	resetLoop = false;
	breakLoop = false;

	//Turn "off" all switches that were matched
	const vector<unsigned int>& switchesToOff = result.getPendingSwitches();
	for (vector<unsigned int>::const_iterator it=switchesToOff.begin(); it!=switchesToOff.end(); it++)
		switches[*it] = false;

	//We've got a match! Apply our replacements algorithm
	std::wstringstream replacementStr;
	for (std::vector<Rule>::iterator repRule=result.replacementRules.begin(); repRule!=result.replacementRules.end(); repRule++) {
		switch(repRule->type) {
			//String: Just append it.
			case KMRT_STRING:
				replacementStr <<repRule->str;
				break;

			//Variable: try to append that variable.
			case KMRT_VARIABLE:
			{
				//To-do: Right now, this only applies for simple & semi-complex rules.
				Rule toAdd  = compressToSingleStringRule(variables[repRule->id]);
				replacementStr <<toAdd.str;
				break;
			}

			//Turn switches back on.
			case KMRT_SWITCH:
				switches[repRule->id] = true;
				break;

			//Vararray: just add that character
			case KMRT_VARARRAY:
			{
				Rule toAdd  = compressToSingleStringRule(variables[repRule->id]);
				replacementStr <<toAdd.str[repRule->val-1];
				break;
			}

			//Variable: Just add the saved backref
			case KMRT_MATCHVAR:
				replacementStr <<result.getMatch(repRule->val-1);
				break;

			//Vararray backref: Requires a little more indexing
			case KMRT_VARARRAY_BACKREF:
			{
				Rule toAdd  = compressToSingleStringRule(variables[repRule->id]);
				replacementStr <<toAdd.str[result.getMatchID(repRule->val-1)];
 				break;
			}

			//Anything else (WILDCARD, VARARRAY_SPECIAL , KEYCOMBINATION) is an error.
			default:
				throw std::exception("Bad match: inavlid replacement type.");
		}
	}

	//Apply the "single ASCII replacement" rule
	//NOTE: We need to check "single letters" in $var[$1] as well... the easiest way to check this is to just use the "replacementStr" variable.
	//if (result.replacementRules.size()==1 && result.replacementRules[0].type==KMRT_STRING && result.replacementRules[0].str.length()==1) {
	if (replacementStr.str().length()==1) {
		//wchar_t ch = result.replacementRules[0].str[0];
		wchar_t ch = replacementStr.str()[0];
		if (ch>L'\x020' && ch<L'\x07F') {
			breakLoop = true;
		}
	}

	//Save new string
	wstring newStr = replacementStr.str();

	//Reset for the next rule.
	resetLoop = true;

	return newStr;
}



void KeyMagicInputMethod::handleStop(bool isFull, VirtKey& vkey)
{
	//Handle this as a regular key press
	wstring currStr = this->isHelpInput() ? typedCandidateStr.str() : typedSentenceStr.str();
	pair<wstring, bool> next = appendTypedLetter(currStr, vkey);
	if (this->isHelpInput()) {
		typedCandidateStr.str(L"");
		typedCandidateStr <<next.first;
	} else {
		typedSentenceStr.str(L"");
		typedSentenceStr <<next.first;
	}
	viewChanged = true;
}



void KeyMagicInputMethod::handleBackspace(VirtKey& vkey)
{
	//Simply handle this key-press
	wstring currStr = this->isHelpInput() ? typedCandidateStr.str() : typedSentenceStr.str();
	pair<wstring, bool> next = appendTypedLetter(currStr, vkey);
	if (currStr == next.first) {
		//Default backspace rules
		LetterInputMethod::handleBackspace(vkey);
	} else {
		if (this->isHelpInput()) {
			typedCandidateStr.str(L"");
			typedCandidateStr <<next.first;
			updateRomanHelpString(); //TODO: Fix; bad to have this in two places...
		} else {
			typedSentenceStr.str(L"");
			typedSentenceStr <<next.first;
		}
		viewChanged = true;
	}
}




pair<wstring, bool> KeyMagicInputMethod::appendTypedLetter(const wstring& prevStr, VirtKey& vkey)
{
	//Append, apply rules
	char c = vkey.alphanum;
	if (c>='a' && c<='z' && vkey.modShift) 
		c = (c-'a') + 'A';

	//Don't append "unknown" key strokes.
	wstring appended = prevStr;
	//if (c!='\0')
	appended += wstring(1,c);

	//Now apply.
	wstring result = applyRules(appended, vkey.toKeyMagicVal());
	//Remove all trailing '\0's //TODO: Replace this with a "removeall" at some point...
	for (;;) {
		size_t zeroIndex = result.rfind(L'\0');
		if (zeroIndex==wstring::npos)
			break;
		result.replace(zeroIndex, 1, L"");
	}
	return pair<wstring, bool>(result, true);
}



wstring KeyMagicInputMethod::applyRules(const wstring& origInput, unsigned int vkeyCode)
{
	KeyMagicInputMethod::writeLogLine(L"User typed:  " + origInput);

	//Volatile version of our input
	wstring input = origInput;

	//First, turn all switches off ///WHY?
	/*for (size_t i=0; i<switches.size(); i++)
		switches[i] = false;*/

	//For each rule, generate and match a series of candidates
	//  As we do this, move the "dot" from position 0 to position len(input)
	bool matchedOneVirtualKey = false;
	bool resetLoop = false;
	bool breakLoop = false;
	unsigned int totalMatchesOverall = 0;
	for (int rpmID=0; rpmID<(int)replacements.size(); rpmID++) {
		//Somewhat of a hack...
		if (resetLoop) {
			rpmID = 0;
			resetLoop = false;
		}

		//Match this rule
		pair<Candidate, bool> result = getCandidateMatch(replacements[rpmID], input, vkeyCode, matchedOneVirtualKey);

		//Did we match anything?
		if (result.second) {
			//Log match rule
			KeyMagicInputMethod::writeLogLine(L"   " + (!replacements[rpmID].debugRuleText.empty() ? replacements[rpmID].debugRuleText : L"<Empty Rule Text>"));

			//Before we apply the rule, check if we've looped "forever"
			if (++totalMatchesOverall >= std::max<size_t>(50, replacements.size())) {
				//To do: We might also consider logging this, later.
				throw std::exception(waitzar::glue(L"Error on keymagic regex; infinite loop on input: \n   ", input).c_str());
			}

			//Apply, update input.
			wstring part1 = input.substr(0, result.first.dotStartID);
			wstring part2 = applyMatch(result.first, resetLoop, breakLoop);
			wstring part3 = input.substr(result.first.dotEndID, input.size());
			input = part1 + part2 + part3;
			KeyMagicInputMethod::writeLogLine(L"      ==>" + input);

			if (breakLoop)
				break;
		}
	}
	KeyMagicInputMethod::writeLogLine();

	return input;
}


void KeyMagicInputMethod::reset(bool resetCandidates, bool resetRoman, bool resetSentence, bool performFullReset)
{
	//Reset all switches.
	if (performFullReset) {
		for (size_t i=0; i<switches.size(); i++) 
			switches[i] = false;
	}

	//Normal reset
	LetterInputMethod::reset(resetCandidates, resetRoman, resetSentence, performFullReset);
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
