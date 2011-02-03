/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */


#include "ConfigTreeWalker.h"


//Define
TransformNode ConfigTreeWalker::verifyTree;


//Helper
template <typename T>
T& ConfigTreeWalker::AddOrCh(std::map<std::wstring, T>& existing, const StringNode& node, bool addAllowed, bool chgAllowed) {
	//Temp
	std::wstring key = node.getKeyInParentMap();

	//Can change?
	if (!chgAllowed)
		throw std::runtime_error("Can't modify existing item in this set.");

	//Add it if it doesn't exist
	if (existing.count(key)==0) {
		if (!addAllowed)
			throw std::runtime_error("Can't add a new item to this set.");
		existing[key] = T(key);
	}

	//Return it
	return existing[key];
}


//Actual implementation
void ConfigTreeWalker::buildVerifyTree()
{
	//Root nodes
	verifyTree.addChild(L"settings", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Advance
		return dynamic_cast<ConfigRoot&>(d).settings;
	});
	verifyTree.addChild(L"languages", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//We don't have the language ID, so return the same node
		return d;
	});
	verifyTree.addChild(L"extensions", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//We don't have the extension ID, so return the same node
		return d;
	});

	//Settings
	verifyTree[L"settings"].addChild(L"hotkey" , [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		dynamic_cast<SettingsNode&>(d).hotkey = HotkeyData(waitzar::purge_filename(s.str()));
		return d;
	});
	verifyTree[L"settings"].addChild(L"silence-mywords-errors" , [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		dynamic_cast<SettingsNode&>(d).silenceMywordsErrors = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"balloon-start" , [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		dynamic_cast<SettingsNode&>(d).balloonStart = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"always-elevate" , [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		dynamic_cast<SettingsNode&>(d).alwaysElevate = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"track-caret" , [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		dynamic_cast<SettingsNode&>(d).trackCaret = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"lock-windows" , [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		dynamic_cast<SettingsNode&>(d).lockWindows = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"suppress-virtual-keyboard" , [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		dynamic_cast<SettingsNode&>(d).suppressVirtualKeyboard = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"default-language" , [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		auto& set = dynamic_cast<SettingsNode&>(d);
		set.defaultLanguage = waitzar::sanitize_id(s.str());
		set.defaultLanguageStack = s.getStringStack();
		return d;
	});
	verifyTree[L"settings"].addChild(L"whitespace-characters" , [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		dynamic_cast<SettingsNode&>(d).whitespaceCharacters = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"ignored-characters" , [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		dynamic_cast<SettingsNode&>(d).ignoredCharacters = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"settings"].addChild(L"hide-whitespace-markings" , [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		dynamic_cast<SettingsNode&>(d).hideWhitespaceMarkings = waitzar::read_bool(s.str());
		return d;
	});


	//Extensions
	verifyTree[L"extensions"].addChild(L"*", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		return ConfigTreeWalker::AddOrCh(dynamic_cast<ConfigRoot&>(d).extensions, s, perms.addExtension, perms.chgExtension);
	});

	//Single extension properties
	verifyTree[L"extensions"][L"*"].addChild(L"library-file", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
    	//Set it, return same
		dynamic_cast<ExtendNode&>(d).libraryFilePath = s.str();
		return d;
	});
	verifyTree[L"extensions"][L"*"].addChild(L"enabled", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
    	//Set it, return same
		dynamic_cast<ExtendNode&>(d).enabled = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"extensions"][L"*"].addChild(L"md5-hash", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
    	//Set it, return same
		dynamic_cast<ExtendNode&>(d).libraryFileChecksum = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"extensions"][L"*"].addChild(L"check-md5", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
    	//Set it, return same
		dynamic_cast<ExtendNode&>(d).requireChecksum = waitzar::read_bool(s.str());
		return d;
	});


	//Languages
	verifyTree[L"languages"].addChild(L"*", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		return ConfigTreeWalker::AddOrCh(dynamic_cast<ConfigRoot&>(d).languages, s, perms.addLanguage, perms.chgLanguage);
	});

	//Language properties
	verifyTree[L"languages"][L"*"].addChild(L"display-name", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
    	//Set it, return same
		dynamic_cast<LangNode&>(d).displayName = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"default-display-method", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
    	//Set pointer pair, return node
		dynamic_cast<LangNode&>(d).defaultDisplayMethodReg = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"default-display-method-small", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
    	//Set pointer pair, return node
		dynamic_cast<LangNode&>(d).defaultDisplayMethodSmall = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"default-output-encoding", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
    	//Set pointer pair, return node
		auto& lang = dynamic_cast<LangNode&>(d);
		lang.defaultOutputEncoding = waitzar::sanitize_id(s.str());
		lang.defaultOutEncStack = s.getStringStack();
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"default-input-method", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
    	//Set pointer pair, return node
		auto& lang = dynamic_cast<LangNode&>(d);
		lang.defaultInputMethod = waitzar::sanitize_id(s.str());
		lang.defaultInMethStack = s.getStringStack();
		return d;
	});


	//Language sub-classes
	verifyTree[L"languages"][L"*"].addChild(L"input-methods", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//We don't have the language ID, so return the same node
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"encodings", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//We don't have the language ID, so return the same node
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"transformations", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//We don't have the language ID, so return the same node
		return d;
	});
	verifyTree[L"languages"][L"*"].addChild(L"display-methods", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//We don't have the language ID, so return the same node
		return d;
	});


	//Language containers
	verifyTree[L"languages"][L"*"][L"input-methods"].addChild(L"*", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		return ConfigTreeWalker::AddOrCh(dynamic_cast<LangNode&>(d).inputMethods, s, perms.addLangInputMeth, perms.chgLangInputMeth);
	});
	verifyTree[L"languages"][L"*"][L"display-methods"].addChild(L"*", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		return ConfigTreeWalker::AddOrCh(dynamic_cast<LangNode&>(d).displayMethods, s, perms.addLangDispMeth, perms.chgLangDispMeth);
	});
	verifyTree[L"languages"][L"*"][L"encodings"].addChild(L"*", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		return ConfigTreeWalker::AddOrCh(dynamic_cast<LangNode&>(d).encodings, s, perms.addLangEncoding, perms.chgLangEncoding);
	});
	verifyTree[L"languages"][L"*"][L"transformations"].addChild(L"*", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		return ConfigTreeWalker::AddOrCh(dynamic_cast<LangNode&>(d).transformations, s, perms.addLangTransform, perms.chgLangTransform);
	});



	//Encoding
	verifyTree[L"languages"][L"*"][L"encodings"][L"*"].addChild(L"display-name", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<EncNode&>(d).displayName = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"encodings"][L"*"].addChild(L"initial", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<EncNode&>(d).initial = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"encodings"][L"*"].addChild(L"image", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<EncNode&>(d).imagePath = s.str();
		return d;
	});
	verifyTree[L"languages"][L"*"][L"encodings"][L"*"].addChild(L"use-as-output", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<EncNode&>(d).canUseAsOutput = waitzar::read_bool(s.str());
		return d;
	});



	//Transformations
	verifyTree[L"languages"][L"*"][L"transformations"][L"*"].addChild(L"from-encoding", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<TransNode&>(d).fromEncoding = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"transformations"][L"*"].addChild(L"to-encoding", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<TransNode&>(d).toEncoding = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"transformations"][L"*"].addChild(L"type", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<TransNode&>(d).type = waitzar::read_transform_type(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"transformations"][L"*"].addChild(L"has-priority", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<TransNode&>(d).hasPriority = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"transformations"][L"*"].addChild(L"source-file", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<TransNode&>(d).sourceFile = s.str();
		return d;
	});


	//Input method
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"display-name", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).displayName = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"encoding", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).encoding = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"user-words-file", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).userWordsFile = s.str();
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"wordlist", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).extraWordsFile = s.str();
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"keyboard-file", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).keyboardFile = s.str();
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"type", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).type = waitzar::read_input_type(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"type-burmese-numerals", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).typeBurmeseNumbers = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"suppress-uppercase", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).suppressUppercase = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"control-keys", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).controlKeyStyle = waitzar::read_control_key_style(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"numeral-conglomerate", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).typeNumeralConglomerates = waitzar::read_bool(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"input-methods"][L"*"].addChild(L"disable-cache", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<InMethNode&>(d).disableCache = waitzar::read_bool(s.str());
		return d;
	});


	//Display method
	verifyTree[L"languages"][L"*"][L"display-methods"][L"*"].addChild(L"encoding", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<DispMethNode&>(d).encoding = waitzar::sanitize_id(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"display-methods"][L"*"].addChild(L"type", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<DispMethNode&>(d).type = waitzar::read_display_type(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"display-methods"][L"*"].addChild(L"font-face-name", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<DispMethNode&>(d).fontFaceName = waitzar::purge_filename(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"display-methods"][L"*"].addChild(L"point-size", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<DispMethNode&>(d).pointSize = waitzar::read_int(s.str());
		return d;
	});
	verifyTree[L"languages"][L"*"][L"display-methods"][L"*"].addChild(L"font-file", [](const StringNode& s, GhostNode& d, const CfgPerm& perms)->GhostNode&{
		//Cast and set
		dynamic_cast<DispMethNode&>(d).fontFile = s.str();
		return d;
	});
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
