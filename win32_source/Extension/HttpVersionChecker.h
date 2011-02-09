/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once


#include <string>

#include "windows_wz.h"

#include "NGram/wz_utilities.h"
#include "Extension.h"


/**
 * Simple class for loading a URL and testing if WaitZar is up-to-date
 */
class HttpVersionChecker : public Extension {
public:
	virtual void InitDLL(bool& enabled, bool requireChecksum, const std::wstring& libraryFilePath, const std::wstring& libraryFileChecksum);
	bool IsUpdateAvailable(const std::string& currVersionID) const;

private:
	//Additional properties required by the DLL
	static HMODULE module;
	bool (*dll_check_version_ptr)(const char*, const char*);

	//What we're passing in
	static std::string WZVersionsFileURL;
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
