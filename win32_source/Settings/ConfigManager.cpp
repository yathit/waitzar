/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */


#include "ConfigManager.h"

using std::vector;
using std::pair;
using std::wstring;
using json_spirit::wValue;
using json_spirit::wObject;
using json_spirit::wPair;
using json_spirit::obj_type;
using json_spirit::str_type;


ConfigManager::ConfigManager(void){
	this->loadedSettings = false;
	this->loadedLanguageMainFiles = false;
	this->loadedLanguageSubFiles = false;
}

ConfigManager::~ConfigManager(void){}


/**
 * Load our maing config file:
 *   config/config.txt
 *
 * This file usually only contains SETTINGS (not languages, etc.) but for the
 *   purose of brevity, we can actually load the entire configuration (inc.
 *   languages, keyboards, etc.) here.
 * This is the only config. file that is actually required.
 */
void ConfigManager::initMainConfig(const std::string& configFile)
{
	//Save the file, we will load it later when we need it
	this->mainConfig = JsonFile(configFile);
}

void ConfigManager::initMainConfig(const std::wstring& configStream)
{
	//Save a copy of the string so that we can reclaim it later
	this->mainConfig = JsonFile(configStream);
}


/**
 * Load all config files for a language:
 *   config/Myanmar/
 *                 config.txt
 *                 ZgInput/config.txt
 *                 StdTransformers/config.txt
 *                 Burglish/config.txt
 *   ...etc.
 *
 * These files may not contain general SETTINGS. The first config file (directly in the Language
 *   folder) must contain basic language information (like the id). All other config files must be in
 *   immediate sub-directories; beyond that, the structure is arbitrary. For example, we load "Burglish"
 *   and "Zawgyi Input" in separate folders, but we load a series of "Standard Transformers" all from
 *   one directory.
 * These files are optional, but heavily encouraged in all distributions except the Web Demo.
 */
void ConfigManager::initAddLanguage(const std::string& configFile, const std::vector<std::string>& subConfigFiles)
{
	//Convert std::strings to JsonFiles
	std::vector<JsonFile> cfgs;
	for (size_t i=0; i<subConfigFiles.size(); i++)
		cfgs.push_back(subConfigFiles[i]);

	//Save the file, we will load it later when we need it
	this->langConfigs[JsonFile(configFile)] = cfgs;
}


/**
 * Load the application-maintained settings override file:
 *   %USERPROFILE%\AppData\Local\WaitZar\config.override.txt
 *   (actually, calls SHGetKnownFolderPath(FOLDERID_LocalAppData) \ WaitZar\config.override.json.txt)
 *
 * This json config file contains one single array of name-value pairs. The names are fully-qualified:
 *   "language.myanmar.defaultdisplayencoding" = "zawgyi-one", for example. These override all 
 *   WaitZar config options where applicable.
 * WaitZar's GUI config window will alter this file. Loading it is optional, but it's generally a good idea
 *  (otherwise, users' settings won't get loaded when they exit and re-load WaitZar).
 */
void ConfigManager::initLocalConfig(const std::string& configFile)
{
	//Save the file, we will load it later when we need it
	this->localConfig = configFile;
}


/**
 * Load the user-maintained settings override file:
 *   %USERPROFILE%\Documents\waitzar.config.txt
 *   (actually, calls SHGetKnownFolderPath(FOLDERID_Documents) \ waitzar.config.json.txt)
 *
 * This json config file contains one single array of name-value pairs. The names are fully-qualified:
 *   "settings.showballoon" = "false", for example. These override all of WaitZar's config options, 
 *   AND they override the settings in LocalConfig (see initAddLocalConfig). They can contain any
 *   options, but are really only intended only to contain settings overrides (not language overrides, etc.)
 * This is the file that users will tweak on their own. It is HIGHLY recommended to load this file, if it exists.
 */
void ConfigManager::initUserConfig(const std::string& configFile)
{
	//Save the file, we will load it later when we need it
	this->userConfig = configFile;
}


