/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once

/*
 * These classes are used to hold all relevant config properties for any config object.
 *    They also contain pairs of id-to-pointer for any class which will be resolved later,
 *    such as InputMethods or Transformations.
 * Each class declares "CfgLoader" as a friend class, which allows that class to modify
 *    the class's inner workings. Thus, when MainFile gets a Config option from this
 *    tree, there's no risk of accidentally modifying anything.
 * Wherever there is a map or pair with ->second containing another *Node type, do the following:
 *    1) When building, store the ID of the Encoding/Tranform/EtcNode, and an empty id (invalid class)
 *    2) In a second pass, for each UNIQUE map/pair->first, initialize the pointer *impl ONCE.
 *       Then, "copy" this *Node object into each map/pair->second. The default copy constructor
 *       will ensure that the pointer is copied.
 *    3) Adding a new item to the tree later will not invalidate any pointers; *Nodes can be moved
 *       around within maps or copied in pairs without invalidating any references. Note that, if
 *       we choose to later reclaim memory from these pointers, simply delete the pointer ONCE
 *       for each UNIQUE map/pair->first.
 * Note: We define the classes in "reverse" order, to resolve dependencies. (The other option is
 *       to put them all into their own header files, which I see no need for).
 */

#include <vector>
#include <map>
#include <string>

#include "Extension/Extension.h"
#include "Display/DisplayMethod.h"
#include "Input/InputMethod.h"
#include "Transform/Transformation.h"
#include "Settings/HotkeyData.h"
#include "Settings/Types.h"


//Fun times!
class nodeset_exception {
public:
	nodeset_exception(const char* what, const wchar_t* key)  {
		//The "n" functions pad with zero
		strncpy(what_, what, 1023);
		wcsncpy(key_, key, 1023);
	}
	const char* what() { return what_; }
	const wchar_t* key() { return key_; }
private:
	char what_[1024];
	wchar_t key_[1024];
};




//Everthing extends this; makes passing arguments easier.
//  A "Ghost" is easy to describe by looking at it (properties), but has no substance (implementation).
//  Plus, it's much better than the previous name: "TNode"
class GhostNode {
public:
	mutable std::wstring id;

	GhostNode(const std::wstring& id=L"") : id(id) {}

	//Logical equals
	bool operator==(const GhostNode& other) const {
		return id == other.id;
	}
	bool operator==(const std::wstring& other) const {
		return id == other;
	}

private:
	//Class needs at least one virtual function to be polymorphic (and thus to allow dynamic_cast)
	virtual void empty(){};

	//FYI: In case you're curious, sub-classes don't inherit friends
};


class EncNode : public GhostNode {
public:
	//Simple properties
	bool canUseAsOutput;
	std::wstring displayName;
	std::wstring initial;
	std::wstring imagePath;


private:
	//Implementation
	//Encoding* impl;

	//For loading
	friend class ConfigManager;


public:
	//Constructor: set the ID here and nowhere else
	EncNode(const std::wstring& id=L"") : GhostNode(id) {
		//this->impl = NULL;
		this->canUseAsOutput = false;
	}
};



class DispMethNode : public GhostNode {
public:
	//Simple properties
	DISPLAY_TYPE type;

	//Inherited properties
	int pointSize;
	std::wstring fontFaceName;
	std::wstring fontFile;

	//Reference value
	std::wstring encoding;


private:
	//Implementation
	DisplayMethod* impl;

	//For loading
	friend class ConfigManager;


public:
	//Constructor: set the ID here and nowhere else
	DispMethNode(const std::wstring& id=L"") : GhostNode(id) {
		this->impl = NULL;
		this->type = DISPLAY_TYPE::UNDEFINED;
		this->pointSize = 0;
	}

	//Get
	DisplayMethod* const getImpl() const {
		if (impl==NULL)
			throw std::runtime_error(waitzar::glue(L"Implementation not defined for: ", id).c_str());
		return impl;
	}

};



class InMethNode : public GhostNode {
public:
	//Simple properties
	std::wstring displayName;
	INPUT_TYPE type;
	bool suppressUppercase;
	bool typeNumeralConglomerates;
	bool disableCache;
	bool typeBurmeseNumbers;
	CONTROL_KEY_TYPE controlKeyStyle;

	//Derived properties
	std::wstring userWordsFile;
	std::wstring extraWordsFile;
	std::wstring keyboardFile;

