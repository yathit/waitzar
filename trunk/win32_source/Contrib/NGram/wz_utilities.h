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
	std::string escape_wstr(const std::wstring& str);
	std::string escape_wstr(const std::wstring& str, bool errOnUnicode);
	std::string wcs2mbs(const std::wstring& str);
	std::wstring mbs2wcs(const std::string& str);
	std::wstring preparse_json(const std::wstring& str);
	std::wstring normalize_bgunicode(const std::wstring& str);
	std::wstring removeZWS(const std::wstring& str, const std::wstring& filterStr);
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

	
	//These two functions can be included (with "using") to clear up complaints of "ambiguous" by the compiler. 
	static long max(const long &a, const int &b) { return std::max<long>(a, b); }
	static long min(const long &a, const int &b) { return std::min<long>(a,b); }


	//Helper predicate
	//TODO: Make private
	/*template <class T>
	class is_id_delim : public std::unary_function<T, bool>
	{
	public:
	 bool operator ()(T t) const
	 {
	  if ((t==' ')||(t=='\t')||(t=='\n')||(t=='-')||(t=='_'))
	   return true; //Remove it
	  return false; //Don't remove it
	 }
	};*/

	//Used elsewhere...
	static std::wstring sanitize_id(const std::wstring& str)
	{
		std::wstring res = str; //Copy out the "const"-ness.
		//res = std::wstring(res.begin(), std::remove_if(res.begin(), res.end(), is_id_delim<wchar_t>()));
		auto is_id_delim = [](wchar_t letter)->bool {
			  if ((letter==' ')||(letter=='\t')||(letter=='\n')||(letter=='-')||(letter=='_'))
			   return true; //Remove it
			  return false; //Don't remove it
		};
		res = std::wstring(res.begin(), std::remove_if(res.begin(), res.end(), is_id_delim));
		loc_to_lower(res); //Operates in-place.
		return res;
	}
	static bool read_bool(const std::wstring& str)
	{
		std::wstring test = str;
		loc_to_lower(test);
		if (test == L"yes" || test==L"true")
			return true;
		else if (test==L"no" || test==L"false")
			return false;
		else
			throw std::runtime_error(glue(L"Bad boolean value: \"", str, L"\"").c_str());
	}
	static int read_int(const std::wstring& str)
	{
		//Read
		int resInt;
		std::wistringstream reader(str);
		reader >>resInt;

		//Problem?
		if (reader.fail())
			throw std::runtime_error(glue(L"Bad integer value: \"", str, L"\"").c_str());

		//Done
		return resInt;
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

