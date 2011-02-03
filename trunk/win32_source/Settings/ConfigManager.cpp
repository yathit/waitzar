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