	//Refer-by-name
	std::wstring encoding;

private:
	//Implementation
	InputMethod* impl;

	//For loading
	friend class ConfigManager;


public:
	//Constructor: set the ID here and nowhere else
	InMethNode(const std::wstring& id=L"") : GhostNode(id) {
		this->impl = NULL;
		this->suppressUppercase = true;
		this->typeNumeralConglomerates = false;
		this->disableCache = false;
		this->typeBurmeseNumbers = true;
		this->controlKeyStyle = CONTROL_KEY_TYPE::CHINESE;
		this->type = INPUT_TYPE::UNDEFINED;
	}

	//Get
	InputMethod* const getImpl() const {
		if (impl==NULL)
			throw std::runtime_error(waitzar::glue(L"Implementation not defined for: ", id).c_str());
		return impl;
	}
};



class TransNode : public GhostNode {
public:
	//Simple properties
	bool hasPriority;
	TRANSFORM_TYPE type;

	//Derived properties
	std::wstring sourceFile;

	//Reference values
	std::wstring fromEncoding;
	std::wstring toEncoding;

private:
	//Implementation
	Transformation* impl;

	//For loading
	friend class ConfigManager;


public:
	//Constructor: set the ID here and nowhere else
	TransNode(const std::wstring& id=L"") : GhostNode(id) {
		this->impl = NULL;
		this->hasPriority = false;
		this->type = TRANSFORM_TYPE::UNDEFINED;
	}

	//Get
	const Transformation* const getImpl() const {
		if (impl==NULL)
			throw std::runtime_error(waitzar::glue(L"Implementation not defined for: ", id).c_str());
		return impl;
	}
};



class LangNode : public GhostNode {
public:
	//Simple
	std::wstring displayName;

	//Reference properties
	std::wstring   defaultOutputEncoding;
	std::wstring   defaultDisplayMethodReg;
	std::wstring   defaultDisplayMethodSmall;
	std::wstring   defaultInputMethod;

	//Map of pointers by id
	std::map<std::wstring, InMethNode>    inputMethods;
	std::map<std::wstring, EncNode>       encodings;
	std::map<std::wstring, TransNode>     transformations;
	std::map<std::wstring, DispMethNode>  displayMethods;

private:
	//For loading
	friend class ConfigManager;
	friend class WZFactory;

public:
	//Constructor: set the ID here and nowhere else
	LangNode(const std::wstring& id=L"") : GhostNode(id) {
		//this->impl = NULL;
	}
};



class ExtendNode : public GhostNode {
public:
	//Struct-like properties
	std::wstring libraryFilePath;
	std::wstring libraryFileChecksum;
	bool enabled;
	bool requireChecksum;

private:
	//Actual pointer
	Extension* impl;

	//For loading
	friend class ConfigManager;
	friend class WZFactory;

public:
	//Constructor: set the ID here and nowhere else
	ExtendNode(const std::wstring& id=L"") : GhostNode(id) {
		this->impl = NULL;
		this->enabled = false;
		this->requireChecksum = true;
	}

	//Get
	const Extension* const getImpl() const {
		if (impl==NULL)
			throw std::runtime_error(waitzar::glue(L"Implementation not defined for: ", id).c_str());
		return impl;
	}
};


class SettingsNode : public GhostNode {
public:
	//Simple
	HotkeyData hotkey;
	bool silenceMywordsErrors;
	bool balloonStart;
	bool alwaysElevate;
	bool trackCaret;
	bool lockWindows;
	bool suppressVirtualKeyboard;
	std::wstring whitespaceCharacters;
	std::wstring ignoredCharacters;
	bool hideWhitespaceMarkings;

	//Reference values
	std::wstring      defaultLanguage;

private:
	//For loading
	friend class ConfigManager;

public:
	SettingsNode() {
		this->silenceMywordsErrors = false;
		this->balloonStart = true;
		this->alwaysElevate = false;
		this->trackCaret = true;
		this->lockWindows = true;
		this->suppressVirtualKeyboard = false;
		this->whitespaceCharacters = L"\u200B";
		this->ignoredCharacters = L"\u200B";
		this->hideWhitespaceMarkings = true;
	}
};


class ConfigRoot : public GhostNode {
public:
	SettingsNode settings;
	std::map<std::wstring, LangNode> languages;
	std::map<std::wstring, ExtendNode> extensions;

	//For loading
	friend class ConfigManager;
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
