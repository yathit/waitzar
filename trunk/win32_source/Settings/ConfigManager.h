/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _CONFIGMANAGER
#define _CONFIGMANAGER

#include <vector>
#include <set>
#include <algorithm>
#include <limits>
#include <functional>
#include <locale>
#include "Json Spirit/json_spirit_value.h"
#include "Settings/WZFactory.h"
#include "Settings/Language.h"
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
		this->text = L"";
		this->hasReadFile = false;
		this->hasParsed = false;
	}
	JsonFile(const std::wstring& text) //Confusing, I know.
	{
		this->path = "";
		this->text = text;
		this->hasReadFile = true;
		this->hasParsed = false;
	}
	json_spirit::wValue json() const
	{
		if (!hasParsed) {
			if (!this->hasReadFile) {
				text = waitzar::readUTF8File(path);
				this->hasReadFile = true;
			}
			try {
				json_spirit::read_or_throw(text, root);

				//Save space
				text = L"";
			} catch (json_spirit::Error_position ex) {
				std::stringstream errMsg;
				errMsg << "Invalid json config file: " << path;
				errMsg << std::endl << "  Problem: " << ex.reason_;
				errMsg << std::endl << "    on line: " << ex.line_;
				errMsg << std::endl << "    at column: " << ex.column_;
				throw std::exception(errMsg.str().c_str());
			}

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
	//For map indexing:
	bool operator<(const JsonFile& j) const
	{
		return this->path < j.path;
	}
private:
	std::string path;
	mutable std::wstring text;
	mutable json_spirit::wValue root;
	mutable bool hasReadFile;
	mutable bool hasParsed;
};


//Options for our ConfigManager class
struct Settings {
	//Simple
	std::wstring hotkey;
	bool silenceMywordsErrors;
	bool balloonStart;
	bool alwaysElevate;
	bool trackCaret;
	bool lockWindows;
	std::wstring defaultLanguage;
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
	ConfigManager(void);
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

	//Control
	Language activeLanguage;
	Encoding activeOutputEncoding;
	InputMethod* activeInputMethod;
	DisplayMethod* activeDisplayMethod;
	Encoding unicodeEncoding;

	//Quality control
	void validate(HINSTANCE& hInst, MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, OnscreenKeyboard* helpKeyboard);

	//Useful
	static std::wstring sanitize_id(const std::wstring& str);
	static std::wstring sanitize(const std::wstring& str);
	static void loc_to_lower(std::wstring& str);
	static std::vector<std::wstring> separate(std::wstring str, wchar_t delim);
	static bool read_bool(const std::wstring& str);

	//For stl exceptions...
	static std::string glue(const std::string& str1, const std::string& str2, const std::string& str3, const std::string& str4)
	{
		std::stringstream msg;
		msg <<str1 <<str2 <<str3 <<str4;
		return msg.str();
	}
	static std::string glue(const std::string& str1, const std::string& str2, const std::string& str3)
	{
		return glue(str1, str2, str3, "");
	}
	static std::string glue(const std::string& str1, const std::string& str2)
	{
		return glue(str1, str2, "", "");
	}
	static std::string glue(const std::wstring& str1, const std::wstring& str2, const std::wstring& str3, const std::wstring& str4)
	{
		return glue(waitzar::escape_wstr(str1, false), waitzar::escape_wstr(str2, false), waitzar::escape_wstr(str3, false), waitzar::escape_wstr(str4, false));
	}
	static std::string glue(const std::wstring& str1, const std::wstring& str2, const std::wstring& str3)
	{
		return glue(str1, str2, str3, L"");
	}
	static std::string glue(const std::wstring& str1, const std::wstring& str2)
	{
		return glue(str1, str2, L"", L"");
	}



private:
	void readInConfig(json_spirit::wValue root, std::vector<std::wstring> &context, bool restricted);
	void setSingleOption(const std::vector<std::wstring>& name, const std::wstring& value, bool restricted);

	void resolvePartialSettings();
	void generateInputsDisplaysOutputs();

private:
	//Our many config files.
	JsonFile mainConfig;
	std::map<JsonFile , std::vector<JsonFile> > langConfigs;
	JsonFile localConfig;
	JsonFile userConfig;

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


//And finally, locale-driven nonsense with to_lower:
template<class T>
class ToLower {
public:
     ToLower(const std::locale& loc):loc(loc)
     {
     }
     T operator()(const T& src) const
     {
          return std::tolower<T>(src, loc);
     }
protected:
     const std::locale& loc;
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

