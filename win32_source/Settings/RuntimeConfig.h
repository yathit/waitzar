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
	RuntimeConfig(const ConfigRoot& config=ConfigRoot(), const std::map<std::wstring, std::wstring>& localOpts=std::map<std::wstring, std::wstring>());

	//Get from active properties
	const SettingsNode& getSettings();
	const std::vector<ExtendNode>& getExtensions();
	const std::vector<LangNode>& getLanguages();
	const std::vector<InMethNode>& getInputMethods();
	const std::vector<DispMethNode>& getDisplayMethods();
	const std::vector<EncNode>& getEncodings();
	const TransNode& getTransformation(const std::wstring& lang, std::wstring fromEnc, std::wstring toEnc);

	//Helper
	void ChangeLangInputOutput(const std::wstring& langid, const std::wstring& inputid, const std::wstring& outputid);

	//Control what "active" means
	std::wstring activeLanguage;
	std::wstring activeOutputEncoding;
	std::wstring activeInputMethod;
	std::vector<std::wstring> activeDisplayMethods; //Normal, small


private:
	//Stored data
	ConfigRoot config;

	//Cached values
	std::vector<ExtendNode> cachedExtensions;
	std::vector<LangNode> cachedLangauges;
	std::map<std::wstring, std::vector<InMethNode>> cachedInputs;
	std::map<std::wstring, std::vector<DispMethNode>> cachedDisplays;
	std::map<std::wstring, std::vector<EncNode>> cachedEncodings;
	std::map<std::wstring, std::map<std::pair<std::wstring, std::wstring>, TransNode>> cachedTransformations;
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


