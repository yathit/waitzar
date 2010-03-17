/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */


#include "ConfigManager.h"

using std::vector;
using std::pair;
using std::wstring;
using std::string;
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
		std::map<std::pair<std::wstring,std::wstring>, std::map<std::wstring, std::wstring> >& currMap = i==PART_INPUT ? partialInputMethods : i==PART_ENC ? partialEncodings : i==PART_TRANS ? partialTransformations : partialDisplayMethods;
		for (std::map<std::pair<std::wstring,std::wstring>, std::map<std::wstring, std::wstring> >::iterator it=currMap.begin(); it!=currMap.end(); it++) {
			//Get the language and identifier
			wstring langName = it->first.first;
			wstring id = it->first.second;

			//Call the factory method, add it to the current language
			std::set<Language>::iterator lang = FindKeyInSet<Language>(options.languages, langName);
			if (lang==options.languages.end())
				throw std::exception(glue(L"Language \"", langName , L"\" expected but not found...").c_str());

			//TODO: Streamline 
			if (i==PART_INPUT)
				lang->inputMethods.insert(WZFactory<waitzar::WordBuilder>::makeInputMethod(id, *lang, it->second));
			else if (i==PART_ENC) 
				lang->encodings.insert(WZFactory<waitzar::WordBuilder>::makeEncoding(id, it->second));
			else if (i==PART_TRANS) 
				lang->transformations.insert(WZFactory<waitzar::WordBuilder>::makeTransformation(id, it->second));
			else if (i==PART_DISP) 
				lang->displayMethods.insert(WZFactory<waitzar::WordBuilder>::makeDisplayMethod(id, *lang, it->second));
		}

		//Clear all entries from this map
		currMap.clear();
	}
}


//Make our model worrrrrrrrk......
// (Note: We also need to replace all of our placeholder encodings with the real thing.
//        We can't use references, since those might be in validated if we somehow resized the container).
void ConfigManager::validate(HINSTANCE& hInst, MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, OnscreenKeyboard* helpKeyboard) 
{
	//Step 1: Read
	getSettings();
	getLanguages();
	//getEncodings();
	//getInputManagers();

	//TODO: Add more tests here. We don't want the settings to explode when the user tries to access new options. 
	WZFactory<waitzar::WordBuilder>::InitAll(hInst, mainWindow, sentenceWindow, helpWindow, memoryWindow, helpKeyboard);
	waitzar::BurglishBuilder::InitStatic();

	//Step 2: Un-cache
	resolvePartialSettings();

	//Step 3: Make it useful
	generateInputsDisplaysOutputs();

	//Step 4: Set our current language, input method, etc.
	activeLanguage = *FindKeyInSet(options.languages, options.settings.defaultLanguage);
	activeOutputEncoding = activeLanguage.defaultOutputEncoding;
	activeInputMethod = *FindKeyInSet(activeLanguage.inputMethods, activeLanguage.defaultInputMethod);
	activeDisplayMethods.clear();
	activeDisplayMethods.push_back(*FindKeyInSet(activeLanguage.displayMethods, activeLanguage.defaultDisplayMethodReg));
	activeDisplayMethods.push_back(*FindKeyInSet(activeLanguage.displayMethods, activeLanguage.defaultDisplayMethodSmall));
	if (activeDisplayMethods[0]->encoding != activeDisplayMethods[1]->encoding)
		throw std::exception("Error: \"small\" and \"regular\" sized display methods have different encodings");
}



