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
#include <wchar.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <functional>
#include <locale>
#include <ctype.h>

//Should probably move 'mymb' function here..
#include "WordBuilder.h"
//#include "Logger.h" //Will eventually need to become part of "contrib"


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

	std::wstring readUTF8File(const std::string& path);

	//Other useful methods
	std::string escape_wstr(const std::wstring& str);
	std::string escape_wstr(const std::wstring& str, bool errOnUnicode);
	std::string wcs2mbs(const std::wstring& str);
	std::wstring preparse_json(const std::wstring& str);
	std::wstring normalize_bgunicode(const std::wstring& str);
	std::wstring removeZWS(const std::wstring& str);
	size_t count_letter(const std::wstring& str, wchar_t letter);

	//And finally, locale-driven nonsense with to_lower:
	template<class T>
	class ToLower {
	public:
		 ToLower(const std::locale& loc):loc(loc)
		 {
		 }
		 T operator()(const T& src) const
		 {
			  return std::tolower<T>(src, loc);
		 }
	protected:
		 const std::locale& loc;
	};

	static void loc_to_lower(std::wstring& str)
	{
		//Locale-aware "toLower" converter
		std::locale loc(""); //Get native locale
		std::transform(str.begin(),str.end(),str.begin(),ToLower<wchar_t>(loc));
	}



	//For stl exceptions...
	static std::string glue(const std::string& str1, const std::string& str2, const std::string& str3, const std::string& str4)
	{
		std::stringstream msg;
		msg <<str1 <<str2 <<str3 <<str4;
		return msg.str();
	}
	static std::string glue(const std::string& str1, const std::string& str2, const std::string& str3)
	{
		return glue(str1, str2, str3, "");
	}
	static std::string glue(const std::string& str1, const std::string& str2)
	{
		return glue(str1, str2, "", "");
	}
	static std::string glue(const std::wstring& str1, const std::wstring& str2, const std::wstring& str3, const std::wstring& str4)
	{
		return glue(waitzar::escape_wstr(str1, false), waitzar::escape_wstr(str2, false), waitzar::escape_wstr(str3, false), waitzar::escape_wstr(str4, false));
	}
	static std::string glue(const std::wstring& str1, const std::wstring& str2, const std::wstring& str3)
	{
		return glue(str1, str2, str3, L"");
	}
	static std::string glue(const std::wstring& str1, const std::wstring& str2)
	{
		return glue(str1, str2, L"", L"");
	}


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

