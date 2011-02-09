/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */


#include "HttpVersionChecker.h"

using std::string;

HMODULE HttpVersionChecker::module = NULL;
string HttpVersionChecker::WZVersionsFileURL = "http://waitzar.googlecode.com/svn/trunk/win32_source/waitzar_versions.txt";


void HttpVersionChecker::InitDLL(bool& enabled, bool requireChecksum, const std::wstring& libraryFilePath, const std::wstring& libraryFileChecksum)
{
	//Return if we don't want to use this module
	if (!enabled)
		return;

	//Return if we've already loaded a Curl DLL
	if (HttpVersionChecker::module!=NULL)
		return;

	//Check its MD5 hash
	if (requireChecksum) {
		string checksum = waitzar::GetMD5Hash(waitzar::escape_wstr(libraryFilePath, false));
		if (checksum!=waitzar::escape_wstr(libraryFileChecksum, false)) {
			enabled = false;
			return;
		}
	}

	//Load the module. Return and disable when in error
	HttpVersionChecker::module = LoadLibrary(libraryFilePath.c_str());
	if (HttpVersionChecker::module==NULL) {
		enabled = false;
		return;
	}

	//Get the relevant function.
	FARPROC fproc = GetProcAddress(HttpVersionChecker::module, "IsNewVersionAvailable");
	if (fproc==NULL) {
		FreeLibrary(HttpVersionChecker::module);
		HttpVersionChecker::module = NULL;
		enabled = false;
		return;
	}

	//Cast it to the correct type
	this->dll_check_version_ptr = (bool (*)(const char*, const char*))fproc;
}


bool HttpVersionChecker::IsUpdateAvailable(const string& currVersionID) const
{
	//Just to be safe:
	if (HttpVersionChecker::module==NULL)
		return false;

	//Call our function
	return dll_check_version_ptr(HttpVersionChecker::WZVersionsFileURL.c_str(), currVersionID.c_str());
}


