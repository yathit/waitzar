/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "RuntimeConfig.h"

using std::vector;
using std::map;
using std::wstring;



/**
 * Load
 */
RuntimeConfig::RuntimeConfig(const ConfigRoot& config, const map<wstring, wstring>& localOpts)
{
	//Do nothing if no languages
	if (config.languages.empty())
		return;

	//Full-copy of the config tree; this allows us to delete ConfigManager without worry of corrupted references.
	this->config = config;

	//Prepare the caches with the set of "valid" languages
	for (auto it=config.languages.begin(); it!=config.languages.end(); it++) {
		cachedInputs[it->first]; //Initialize
		cachedDisplays[it->first];
		cachedEncodings[it->first];
		cachedTransformations[it->first];
	}

	//Now (optional), initialize all relevant caches completely
	getSettings();
	getExtensions();
	for (auto it=config.languages.begin(); it!=config.languages.end(); it++) {
		activeLanguage = it->first;
		getEncodings();
		getInputMethods();
		getDisplayMethods();
		for (auto trIt=it->second.transformations.begin(); trIt!=it->second.transformations.end(); trIt++) {
			getTransformation(activeLanguage, trIt->second.fromEncoding, trIt->second.toEncoding);
		}
	}
}


const SettingsNode& RuntimeConfig::getSettings()
{
	return config.settings;
}

const vector<ExtendNode>& RuntimeConfig::getExtensions()
{
	//Rebuild
	if (cachedExtensions.empty()) {
		std::transform(config.extensions.begin(), config.extensions.end(),
		    std::back_inserter(cachedExtensions), [](const map<wstring, ExtendNode>::value_type &pair) {
				return pair.second;
			});
	}

	//Return
	return cachedExtensions;
}


const vector<LangNode>& RuntimeConfig::getLanguages()
{
	//Rebuild
	if (cachedLangauges.empty()) {
		std::transform(config.languages.begin(), config.languages.end(),
		    std::back_inserter(cachedLangauges), [](const map<wstring, LangNode>::value_type &pair) {
				return pair.second;
			});
	}

	//Return
	return cachedLangauges;
}

const vector<InMethNode>& RuntimeConfig::getInputMethods()
{
	//Retrieve
	auto imMap = cachedInputs.find(activeLanguage);
	auto inMethMap = config.languages.find(activeLanguage);
	if (imMap==cachedInputs.end() || inMethMap==config.languages.end())
		throw std::runtime_error(waitzar::glue(L"Language Input Methods not mapped: ", activeLanguage).c_str());

	//Rebuild
	if (imMap->second.empty()) {
		std::transform(inMethMap->second.inputMethods.begin(), inMethMap->second.inputMethods.end(),
		    std::back_inserter(imMap->second), [](const map<wstring, InMethNode>::value_type &pair) {
				return pair.second;
			});
	}

	//Return
	return imMap->second;
}

const vector<DispMethNode>& RuntimeConfig::getDisplayMethods()
{
	//Retrieve
	auto dmMap = cachedDisplays.find(activeLanguage);
	auto langMap = config.languages.find(activeLanguage);
	if (dmMap==cachedDisplays.end() || langMap==config.languages.end())
		throw std::runtime_error(waitzar::glue(L"Language Display Methods not mapped: ", activeLanguage).c_str());

	//Rebuild
	if (dmMap->second.empty()) {
		std::transform(langMap->second.displayMethods.begin(), langMap->second.displayMethods.end(),
		    std::back_inserter(dmMap->second), [](const map<wstring, DispMethNode>::value_type &pair) {
				return pair.second;
			});
	}

	//Return
	return dmMap->second;
}


const vector<EncNode>& RuntimeConfig::getEncodings()
{
	//Retrieve
	auto enMap = cachedEncodings.find(activeLanguage);
	auto langIt = config.languages.find(activeLanguage);
	if (enMap==cachedEncodings.end() || langIt==config.languages.end())
		throw std::runtime_error(waitzar::glue(L"Language Encodings not mapped: ", activeLanguage).c_str());

	//Rebuild
	if (enMap->second.empty()) {
		std::transform(langIt->second.encodings.begin(), langIt->second.encodings.end(),
		    std::back_inserter(enMap->second), [](const map<wstring, EncNode>::value_type &pair) {
				return pair.second;
			});
	}

	//Return
	return enMap->second;
}


const TransNode& RuntimeConfig::getTransformation(const wstring& lang, wstring fromEnc, wstring toEnc)
{
	//self2self is _always_ uni2uni
	if (fromEnc == toEnc) {
		fromEnc = L"unicode";
		toEnc = L"unicode";
	}

	//Retrieve
	auto transIt = cachedTransformations.find(activeLanguage);
	auto langIt = config.languages.find(activeLanguage);
	if (transIt==cachedTransformations.end() || langIt==config.languages.end())
		throw std::runtime_error(waitzar::glue(L"Language Transformations not mapped: ", activeLanguage).c_str());

	//Rebuild
	if (transIt->second.empty()) {
		for (auto it=langIt->second.transformations.begin(); it!=langIt->second.transformations.end(); it++) {
			auto key = std::make_pair(it->second.fromEncoding, it->second.toEncoding);
			auto existingEntry = transIt->second.find(key);
			if (existingEntry==transIt->second.end())
				transIt->second[key] = it->second;
			else {
				//A key already exists; can we override it?
				if (!existingEntry->second.hasPriority && it->second.hasPriority)
					transIt->second[key] = it->second;
				else
					throw std::runtime_error(waitzar::glue(L"Cannot add new Transformation (", it->first, L") without priority over one with priority: ", existingEntry->second.id).c_str());
			}
		}
	}

	//Retrieve again
	auto finalIt = transIt->second.find(std::make_pair(fromEnc, toEnc));
	if (finalIt==transIt->second.end())
		throw std::runtime_error(waitzar::glue(L"Error! An un-validated transformation exists in the configuration model: ", fromEnc, L"->", toEnc).c_str());

	//Return
	return finalIt->second;
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



