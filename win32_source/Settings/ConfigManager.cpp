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

/*using json_spirit::wValue;
using json_spirit::wObject;
using json_spirit::wPair;
using json_spirit::obj_type;
using json_spirit::str_type;*/
using Json::Value;


ConfigManager::ConfigManager(std::string (*myMD5Function)(const std::string&)){
	this->loadedSettings = false;
	this->loadedLanguageMainFiles = false;
	this->loadedLanguageSubFiles = false;

	this->getMD5Function = myMD5Function;

	//Save the current working directory
	/*char* buffer;
	std::wstringstream txt;
	if((buffer = _getcwd( NULL, 0)) != NULL) {
		txt <<buffer;
		free(buffer);
	}
	this->workingDir = txt.str();*/
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
void ConfigManager::initMainConfig(const std::string& configFile, bool fileIsStream)
{
	//Save the file, we will load it later when we need it
	this->mainConfig = JsonFile(configFile, fileIsStream);
}



void ConfigManager::initCommonConfig(const std::string& configFile)
{
	//Save the file, we will load it later when we need it
	this->commonConfig = JsonFile(configFile, false);
}


/*void ConfigManager::initMainConfig(const std::wstring& configStream)
{
	//Save a copy of the string so that we can reclaim it later
	this->mainConfig = JsonFile(configStream);
}*/


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
		auto& currMap = i==PART_INPUT ? partialInputMethods : i==PART_ENC ? partialEncodings : i==PART_TRANS ? partialTransformations : partialDisplayMethods;
		for (auto it=currMap.begin(); it!=currMap.end(); it++) {
			//Get the language and identifier
			wstring langName = it->first.first;
			wstring id = it->first.second;

			//Call the factory method, add it to the current language
			//std::set<Language>::const_iterator lang = FindKeyInSet<Language>(options.languages, langName);
			auto lang = options.languages.find(langName);
			if (lang==options.languages.end())
				throw std::runtime_error(glue(L"Language \"", langName , L"\" expected but not found...").c_str());

			//TODO: Streamline 
			if (i==PART_INPUT)
				const_cast<Language&>(*lang).inputMethods.insert(WZFactory<waitzar::WordBuilder>::makeInputMethod(id, *lang, it->second, getMD5Function));
			else if (i==PART_ENC) 
				const_cast<Language&>(*lang).encodings.insert(WZFactory<waitzar::WordBuilder>::makeEncoding(id, it->second));
			else if (i==PART_TRANS) {
				auto jsIt = FindKeyInSet(options.extensions, L"javascript");
				JavaScriptConverter* js = (jsIt==options.extensions.end() ? NULL : (JavaScriptConverter*)const_cast<Extension*>(*jsIt));
				const_cast<Language&>(*lang).transformations.insert(WZFactory<waitzar::WordBuilder>::makeTransformation(id, it->second, js));
			} else if (i==PART_DISP)
				const_cast<Language&>(*lang).displayMethods.insert(WZFactory<waitzar::WordBuilder>::makeDisplayMethod(id, *lang, it->second));
		}

		//Clear all entries from this map
		currMap.clear();
	}
}


//Make our model worrrrrrrrk......
// (Note: We also need to replace all of our placeholder encodings with the real thing.
//        We can't use references, since those might be in validated if we somehow resized the container).
void ConfigManager::validate(HINSTANCE& hInst, MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, OnscreenKeyboard* helpKeyboard, const map<wstring, vector<wstring> >& lastUsedSettings) 
{
	//Step 1: Read
	localConfError = false;
	getSettings();
	getExtensions();
	getLanguages();
	getEncodings();
	getInputMethods();
	getDisplayMethods();
	Logger::markLogTime('L', L"Read physical files, parsed JSON");

	//TODO: Add more tests here. We don't want the settings to explode when the user tries to access new options. 
	WZFactory<waitzar::WordBuilder>::InitAll(hInst, mainWindow, sentenceWindow, helpWindow, memoryWindow, helpKeyboard);
	waitzar::BurglishBuilder::InitStatic();
	Logger::markLogTime('L', L"Initialized static classes with relevant information.");

	//Step 1.5; check all DLLs
	for (auto it=options.extensions.begin(); it!=options.extensions.end(); it++) {
		if (const_cast<Extension*>(*it)->id==L"javascript")
			((JavaScriptConverter*)*it)->InitDLL(getMD5Function);
	}


	//Step 2: Un-cache
	resolvePartialSettings();
	Logger::markLogTime('L', L"Resolved partial settings.");

	//Step 3: Make it useful
	generateInputsDisplaysOutputs(lastUsedSettings);
	Logger::markLogTime('L', L"Generated list of input/output/display/encodings.");

	//Step 4: Set our current language, input method, etc.
	//activeLanguage = *FindKeyInSet(options.languages, options.settings.defaultLanguage);
	activeLanguage = *options.languages.find(options.settings.defaultLanguage);
	activeOutputEncoding = activeLanguage.defaultOutputEncoding;
	activeInputMethod = *FindKeyInSet(activeLanguage.inputMethods, activeLanguage.defaultInputMethod);
	activeDisplayMethods.clear();
	activeDisplayMethods.push_back(*FindKeyInSet(activeLanguage.displayMethods, activeLanguage.defaultDisplayMethodReg));
	activeDisplayMethods.push_back(*FindKeyInSet(activeLanguage.displayMethods, activeLanguage.defaultDisplayMethodSmall));
	if (activeDisplayMethods[0]->encoding != activeDisplayMethods[1]->encoding)
		throw std::runtime_error("Error: \"small\" and \"regular\" sized display methods have different encodings");
	Logger::markLogTime('L', L"Set \"active\" input/output/display/encodings.");
}



