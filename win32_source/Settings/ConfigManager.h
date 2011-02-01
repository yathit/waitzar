/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _CONFIGMANAGER
#define _CONFIGMANAGER

#include "windows_wz.h"

#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
#include <limits>
#include <functional>
#include <locale>
#include <stdexcept>

#include "Json CPP/value.h"
#include "Json CPP/reader.h"

#include "Settings/ConfigTreeContainers.h"
#include "Settings/CfgPerm.h"
#include "Settings/TransformNode.h"
#include "Settings/WZFactory.h"
#include "Settings/Language.h"
#include "Settings/Node.h"
#include "Settings/JsonFile.h"
#include "NGram/BurglishBuilder.h"
#include "Input/InputMethod.h"
#include "Extension/Extension.h"
#include "Extension/JavaScriptConverter.h"
#include "NGram/wz_utilities.h"
#include "Settings/HotkeyData.h"
#include "Settings/RuntimeConfig.h"




//Options for our ConfigManager class
struct Settings {
	//Simple
	std::wstring hotkeyStrRaw;
	bool silenceMywordsErrors;
	bool balloonStart;
	bool alwaysElevate;
	bool trackCaret;
	bool lockWindows;
	bool suppressVirtualKeyboard;
	std::wstring defaultLanguage;
	std::wstring whitespaceCharacters;
	std::wstring ignoredCharacters;
	bool hideWhitespaceMarkings;

	//Derived
	HotkeyData hotkey;
};
struct OptionTree {
	Settings settings;
	std::set<Language> languages;
	std::set<Extension*> extensions;
};



/**
 * This class exists for managing configuration options automatically. It is also
 *  the main access point for WaitZar into the SpiritJSON library, without requiring 
 *  us to link against Boost.
 */
class ConfigManager
{
public:
	ConfigManager() {
		//Once sealed, you can't load any more files.
		this->sealed = false;
	}

	//Build our config. manager up slowly
	//void initMainConfig(const std::string& configFile, bool fileIsStream=false);
	//void initCommonConfig(const std::string& configFile);
	//void initMainConfig(const std::wstring& configStream);
	//void initAddLanguage(const std::string& configFile, const std::vector<std::string>& subConfigFiles);
	//void initLocalConfig(const std::string& configFile);
	//void initUserConfig(const std::string& configFile);


	//Accessible by our outside class
	/*const Settings& getSettings();
	const std::set<Extension*>& getExtensions();
	const std::set<Language>& getLanguages();
	const std::set<InputMethod*>& getInputMethods();
	const std::set<Encoding>& getEncodings();
	const Transformation* getTransformation(const Language& lang, const Encoding& fromEnc, const Encoding& toEnc) const;
	const std::set<DisplayMethod*>& getDisplayMethods();*/
	void overrideSetting(const std::wstring& settingName, bool value);

	//Helpful
	std::wstring getLocalConfigOpt(const std::wstring& key);
	void clearLocalConfigOpt(const std::wstring& key);
	void setLocalConfigOpt(const std::wstring& key, const std::wstring& val);
	void saveLocalConfigFile(const std::wstring& path, bool emptyFile);
	void saveUserConfigFile(const std::wstring& path, bool emptyFile);
	bool localConfigCausedError();
	void backupLocalConfigOpts();
	void restoreLocalConfigOpts();

	//Control
	Language activeLanguage;
	Encoding activeOutputEncoding;
	InputMethod* activeInputMethod;
	std::vector<DisplayMethod*> activeDisplayMethods; //Normal, small
	Encoding unicodeEncoding;

	//Quality control
	void validate(HINSTANCE& hInst, MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, OnscreenKeyboard* helpKeyboard, const std::map<std::wstring, std::vector<std::wstring>>& lastUsedSettings);

	//Useful
	//bool IsProbablyFile(const std::wstring& str);
	//std::wstring purge_filename(const std::wstring& str);
	//std::wstring sanitize_id(const std::wstring& str);
	//std::wstring sanitize_value(const std::wstring& str, const std::wstring& filePath);
	//std::vector<std::wstring> separate(std::wstring str, wchar_t delim);
	//bool read_bool(const std::wstring& str);
	//int read_int(const std::wstring& str);

