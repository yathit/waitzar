/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once

#include <string.h>


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



//For our keyboards
enum class CONTROL_KEY_TYPE {
	CHINESE,
	JAPANESE,
};


//Enum types for our various configuration classes
enum class INPUT_TYPE {
	UNDEFINED,
	BUILTIN,
	KEYBOARD,
	ROMAN,
};

enum class DISPLAY_TYPE {
	UNDEFINED,
	BUILTIN,
	TTF,
	PNG,
};

enum class TRANSFORM_TYPE {
	UNDEFINED,
	BUILTIN,
	JAVASCRIPT,
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