//Validate all Input Managers, Display Managers, Outputs, and Transformations; make
//     sure the right encodings (and transformations) exist for each.
//Then, build fast-to-lookup data structures for actual use in WZ.
//   lastUsedSettings = languageid -> [lastInput, lastOutput, lastDispBig, lastDispSmall]
void ConfigManager::generateInputsDisplaysOutputs(const map<wstring, vector<wstring> >& lastUsedSettings) 
{
	//Cache our self2self lookup
	self2self = new Self2Self();

	//What was the last-used language?
	map<wstring, vector<wstring> >::const_iterator lastDefaultLang = lastUsedSettings.find(L"language.default");
	wstring lastUsedLang = lastDefaultLang==lastUsedSettings.end() ? L"" : lastDefaultLang->second[0];

	//Check and replace the "lastused" language, if it exists. 
	size_t luID = options.settings.defaultLanguage.find(L"lastused");
	if (luID!=wstring::npos) {
		//Retrieve the default value.
		wstring defaultVal = options.settings.defaultLanguage.substr(0, luID-1);

		//Apply it if we have a last-used lang
		//Also check if the language exists
		//if (lastUsedLang.empty() || FindKeyInSet(options.languages, lastUsedLang)==options.languages.end())
		if (lastUsedLang.empty() || options.languages.find(lastUsedLang)==options.languages.end())
			options.settings.defaultLanguage = defaultVal;
		else
			options.settings.defaultLanguage = lastUsedLang;
	}

	//Validate our settings
	//TODO: Check the hotkey, later
	//if (FindKeyInSet(options.languages, options.settings.defaultLanguage)==options.languages.end()) {
	if (options.languages.find(options.settings.defaultLanguage)==options.languages.end()) {
		if (getLocalConfigOpt(L"settings.defaultlanguage")==options.settings.defaultLanguage)
			localConfError = true;
		throw std::runtime_error(glue(L"Settings references non-existant default language: ", options.settings.defaultLanguage).c_str());
	}

	//Validate over each language
	for (std::set<Language>::const_iterator lg=options.languages.begin(); lg!=options.languages.end(); lg++) {
		//Get the current "backup" input/encoding for this language
		map<wstring, vector<wstring> >::const_iterator thisLangLast =  lastUsedSettings.find(lg->id);
		wstring lastUsedInput = thisLangLast==lastUsedSettings.end() ? L"" : thisLangLast->second[0];
		wstring lastUsedOutput = thisLangLast==lastUsedSettings.end() ? L"" : thisLangLast->second[1];

		//First, if either of the default input/output methods/encodings is "lastused", then see if their respective 
		// lastUsed items exists. If so, set that. If not, keep the setting.
		luID = lg->defaultInputMethod.find(L"lastused");
		if (luID != wstring::npos) {
			//Retrieve the default value.
			wstring defaultVal = lg->defaultInputMethod.substr(0, luID-1);

			//Check if the input method exists
			if (lastUsedInput.empty() || FindKeyInSet(lg->inputMethods, lastUsedInput)==lg->inputMethods.end())
				const_cast<Language&>(*lg).defaultInputMethod = defaultVal;
			else
				const_cast<Language&>(*lg).defaultInputMethod = lastUsedInput;
		}
		luID = lg->defaultOutputEncoding.id.find(L"lastused");
		if (luID != wstring::npos) {
			//Retrieve the default value.
			wstring defaultVal = lg->defaultOutputEncoding.id.substr(0, luID-1);

			//Check if the input method exists
			const_cast<Language&>(*lg).defaultOutputEncoding.id = lastUsedOutput;
			if (lastUsedOutput.empty() || lg->encodings.find(lg->defaultOutputEncoding)==lg->encodings.end())
				const_cast<Language&>(*lg).defaultOutputEncoding.id = defaultVal;
			else
				const_cast<Language&>(*lg).defaultOutputEncoding.id = lastUsedOutput;
		}

		//Substitute the encoding
		std::set<Encoding>::iterator defEnc = const_cast<Language&>(*lg).encodings.find(lg->defaultOutputEncoding);
		if (defEnc!=lg->encodings.end())
			const_cast<Language&>(*lg).defaultOutputEncoding = *defEnc; //We have to re-set it, since the original "defaultOutputEncoding" is just a placeholder.
		else {
			if (getLocalConfigOpt(L"languages." + lg->id + L".defaultoutputencoding")==lg->defaultOutputEncoding.id)
				localConfError = true;
			throw std::runtime_error(glue(L"Language \"" , lg->id , L"\" references non-existant default output encoding: ", lg->defaultOutputEncoding.id).c_str());
		}

		//Next, validate some default settings of the language
		if (!defEnc->canUseAsOutput)
			throw std::runtime_error(glue(L"Language \"" , lg->id , L"\" uses a default output encoding which does not support output.").c_str());
		if (FindKeyInSet(lg->displayMethods, lg->defaultDisplayMethodReg)==lg->displayMethods.end())
			throw std::runtime_error(glue(L"Language \"" , lg->id , L"\" references non-existant default display method: ", lg->defaultDisplayMethodReg).c_str());
		if (FindKeyInSet(lg->displayMethods, lg->defaultDisplayMethodSmall)==lg->displayMethods.end())
			throw std::runtime_error(glue(L"Language \"" , lg->id , L"\" references non-existant \"small\" default display method: ", lg->defaultDisplayMethodSmall).c_str());
		if (FindKeyInSet(lg->inputMethods, lg->defaultInputMethod)==lg->inputMethods.end()) {
			if (getLocalConfigOpt(L"languages." + lg->id + L".defaultinputmethod")==lg->defaultInputMethod)
				localConfError = true;
			throw std::runtime_error(glue(L"Language \"" , lg->id , L"\" references non-existant default input method: ", lg->defaultInputMethod).c_str());
		}

		//TODO: Right now, "unicode" is hard-coded into a lot of places. Is there a better way?
		//std::set<Encoding>::const_iterator uniEnc = FindKeyInSet(lg->encodings, L"unicode");
		auto uniEnc = lg->encodings.find(L"unicode");
		if (uniEnc==lg->encodings.end())
			throw std::runtime_error(glue(L"Language \"" , lg->id , L"\" does not include \"unicode\" as an encoding.").c_str());
		unicodeEncoding = *uniEnc;

		//Validate transformations & cache a lookup table.
		for (std::set<Transformation*>::const_iterator it=lg->transformations.begin(); it!=lg->transformations.end(); it++) {
			//Make sure this transformation references existing encodings. Replace them as we find them
			{
			std::set<Encoding>::const_iterator frEnc = lg->encodings.find((*it)->fromEncoding);
			if (frEnc!=lg->encodings.end())
				(*it)->fromEncoding = *frEnc;
			else
				throw std::runtime_error(glue(L"Transformation \"" , (*it)->id , L"\" references non-existant from-encoding: ", (*it)->fromEncoding.id).c_str());
			}
			{
			std::set<Encoding>::const_iterator toEnc = lg->encodings.find((*it)->toEncoding);
			if (toEnc!=lg->encodings.end())
				(*it)->toEncoding = *toEnc;
			else
				throw std::runtime_error(glue(L"Transformation \"" , (*it)->id , L"\" references non-existant to-encoding: ", (*it)->toEncoding.id).c_str());
			}

			//Add to our lookup table, conditional on a few key points
			//TODO: Re-write... slightly messy.
			std::pair<Encoding, Encoding> newPair;
			newPair.first = *(lg->encodings.find((*it)->fromEncoding));
			newPair.second = *(lg->encodings.find((*it)->toEncoding));
			std::map< std::pair<Encoding, Encoding>, Transformation* >::iterator foundPair = const_cast<Language&>(*lg).transformationLookup.find(newPair);
			if (foundPair==lg->transformationLookup.end())
				const_cast<Language&>(*lg).transformationLookup[newPair] = *it;
			else if (foundPair->second->hasPriority)
				throw std::runtime_error(glue(L"Cannot add new Transformation (", (*it)->id, L") over one with priority: ", foundPair->second->id).c_str());
			else if (!(*it)->hasPriority)
				throw std::runtime_error(glue(L"Cannot add new Transformation (", (*it)->id, L"); it does not set \"hasPriority\"").c_str());
			else
				const_cast<Language&>(*lg).transformationLookup[newPair] = *it;
		}

		//Validate each input method
		for (std::set<InputMethod*>::const_iterator it=lg->inputMethods.begin(); it!=lg->inputMethods.end(); it++) {
			//Make sure this input method references an existing encoding. Replace it as we find it
			{
			std::set<Encoding>::const_iterator inEnc = lg->encodings.find((*it)->encoding);
			if (inEnc!=lg->encodings.end())
				(*it)->encoding = *inEnc;
			else
				throw std::runtime_error(glue(L"Input Method (", (*it)->id, L") references non-existant encoding: ", (*it)->encoding.id).c_str());
			}

			//Make sure that our encoding is EITHER the default, OR there is an appropriate transform.
			if ((*it)->encoding!=L"unicode") {
				std::pair<Encoding, Encoding> lookup;
				lookup.first = *(lg->encodings.find((*it)->encoding));
				//lookup.second = *FindKeyInSet(lg->encodings, L"unicode");
				lookup.second = *lg->encodings.find(L"unicode");
				if (lg->transformationLookup.find(lookup)==lg->transformationLookup.end())
					throw std::runtime_error(glue(L"No \"transformation\" exists for input method(", (*it)->id, L").").c_str());
			}
		}

		//Validate each display method
		for (std::set<DisplayMethod*>::const_iterator it=lg->displayMethods.begin(); it!=lg->displayMethods.end(); it++) {
			//Make sure this display method references an existing encoding. Replace it as we find it.
			{
			std::set<Encoding>::const_iterator outEnc = lg->encodings.find((*it)->encoding);
			if (outEnc!=lg->encodings.end())
				(*it)->encoding = *outEnc;
			else
				throw std::runtime_error(glue(L"Display Method (", (*it)->id, L") references non-existant encoding: ", (*it)->encoding.id).c_str());
			}

			//Make sure that our encoding is EITHER the default, OR there is an appropriate transform.
			if ((*it)->encoding.id != L"unicode") {
				std::pair<Encoding, Encoding> lookup;
				lookup.first = *(lg->encodings.find((*it)->encoding));
				//lookup.second = *FindKeyInSet(lg->encodings, L"unicode");
				lookup.second = *lg->encodings.find(L"unicode");
				if (lg->transformationLookup.find(lookup)==lg->transformationLookup.end())
					throw std::runtime_error(glue(L"No \"transformation\" exists for display method(", (*it)->id, L").").c_str());
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
		throw std::runtime_error(glue(L"Error! An unvalidated transformation exists in the configuration model: ", fromEnc.id, L"->", toEnc.id).c_str());

	//Done
	return found->second;
}



//Right now, this is only used to turn "track caret" off. 
// We need to merge this with standard code later.
void ConfigManager::overrideSetting(const wstring& settingName, bool value)
{
	wstring settingPure = sanitize_id(settingName);

	//TODO: We should merge this later with the regular settings code, to avoid duplicating
	//      the list of options
			//Set this based on name/value pair
	if (settingPure == L"hotkey")
		throw std::runtime_error("Cannot override hotkey setting");
	else if (settingPure == sanitize_id(L"default-language"))
		throw std::runtime_error("Cannot override default language setting");
	else if (settingPure == sanitize_id(L"silence-mywords-errors"))
		options.settings.silenceMywordsErrors = value;
	else if (settingPure == sanitize_id(L"balloon-start"))
		options.settings.balloonStart = value;
	else if (settingPure == sanitize_id(L"always-elevate"))
		options.settings.alwaysElevate = value;
	else if (settingPure == sanitize_id(L"track-caret"))
		options.settings.trackCaret = value;
	else if (settingPure == sanitize_id(L"lock-windows"))
		options.settings.lockWindows = value;
	else
		throw std::runtime_error(waitzar::glue(L"Cannot override setting: ", settingName).c_str());
}


void ConfigManager::backupLocalConfigOpts()
{
	localOptsBackup.clear();
	localOptsBackup.insert(localOpts.begin(), localOpts.end());
}


void ConfigManager::restoreLocalConfigOpts()
{
	localOpts.clear();
	localOpts.insert(localOptsBackup.begin(), localOptsBackup.end());
}


wstring ConfigManager::getLocalConfigOpt(const wstring& key)
{
	if (localOpts.count(key)>0)
		return localOpts[key];
	return L"";
}


void ConfigManager::clearLocalConfigOpt(const wstring& key)
{
	localOpts.erase(key);
}


void ConfigManager::setLocalConfigOpt(const wstring& key, const wstring& val)
{
	localOpts[key] = val;
}


bool ConfigManager::localConfigCausedError()
{
	return localConfError;
}



void ConfigManager::saveUserConfigFile(const std::wstring& path, bool emptyFile) 
{
	//Create the file
	std::ofstream cfgFile;
	cfgFile.open(waitzar::escape_wstr(path, false).c_str());

	//Add an empty set of parameters "{}" and some comments.
	cfgFile << "# This file contains user-specific settings for the WaitZar" <<std::endl
		<< "# typing program. These are the highest-level overrides possible;" <<std::endl
		<< "# to set an option, provide its full path and value. Separate" <<std::endl
		<< "# different settings with commas." <<std::endl
		<< "{" <<std::endl
		<< "  # Example: un-comment the following two lines (remove the \"#\")" <<std::endl
		<< "  #  to activate these settings:" <<std::endl
		<< std::endl
		<< "  #\"settings.track-caret\" : \"no\"," <<std::endl
		<< "  #\"settings.hotkey\" : \"Ctrl + Space\"" <<std::endl
		<< "}" <<std::endl;

	//Save
	cfgFile.flush();
	cfgFile.close();
}


void ConfigManager::saveLocalConfigFile(const std::wstring& path, bool emptyFile)
{
	//Open the file
	std::ofstream cfgFile;
	cfgFile.open(waitzar::escape_wstr(path, false).c_str());

	//Add an empty set of parameters "{}" and some comments.
	cfgFile << "# This file contains application-specific overrides for the WaitZar" <<std::endl
		<< "# typing program. Please do not edit this file. To provide user" <<std::endl
		<< "# overrides, edit \"waitzar.config.json.txt\", located in your \"Documents\"" <<std::endl
		<< "# or \"My Documents\" folder." <<std::endl
		<< "{" <<std::endl;

	//Empty?
	if (emptyFile) {
		cfgFile << std::endl;
	} else {
		//Save each option
		//TODO: Support UTF-8, maybe...
		string nl = "";
		for (map<wstring,wstring>::iterator it=localOpts.begin(); it!=localOpts.end(); it++) {
			cfgFile <<nl <<"    \"" <<waitzar::escape_wstr(it->first, false) <<"\" : \"" <<waitzar::escape_wstr(it->second, false) <<"\"";
			nl = ",\n";
		}
		cfgFile << std::endl;
	}
		
	//Done
	cfgFile << "}" <<std::endl;

	//Save
	cfgFile.flush();
	cfgFile.close();
}






//Bulid the tree of nodes we plan on using for verifying the syntax of the string-map tree
void ConfigManager::buildVerifyTree() {
	//Root nodes
	verifyTree.addChild(L"settings", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Advance
		return dynamic_cast<ConfigRoot&>(d).settings;
	});
	verifyTree.addChild(L"languages", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//We don't have the language ID, so return the same node
		return d;
	});
	verifyTree.addChild(L"extensions", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//We don't have the extension ID, so return the same node
		return d;
	});

	//Settings
	verifyTree[L"settings"].addChild(L"hotkey" , [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		dynamic_cast<SettingsNode&>(d).hotkey = HotkeyData(waitzar::purge_filename(s.str()));
		return d;
	});
	verifyTree[L"settings"].addChild(L"silence-mywords-errors" , [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		dynamic_cast<SettingsNode&>(d).silenceMywordsErrors = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"balloon-start" , [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		dynamic_cast<SettingsNode&>(d).balloonStart = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"always-elevate" , [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		dynamic_cast<SettingsNode&>(d).alwaysElevate = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"track-caret" , [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		dynamic_cast<SettingsNode&>(d).trackCaret = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"lock-windows" , [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		dynamic_cast<SettingsNode&>(d).lockWindows = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"suppress-virtual-keyboard" , [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		dynamic_cast<SettingsNode&>(d).suppressVirtualKeyboard = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"default-language" , [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		dynamic_cast<SettingsNode&>(d).defaultLanguage.first = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"whitespace-characters" , [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		dynamic_cast<SettingsNode&>(d).whitespaceCharacters = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"ignored-characters" , [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		dynamic_cast<SettingsNode&>(d).ignoredCharacters = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"hide-whitespace-markings" , [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		dynamic_cast<SettingsNode&>(d).hideWhitespaceMarkings = waitzar::read_bool(s.str());
		return d;
	});


	//Extensions
	verifyTree[L"extensions"].addChild(L"*", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		auto& extMap = dynamic_cast<ConfigRoot&>(d).extensions;
		std::wstring key = s.getKeyInParentMap();

		//Can change?
		if (!perms.chgExtension)
			throw std::runtime_error("Can't modify existing \"extensions\".");

		//Add it if it doesn't exist
		if (extMap.count(key)==0) {
			if (!perms.addExtension)
				throw std::runtime_error("Can't add a new \"extension\".");
			extMap[key] = ExtendNode(key);
		}

		//Return it
		return extMap[key];
	});

	//Single extension properties
	verifyTree[L"extensions"][L"*"].addChild(L"library-file", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
    	//Set it, return same
		dynamic_cast<ExtendNode&>(d).libraryFilePath = s.str();
		return d;
	});
	verifyTree[L"extensions"][L"*"].addChild(L"enabled", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
    	//Set it, return same
		dynamic_cast<ExtendNode&>(d).enabled = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"extensions"][L"*"].addChild(L"md5-hash", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
    	//Set it, return same
		dynamic_cast<ExtendNode&>(d).libraryFileChecksum = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"extensions"][L"*"].addChild(L"check-md5", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
    	//Set it, return same
		dynamic_cast<ExtendNode&>(d).requireChecksum = waitzar::read_bool(s.str());
		return d;
	});


	//Languages
	verifyTree[L"languages"].addChild(L"*", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		auto& langMap = dynamic_cast<ConfigRoot&>(d).languages;
		std::wstring key = s.getKeyInParentMap();

		//Can change?
		if (!perms.chgLanguage)
			throw std::runtime_error("Can't modify existing \"languages\".");

		//Add it if it doesn't exist
		if (langMap.count(key)==0) {
			if (!perms.addLanguage)
				throw std::runtime_error("Can't add a new \"language\".");
			langMap[key] = LangNode(key);
		}

		//Return it
		return langMap[key];
	});

	//Language properties
	verifyTree[L"languages"][L"*"].addChild(L"display-name", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
    	//Set it, return same
		dynamic_cast<LangNode&>(d).displayName = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"default-display-method", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
    	//Set pointer pair, return node
		dynamic_cast<LangNode&>(d).defaultDisplayMethodReg.first = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"default-display-method-small", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
    	//Set pointer pair, return node
		dynamic_cast<LangNode&>(d).defaultDisplayMethodSmall.first = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"default-output-encoding", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
    	//Set pointer pair, return node
		dynamic_cast<LangNode&>(d).defaultOutputEncoding.first = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"default-input-method", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
    	//Set pointer pair, return node
		dynamic_cast<LangNode&>(d).defaultInputMethod.first = waitzar::sanitize_id(s.str());
		return d;
	});


	//Language sub-classes
	verifyTree[L"languages"][L"*"].addChild(L"input-methods", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//We don't have the language ID, so return the same node
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"encodings", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//We don't have the language ID, so return the same node
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"transformations", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//We don't have the language ID, so return the same node
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"display-methods", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//We don't have the language ID, so return the same node
		return d;
	});


	//Language containers
	verifyTree[L"languages"][L"*"][L"input-methods"].addChild(L"*", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		auto& imMap = dynamic_cast<LangNode&>(d).inputMethods;
		std::wstring key = s.getKeyInParentMap();

		//Can change?
		if (!perms.chgLangInputMeth)
			throw std::runtime_error("Can't modify existing \"input-method\".");

		//Add it if it doesn't exist
		if (imMap.count(key)==0) {
			if (!perms.addLangInputMeth)
				throw std::runtime_error("Can't add a new \"input-method\".");
			imMap[key] = InMethNode(key);
		}

		//Return it
		return imMap[key];
	});
	verifyTree[L"languages"][L"*"][L"display-methods"].addChild(L"*", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		auto& dmMap = dynamic_cast<LangNode&>(d).displayMethods;
		std::wstring key = s.getKeyInParentMap();

		//Can change?
		if (!perms.chgLangDispMeth)
			throw std::runtime_error("Can't modify existing \"display-method\".");

		//Add it if it doesn't exist
		if (dmMap.count(key)==0) {
			if (!perms.addLangDispMeth)
				throw std::runtime_error("Can't add a new \"display-method\".");
			dmMap[key] = DispMethNode(key);
		}

		//Return it
		return dmMap[key];
	});
	verifyTree[L"languages"][L"*"][L"encodings"].addChild(L"*", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		auto& encMap = dynamic_cast<LangNode&>(d).encodings;
		std::wstring key = s.getKeyInParentMap();

		//Can change?
		if (!perms.chgLangEncoding)
			throw std::runtime_error("Can't modify existing \"encoding\".");

		//Add it if it doesn't exist
		if (encMap.count(key)==0) {
			if (!perms.addLangEncoding)
				throw std::runtime_error("Can't add a new \"encoding\".");
			encMap[key] = EncNode(key);
		}

		//Return it
		return encMap[key];
	});
	verifyTree[L"languages"][L"*"][L"transformations"].addChild(L"*", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		auto& transMap = dynamic_cast<LangNode&>(d).transformations;
		std::wstring key = s.getKeyInParentMap();

		//Can change?
		if (!perms.chgLangTransform)
			throw std::runtime_error("Can't modify existing \"transformation\".");

		//Add it if it doesn't exist
		if (transMap.count(key)==0) {
			if (!perms.addLangTransform)
				throw std::runtime_error("Can't add a new \"transformation\".");
			transMap[key] = TransNode(key);
		}

		//Return it
		return transMap[key];
	});



	//Encoding
	verifyTree[L"languages"][L"*"][L"encodings"][L"*"].addChild(L"display-name", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<EncNode&>(d).displayName = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"encodings"][L"*"].addChild(L"initial", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<EncNode&>(d).initial = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"encodings"][L"*"].addChild(L"image", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<EncNode&>(d).imagePath = s.str();
		return d;
	});
	verifyTree[L"languages"][L"*"][L"encodings"][L"*"].addChild(L"use-as-output", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<EncNode&>(d).canUseAsOutput = waitzar::read_bool(s.str());
		return d;
	});



	//Transformations
	verifyTree[L"languages"][L"*"][L"transformations"][L"*"].addChild(L"from-encoding", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<TransNode&>(d).fromEncoding.first = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"transformations"][L"*"].addChild(L"to-encoding", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<TransNode&>(d).toEncoding.first = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"transformations"][L"*"].addChild(L"type", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<TransNode&>(d).type = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"transformations"][L"*"].addChild(L"has-priority", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<TransNode&>(d).hasPriority = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"transformations"][L"*"].addChild(L"source-file", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<TransNode&>(d).sourceFile = s.str();
		return d;
	});


	//Input method
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"display-name", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).displayName = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"encoding", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).encoding.first = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"user-words-file", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).userWordsFile = s.str();
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"wordlist", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).extraWordsFile = s.str();
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"keyboard-file", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).keyboardFile = s.str();
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"type", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).type = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"type-burmese-numerals", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).typeBurmeseNumbers = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"suppress-uppercase", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).suppressUppercase = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"control-keys", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).controlKeyStyle = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"numeral-conglomerate", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).typeNumeralConglomerates = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"disable-cache", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).disableCache = waitzar::read_bool(s.str());
		return d;
	});


	//Display method
	verifyTree[L"languages"][L"*"][L"display-methods"][L"*"].addChild(L"encoding", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<DispMethNode&>(d).encoding.first = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"display-methods"][L"*"].addChild(L"type", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<DispMethNode&>(d).type = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"display-methods"][L"*"].addChild(L"font-face-name", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<DispMethNode&>(d).fontFaceName = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"display-methods"][L"*"].addChild(L"point-size", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<DispMethNode&>(d).pointSize = waitzar::read_int(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"display-methods"][L"*"].addChild(L"font-file", [](const Node& s, TNode& d, const CfgPerm& perms)->TNode&{
		//Cast and set
		dynamic_cast<DispMethNode&>(d).fontFile = s.str();
		return d;
	});

}


