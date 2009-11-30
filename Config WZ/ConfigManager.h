/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _CONFIGMANAGER
#define _CONFIGMANAGER

#include <vector>
#include <algorithm>
#include <limits>
#include <functional>
#include "wz_utilities.h"
#include "json_spirit.h"
#include "Interfaces.h"


//Simple class to help us load json files easier
class JsonFile {
public:
	JsonFile(const std::string& path="")
	{
		this->path = path;
	}
	json_spirit::wValue json() const
	{
		if (!hasParsed) {
			const std::wstring text = waitzar::readUTF8File(path);
			json_spirit::read(text, root);
			hasParsed = true;
		}
		return root;
	}
	std::string getPath() const
	{
		return this->path;
	}
	//For map indexing:
	bool operator<(const JsonFile& j) const
	{
		return this->path < j.path;
	}
private:
	std::string path;
	mutable json_spirit::wValue root;
	mutable bool hasParsed;
};





//Options for our ConfigManager class
struct Settings {
	Option<std::wstring> hotkey;
	Option<bool> silenceMywordsErrors;
	Option<bool> balloonStart;
	Option<bool> alwaysElevate;
	Option<bool> trackCaret;
	Option<bool> lockWindows;
};
struct Language {
	Option<std::wstring> displayName;
	Option<std::wstring> defaultOutputEncoding;
	Option<std::wstring> defaultDisplayMethod;
	Option<std::wstring> defaultInputMethod;
	std::map<std::wstring, InputMethod*> inputMethods;
	std::map<std::wstring, Encoding> encodings;
	std::map<std::wstring, Transformation*> transformations;
	std::map<std::wstring, DisplayMethod*> displayMethods;
};
struct OptionTree {
	Settings settings;
	std::map<std::wstring, Language> languages;
};


//Utility enum
enum WRITE_OPTS {WRITE_MAIN, WRITE_LOCAL, WRITE_USER};


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
	void initAddLanguage(const std::string& configFile, const std::vector<std::string>& subConfigFiles);
	void initLocalConfig(const std::string& configFile);
	void initUserConfig(const std::string& configFile);

	//Accessible by our outside class
	Settings getSettings();
	std::vector<std::wstring> getLanguages() const;
	std::vector<std::wstring> getInputManagers() const;
	std::vector<std::wstring> getEncodings() const;

	//Control
	std::wstring getActiveLanguage() const;
	void changeActiveLanguage(const std::wstring& newLanguage);
	std::wstring loc_to_lower(const std::wstring& str);


private:
	void readInConfig(json_spirit::wValue root, std::wstring context, WRITE_OPTS writeTo);
	std::wstring sanitize_id(const std::wstring& str);
	std::wstring sanitize(const std::wstring& str);
	std::string escape_wstr(const std::wstring& str) const;
	bool read_bool(const std::wstring& str);
	void setSingleOption(const std::wstring& name, const std::wstring& value);

private:
	//Our many config files.
	JsonFile mainConfig;
	std::map<JsonFile , std::vector<JsonFile> > langConfigs;
	JsonFile localConfig;
	JsonFile userConfig;

	//The actual representation
	OptionTree options;
};



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