//Validate all Input Managers, Display Managers, Outputs, and Transformations; make
//     sure the right encodings (and transformations) exist for each.
//Then, build fast-to-lookup data structures for actual use in WZ.
void ConfigManager::generateInputsDisplaysOutputs() 
{
	//Cache our self2self lookup
	self2self = new Self2Self();

	//Validate our settings
	//TODO: Check the hotkey, later
	if (FindKeyInSet(options.languages, options.settings.defaultLanguage)==options.languages.end())
		throw std::exception(glue(L"Settings references non-existant default language: ", options.settings.defaultLanguage).c_str());

	//Validate over each language
	for (std::set<Language>::iterator lg=options.languages.begin(); lg!=options.languages.end(); lg++) {
		//Substitute the encoding
		std::set<Encoding>::iterator defEnc = lg->encodings.find(lg->defaultOutputEncoding);
		if (defEnc!=lg->encodings.end())
			lg->defaultOutputEncoding = *defEnc;
		else
			throw std::exception(glue(L"Language \"" , lg->id , L"\" references non-existant default output encoding: ", lg->defaultOutputEncoding.id).c_str());

		//Next, validate some default settings of the language
		if (!defEnc->canUseAsOutput)
			throw std::exception(glue(L"Language \"" , lg->id , L"\" uses a default output encoding which does not support output.").c_str());
		if (FindKeyInSet(lg->displayMethods, lg->defaultDisplayMethodReg)==lg->displayMethods.end())
			throw std::exception(glue(L"Language \"" , lg->id , L"\" references non-existant default display method: ", lg->defaultDisplayMethodReg).c_str());
		if (FindKeyInSet(lg->displayMethods, lg->defaultDisplayMethodSmall)==lg->displayMethods.end())
			throw std::exception(glue(L"Language \"" , lg->id , L"\" references non-existant \"small\" default display method: ", lg->defaultDisplayMethodSmall).c_str());
		if (FindKeyInSet(lg->inputMethods, lg->defaultInputMethod)==lg->inputMethods.end())
			throw std::exception(glue(L"Language \"" , lg->id , L"\" references non-existant default input method: ", lg->defaultInputMethod).c_str());

		//TODO: Right now, "unicode" is hard-coded into a lot of places. Is there a better way?
		std::set<Encoding>::iterator uniEnc = FindKeyInSet(lg->encodings, L"unicode");
		if (uniEnc==lg->encodings.end())
			throw std::exception(glue(L"Language \"" , lg->id , L"\" does not include \"unicode\" as an encoding.").c_str());
		unicodeEncoding = *uniEnc;

		//Validate transformations & cache a lookup table.
		for (std::set<Transformation*>::iterator it=lg->transformations.begin(); it!=lg->transformations.end(); it++) {
			//Make sure this transformation references existing encodings. Replace them as we find them
			{
			std::set<Encoding>::iterator frEnc = lg->encodings.find((*it)->fromEncoding);
			if (frEnc!=lg->encodings.end())
				(*it)->fromEncoding = *frEnc;
			else
				throw std::exception(glue(L"Transformation \"" , (*it)->id , L"\" references non-existant from-encoding: ", (*it)->fromEncoding.id).c_str());
			}
			{
			std::set<Encoding>::iterator toEnc = lg->encodings.find((*it)->toEncoding);
			if (toEnc!=lg->encodings.end())
				(*it)->toEncoding = *toEnc;
			else
				throw std::exception(glue(L"Transformation \"" , (*it)->id , L"\" references non-existant to-encoding: ", (*it)->toEncoding.id).c_str());
			}

			//Add to our lookup table, conditional on a few key points
			//TODO: Re-write... slightly messy.
			std::pair<Encoding, Encoding> newPair;
			newPair.first = *(lg->encodings.find((*it)->fromEncoding));
			newPair.second = *(lg->encodings.find((*it)->toEncoding));
			std::map< std::pair<Encoding, Encoding>, Transformation* >::iterator foundPair = lg->transformationLookup.find(newPair);
			if (foundPair==lg->transformationLookup.end())
				lg->transformationLookup[newPair] = *it;
			else if (foundPair->second->hasPriority)
				throw std::exception(glue(L"Cannot add new Transformation (", (*it)->id, L") over one with priority: ", foundPair->second->id).c_str());
			else if (!(*it)->hasPriority)
				throw std::exception(glue(L"Cannot add new Transformation (", (*it)->id, L"); it does not set \"hasPriority\"").c_str());
			else
				lg->transformationLookup[newPair] = *it;
		}

		//Validate each input method
		for (std::set<InputMethod*>::iterator it=lg->inputMethods.begin(); it!=lg->inputMethods.end(); it++) {
			//Make sure this input method references an existing encoding. Replace it as we find it
			{
			std::set<Encoding>::iterator inEnc = lg->encodings.find((*it)->encoding);
			if (inEnc!=lg->encodings.end())
				(*it)->encoding = *inEnc;
			else
				throw std::exception(glue(L"Input Method (", (*it)->id, L") references non-existant encoding: ", (*it)->encoding.id).c_str());
			}

			//Make sure that our encoding is EITHER the default, OR there is an appropriate transform.
			if ((*it)->encoding!=L"unicode") {
				std::pair<Encoding, Encoding> lookup;
				lookup.first = *(lg->encodings.find((*it)->encoding));
				lookup.second = *FindKeyInSet(lg->encodings, L"unicode");
				if (lg->transformationLookup.find(lookup)==lg->transformationLookup.end())
					throw std::exception(glue(L"No \"transformation\" exists for input method(", (*it)->id, L").").c_str());
			}
		}

		//Validate each display method
		for (std::set<DisplayMethod*>::iterator it=lg->displayMethods.begin(); it!=lg->displayMethods.end(); it++) {
			//Make sure this display method references an existing encoding. Replace it as we find it.
			{
			std::set<Encoding>::iterator outEnc = lg->encodings.find((*it)->encoding);
			if (outEnc!=lg->encodings.end())
				(*it)->encoding = *outEnc;
			else
				throw std::exception(glue(L"Display Method (", (*it)->id, L") references non-existant encoding: ", (*it)->encoding.id).c_str());
			}

			//Make sure that our encoding is EITHER the default, OR there is an appropriate transform.
			if ((*it)->encoding.id != L"unicode") {
				std::pair<Encoding, Encoding> lookup;
				lookup.first = *(lg->encodings.find((*it)->encoding));
				lookup.second = *FindKeyInSet(lg->encodings, L"unicode");
				if (lg->transformationLookup.find(lookup)==lg->transformationLookup.end())
					throw std::exception(glue(L"No \"transformation\" exists for display method(", (*it)->id, L").").c_str());
			}
		}
	}
}