const Settings& ConfigManager::getSettings() 
{
	//Load if needed
	root = Node();
	troot = ConfigRoot();
	if (verifyTree.isLeaf()) {
		buildVerifyTree();
	}

	if (!loadedSettings) {
		//We need at least one config file to parse.
		if (this->mainConfig.isEmpty())
			throw std::runtime_error("No main config file defined.");

		//Parse each config file in turn.
		//First: main config
		{
		vector<wstring> ctxt;
		this->readInConfig(this->mainConfig.json(), this->mainConfig.getFolderPath(), ctxt, false, false, NULL);
		this->buildAndWalkConfigTree(this->mainConfig, root, troot, verifyTree, PrimaryCfgPerm());
		//this->buildUpConfigTree(this->mainConfig.json(), &root, this->mainConfig.getFolderPath(), PrimaryCfgPerm());
		}

		//Second: extensions config
		if (!this->commonConfig.isEmpty()){
			vector<wstring> ctxt;
			this->readInConfig(this->commonConfig.json(), this->commonConfig.getFolderPath(), ctxt, false, true, NULL);
			this->buildAndWalkConfigTree(this->commonConfig, root, troot, verifyTree, ExtendCfgPerm());
			//this->buildUpConfigTree(this->commonConfig.json(), &root, this->commonConfig.getFolderPath(), ExtendCfgPerm());
		}

		//Parse each language config file.
		// (Note: This will all have to be moved out eventually.... we can't put this into loadLanguageMainFiles() because
		//   the local/user configs might expect certain languages to be loaded.)
		{
			vector<wstring> ctxt;
			for (std::map<JsonFile , std::vector<JsonFile> >::const_iterator it = langConfigs.begin(); it!=langConfigs.end(); it++) {
				this->readInConfig(it->first.json(), it->first.getFolderPath(), ctxt, false, false, NULL);
				this->buildAndWalkConfigTree(it->first, root, troot, verifyTree, LangLevelCfgPerm());
				//this->buildUpConfigTree(it->first.json(), &root, it->first.getFolderPath(), LangLevelCfgPerm());
				for (std::vector<JsonFile>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
					this->readInConfig(it2->json(), it2->getFolderPath(), ctxt, false, false, NULL);
					this->buildAndWalkConfigTree(*it2, root, troot, verifyTree, LangLevelCfgPerm());
					//this->buildUpConfigTree(it2->json(), &root, it2->getFolderPath(), LangLevelCfgPerm());
				}
			}
		}

		//Next: local and user configs
		vector<wstring> ctxt;
		if (this->localConfig.isSet()) {
			this->readInConfig(this->localConfig.json(), this->localConfig.getFolderPath(), ctxt, true, false, &localOpts);

			//Save local opts!
			/*this->buildUpConfigTree(this->localConfig.json(), &root, this->localConfig.getFolderPath(), UserLocalCfgPerm(),*/
			this->buildAndWalkConfigTree(this->localConfig, root, troot, verifyTree, UserLocalCfgPerm(),
				[&locallySetOptions](const Node& n) {
					locallySetOptions[n.getFullyQualifiedKeyName()] = n.str();
				}
			);
		}
		if (this->userConfig.isSet()) {
			this->readInConfig(this->userConfig.json(), this->userConfig.getFolderPath(), ctxt, true, false, NULL);
			this->buildAndWalkConfigTree(this->userConfig, root, troot, verifyTree, UserLocalCfgPerm());
			//this->buildUpConfigTree(this->userConfig.json(), &root, this->userConfig.getFolderPath(), UserLocalCfgPerm());
		}


		//Now walk it and set all settings
		/*if (!this->root.isEmpty())
			this->walkConfigTree(this->root, this->troot, this->verifyTree);*/



		//TEST
/*		for (auto it=localOpts.begin(); it!=localOpts.end(); it++) {
			if (locallySetOptions.count(it->first)>0 && locallySetOptions[it->first]==it->second)
				std::cout <<"Test passed" <<std::endl;
			else
				std::cout <<"Test FAILED" <<std::endl;
		}
		for (auto it=locallySetOptions.begin(); it!=locallySetOptions.end(); it++) {
			if (localOpts.count(it->first)==0)
				std::cout <<"Test FAILED: extra option!" <<std::endl;
		}*/
		//END TEST


		//Minor post-processing
		options.settings.hotkey = HotkeyData(options.settings.hotkeyStrRaw);
		//generateHotkeyValues(options.settings.hotkeyStrRaw, options.settings.hotkey);

		//Done
		loadedSettings = true;
	}

	//Return the object
	return this->options.settings;
}



