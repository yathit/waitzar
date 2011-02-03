/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */


#include "ConfigManager.h"

using std::map;
using std::vector;
using std::pair;
using std::wstring;
using std::string;

using Json::Value;



/**
 * Currently this only allows us to override something in the settings department. Theoretically,
 *    we could easily extend it to most other settings, provided that:
 *      1) Our "permissions" never allows us to ADD an item (this would mess up references)
 *      2) We re-validate things like existing/default encodings, and re-build, say, KeyboardInputMethods if their keyboard files change.
 *      3) We are very careful acout "last used"
 */
void ConfigManager::OverrideSingleSetting(RuntimeConfig& currConfig, const std::wstring& name, const std::wstring& value)
{
	//Temp
	if (value==L"lastused")
		throw std::runtime_error("Cannot call OverrideSingleSetting with \"last-used\" as the value");

	//Make a "json file" using a stream, with this option as the only key
	string key = "{\"" + waitzar::escape_wstr(name) + "\":\"" + waitzar::escape_wstr(value) + "\"}";
	JsonFile f(key, true);
	StringNode str;

	//Walk!
	BuildAndWalkConfigTree(f, str, currConfig.config, ConfigTreeWalker::GetWalkerRoot(), OverrideSettingCfgPerm());

}



void ConfigManager::mergeInConfigFile(const string& cfgFile, const CfgPerm& perms, bool fileIsStream, std::function<void (const StringNode& n)> OnSetCallback, std::function<void (const std::wstring& k)> OnError)
{
	//Can't modify a sealed configuration
	if (this->sealed)
		throw std::runtime_error("Can't add to ConfigManager; instance has been sealed.");

	//Get a JsonFile representing this path/stream
	JsonFile file = JsonFile(cfgFile, fileIsStream);

	//Merge it into the tree
	BuildAndWalkConfigTree(file, root, troot, ConfigTreeWalker::GetWalkerRoot(), perms, OnSetCallback, OnError);
}



//Validate all Input Managers, Display Managers, Outputs, and Transformations; make
//     sure the right encodings (and transformations) exist for each.
//Then, build fast-to-lookup data structures for actual use in WZ.
//   lastUsedSettings = languageid -> [lastInput, lastOutput, lastDispBig, lastDispSmall]
/*void ConfigManager::generateInputsDisplaysOutputs(const map<wstring, vector<wstring> >& lastUsedSettings)
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
}*/



//
// This is the only way to get an instance of TNode from the config manager; use it to load a RuntimeConfig() object
//
const ConfigRoot& ConfigManager::sealConfig(const map<wstring, vector<wstring> >& lastUsedSettings, std::function<void (const std::wstring& k)> OnError)
{
	//Shortcut; already sealed once
	if (this->sealed)
		return troot;

	//Load all extensions
	try {
		for (auto extIt=troot.extensions.begin(); extIt!=troot.extensions.end(); extIt++) {
			extIt->second.impl = WZFactory::makeAndVerifyExtension(extIt->first, extIt->second);
		}

		//Load all objects using our factory methods
		for (auto langIt=troot.languages.begin(); langIt!=troot.languages.end(); langIt++) {
			//First, add a "self2self" transformation
			langIt->second.transformations[L"self2self"] = TransNode(L"self2self");
			langIt->second.transformations[L"self2self"].fromEncoding = L"unicode";
			langIt->second.transformations[L"self2self"].toEncoding = L"unicode";
			langIt->second.transformations[L"self2self"].type = TRANSFORM_TYPE::BUILTIN;
			langIt->second.transformations[L"self2self"].hasPriority = true;

			//Encodings
			for (auto encIt=langIt->second.encodings.begin(); encIt!=langIt->second.encodings.end(); encIt++) {
				WZFactory::verifyEncoding(encIt->first, encIt->second);
			}

			//Transformations
			for (auto trIt=langIt->second.transformations.begin(); trIt!=langIt->second.transformations.end(); trIt++) {
				trIt->second.impl = WZFactory::makeAndVerifyTransformation(troot, langIt->second, trIt->first, trIt->second);
			}

			//Input methods
			for (auto inIt=langIt->second.inputMethods.begin(); inIt!=langIt->second.inputMethods.end(); inIt++) {
				inIt->second.impl = WZFactory::makeAndVerifyInputMethod(langIt->second, inIt->first, inIt->second);
			}

			//Display methods
			for (auto dispIt=langIt->second.displayMethods.begin(); dispIt!=langIt->second.displayMethods.end(); dispIt++) {
				dispIt->second.impl = WZFactory::makeAndVerifyDisplayMethod(langIt->second, dispIt->first, dispIt->second);
			}

			//And finally, verify the language itself
			WZFactory::verifyLanguage(langIt->first, langIt->second, WZFactory::InterpretFlashSave(lastUsedSettings, langIt->first, 0), WZFactory::InterpretFlashSave(lastUsedSettings, langIt->first, 1));
		}

		//Also verify the settings themselves
		WZFactory::verifySettings(troot, troot.settings, WZFactory::InterpretFlashSave(lastUsedSettings, L"language.default", 0));
	} catch (nodeset_exception& ex) {
		//User action
		if (OnError) {
			try {
				OnError(wstring(ex.key()));
			} catch (...) {}
		}

		//Bad option; we can catch some of these here.
		std::wstringstream msg;
		msg <<L"Error sealing config tree:" <<std::endl
			<<L"...on property:" <<std::endl <<L"   " <<ex.key() <<std::endl
			<<L"...error was:" <<std::endl <<L"   " <<ex.what() <<std::endl;
		throw std::runtime_error(waitzar::escape_wstr(msg.str()).c_str());
	}


	//Done
	this->sealed = true;
	return troot;
}



