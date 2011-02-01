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
		getEncodings(it->first);
		getInputMethods(it->first);
		getDisplayMethods(it->first);
		for (auto trIt=it->second.transformations.begin(); trIt!=it->second.transformations.end(); trIt++) {
			getTransformation(it->first, trIt->second.fromEncoding, trIt->second.toEncoding);
		}
	}

	//Finally, set all strings
	setActiveLanguage(config.settings.defaultLanguage);
}


void RuntimeConfig::setActiveLanguage(const std::wstring& id)
{
	if (id.empty())
		return;


	//Changing the language changes just about everything.
	activeLanguage = id;
	activeDisplayMethod = {
			getActiveLanguage().defaultDisplayMethodReg,
			getActiveLanguage().defaultDisplayMethodSmall,
	};
	setActiveInputMethod(getActiveLanguage().defaultInputMethod);
	setActiveOutputEncoding(getActiveLanguage().defaultOutputEncoding);
}


void RuntimeConfig::setActiveInputMethod(const std::wstring& id)
{
	if (id.empty())
		return;

	activeInputMethod = getInputMethod(activeLanguage, id).id;
}

void RuntimeConfig::setActiveOutputEncoding(const std::wstring& id)
{
	if (id.empty())
		return;

	activeOutputEncoding = getEncoding(activeLanguage, id).id;
}



const LangNode& RuntimeConfig::getLanguage(const std::wstring& langID)
{
	auto it = config.languages.find(langID);
	if (it==config.languages.end())
		throw std::runtime_error(waitzar::glue(L"Language is invalid: ", langID).c_str());
	return it->second;
}


const InMethNode& RuntimeConfig::getInputMethod(const std::wstring& langID, const std::wstring& inmethID)
{
	const LangNode& lang = getLanguage(langID);
	auto it = lang.inputMethods.find(inmethID);
	if (it==lang.inputMethods.end())
		throw std::runtime_error(waitzar::glue(L"Input Method is invalid: ", inmethID, L" for language: ", langID).c_str());
	return it->second;
}


std::pair<DispMethNode, DispMethNode> RuntimeConfig::getDisplayMethodPair(const std::wstring& langID, const std::wstring& dispmeth1, const std::wstring& dispmeth2)
{
	const LangNode& lang = getLanguage(langID);
	auto it1 = lang.displayMethods.find(dispmeth1);
	auto it2 = lang.displayMethods.find(dispmeth2);
	if (it1==lang.displayMethods.end())
		throw std::runtime_error(waitzar::glue(L"Display Method is invalid: ", dispmeth1, L" for language: ", langID).c_str());
	if (it2==lang.displayMethods.end())
		throw std::runtime_error(waitzar::glue(L"Display Method is invalid: ", dispmeth2, L" for language: ", langID).c_str());

	return std::pair<DispMethNode, DispMethNode>(it1->second, it2->second);
}


const EncNode& RuntimeConfig::getEncoding(const std::wstring& langID, const std::wstring& encID)
{
	const LangNode& lang = getLanguage(langID);
	auto it = lang.encodings.find(encID);
	if (it==lang.encodings.end())
		throw std::runtime_error(waitzar::glue(L"Encoding is invalid: ", encID, L" for language: ", langID).c_str());
	return it->second;
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

const vector<InMethNode>& RuntimeConfig::getInputMethods(const wstring& langID)
{
	//Retrieve
	auto imMap = cachedInputs.find(langID);
	auto inMethMap = config.languages.find(langID);
	if (imMap==cachedInputs.end() || inMethMap==config.languages.end())
		throw std::runtime_error(waitzar::glue(L"Language Input Methods not mapped: ", langID).c_str());

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

const vector<DispMethNode>& RuntimeConfig::getDisplayMethods(const wstring& langID)
{
	//Retrieve
	auto dmMap = cachedDisplays.find(langID);
	auto langMap = config.languages.find(langID);
	if (dmMap==cachedDisplays.end() || langMap==config.languages.end())
		throw std::runtime_error(waitzar::glue(L"Language Display Methods not mapped: ", langID).c_str());

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


const vector<EncNode>& RuntimeConfig::getEncodings(const wstring& langID)
{
	//Retrieve
	auto enMap = cachedEncodings.find(langID);
	auto langIt = config.languages.find(langID);
	if (enMap==cachedEncodings.end() || langIt==config.languages.end())
		throw std::runtime_error(waitzar::glue(L"Language Encodings not mapped: ", langID).c_str());

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


const TransNode& RuntimeConfig::getTransformation(const wstring& langID, wstring fromEnc, wstring toEnc)
{
	//self2self is _always_ uni2uni
	if (fromEnc == toEnc) {
		fromEnc = L"unicode";
		toEnc = L"unicode";
	}

	//Retrieve
	auto transIt = cachedTransformations.find(langID);
	auto langIt = config.languages.find(langID);
	if (transIt==cachedTransformations.end() || langIt==config.languages.end())
		throw std::runtime_error(waitzar::glue(L"Language Transformations not mapped: ", langID).c_str());

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



