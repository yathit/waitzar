/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "JavaScriptConverter.h"

using std::wstring;
using std::string;
using std::pair;


//Main DLL module. All classes share one instance
HMODULE JavaScriptConverter::module = NULL;
wchar_t* JavaScriptConverter::large_array = new wchar_t[JavaScriptConverter::MAX_INOUT_SIZE];


//Load the DLL. Has no effect if the DLL has already been loaded.
//Sets its own status to "disabled" if an error occurs.
void JavaScriptConverter::InitDLL(/*std::string (*MD5Function)(const std::string&)*/)
{
	//Return if we don't want to use this module
	if (!this->enabled)
		return;

	//Return if we've already loaded a javascript DLL
	if (JavaScriptConverter::module!=NULL)
		return;

	//Check its MD5 hash
	if (this->requireChecksum) {
		string checksum = waitzar::GetMD5Hash(waitzar::escape_wstr(this->libraryFilePath, false));
		if (checksum!=waitzar::escape_wstr(this->libraryFileChecksum, false)) {
			this->enabled = false;
			return;
		}
	}

	//Load the module. Return and disable when in error
	JavaScriptConverter::module = LoadLibrary(this->libraryFilePath.c_str());
	if (JavaScriptConverter::module==NULL) {
		this->enabled = false;
		return;
	}

	//Get the relevant function.
	FARPROC fproc = GetProcAddress(JavaScriptConverter::module, "ConvertString");
	if (fproc==NULL) {
		FreeLibrary(JavaScriptConverter::module);
		JavaScriptConverter::module = NULL;
		this->enabled = false;
		return;
	}

	//Cast it to the correct type
	this->dll_str_convert_ptr = (void (*)(uint16_t*, uint16_t*, uint16_t))fproc;
}


pair<bool, wstring> JavaScriptConverter::ConvertString(const wstring& jsSource, const wstring& input) const
{
	//Safety check
	if (JavaScriptConverter::module==NULL)
		return pair<bool, wstring>(false, L"<fptr_error>");

	//Check the input size
	if (input.length()+1 >= JavaScriptConverter::MAX_INOUT_SIZE)
		return pair<bool, wstring>(false, L"Input string too large!");

	//Copy the input string into the large array
	wcscpy(large_array, input.c_str());

	//Run our code
	dll_str_convert_ptr((uint16_t*)jsSource.c_str(), (uint16_t*)large_array, (uint16_t)JavaScriptConverter::MAX_INOUT_SIZE);

	//Convert the return value
	uint16_t errorCode = large_array[0];
	wstring retStr = wstring(&large_array[1]);

	//Return
	return pair<bool, wstring>(errorCode==0, retStr);
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
