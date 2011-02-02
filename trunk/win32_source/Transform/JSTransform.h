/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _TRANSFORM_JSTRANS
#define _TRANSFORM_JSTRANS

#include <string>

#include "Transform/Transformation.h"
#include "NGram/wz_utilities.h"
#include "Extension/JavaScriptConverter.h"

/**
 * Used for one of a variety of javascript transformation classes.
 */
class JSTransform : public Transformation
{
public:
	JSTransform(const std::string& jsSourcePath, const JavaScriptConverter& jsInterpreter) {
		this->sourceCode = waitzar::readUTF8File(jsSourcePath);
		this->jsInterpreter = jsInterpreter;
	}

	//Convert
	void convertInPlace(std::wstring& src) const {
		//Do nothing if empty
		if (src.empty())
			return;

		//Else, call our DLL
		std::pair<bool, std::wstring> res = jsInterpreter.ConvertString(this->sourceCode, src);
		src = res.second;
		if (!res.first)
			throw std::runtime_error(waitzar::glue(L"JavaScript error: ", src).c_str());
	}

private:
	std::wstring sourceCode;
	JavaScriptConverter jsInterpreter;
};


#endif //_TRANSFORM_JSTRANS

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