//Build the tree, then walk it into the existing setup
void ConfigManager::buildAndWalkConfigTree(const JsonFile& file, Node& rootNode, TNode& rootTNode, const TransformNode& rootVerifyNode, const CfgPerm& perm, std::function<void (const Node& n)> OnSetCallback)
{
	std::string folderPath = file.getFilePath();
	std::cout <<"Building: " <<(folderPath.empty()?"<default>":folderPath) <<std::endl;

	//Better error reporting
	try {
		this->buildUpConfigTree(file.json(), rootNode, file.getFolderPath(), OnSetCallback);
		this->walkConfigTree(rootNode, rootTNode, rootVerifyNode, perm);
	} catch (nodeset_exception& ex) {
		//Bad option; we can catch some of these here.
		std::wstringstream msg;
		msg <<L"Error loading file: " <<std::endl <<L"   " <<file.getFilePath().c_str() <<std::endl
			<<L"...on property:" <<std::endl <<L"   " <<ex.key() <<std::endl
			<<L"...error was:" <<std::endl <<L"   " <<ex.what() <<std::endl;
		throw std::runtime_error(waitzar::escape_wstr(msg.str()).c_str());
	}
}


void ConfigManager::buildUpConfigTree(const Json::Value& root, Node& currNode, const std::wstring& currDirPath, std::function<void (const Node& n)> OnSetCallback)
{
	//The root node is a map; get its keys and iterate
	Value::Members keys = root.getMemberNames();
	for (auto itr=keys.begin(); itr!=keys.end(); itr++) {
		try {
			//Key: For each dot-seperated ID, advance the current node
			Node* childNode = &currNode;
			vector<wstring> opts = separate(sanitize_id(waitzar::mbs2wcs(*itr)), L'.');
			for (auto key=opts.begin(); key!=opts.end(); key++) {
				childNode = &childNode->getOrAddChild(*key);
			}

			//Value: Store another child node (and recurse) or make this a leaf node
			const Value* value = &root[*itr];
			if (value->isObject()) {
				//Inductive case: Continue reading all options under this type
				this->buildUpConfigTree(*value, *childNode, currDirPath, OnSetCallback);
			} else if (value->isString()) {
				//Base case: the "value" is also a string (set the property)
				childNode->str(sanitize_value(waitzar::mbs2wcs(value->asString()), currDirPath));
				childNode->setAndPropagateDirty(true);

				//Callback
				if (OnSetCallback) {
				//for (auto it=OnSetCallbacks.begin(); it!=OnSetCallbacks.end(); it++) {
					//(*it)(*childNode);
					OnSetCallback(*childNode);
				}
			} else {
				throw std::runtime_error("ERROR: Config file options should always be string or hash types.");
			}
		} catch (std::exception& ex) {
			//Gracefully catch
			wstring fullKey;
			try {
				fullKey = currNode.getFullyQualifiedKeyName();
			} catch (std::exception& ex2) {
				//NOTE: std::exception() seems to share its "what" variable,
				//      so we may need to copy it out BEFORE trying to get the "full key"
				fullKey = L"<undefined>";
			}
			throw nodeset_exception(ex.what(), fullKey.c_str());
		}
	}
}