const Transformation* ConfigManager::getTransformation(const Language& lang, const Encoding& fromEnc, const Encoding& toEnc) const
{
	//Self to self?
	if (fromEnc==toEnc)
		return self2self;

	//Lookup
	std::pair<Encoding, Encoding> lookup(fromEnc, toEnc);
	std::map< std::pair<Encoding, Encoding>, Transformation* >::const_iterator found = lang.transformationLookup.find(lookup);
	if (found==lang.transformationLookup.end())
		throw std::exception(glue(L"Error! An unvalidated transformation exists in the configuration model: ", fromEnc.id, L"->", toEnc.id).c_str());

	//Done
	return found->second;
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
		this->readInConfig(this->mainConfig.json(), this->mainConfig.getFolderPath(), ctxt, false);

		//Next: local and user configs
		if (this->localConfig.isSet())
			this->readInConfig(this->localConfig.json(), this->localConfig.getFolderPath(), ctxt, true);
		if (this->userConfig.isSet())
			this->readInConfig(this->userConfig.json(), this->userConfig.getFolderPath(), ctxt, true);

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
		this->readInConfig(it->first.json(), it->first.getFolderPath(), ctxt, false);
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
			this->readInConfig(it2->json(), it2->getFolderPath(), ctxt, false);
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

	return this->options.languages;
}

const std::set<InputMethod*>& ConfigManager::getInputMethods()
{
	//Languages can ONLY be defined in top-level language directories.
	//  So we don't need to load user-defined plugins yet. 
	//TODO: Why 2 flags?
	if (!this->loadedLanguageMainFiles)
		this->loadLanguageMainFiles();
	if (!this->loadedLanguageSubFiles)
		this->loadLanguageSubFiles();

	return this->activeLanguage.inputMethods;
}

const std::set<DisplayMethod*>& ConfigManager::getDisplayMethods()
{
	//Languages can ONLY be defined in top-level language directories.
	//  So we don't need to load user-defined plugins yet. 
	//TODO: Why 2 flags?
	if (!this->loadedLanguageMainFiles)
		this->loadLanguageMainFiles();
	if (!this->loadedLanguageSubFiles)
		this->loadLanguageSubFiles();

	return this->activeLanguage.displayMethods;
}

const std::set<Encoding>& ConfigManager::getEncodings()
{
	//Languages can ONLY be defined in top-level language directories.
	//  So we don't need to load user-defined plugins yet. 
	//TODO: Why 2 flags?
	if (!this->loadedLanguageMainFiles)
		this->loadLanguageMainFiles();
	if (!this->loadedLanguageSubFiles)
		this->loadLanguageSubFiles();

	return this->activeLanguage.encodings;
}




