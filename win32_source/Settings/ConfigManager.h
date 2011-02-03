/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once

#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
#include <limits>
#include <functional>
#include <locale>
#include <stdexcept>

#include "Json CPP/value.h"

#include "Settings/WZFactory.h"
#include "Settings/RuntimeConfig.h"
#include "Settings/ConfigTreeWalker.h"
#include "Settings/ConfigTreeContainers.h"
#include "Settings/Types.h"
#include "Settings/CfgPerm.h"
#include "Settings/TransformNode.h"
#include "Settings/StringNode.h"
#include "Settings/JsonFile.h"
#include "NGram/wz_utilities.h"
#include "NGram/BurglishBuilder.h"



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

	//Means of building up a configuration, and then gettings its "Config" root node
	void mergeInConfigFile(const std::string& cfgFile, const CfgPerm& perms, bool fileIsStream=false, std::function<void (const StringNode& n)> OnSetCallback=std::function<void (const StringNode& n)>(), std::function<void (const std::wstring& k)> OnError=std::function<void (const std::wstring& k)>());
	const ConfigRoot& sealConfig(std::function<void (const std::wstring& k)> OnError=std::function<void (const std::wstring& k)>());
	static void OverrideSingleSetting(RuntimeConfig& currConfig, const std::wstring& name, const std::wstring& value);

	//Static helpers for loading/saving the "automated" config files.
	static void SaveLocalConfigFile(const std::wstring& path, const std::map<std::wstring, std::wstring>& properties=std::map<std::wstring, std::wstring>());
	static void SaveUserConfigFile(const std::wstring& path);


private:
	//Internal methods used for parsing a javascript tree and merging it into the config tree piece-wise.
	//  The first method calls the second two, and manages error messages automatically.
	static void BuildAndWalkConfigTree(const JsonFile& file, StringNode& rootNode, GhostNode& rootTNode, const TransformNode& rootVerifyNode, const CfgPerm& perm, std::function<void (const StringNode& n)> OnSetCallback=std::function<void (const StringNode& n)>(), std::function<void (const std::wstring& k)> OnError=std::function<void (const std::wstring& k)>());
	static void BuildUpConfigTree(const Json::Value& root, StringNode& currNode, const std::wstring& currDirPath, std::function<void (const StringNode& n)> OnSetCallback);
	static void WalkConfigTree(StringNode& source, GhostNode& dest, const TransformNode& verify, const CfgPerm& perm);

	//Internal storage data.
	StringNode root;
	ConfigRoot troot;
	bool sealed;

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