void ConfigManager::resolvePartialSettings()
{
	//TODO: Make this cleaner
	unsigned int PART_INPUT = 0;
	unsigned int PART_ENC = 1;
	unsigned int PART_TRANS = 2;
	unsigned int PART_DISP = 3;

	//For each option
	for (unsigned int i=PART_INPUT; i<=PART_DISP; i++) {
		std::map<std::wstring, std::map<std::wstring, std::wstring> >& currMap = i==PART_INPUT ? partialInputMethods : i==PART_ENC ? partialEncodings : i==PART_TRANS ? partialTransformations : partialDisplayMethods;
		for (std::map<std::wstring, std::map<std::wstring, std::wstring> >::iterator it=currMap.begin(); it!=currMap.end(); it++) {
			//Get the language and identifier
			wstring langName;
			wstring id;
			std::wstringstream item;
			for (size_t pos=0; pos<it->first.size(); pos++) {
				if (it->first[pos]==L'.') {
					langName = item.str();
					item.str(L"");
				} else 
					item <<it->first[pos];
			}
			id = item.str();

			//Call the factory method, add it to the current language
			std::set<Language>::iterator lang = FindKeyInSet<Language>(options.languages, langName);
			if (lang==options.languages.end())
				throw std::exception("Language expected but not found..");

			//TODO: Streamline 
			if (i==PART_INPUT)
				lang->inputMethods.insert(WZFactory::makeInputMethod(id, it->second));
			else if (i==PART_ENC) 
				lang->encodings.insert(WZFactory::makeEncoding(id, it->second));
			else if (i==PART_TRANS) 
				lang->transformations.insert(WZFactory::makeTransformation(id, it->second));
			else if (i==PART_DISP) 
				lang->displayMethods.insert(WZFactory::makeDisplayMethod(id, it->second));
		}

		//Clear all entries from this map
		currMap.clear();
	}
}


//Access all things that will require json reads
void ConfigManager::testAllFiles() {
	getSettings();
	getLanguages();
	//getEncodings();
	//getInputManagers();

	//TODO: Add more tests here. We don't want the settings to explode when the user tries to access new options. 
}


const Settings& ConfigManager::getSettings() 
{
	//Load if needed
	if (!loadedSettings) {
		//We need at least one config file to parse.
		if (this->mainConfig.isEmpty())
			throw std::exception("No main config file defined.");

		//Parse each config file in turn.
		//First: main config
		vector<wstring> ctxt;
		this->readInConfig(this->mainConfig.json(), ctxt, false);

		//Next: local and user configs
		if (this->localConfig.isSet())
			this->readInConfig(this->localConfig.json(), ctxt, true);
		if (this->userConfig.isSet())
			this->readInConfig(this->userConfig.json(), ctxt, true);

		//Done
		loadedSettings = true;
	}

	//Return the object
	return this->options.settings;
}



void ConfigManager::loadLanguageMainFiles()
{
	//Main config file must be read by now
	if (!this->loadedSettings)
		throw std::exception("Must load settings before language main files.");

	//Parse each language config file.
	vector<wstring> ctxt;
	for (std::map<JsonFile , std::vector<JsonFile> >::const_iterator it = langConfigs.begin(); it!=langConfigs.end(); it++) {
		this->readInConfig(it->first.json(), ctxt, false);
	}

	//Done
	loadedLanguageMainFiles = true;
}


void ConfigManager::loadLanguageSubFiles()
{
	//Main config file must be read by now
	if (!this->loadedSettings)
		throw std::exception("Must load settings before language sub files.");

	//Parse each language config file.
	vector<wstring> ctxt;
	for (std::map<JsonFile , std::vector<JsonFile> >::const_iterator it = langConfigs.begin(); it!=langConfigs.end(); it++) {
		for (std::vector<JsonFile>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			this->readInConfig(it2->json(), ctxt, false);
		}
	}

	//Done
	loadedLanguageMainFiles = true;
}


const std::set<Language>& ConfigManager::getLanguages()  
{
	//Languages can ONLY be defined in top-level language directories.
	//  So we don't need to load user-defined plugins yet. 
	//TODO: Why 2 flags?
	if (!this->loadedLanguageMainFiles)
		this->loadLanguageMainFiles();
	if (!this->loadedLanguageSubFiles)
		this->loadLanguageSubFiles();

	//Now, build our cache (if necessary)
	//  Our algorithm is wasteful if there are no languages defined,
	//  but this is minor (if there are no languages defined, the program
	//  won't really function anyway).
	/*if (this->cachedLanguages.empty()) {
		for (std::map<wstring, Language>::iterator it=options.languages.begin(); it!=options.languages.end(); it++) {
			this->cachedLanguages.push_back(it->first);
		}
	}*/

	//return this->cachedLanguages;
	return this->options.languages;
}