//Note: Context is managed automatically; never copied.
//Restricted means don't load new languages, etc.
void ConfigManager::readInConfig(wValue root, const wstring& folderPath, vector<wstring> &context, bool restricted) 
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
			this->readInConfig(itr->value_, folderPath, context, restricted);
		} else if (itr->value_.type()==str_type) {
			//Base case: the "value" is also a string (set the property)
			this->setSingleOption(folderPath, context, sanitize(itr->value_.get_value<std::wstring>()), restricted);
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


void ConfigManager::setSingleOption(const wstring& folderPath, const vector<wstring>& name, const std::wstring& value, bool restricted)
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
			else if (name[1] == sanitize_id(L"default-language"))
				options.settings.defaultLanguage = sanitize_id(value);
			else
				throw 1;

			//Done
			return;
		} else if (name[0] == L"languages") {
			//Need to finish all partial settings
			if (name.size()<=2)
				throw 1;

			//Get the language id
			//TODO: Add better error messages using the glue() functions.
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
				lang->defaultOutputEncoding.id = sanitize_id(value);
			else if (name[2] == sanitize_id(L"default-display-method"))
				lang->defaultDisplayMethodReg = sanitize_id(value);
			else if (name[2] == sanitize_id(L"default-display-method-small"))
				lang->defaultDisplayMethodSmall = sanitize_id(value);
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
					pair<wstring,wstring> key = pair<wstring,wstring>(langName,inputName);
					partialInputMethods[key][sanitize_id(name[4])] = value;
					if (partialInputMethods[key].size()==1) //First option
						partialInputMethods[key][sanitize_id(L"current-folder")] = folderPath;
				} else if (name[2] == sanitize_id(L"encodings")) {
					//Encodings
					wstring encName = name[3];

					//Allowed to add new Encodings?
					if (FindKeyInSet(lang->encodings, encName)==lang->encodings.end() && restricted)
						throw std::exception("Cannot create a new Encoding in user or system-local config files.");

					//Just save all its options. Then, call a Factory method when this is all done
					pair<wstring, wstring> key = pair<wstring, wstring>(langName,encName);
					partialEncodings[key][sanitize_id(name[4])] = value;
					if (partialEncodings[key].size()==1) //First option
						partialEncodings[key][sanitize_id(L"current-folder")] = folderPath;
				} else if (name[2] == sanitize_id(L"tranformations")) {
					//Transformations
					wstring transName = name[3];

					//Allowed to add new Transformations?
					if (FindKeyInSet(lang->transformations, transName)==lang->transformations.end() && restricted)
						throw std::exception("Cannot create a new Tranformation in user or system-local config files.");

					//Just save all its options. Then, call a Factory method when this is all done
					pair<wstring, wstring> key = pair<wstring, wstring>(langName,transName);
					partialTransformations[key][sanitize_id(name[4])] = value;
					if (partialTransformations[key].size()==1) //First option
						partialTransformations[key][sanitize_id(L"current-folder")] = folderPath;
				} else if (name[2] == sanitize_id(L"display-methods")) {
					//Display methods
					wstring dispMethod = name[3];

					//Allowed to add new Display Method?
					if (FindKeyInSet(lang->displayMethods, dispMethod)==lang->displayMethods.end() && restricted)
						throw std::exception("Cannot create a new Display Method in user or system-local config files.");

					//Just save all its options. Then, call a Factory method when this is all done
					pair<wstring, wstring> key = pair<wstring, wstring>(langName,dispMethod);
					partialDisplayMethods[key][sanitize_id(name[4])] = value;
					if (partialDisplayMethods[key].size()==1) //First option
						partialDisplayMethods[key][sanitize_id(L"current-folder")] = folderPath;
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
		throw std::exception(glue(L"Bad boolean value: \"", str, L"\"").c_str());
}


int ConfigManager::read_int(const std::wstring& str)
{
	//Read
	int resInt;
	std::wistringstream reader(str);
	reader >>resInt;

	//Problem?
	if (reader.fail())
		throw std::exception(glue(L"Bad integer value: \"", str, L"\"").c_str());

	//Done
	return resInt;
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
