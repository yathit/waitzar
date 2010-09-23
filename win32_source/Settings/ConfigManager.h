/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _CONFIGMANAGER
#define _CONFIGMANAGER

#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
#include <limits>
#include <functional>
#include <locale>

#include "Json Spirit/json_spirit_value.h"
#include "Settings/WZFactory.h"
#include "Settings/Language.h"
#include "NGram/BurglishBuilder.h"
#include "Input/InputMethod.h"


//json_spirit_reader is defined elsewhere
/*namespace json_spirit {
	extern bool read( const std::wstring& s, wValue& value );
	//TODO: Need to add a few more if we want it this way...
}*/
#include "Json Spirit/json_spirit_reader.h"


//Lastly, to avoid LO/HI byte re-definition.
#include "NGram/wz_utilities.h"


//Simple class to help us load json files easier
class JsonFile {
public:
	JsonFile(const std::string& path="") //Confusing, I know. (TODO: Make a better way of loading a file OR a string)
	{
		this->path = path;
		this->folderPath = L"";
		this->text = L"";
		this->hasReadFile = false;
		this->hasParsed = false;

		//Set the folder path
		int fwIndex = path.rfind("/");
		int bwIndex = path.rfind("\\");
		int index = std::max<int>(fwIndex, bwIndex);
		if (index!=-1) {
			std::wstringstream temp;
			temp <<path.substr(0, index+1).c_str();
			folderPath = temp.str();
		}
	}
	JsonFile(const std::wstring& text) //Confusing, I know.
	{
		this->path = "";
		this->folderPath = L"";
		this->text = waitzar::preparse_json(text);
		this->hasReadFile = true;
		this->hasParsed = false;
	}
	json_spirit::wValue json() const
	{
		if (!hasParsed) {
			if (!this->hasReadFile) {
				text = waitzar::readUTF8File(path);
				text = waitzar::preparse_json(text);
				this->hasReadFile = true;
			}

			//First, try to just read it. If there's an error, then try "read or throw" and get a better error message.
			if (!json_spirit::read(text, root)) {
				try {
					json_spirit::read_or_throw(text, root);	
				} catch (json_spirit::Error_position ex) {
					//First, try to build a representative line of text (+/-8 chars)
					std::wstring segment;
					if (text.length()>0) {
						int amt = 8;
						int startID = std::max<int>(0, ex.column_ - amt);
						int endID = std::min<int>(text.length()-1, ex.column_ + amt);
						segment = text.substr(startID, endID-startID);
					}

					//Now, throw the error.
					std::stringstream errMsg;
					errMsg << "Invalid json config file: " << path;
					errMsg << std::endl << "  Problem: " << ex.reason_;
					errMsg << std::endl << "  Surrounding Text: ";
					errMsg << std::endl << "      " <<"..." << waitzar::escape_wstr(segment, false) <<"...";
					throw std::exception(errMsg.str().c_str());
				}
			}

			//Save space
			text = L"";

			hasParsed = true;
		}
		return root;
	}
	bool isEmpty() const
	{
		return this->path.empty() && this->text.empty();
	}
	bool isSet() const //Should be a better way of automating this... maybe a singleton JSON object to return by default?
	{
		return this->path.length() > 0;
	}
	const std::wstring& getFolderPath() const
	{
		return this->folderPath;
	}
	//For map indexing:
	bool operator<(const JsonFile& j) const
	{
		return this->path < j.path;
	}
private:
	std::string path;
	std::wstring folderPath;
	mutable std::wstring text;
	mutable json_spirit::wValue root;
	mutable bool hasReadFile;
	mutable bool hasParsed;
};

//Hotkey wrapper
struct HotkeyData {
	wstring hotkeyStrFormatted;
	int hotkeyID;
	unsigned int hkModifiers;
	unsigned int hkVirtKeyCode;
};

//Options for our ConfigManager class
struct Settings {
	//Simple
	std::wstring hotkeyStrRaw;
	bool silenceMywordsErrors;
	bool balloonStart;
	bool alwaysElevate;
	bool trackCaret;
	bool lockWindows;
	std::wstring defaultLanguage;

	//Derived
	HotkeyData hotkey;
};
struct OptionTree {
	Settings settings;
	std::set<Language> languages;
};



/**
 * This class exists for managing configuration options automatically. It is also
 *  the main access point for WaitZar into the SpiritJSON library, without requiring 
 *  us to link against Boost.
 */
class ConfigManager
{
public:
	ConfigManager(std::string (*myMD5Function)(const std::string&));
	~ConfigManager(void);

	//Build our config. manager up slowly
	void initMainConfig(const std::string& configFile);
	void initMainConfig(const std::wstring& configStream);
	void initAddLanguage(const std::string& configFile, const std::vector<std::string>& subConfigFiles);
	void initLocalConfig(const std::string& configFile);
	void initUserConfig(const std::string& configFile);