	//Kind of out of place, but it works
	//void generateHotkeyValues(const wstring& srcStr, HotkeyData& hkData);


public:
	void mergeInConfigFile(const std::string& cfgFile, const CfgPerm& perms, bool fileIsStream=false, std::function<void (const Node& n)> OnSetCallback=std::function<void (const Node& n)>());
	const ConfigRoot& sealConfig();

private:
	//These two functions replace "readInConfig"
	void buildAndWalkConfigTree(const JsonFile& file, Node& rootNode, TNode& rootTNode, const TransformNode& rootVerifyNode, const CfgPerm& perm=CfgPerm(), std::function<void (const Node& n)> OnSetCallback=std::function<void (const Node& n)>());
	void buildUpConfigTree(const Json::Value& root, Node& currNode, const std::wstring& currDirPath, std::function<void (const Node& n)> OnSetCallback);
	void walkConfigTree(Node& source, TNode& dest, const TransformNode& verify, const CfgPerm& perm);
	void buildVerifyTree();
	//std::map<std::wstring, std::wstring> locallySetOptions; //TODO: Replace later
	Node root;
	ConfigRoot troot;
	TransformNode verifyTree;
	bool sealed;


	template <typename T>
	static T& AddOrCh(std::map<std::wstring, T>& existing, const Node& node, bool addAllowed, bool chgAllowed);


	void readInConfig(const Json::Value& root, const std::wstring& folderPath, std::vector<std::wstring> &context, bool restricted, bool allowDLL, std::map<std::wstring, std::wstring>* const optionsSet);
	void setSingleOption(const std::wstring& folderPath, const std::vector<std::wstring>& name, const std::wstring& value, bool restricted, bool allowDLL);

	//void resolvePartialSettings();
	//void generateInputsDisplaysOutputs(const std::map<std::wstring, std::vector<std::wstring> >& lastUsedSettings);

private:
	//Our many config files.
	JsonFile mainConfig;
	JsonFile commonConfig;
	std::map<JsonFile , std::vector<JsonFile> > langConfigs;
	JsonFile localConfig;
	JsonFile userConfig;

	//Workaround
	//std::string (*getMD5Function)(const std::string&);


	//Current working directory
	//std::wstring workingDir;

	//Have we loaded...?
	/*bool loadedSettings;
	bool loadedLanguageMainFiles;
	bool loadedLanguageSubFiles;*/

	//Functions for loading some of these.
	//void loadLanguageMainFiles();
	//void loadLanguageSubFiles();

	//Temporary option caches for constructing complex structures
	//Will eventually be converted into real InputManager*, etc.
	//Store as <lang_name,item_name>, for fast lookup.
	std::map<std::pair<std::wstring,std::wstring>, std::map<std::wstring, std::wstring> > partialInputMethods;
	std::map<std::pair<std::wstring,std::wstring>, std::map<std::wstring, std::wstring> > partialEncodings;
	std::map<std::pair<std::wstring,std::wstring>, std::map<std::wstring, std::wstring> > partialTransformations;
	std::map<std::pair<std::wstring,std::wstring>, std::map<std::wstring, std::wstring> > partialDisplayMethods;

	//The actual representation
	OptionTree options;
	std::map<std::wstring, std::wstring> localOpts;
	std::map<std::wstring, std::wstring> localOptsBackup;
	bool localConfError;

	//Cache
	Transformation* self2self;
};



//Find and return an item in a Set, indxed only by its wstring
//   * This is not needed for value-type sets, since we can just construct temporaries
//   * We don't need a non-const version, because set iterators are always const
template <class T> 
typename std::set<T*>::const_iterator FindKeyInSet(const std::set<T*>& container, const std::wstring& key)
{
	typename std::set<T*>::const_iterator it=container.begin();
	for (; it!=container.end(); it++)  {
		if ((*(*it)) == key)
			break;
	}
	return it;
}




#endif //_CONFIGMANAGER

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

