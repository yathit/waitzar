/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _INTERFACES
#define _INTERFACES

#include <string>
#include <map>

#include "Input/InputMethod.h"


//A struct... will refactor into separate classes later.
struct Encoding {
	std::wstring id;
	std::wstring displayName;
	std::wstring initial;
	std::wstring imagePath;

	//Allow map comparison 
	bool operator<(const Encoding& other) const {
		return id < other.id;
	}

	//Allow logical equals and not equals
	bool operator==(const Encoding &other) const {
		return id == other.id;
	}
	bool operator!=(const Encoding &other) const {
		return id != other.id;
	}

	//Allow eq/neq on strings, too
	bool operator==(const std::wstring& other) const {
		return id == other;
	}
	bool operator!=(const std::wstring& other) const {
		return id != other;
	}
};

//Expected interface: "Input Method"
class Transformation {
public:
	//Struct-like properties
	std::wstring id;
	std::wstring fromEncoding;
	std::wstring toEncoding;
	TYPES type;

	//Convert from fromEncoding to toEncoding.
	//  The const references allow us to save processing if the source and destination are the same.
	const virtual std::wstring& convert(const std::wstring& src) = 0;

	//Allow map comparison 
	bool operator<(const Transformation& other) const {
		return id < other.id;
	}

	//Allow logical equals and not equals
	bool operator==(const Transformation &other) const {
		return id == other.id;
	}
	bool operator!=(const Transformation &other) const {
		return id != other.id;
	}

	//Allow eq/neq on strings, too
	bool operator==(const std::wstring& other) const {
		return id == other;
	}
	bool operator!=(const std::wstring& other) const {
		return id != other;
	}
};

//Expected interface: "Display Method"
class DisplayMethod {
public:
	//Struct-like properties
	std::wstring id;
	std::wstring displayName;
	std::wstring encoding;
	TYPES type;

	//Temp for now: just force this to be virtual
	virtual void typeText() = 0;

	//Allow map comparison 
	bool operator<(const DisplayMethod& other) const {
		return id < other.id;
	}

	//Allow logical equals and not equals
	bool operator==(const DisplayMethod &other) const {
		return id == other.id;
	}
	bool operator!=(const DisplayMethod &other) const {
		return id != other.id;
	}

	//Allow eq/neq on strings, too
	bool operator==(const std::wstring& other) const {
		return id == other;
	}
	bool operator!=(const std::wstring& other) const {
		return id != other;
	}
};



#endif //_INTERFACES

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

