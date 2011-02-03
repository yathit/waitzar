/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once

#include <map>
#include <vector>
#include <string>
#include <stdexcept>

#include "Settings/ConfigTreeContainers.h"
#include "NGram/wz_utilities.h"


/**
 * This class provides all of the Runtime information required by the Main code.
 *   It wraps a TNode which is loaded by ConfigManager, which allows us to
 *   separate loading from running.
 */
class RuntimeConfig {
public:
	RuntimeConfig(const ConfigRoot& config=ConfigRoot());

	//Actual functions
	const SettingsNode& getSettings();
	const std::vector<ExtendNode>& getExtensions();
	const std::vector<LangNode>& getLanguages();

	//Language-specific variants
	const std::vector<InMethNode>& getInputMethods(const std::wstring& langID);
	const std::vector<DispMethNode>& getDisplayMethods(const std::wstring& langID);
	const std::vector<EncNode>& getEncodings(const std::wstring& langID);
	const TransNode& getTransformation(const std::wstring& langID, std::wstring fromEnc, std::wstring toEnc);

	//"Active" variants
	const std::vector<InMethNode>& getActiveInputMethods() { return getInputMethods(activeLanguage); }
	const std::vector<DispMethNode>& getActiveDisplayMethods() { return getDisplayMethods(activeLanguage); }
	const std::vector<EncNode>& getActiveEncodings() { return getEncodings(activeLanguage); }
	const TransNode& getActiveTransformation(std::wstring fromEnc, std::wstring toEnc) { return getTransformation(activeLanguage, fromEnc, toEnc); }

	//Directs
	const LangNode& getLanguage(const std::wstring& langID);
	const InMethNode& getInputMethod(const std::wstring& langID, const std::wstring& inmethID);
	std::pair<DispMethNode, DispMethNode> getDisplayMethodPair(const std::wstring& langID, const std::wstring& dispmeth1, const std::wstring& dispmeth2);
	const EncNode& getEncoding(const std::wstring& langID, const std::wstring& encID);

	//"Active" directs
	const LangNode& getActiveLanguage() { return getLanguage(activeLanguage); }
	const InMethNode& getActiveInputMethod() { return getInputMethod(activeLanguage, activeInputMethod); }
	std::pair<DispMethNode, DispMethNode> getActiveDisplayMethodPair() { return getDisplayMethodPair(activeLanguage, activeDisplayMethod.first, activeDisplayMethod.second); }
	const EncNode& getActiveOutputEncoding() { return getEncoding(activeLanguage, activeOutputEncoding); }

	//Finally, setters
	void setActiveLanguage(const std::wstring& id);
	void setActiveInputMethod(const std::wstring& id);
	void setActiveOutputEncoding(const std::wstring& id);


private:
	//Stored data
	ConfigRoot config;

	//Control what "active" means
	std::wstring activeLanguage;
	std::wstring activeOutputEncoding;
	std::wstring activeInputMethod;
	std::pair<std::wstring, std::wstring> activeDisplayMethod;  //Normal, small

	//Cached values
	std::vector<ExtendNode> cachedExtensions;
	std::vector<LangNode> cachedLangauges;
	std::map<std::wstring, std::vector<InMethNode>> cachedInputs;
	std::map<std::wstring, std::vector<DispMethNode>> cachedDisplays;
	std::map<std::wstring, std::vector<EncNode>> cachedEncodings;
	std::map<std::wstring, std::map<std::pair<std::wstring, std::wstring>, TransNode>> cachedTransformations;

	//ConfigManager is everyone's friend! But no-one invites him over for Christmas. :(
	friend class ConfigManager;


	//
	// TODO: Currently un-implemented!
	//
public:
	//void overrideSetting(const std::wstring& settingName, bool value) {}


	// (We might put the "configOpts" stuff into its own class, like "LocalConfigOpts".
	//  It really doesn't even have to be in RuntimeConfig)
	//void backupLocalConfigOpts() {}
	//void restoreLocalConfigOpts() {}
	//std::wstring getLocalConfigOpt(const std::wstring& key) {return L"";}
	//void clearLocalConfigOpt(const std::wstring& key) {}
	//void setLocalConfigOpt(const std::wstring& key, const std::wstring& val) {}
	//void saveLocalConfigFile(const std::wstring& path, bool emptyFile) {}
	//void saveUserConfigFile(const std::wstring& path, bool emptyFile) {}


	//Not sure what to do about this...
	//bool localConfigCausedError() {return false;}
};



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


