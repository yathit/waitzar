/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include <sstream>
#include <string>
#include <vector>


//#include <iostream> //todo: Remove


#include "curl.h"


/**
 * This file provides an interface into the "libcurl" library, so that we can load it as an extension.
 */


//Export the C-style function without decorating it
extern "C" {


//Curl callback function
std::ostringstream buffer;
static size_t curl_writeback(void *data, size_t elemSize, size_t numElem)
{
	//Write to the stream
	buffer.write((const char*)data, numElem*elemSize);
	if (buffer.bad())
		return -1; //Failed
	return numElem*elemSize;
}



bool CheckVersion(const std::string& fileTxt, const char* latestVersionStr)
{
	//If empty, no new version
	if (fileTxt.empty())
		return false;

	//Second, open the file and parse it line-by-line
	//std::ifstream txtFile(pathLocalLastSavedVersionInfo.c_str(), std::ios::in|std::ios::binary);
	//if (txtFile.fail())
	//	return 0;

	//Get the size, rewind
	//txtFile.seekg(0, std::ios::end);
	//int file_size = txtFile.tellg();
	//txtFile.seekg(0, std::ios::beg);
	//if (file_size==-1)
	//	return 0;

	//Load the entire file at once to minimize file I/O
	//unsigned char* buffer = new unsigned char [file_size];
	//txtFile.read((char*)(&buffer[0]), file_size);
	//txtFile.close();

	//Now, loop
	std::vector<std::string> lines;
	std::stringstream currLine;
	for (size_t i=0; i<fileTxt.size(); i++) {
		//Skip \r, space
		if (fileTxt[i]=='\r' || fileTxt[i]==' ')
			continue;

		//If we encounter a '#', skip to the end of the line
		if (fileTxt[i]=='#') {
			while (fileTxt[i]!='\n' && i<fileTxt.size())
				i++;
		}

		//If we're at the end of the line, add an entry
		if (fileTxt[i]=='\n' || i==(fileTxt.size()-1)) {
			if (!currLine.str().empty()) {
				lines.push_back(currLine.str());
				currLine.str("");
			}
		} else {
			//Otherwise, just add the letter
			currLine <<fileTxt[i];
		}
	}

	//Finally, check. For now, we only fail if our item is in the list but not in the first position.
	//  This makes it easier to whitelist, say, a "long-term" support version later.
	int ourIndex = -1;
	std::string verString = std::string(latestVersionStr);
	for (size_t i=0; i<lines.size(); i++) {
		if (lines[i] == verString) {
			ourIndex = i;
			break;
		}
	}

	return ourIndex>0;
}



//The function our DLL will use to handle data conversion
// NOTE: We use char* here because we're using the same compiler for WZ and Curl, so we know the data types will alias.
__declspec(dllexport) bool IsNewVersionAvailable(const char* fileURL, const char* latestVersionStr)
{
	try {
		//Init curl; get the curl object
		buffer.str(""); //Reset our stream
		curl_global_init(CURL_GLOBAL_DEFAULT);
		CURL* curl = curl_easy_init();
		if (curl) {
			//Option: Url to download
			curl_easy_setopt(curl, CURLOPT_URL, fileURL);
			//Option: callback function for writing data
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writeback);
			//Option: debug output
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
			//Option: Some servers don't like requests that are made without a user-agent field, so we provide one
			curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

			//Call curl
			CURLcode resCode = curl_easy_perform(curl);

			//Cleanup
			curl_easy_cleanup(curl);

			//React
			if (resCode != CURLE_OK)
				buffer.str(""); //Reset our stream
		}

		//Cleaup curl globally
		curl_global_cleanup();

		//Finally, if "buffer" is not empty, process it
		return CheckVersion(buffer.str(), latestVersionStr);
	} catch (...) {} //Exceptions should not cross DLL boundaries.

	//Return false in case of error
	return false;
}

}


//
// Test code: uncomment to use
//

/*int main(int argc, char* argv[]) {
	const char* url = "http://waitzar.googlecode.com/svn/trunk/win32_source/waitzar_versions.txt";
	const char* currVer = "1.7";
	bool isNew = IsNewVersionAvailable(url, currVer);

	if (isNew)
		std::cout <<"Newer version available!" <<std::endl;
	else
		std::cout <<"Using latest version" <<std::endl;

}*/










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