//Walk the root, build up options as you go.
void ConfigManager::walkConfigTree(Node& source, TNode& dest, const TransformNode& verify, const CfgPerm& perm)
{
	//Iterate to its children.
	for (auto it=source.getChildNodes().begin(); it!=source.getChildNodes().end(); it++) {
		try {
			//First, make sure it's non-empty
			if (it->second.isEmpty())
				throw std::runtime_error(waitzar::glue(L"Node is empty: ", it->second.getFullyQualifiedKeyName()).c_str());

			//Skip clean nodes
			if (!it->second.isDirty())
				continue;

			//Next, get its corresponding "verify" node
			const TransformNode& nextVerify = verify[it->first];

			//If the node types don't match, it's also an error
			if (it->second.isLeaf()!=nextVerify.isLeaf()) {
				wstring message = nextVerify.isLeaf() ? L"Expected leaf node has children: " : L"Expected interior node has no children: ";
				throw std::runtime_error(waitzar::glue(message, it->second.getFullyQualifiedKeyName()).c_str());
			}

			std::cout <<"  Iterate: " <<waitzar::escape_wstr(it->second.getFullyQualifiedKeyName()) <<std::endl;

			//Get and apply the "match" function. Once all 3 points line up, call "walkConfigTree" if appropriate
			const std::function<TNode& (const Node& src, TNode& dest, const CfgPerm& perms)>& matchAction = nextVerify.getMatchAction();
			TNode& nextTN = matchAction(it->second, dest, perm);
			if (!it->second.isLeaf())
				walkConfigTree(it->second, nextTN, nextVerify, perm);
			else
				it->second.setAndPropagateDirty(false); //Clean it; we've read it
		} catch (std::exception& ex) {
			//Gracefully catch
			wstring fullKey;
			try {
				fullKey = source.getFullyQualifiedKeyName();
			} catch (std::exception& ex2) {
				//NOTE: std::exception() seems to share its "what" variable,
				//      so we may need to copy it out BEFORE trying to get the "full key"
				fullKey = L"<undefined>";
			}
			throw nodeset_exception(ex.what(), fullKey.c_str());
		}
	}
}