//Note: Context is managed automatically; never copied.
//Restricted means don't load new languages, etc.
void ConfigManager::readInConfig(wValue root, vector<wstring> &context, bool restricted) 
{
	//We always operate on maps:
	json_spirit::Value_type t = root.type();
	wObject currPairs = root.get_value<wObject>();
	for (wObject::iterator itr=currPairs.begin(); itr!=currPairs.end(); itr++) {
 		//Append to the context
		int numToRemove = 0;
		{
		vector<wstring> opts = separate(sanitize_id(itr->name_), L'.');
		context.insert(context.end(), opts.begin(), opts.end());
		numToRemove = opts.size();
		}

		//React to this option/category
		if (itr->value_.type()==obj_type) {
			//Inductive case: Continue reading all options under this type
			this->readInConfig(itr->value_, context, restricted);
		} else if (itr->value_.type()==str_type) {
			//Base case: the "value" is also a string (set the property)
			this->setSingleOption(context, sanitize(itr->value_.get_value<std::wstring>()), restricted);
		} else {
			throw std::exception("ERROR: Config file options should always be string or hash types.");
		}

		//Remove, get ready for the next option
		while (numToRemove>0) {
			context.pop_back();
			numToRemove--;
		}
	}
}


void ConfigManager::setSingleOption(const vector<wstring>& name, const std::wstring& value, bool restricted)
{
	//Read each "context" setting from left to right. Context settings are separated by periods. 
	//   Note: There are much faster/better ways of doing this, but for now we'll keep all the code
	//   centralized and easy to read.
	try {
		//Need at least one setting
		if (name.empty())
			return;

		//Settings? Languages?
		if (name[0] == L"settings") {
			//Need to finish all partial settings
			if (name.size()<=1)
				throw 1;

			//Set this based on name/value pair
			if (name[1] == L"hotkey")
				options.settings.hotkey = sanitize_id(value);
			else if (name[1] == sanitize_id(L"silence-mywords-errors"))
				options.settings.silenceMywordsErrors = read_bool(value);
			else if (name[1] == sanitize_id(L"balloon-start"))
				options.settings.balloonStart = read_bool(value);
			else if (name[1] == sanitize_id(L"always-elevate"))
				options.settings.alwaysElevate = read_bool(value);
			else if (name[1] == sanitize_id(L"track-caret"))
				options.settings.trackCaret = read_bool(value);
			else if (name[1] == sanitize_id(L"lock-windows"))
				options.settings.lockWindows = read_bool(value);
			else
				throw 1;

			//Done
			return;
		} else if (name[0] == L"languages") {
			//Need to finish all partial settings
			if (name.size()<=2)
				throw 1;

			//Get the language id
			wstring langName = name[1];
			std::set<Language>::iterator lang = FindKeyInSet<Language>(options.languages, langName);
			if (lang==options.languages.end()) {
				if(restricted)
					throw std::exception("Cannot create a new Language in user or system-local config files.");
				else
					lang = options.languages.insert(Language(langName)).first;
			}

			//Static settings
			if (name[2] == sanitize_id(L"display-name"))
				lang->displayName = value;
			else if (name[2] == sanitize_id(L"default-output-encoding"))
				lang->defaultOutputEncoding = sanitize_id(value);
			else if (name[2] == sanitize_id(L"default-display-method"))
				lang->defaultDisplayMethod = sanitize_id(value);
			else if (name[2] == sanitize_id(L"default-input-method"))
				lang->defaultInputMethod = sanitize_id(value);
			else {
				//Need to finish all partial settings
				if (name.size()<=4)
					throw 1;

				//Dynamic settings
				//Todo: Make this slightly less wordy.
				if (name[2] == sanitize_id(L"input-methods")) {
					//Input methods
					wstring inputName = name[3];

					//Allowed to add new Input Methods?
					if (FindKeyInSet(lang->inputMethods, inputName)==lang->inputMethods.end() && restricted)
						throw std::exception("Cannot create a new Input Method in user or system-local config files.");

					//Just save all its options. Then, call a Factory method when this is all done
					partialInputMethods[langName + L"." + inputName][sanitize_id(name[4])] = value;
				} else if (name[2] == sanitize_id(L"encodings")) {
					//Encodings
					wstring encName = name[3];

					//Allowed to add new Encodings?
					if (FindKeyInSet(lang->encodings, encName)==lang->encodings.end() && restricted)
						throw std::exception("Cannot create a new Encoding in user or system-local config files.");

					//Just save all its options. Then, call a Factory method when this is all done
					partialEncodings[langName + L"." + encName][sanitize_id(name[4])] = value;
				} else if (name[2] == sanitize_id(L"tranformations")) {
					//Transformations
					wstring transName = name[3];

					//Allowed to add new Transformations?
					if (FindKeyInSet(lang->transformations, transName)==lang->transformations.end() && restricted)
						throw std::exception("Cannot create a new Tranformation in user or system-local config files.");

					//Just save all its options. Then, call a Factory method when this is all done
					partialTransformations[langName + L"." + transName][sanitize_id(name[4])] = value;
				} else if (name[2] == sanitize_id(L"display-methods")) {
					//Display methods
					wstring dispMethod = name[3];

					//Allowed to add new Display Method?
					if (FindKeyInSet(lang->displayMethods, dispMethod)==lang->displayMethods.end() && restricted)
						throw std::exception("Cannot create a new Display Method in user or system-local config files.");

					//Just save all its options. Then, call a Factory method when this is all done
					partialDisplayMethods[langName + L"." + dispMethod][sanitize_id(name[4])] = value;
				} else {
					//Error
					throw 1;
				}
			}
		} else
			throw 1;
	} catch (int) {
		//Bad option
		std::wstringstream nameStr;
		wstring dot = L"";
		for (vector<wstring>::const_iterator nameOpt=name.begin(); nameOpt!=name.end(); nameOpt++) {
			nameStr <<dot <<(*nameOpt);
			dot = L"/"; //To distinguish "dot" errors
		}
		throw std::exception(std::string("Invalid option: \"" + waitzar::escape_wstr(nameStr.str()) + "\", with value: \"" + waitzar::escape_wstr(value) + "\"").c_str());
	}
}




