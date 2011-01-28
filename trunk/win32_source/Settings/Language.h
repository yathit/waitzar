/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _WZ_SETTINGS_LANGUAGE
#define _WZ_SETTINGS_LANGUAGE

#include <string>
#include <set>
#include "Transform/Transformation.h"
#include "Input/InputMethod.h"
#include "Display/DisplayMethod.h"
#include "Settings/Encoding.h"
#include "Settings/Language.h"


struct Language {
	//Basic constructor
	Language(std::wstring id=L"") {
		this->id = id;
	}
	Language(const wchar_t* id) {
		this->id = id;
	}


	//Simple
	std::wstring id;
	std::wstring displayName;
	Encoding defaultOutputEncoding;
	std::wstring defaultDisplayMethodReg;
	std::wstring defaultDisplayMethodSmall;
	std::wstring defaultInputMethod;

	//Structured
	std::set<InputMethod*>    inputMethods;
	std::set<Encoding>        encodings;
	std::set<Transformation*> transformations;
	std::set<DisplayMethod*>  displayMethods;

	//For fast lookup & easy validation
	//Kept internal; would be private except I don't think it matters much.
	std::map< std::pair<Encoding, Encoding>, Transformation* > transformationLookup;

	//Allow map comparison 
	bool operator<(const Language& other) const {
		return id < other.id;
	}

	//Allow logical equals and not equals
	bool operator==(const Language &other) const {
		return id == other.id;
	}
	bool operator!=(const Language &other) const {
		return id != other.id;
	}

	//Allow eq/neq on strings, too
	//No, just do it with temporary classes
	/*bool operator==(const std::wstring& other) const {
		return id == other;
	}
	bool operator!=(const std::wstring& other) const {
		return id != other;
	}*/
};



#endif //_WZ_SETTINGS_LANGUAGE

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