void ConfigManager::loadLanguageMainFiles()
{
	//Main config file must be read by now
	if (!this->loadedSettings)
		throw std::runtime_error("Must load settings before language main files.");

	//Done
	loadedLanguageMainFiles = true;
}


void ConfigManager::loadLanguageSubFiles()
{
	//Main config file must be read by now
	if (!this->loadedSettings)
		throw std::runtime_error("Must load settings before language sub files.");

	//Done
	loadedLanguageSubFiles = true;
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

const std::set<Extension*>& ConfigManager::getExtensions()
{
	//Main config file must be read by now
	if (!this->loadedSettings)
		throw std::runtime_error("Must load settings before language main files.");

	return this->options.extensions;
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
//optionsSet, if non-null, will save the string set. E.g., "settings.defaultlanguage"=>"myanmar"
//DEPRECATED
void ConfigManager::readInConfig(const Value& root, const wstring& folderPath, vector<wstring> &context, bool restricted, bool allowDLL, map<wstring, wstring>* const optionsSet)
{
	//We always operate on maps:
	//json_spirit::Value_type t = root.type();
	//wObject currPairs = root.get_value<wObject>();
	//for (auto &itr : currPairs) {
	Value::Members keys = root.getMemberNames();
	for (auto itr=keys.begin(); itr!=keys.end(); itr++) {
 		//Append to the context
		int numToRemove = 0;
		{
		vector<wstring> opts = separate(sanitize_id(waitzar::mbs2wcs(*itr)), L'.');
		context.insert(context.end(), opts.begin(), opts.end());
		numToRemove = opts.size();
		}

		//React to this option/category
		Value value = root[*itr];
		if (value.isObject()) {
			//Inductive case: Continue reading all options under this type
			this->readInConfig(value, folderPath, context, restricted, allowDLL, optionsSet);
		} else if (value.isString()) {
			//Base case: the "value" is also a string (set the property)
			wstring val = sanitize_value(waitzar::mbs2wcs(value.asString()), folderPath);
			this->setSingleOption(folderPath, context, val, restricted, allowDLL);

			//Save?
			if (optionsSet!=NULL) {
				wstring key = L"";
				for (size_t i=0; i<context.size(); i++) {
					key += context[i] + (i<context.size()-1?L".":L"");
				}
				(*optionsSet)[key] = val;
			}
		} else {
			throw std::runtime_error("ERROR: Config file options should always be string or hash types.");
		}

		//Remove, get ready for the next option
		while (numToRemove>0) {
			context.pop_back();
			numToRemove--;
		}
	}
}


void ConfigManager::setSingleOption(const wstring& folderPath, const vector<wstring>& name, const std::wstring& value, bool restricted, bool allowDLL)
{
	//Read each "context" setting from left to right. Context settings are separated by periods. 
	//   Note: There are much faster/better ways of doing this, but for now we'll keep all the code
	//   centralized and easy to read.
	try {
		//Need at least one setting
		if (name.empty())
			return;

		//Settings? Languages? Extensions?
		if (name[0] == L"settings") {
			//Need to finish all partial settings
			if (name.size()<=1)
				throw std::invalid_argument("");

			//Set this based on name/value pair
			if (name[1] == L"hotkey")
				options.settings.hotkeyStrRaw = sanitize_id(value);
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
			else if (name[1] == sanitize_id(L"suppress-virtual-keyboard"))
				options.settings.suppressVirtualKeyboard = read_bool(value);
			else if (name[1] == sanitize_id(L"whitespace-characters"))
				options.settings.whitespaceCharacters = sanitize_id(value);
			else if (name[1] == sanitize_id(L"ignored-characters"))
				options.settings.ignoredCharacters = sanitize_id(value);
			else if (name[1] == sanitize_id(L"hide-whitespace-markings"))
				options.settings.hideWhitespaceMarkings = read_bool(value);
			else if (name[1] == sanitize_id(L"default-language")) {
				//We have to handle "lastused" values slightly differently.
				wstring defLang = sanitize_id(value);
				if (defLang==L"lastused") {
					//First, make sure we're not setting this in an inappropriate location.
					if (options.settings.defaultLanguage.empty())
						throw std::runtime_error("Cannot specify a default language of \"lastused\" unless a fallback default is specified.");
					if (options.settings.defaultLanguage.find(L"lastused")==wstring::npos)
						options.settings.defaultLanguage += L".lastused"; //E.g., "myanmar.lastused"
				} else {
					//Normal setting
					options.settings.defaultLanguage = defLang;
				}
			} else
				throw std::invalid_argument("");

			//Done
			return;
		} else if (name[0] == L"languages") {
			//Need to finish all partial settings
			if (name.size()<=2)
				throw std::invalid_argument("");

			//Get the language id
			//TODO: Add better error messages using the glue() functions.
			wstring langName = name[1];
			//std::set<Language>::const_iterator lang = FindKeyInSet<Language>(options.languages, langName);
			auto lang = options.languages.find(langName);
			if (lang==options.languages.end()) {
				if(restricted)
					throw std::runtime_error("Cannot create a new Language in user or system-local config files.");
				else
					lang = options.languages.insert(Language(langName)).first;
			}

			//Static settings
			if (name[2] == sanitize_id(L"display-name"))
				const_cast<Language&>(*lang).displayName = purge_filename(value);
			else if (name[2] == sanitize_id(L"default-output-encoding")) {
				//We have to handle "lastused" values slightly differently.
				wstring defOutput = sanitize_id(value);
				if (defOutput==L"lastused") {
					//First, make sure we're not setting this in an inappropriate location.
					if (lang->defaultOutputEncoding.id.empty())
						throw std::runtime_error("Cannot specify a default output encoding of \"lastused\" unless a fallback default is specified.");
					if (lang->defaultOutputEncoding.id.find(L"lastused")==wstring::npos)
						const_cast<Language&>(*lang).defaultOutputEncoding.id += L".lastused"; //E.g., "unicode.lastused"
				} else {
					//Normal setting
					const_cast<Language&>(*lang).defaultOutputEncoding.id = defOutput;
				}
			} else if (name[2] == sanitize_id(L"default-display-method"))
				const_cast<Language&>(*lang).defaultDisplayMethodReg = sanitize_id(value);
			else if (name[2] == sanitize_id(L"default-display-method-small"))
				const_cast<Language&>(*lang).defaultDisplayMethodSmall = sanitize_id(value);
			else if (name[2] == sanitize_id(L"default-input-method")) {
				//We have to handle "lastused" values slightly differently.
				wstring defInput = sanitize_id(value);
				if (defInput==L"lastused") {
					//First, make sure we're not setting this in an inappropriate location.
					if (lang->defaultInputMethod.empty())
						throw std::runtime_error("Cannot specify a default input method of \"lastused\" unless a fallback default is specified.");
					if (lang->defaultInputMethod.find(L"lastused")==wstring::npos)
						const_cast<Language&>(*lang).defaultInputMethod += L".lastused"; //E.g., "ayarkbd.lastused"
				} else {
					//Normal setting
					const_cast<Language&>(*lang).defaultInputMethod = defInput;
				}
			} else {
				//Need to finish all partial settings
				if (name.size()<=4)
					throw std::invalid_argument("");

				//Dynamic settings
				//Todo: Make this slightly less wordy.
				if (name[2] == sanitize_id(L"input-methods")) {
					//Input methods
					wstring inputName = name[3];

					//Allowed to add new Input Methods?
					//auto iM = lang->inputMethods.find(inputName);
					if (FindKeyInSet(lang->inputMethods, inputName)==lang->inputMethods.end() && restricted)
						throw std::runtime_error("Cannot create a new Input Method in user or system-local config files.");

					//Just save all its options. Then, call a Factory method when this is all done
					pair<wstring,wstring> key = pair<wstring,wstring>(langName,inputName);
					partialInputMethods[key][sanitize_id(name[4])] = value;
					if (partialInputMethods[key].size()==1) //First option
						partialInputMethods[key][sanitize_id(L"current-folder")] = L"";
				} else if (name[2] == sanitize_id(L"encodings")) {
					//Encodings
					wstring encName = name[3];

					//Allowed to add new Encodings?
					//if (FindKeyInSet(lang->encodings, encName)==lang->encodings.end() && restricted)
					if (lang->encodings.find(encName)==lang->encodings.end() && restricted)
						throw std::runtime_error("Cannot create a new Encoding in user or system-local config files.");

					//Just save all its options. Then, call a Factory method when this is all done
					pair<wstring, wstring> key = pair<wstring, wstring>(langName,encName);
					partialEncodings[key][sanitize_id(name[4])] = value;
					if (partialEncodings[key].size()==1) //First option
						partialEncodings[key][sanitize_id(L"current-folder")] = L"";
				} else if (name[2] == sanitize_id(L"transformations")) {
					//Transformations
					wstring transName = name[3];

					//Allowed to add new Transformations?
					if (FindKeyInSet(lang->transformations, transName)==lang->transformations.end() && restricted)
						throw std::runtime_error("Cannot create a new Tranformation in user or system-local config files.");

					//Just save all its options. Then, call a Factory method when this is all done
					pair<wstring, wstring> key = pair<wstring, wstring>(langName,transName);
					partialTransformations[key][sanitize_id(name[4])] = value;
					if (partialTransformations[key].size()==1) //First option
						partialTransformations[key][sanitize_id(L"current-folder")] = L"";
				} else if (name[2] == sanitize_id(L"display-methods")) {
					//Display methods
					wstring dispMethod = name[3];

					//Allowed to add new Display Method?
					if (FindKeyInSet(lang->displayMethods, dispMethod)==lang->displayMethods.end() && restricted)
						throw std::runtime_error("Cannot create a new Display Method in user or system-local config files.");

					//Just save all its options. Then, call a Factory method when this is all done
					pair<wstring, wstring> key = pair<wstring, wstring>(langName,dispMethod);
					partialDisplayMethods[key][sanitize_id(name[4])] = value;
					if (partialDisplayMethods[key].size()==1) //First option
						partialDisplayMethods[key][sanitize_id(L"current-folder")] = L"";
				} else {
					//Error
					throw std::invalid_argument("");
				}
			}

		} else if (name[0] == L"extensions") {
			//Need to finish all partial settings
			if (name.size()<=2)
				throw std::invalid_argument("");

			if (!allowDLL)
				throw std::runtime_error(waitzar::glue(L"Attempt to modify an extension(", name[1], L") outside of the directory config/Common: " , folderPath).c_str());

			//Get the extension id; insert if necessary
			wstring extID = name[1];
			auto ext = FindKeyInSet(options.extensions, extID);
			if (ext==options.extensions.end()) {
				//Save some shuffling now
				Extension* newExt = NULL;
				if (extID==L"javascript")
					newExt = new JavaScriptConverter(extID);
				else
					newExt = new Extension(extID);
				ext = options.extensions.insert(newExt).first;
			}

			//Now, react to the individual settings
			if (name[2] == sanitize_id(L"library-file")) {
				//Must be local (checked already!)
				//if (value.find(L'\\')!=wstring::npos || value.find(L'/')!=wstring::npos)
				//	throw std::runtime_error(waitzar::glue(L"DLL path contains a / or \\: ", value).c_str());
				const_cast<Extension*>(*ext)->libraryFilePath = value;
			} else if (name[2] == sanitize_id(L"enabled")) {
				const_cast<Extension*>(*ext)->enabled = read_bool(value);
			} else if (name[2] == sanitize_id(L"check-md5")) {
				const_cast<Extension*>(*ext)->requireChecksum = read_bool(value);
			} else if (name[2] == sanitize_id(L"md5-hash")) {
				const_cast<Extension*>(*ext)->libraryFileChecksum = value;
			} else {
				//Error
				throw std::invalid_argument("");
			}
		} else
			throw std::invalid_argument("");
	} catch (std::invalid_argument&) {
		//Bad option
		std::wstringstream nameStr;
		wstring dot = L"";
		for (vector<wstring>::const_iterator nameOpt=name.begin(); nameOpt!=name.end(); nameOpt++) {
			nameStr <<dot <<(*nameOpt);
			dot = L"/"; //To distinguish "dot" errors
		}
		throw std::runtime_error(std::string("Invalid option: \"" + waitzar::escape_wstr(nameStr.str()) + "\", with value: \"" + waitzar::escape_wstr(value) + "\"").c_str());
	}
}



//Take an educated guess as to whether or not this is a file.
bool ConfigManager::IsProbablyFile(const std::wstring& str)
{
	//First, get the right-most "."
	size_t lastDot = str.rfind(L'.');
	if (lastDot==wstring::npos)
		return false;

	//It's a file if there are between 1 and 4 characters after this dot
	int diff = str.size() - 1 - (int)lastDot;
	return (diff>=1 && diff<=4);
}


//Remove leading and trailing whitespace
wstring ConfigManager::sanitize_value(const wstring& str, const std::wstring& filePath)
{
	//First, remove spurious spaces/tabs/newlines
	size_t firstLetter = str.find_first_not_of(L" \t\n");
	size_t lastLetter = str.find_last_not_of(L" \t\n");
	if (firstLetter==wstring::npos||lastLetter==wstring::npos)
		return L"";

	//Next, try to guess if this represents a file
	wstring res = str.substr(firstLetter, lastLetter-firstLetter+1);
	if (IsProbablyFile(res)) {
		//Replace all "/" with "\\"
		std::replace(res.begin(), res.end(), L'/', L'\\');

		//Ensure it references no sub-directories whatsoever
		if (res.find(L'\\')!=wstring::npos)
			throw std::runtime_error("Config files cannot reference files outside their own directories.");

		//Append the directory (and a "\\" if needed)
		res = filePath + (filePath[filePath.size()-1]==L'\\'?L"":L"\\") + res;
	}
	return res;
}

//Sanitize, then return in lowercase, with '-', '_', and whitespace removed
wstring ConfigManager::sanitize_id(const wstring& str) 
{
	return waitzar::sanitize_id(str);
}


//Remove a filename if we've added it
wstring ConfigManager::purge_filename(const wstring& str)
{
	return waitzar::purge_filename(str);
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
	return waitzar::read_bool(str);
}


int ConfigManager::read_int(const std::wstring& str)
{
	return waitzar::read_int(str);
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