//Right now, this is only used to turn "track caret" off. 
// We need to merge this with standard code later.
/*void ConfigManager::overrideSetting(const wstring& settingName, bool value)
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
}*/





void ConfigManager::SaveUserConfigFile(const std::wstring& path)
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


void ConfigManager::SaveLocalConfigFile(const std::wstring& path, const std::map<std::wstring, std::wstring>& properties)
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

	//Save each option
	//TODO: Support UTF-8, maybe...
	string nl = "";
	for (auto it=properties.begin(); it!=properties.end(); it++) {
		cfgFile <<nl <<"    \"" <<waitzar::escape_wstr(it->first, false) <<"\" : \"" <<waitzar::escape_wstr(it->second, false) <<"\"";
		nl = ",\n";
	}
	cfgFile << std::endl;
		
	//Done
	cfgFile << "}" <<std::endl;

	//Save
	cfgFile.flush();
	cfgFile.close();
}




//Build the tree, then walk it into the existing setup
void ConfigManager::BuildAndWalkConfigTree(const JsonFile& file, StringNode& rootNode, GhostNode& rootTNode, const TransformNode& rootVerifyNode, const CfgPerm& perm, std::function<void (const StringNode& n)> OnSetCallback, std::function<void (const std::wstring& k)> OnError)
{
	if (Logger::isLogging('C')) {
		std::wstringstream msg;
		msg <<L"Building: ";
		std::string folderPath = file.getFilePath();
		if (folderPath.empty())
			msg <<L"<default>";
		else
			msg <<folderPath.c_str();
		Logger::writeLogLine('C',  msg.str());
	}

	//Better error reporting
	try {
		BuildUpConfigTree(file.json(), rootNode, file.getFolderPath(), OnSetCallback);
		WalkConfigTree(rootNode, rootTNode, rootVerifyNode, perm);
	} catch (nodeset_exception& ex) {
		//User action
		if (OnError) {
			try {
				OnError(wstring(ex.key()));
			} catch (...) {}
		}

		//Bad option; we can catch some of these here.
		std::wstringstream msg;
		msg <<L"Error loading file: " <<std::endl <<L"   " <<file.getFilePath().c_str() <<std::endl
			<<L"...on property:" <<std::endl <<L"   " <<ex.key() <<std::endl
			<<L"...error was:" <<std::endl <<L"   " <<ex.what() <<std::endl;
		throw std::runtime_error(waitzar::escape_wstr(msg.str()).c_str());
	}
}


void ConfigManager::BuildUpConfigTree(const Json::Value& root, StringNode& currNode, const std::wstring& currDirPath, std::function<void (const StringNode& n)> OnSetCallback)
{
	//The root node is a map; get its keys and iterate
	Value::Members keys = root.getMemberNames();
	for (auto itr=keys.begin(); itr!=keys.end(); itr++) {
		try {
			//Key: For each dot-seperated ID, advance the current node
			StringNode* childNode = &currNode;
			vector<wstring> opts = waitzar::separate(waitzar::sanitize_id(waitzar::mbs2wcs(*itr)), L'.');
			for (auto key=opts.begin(); key!=opts.end(); key++) {
				childNode = &childNode->getOrAddChild(*key);
			}

			//Value: Store another child node (and recurse) or make this a leaf node
			const Value* value = &root[*itr];
			if (value->isObject()) {
				//Inductive case: Continue reading all options under this type
				BuildUpConfigTree(*value, *childNode, currDirPath, OnSetCallback);
			} else if (value->isString()) {
				//Base case: the "value" is also a string (set the property)
				childNode->str(waitzar::sanitize_value(waitzar::mbs2wcs(value->asString()), currDirPath));
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
void ConfigManager::WalkConfigTree(StringNode& source, GhostNode& dest, const TransformNode& verify, const CfgPerm& perm)
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

			//Log every value reached.
			if (Logger::isLogging('C')) {
				Logger::writeLogLine('C', L"   Walking: " + it->second.getFullyQualifiedKeyName());
				if (it->second.isLeaf())
					Logger::writeLogLine('C', L"      = " + it->second.str() + (it->second.getStringStack().size()>1?L"  (Override)":L""));
			}

			//Get and apply the "match" function. Once all 3 points line up, call "walkConfigTree" if appropriate
			const std::function<GhostNode& (const StringNode& src, GhostNode& dest, const CfgPerm& perms)>& matchAction = nextVerify.getMatchAction();
			GhostNode& nextTN = matchAction(it->second, dest, perm);
			if (!it->second.isLeaf())
				WalkConfigTree(it->second, nextTN, nextVerify, perm);
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


/*void ConfigManager::setSingleOption(const wstring& folderPath, const vector<wstring>& name, const std::wstring& value, bool restricted, bool allowDLL)
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
*/



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
