/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _JAVASCRIPT_CONVERTER
#define _JAVASCRIPT_CONVERTER

#include "windows_wz.h"

#include <string>
#include <vector>

#include "NGram/wz_utilities.h"
#include "Extension.h"


//This class is meant to be sub-classed by various DLL wrappers.
//  Sub-classes will need to add functionality; the DLLs themselves
//  are too varied for me to declare a generalized virtual method.


class JavaScriptConverter : public Extension {
public:
	//Basic constructor
	JavaScriptConverter(std::wstring id=L"") {
		this->id = id;
	}

	//Main functionality
	virtual void InitDLL(std::string (*MD5Function)(const std::string&));
	std::pair<bool, std::wstring> ConvertString(const std::wstring& jsSource, const std::wstring& input);

private:
	//Additional properties required by the DLL
	static HMODULE module;
	void (*dll_str_convert_ptr)(uint16_t*, uint16_t*,  uint16_t);

	//Used to pass data to and from the function
	const static size_t MAX_INOUT_SIZE = 2048; //Max size of input and return string
	static wchar_t* large_array;

};


#endif //_JAVASCRIPT_CONVERTER


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
