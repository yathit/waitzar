/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _WZ_UTILITIES
#define _WZ_UTILITIES

//Don't let Visual Studio warn us to use the _s functions
//#define _CRT_SECURE_NO_WARNINGS

//Necessary libraries
//#include "windows_wz.h"
//#include <wchar.h>
//#include <stdio.h>
#include <vector>
#include <algorithm>
#include <functional>
#include <locale>
#include <stdexcept>
//#include <ctype.h>

//Should probably move 'mymb' function here..
#include "WordBuilder.h"
#include "NGram/Logger.h"
#include "MD5/md5simple.h"


//////////////////////////////////////////////////////////////////////////////////
// Please note that the wz_utilities library is currently considered
//   to be UNSTABLE, and is not included in the Linux build by default.
// This is because most of its functionality is of direct benefit to the
//   Windows build, so we hope to test its correctness through the next
//   two Windows releases of WaitZar. 
// Since there is some universally-useful code here, we will eventually
//   include this in both releases, but we want to avoid bumping libwaitzar's
//   major revision for breakages caused by code that wasn't originally intended
//   for Linux to begin with.
//////////////////////////////////////////////////////////////////////////////////
namespace waitzar 
{
	//Moved from WZFactory
	const std::wstring WZSystemDefinedWords = L"`~!@#$%^&*()-_=+[{]}\\|;:'\"<>/? 1234567890\u200B";


	/**
	 * Sort a unicode string according to the rules defined in UTN-11 and K. Soe Min's blog.
	 *  Works in-place, and has a complexity of O(size(uniString)), single-pass
	 *  Any non-Myanmar letters are ignored (technically, they cause the sorter to reset, which is expected)
	 * We do not sort stacked letters together; rather, we sort U+1039 separately and then re-start the 
	 *  algorithm when the stacked consonant is encountered. This requires special care to be taken for kinzi.
	 * We assume that U+103A and U+1039 (and consonants, and kinzi) are always properly ordered.
	 *  We ignore "visible virama" and treat it always like "asat". This is done because it is equivalent for
	 *  most cases, and exceptions seem to render properly. We will revise this if we find any glaring inconsistencies.
	 */
	std::wstring sortMyanmarString(const std::wstring &uniString);
	std::wstring renderAsZawgyi(const std::wstring &uniString);
	std::string ReadBinaryFile(const std::string& path);
	std::wstring readUTF8File(const std::string& path);

	//Other useful methods
	void loc_to_lower(std::wstring& str);
	std::string escape_wstr(const std::wstring& str, bool errOnUnicode=false);
	std::string wcs2mbs(const std::wstring& str);
	std::wstring mbs2wcs(const std::string& str);
	std::wstring preparse_json(const std::wstring& str);
	std::wstring normalize_bgunicode(const std::wstring& str);
	std::wstring removeZWS(const std::wstring& str, const std::wstring& filterStr);
	size_t count_letter(const std::wstring& str, wchar_t letter);

	//Various "glue" functions for Error messages
	std::string glue(std::string str1, std::string str2=std::string(), std::string str3=std::string(), std::string str4=std::string());
	std::string glue(std::wstring str1, std::wstring str2=std::wstring(), std::wstring str3=std::wstring(), std::wstring str4=std::wstring());

	//Functions imported from ConfigManager
	bool IsProbablyFile(const std::wstring& str);
	std::wstring purge_filename(const std::wstring& str);
	std::wstring sanitize_id(const std::wstring& str);
	std::wstring sanitize_value(const std::wstring& str, const std::wstring& filePath);
	bool read_bool(const std::wstring& str);
	int read_int(const std::wstring& str);
	std::vector<std::wstring> separate(std::wstring str, wchar_t delim);

	//Message digest!
	std::string GetMD5Hash(const std::string& fileName);

} //End waitzar namespace


#endif //_WZ_UTILITIES



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