//Remove leading and trailing whitespace
wstring ConfigManager::sanitize(const wstring& str) 
{
	size_t firstLetter = str.find_first_not_of(L" \t\n");
	size_t lastLetter = str.find_last_not_of(L" \t\n");
	return str.substr(firstLetter, lastLetter-firstLetter+1);
}

//Sanitize, then return in lowercase, with '-', '_', and whitespace removed
wstring ConfigManager::sanitize_id(const wstring& str) 
{
	wstring res = str; //Copy out the "const"-ness.
	res = wstring(res.begin(), std::remove_if(res.begin(), res.end(), is_id_delim<wchar_t>()));
	loc_to_lower(res); //Operates in-place.
	return res;
}

//Tokenize on a character
//Inelegant, but it does what I want it to.
vector<wstring> ConfigManager::separate(wstring str, wchar_t delim)
{
	vector<wstring> tokens;
	std::wstringstream curr;
	for (size_t i=0; i<str.length(); i++) {
		if (str[i]!=delim)
			curr << str[i];
		if (str[i]==delim || i==str.length()-1) {
			tokens.push_back(curr.str());
			curr.str(L"");
		}
	}

	return tokens;
}

bool ConfigManager::read_bool(const std::wstring& str)
{
	std::wstring test = str;
	loc_to_lower(test);
	if (test == L"yes" || test==L"true")
		return true;
	else if (test==L"no" || test==L"false")
		return false;
	else
		throw std::exception(std::string("Bad boolean value: \"" + waitzar::escape_wstr(str) + "\"").c_str());
}

void ConfigManager::loc_to_lower(std::wstring& str)
{
	//Locale-aware "toLower" converter
	std::locale loc(""); //Get native locale
	std::transform(str.begin(),str.end(),str.begin(),ToLower<wchar_t>(loc));
}


//Not yet defined:
//vector<wstring> ConfigManager::getInputManagers() {vector<wstring> res; return res;}
//vector<wstring> ConfigManager::getEncodings() {vector<wstring> res; return res;}
//wstring ConfigManager::getActiveLanguage() const {return L"";}
//void ConfigManager::changeActiveLanguage(const wstring& newLanguage) {}



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