	//Accessible by our outside class
	const Settings& getSettings();
	const std::set<Language>& getLanguages();
	const std::set<InputMethod*>& getInputMethods();
	const std::set<Encoding>& getEncodings();
	const Transformation* getTransformation(const Language& lang, const Encoding& fromEnc, const Encoding& toEnc) const;
	const std::set<DisplayMethod*>& getDisplayMethods();
	void overrideSetting(const std::wstring& settingName, bool value);

	//Helpful
	std::wstring getLocalConfigOpt(const std::wstring& key);
	void clearLocalConfigOpt(const std::wstring& key);
	void setLocalConfigOpt(const std::wstring& key, const std::wstring& val);
	void saveLocalConfigFile(const std::wstring& path, bool emptyFile);
	bool localConfigCausedError();

	//Control
	Language activeLanguage;
	Encoding activeOutputEncoding;
	InputMethod* activeInputMethod;
	std::vector<DisplayMethod*> activeDisplayMethods; //Normal, small
	Encoding unicodeEncoding;

	//Quality control
	void validate(HINSTANCE& hInst, MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, OnscreenKeyboard* helpKeyboard, const map<wstring, vector<wstring> >& lastUsedSettings);

	//Useful
	static std::wstring sanitize_id(const std::wstring& str);
	static std::wstring sanitize(const std::wstring& str);
	static std::vector<std::wstring> separate(std::wstring str, wchar_t delim);
	static bool read_bool(const std::wstring& str);
	static int read_int(const std::wstring& str);


private:
	void readInConfig(json_spirit::wValue root, const std::wstring& folderPath, std::vector<std::wstring> &context, bool restricted, map<wstring, wstring>* const optionsSet);
	void setSingleOption(const std::wstring& folderPath, const std::vector<std::wstring>& name, const std::wstring& value, bool restricted);

	void resolvePartialSettings();
	void generateInputsDisplaysOutputs(const map<wstring, vector<wstring> >& lastUsedSettings);
	void generateHotkeyValues();

private:
	//Our many config files.
	JsonFile mainConfig;
	std::map<JsonFile , std::vector<JsonFile> > langConfigs;
	JsonFile localConfig;
	JsonFile userConfig;

	//Workaround
	std::string (*getMD5Function)(const std::string&);

	//Have we loaded...?
	bool loadedSettings;
	bool loadedLanguageMainFiles;
	bool loadedLanguageSubFiles;

	//Functions for loading some of these.
	void loadLanguageMainFiles();
	void loadLanguageSubFiles();

	//Temporary option caches for constructing complex structures
	//Will eventually be converted into real InputManager*, etc.
	//Store as <lang_name,item_name>, for fast lookup.
	std::map<std::pair<std::wstring,std::wstring>, std::map<std::wstring, std::wstring> > partialInputMethods;
	std::map<std::pair<std::wstring,std::wstring>, std::map<std::wstring, std::wstring> > partialEncodings;
	std::map<std::pair<std::wstring,std::wstring>, std::map<std::wstring, std::wstring> > partialTransformations;
	std::map<std::pair<std::wstring,std::wstring>, std::map<std::wstring, std::wstring> > partialDisplayMethods;

	//The actual representation
	OptionTree options;
	map<wstring, wstring> localOpts;
	bool localConfError;

	//Cache
	Transformation* self2self;
};



//Find and return an item in a Set, indxed only by its wstring
template <class T> 
typename std::set<T>::iterator FindKeyInSet(std::set<T>& container, const std::wstring& key)
{
	std::set<T>::iterator it=container.begin();
	for (; it!=container.end(); it++)  {
		if ((*it) == key)
			break;
	}
	return it;
}

//Same, but for const
template <class T> 
typename std::set<T>::const_iterator FindKeyInSet(const std::set<T>& container, const std::wstring& key)
{
	std::set<T>::const_iterator it=container.begin();
	for (; it!=container.end(); it++)  {
		if ((*it) == key)
			break;
	}
	return it;
}

//Same, but for a set of references.
template <class T> 
typename std::set<T*>::iterator FindKeyInSet(std::set<T*>& container, const std::wstring& key)
{
	std::set<T*>::iterator it=container.begin();
	for (; it!=container.end(); it++)  {
		if ((*(*it)) == key)
			break;
	}
	return it;
}

//Ugh, now THIS as const...
template <class T> 
typename std::set<T*>::const_iterator FindKeyInSet(const std::set<T*>& container, const std::wstring& key)
{
	std::set<T*>::const_iterator it=container.begin();
	for (; it!=container.end(); it++)  {
		if ((*(*it)) == key)
			break;
	}
	return it;
}



//Helper predicate
template <class T> 
class is_id_delim : public std::unary_function<T, bool>
{
public:
 bool operator ()(T t) const
 {
  if ((t==' ')||(t=='\t')||(t=='\n')||(t=='-')||(t=='_'))
   return true; //Remove it
  return false; //Don't remove it
 }
};







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

