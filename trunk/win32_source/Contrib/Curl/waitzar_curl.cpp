/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "curl.h"


/**
 * This file provides an interface into the "libcurl" library, so that we can load it as an extension.
 */

//Export the C-style function without decorating it
extern "C" {


//The function our DLL will use to handle data conversion
//TODO: Later we might consider keeping the context in place, and loading source scripts once only
//      (but how to deal with memory leaks or bad scripts?)
__declspec(dllexport) void IsNewVersionAvailable(uint8_t* fileURL, uint8_t* latestVersionStr)
{
	//Init curl; get the curl object
	curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL* curl = curl_easy_init();
	bool filedownloadsuccess = false;
	if (curl) {
		//Option: Url to download
		curl_easy_setopt(curl, CURLOPT_URL, "http://waitzar.googlecode.com/svn/trunk/win32_source/waitzar_versions.txt");
		//Option: callback function for writing data
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writeback);
		//Option: debug output
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

		//Call curl
		CURLcode resCode = curl_easy_perform(curl);

		//Cleanup
		curl_easy_cleanup(curl);

		//React
		if (resCode == CURLE_OK)
			filedownloadsuccess = true;
	}

	//Always clean up/close file
	if(curlfilestream)
		fclose(curlfilestream); /* close the local file */

	//Cleaup curl globally
	curl_global_cleanup();


}

}


//
// Test code: uncomment to use
//

int main(int argc, char* argv[]) {

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
