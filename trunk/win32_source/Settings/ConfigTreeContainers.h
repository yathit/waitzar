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
 * Wherever there is a pointer to another *Node type, do the following:
 *    1) When building, store the ID of the Encoding/Tranform/EtcNode, and a NULL pointer.
 *    2) In a second pass, replace the NULL pointers with the address of the object.
 *    3) Note that adding, say, a new Input Method would require you to scan ALL
 *       things that may reference Input Methods and replace their pointers with updated ones.
 *       This is because adding an item to a map/vector may change its location in memory.
 *       Ideally, we won't add new InputMethods/etc. after loading the tree for the first time.
 *       (Curently we don't). By encapsulating all modifying commands into the CfgLoader, we
 *       minimize the possibility of accidentally invalidating our pointers.
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
#include "Settings/Language.h"
#include "Settings/Encoding.h"


class CfgLoader {};


//Everthing extends this; makes passing arguments easier.
class TNode {};


class EncNode : public TNode {
public:
	//Simple properties
	const std::wstring id;
	bool canUseAsOutput;
	std::wstring displayName;
	std::wstring initial;
	std::wstring imagePath;


private:
	//Implementation
	Extension* impl;

	//For loading
	friend class CfgLoader;


public:
	//Constructor: set the ID here and nowhere else
	EncNode(const std::wstring& id=L"") : id(id) {
		this->impl = NULL;
	}
};



class DispMethNode : public TNode {
public:
	//Simple properties
	const std::wstring id;
	int type; //We can make this an enum class later

private:
	//Pointer pairs
	std::pair<std::wstring, EncNode*> encoding;

	//Implementation
	DisplayMethod* impl;

	//For loading
	friend class CfgLoader;


public:
	//Constructor: set the ID here and nowhere else
	DispMethNode(const std::wstring& id=L"") : id(id) {
		this->impl = NULL;
	}
};



class InMethNode : public TNode {
public:
	//Simple properties
	std::wstring id;
	std::wstring displayName;
	int type; //enum class later
	bool suppressUppercase;
	bool typeNumeralConglomerates;
	bool disableCache;
	bool typeBurmeseNumbers;
	int controlKeyStyle; //enum class later

private:
	//Pointer-pairs
	std::pair<std::wstring, EncNode*> encoding;

	//Implementation
	InputMethod* impl;

	//For loading
	friend class CfgLoader;


public:
	//Constructor: set the ID here and nowhere else
	InMethNode(const std::wstring& id=L"") : id(id) {
		this->impl = NULL;
	}
};



class TransformNode : public TNode {
public:
	//Simple properties
	const std::wstring id;
	bool hasPriority;
	int type; //make enum later

private:
	//Pointer pairs
	std::pair<std::wstring, EncNode*> fromEncoding;
	std::pair<std::wstring, EncNode*> toEncoding;

	//Implementation
	Transformation* impl;

	//For loading
	friend class CfgLoader;


public:
	//Constructor: set the ID here and nowhere else
	TransformNode(const std::wstring& id=L"") : id(id) {
		this->impl = NULL;
	}
};



class LangNode : public TNode {
public:
	//Simple
	const std::wstring id;
	std::wstring displayName;

private:
	//Actual
	Language* impl;

	//Pointer pairs
	std::pair<std::wstring, EncNode*>      defaultOutputEncoding;
	std::pair<std::wstring, DispMethNode*> defaultDisplayMethodReg;
	std::pair<std::wstring, DispMethNode*> defaultDisplayMethodSmall;
	std::pair<std::wstring, InMethNode*>   defaultInputMethod;

	//Map of pointers by id
	std::map<std::wstring, InMethNode*>    inputMethods;
	std::map<std::wstring, EncNode*>       encodings;
	std::map<std::wstring, TransformNode*> transformations;
	std::map<std::wstring, DispMethNode*>  displayMethods;

	//For loading
	friend class CfgLoader;


public:
	//Constructor: set the ID here and nowhere else
	LangNode(const std::wstring& id=L"") : id(id) {
		this->impl = NULL;
	}
};



class ExtendNode : public TNode {
public:
	//Struct-like properties
	const std::wstring id;
	std::wstring libraryFilePath;
	std::wstring libraryFileChecksum;
	bool enabled;
	bool requireChecksum;

private:
	//Actual pointer
	Extension* impl;

	//For loading
	friend class CfgLoader;


public:
	//Constructor: set the ID here and nowhere else
	ExtendNode(const std::wstring& id=L"") : id(id) {
		impl = NULL;
	}
};


class SettingsNode : public TNode {
public:
	//Simple
	std::wstring hotkeyStrRaw;  //Later replace with HotkeyData;
	bool silenceMywordsErrors;
	bool balloonStart;
	bool alwaysElevate;
	bool trackCaret;
	bool lockWindows;
	bool suppressVirtualKeyboard;
	std::wstring defaultLanguage;
	std::wstring whitespaceCharacters;
	std::wstring ignoredCharacters;
	bool hideWhitespaceMarkings;

	//For loading
	friend class CfgLoader;
};


class ConfigRoot : public TNode {
public:
	SettingsNode settings;
	std::map<std::wstring, LangNode> languages;
	std::map<std::wstring, ExtendNode> extensions;

	//For loading
	friend class CfgLoader;
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
