/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "Uni2WinInnwa.h"

Uni2WinInnwa::Uni2WinInnwa()
{
}


//Convert
//TODO: Streamline; we do two conversions here. :(
void Uni2WinInnwa::convertInPlace(std::wstring& src) const
{
	//Use our code
	src = waitzar::renderAsZawgyi(src);
	
	//Use Ko Soe Min's code for now. (We can pull in some of our Java code later).
	wchar_t srcStr[200];
	wchar_t destStr[200];
	if (src.size() >= 200)
		throw std::runtime_error("String too big in Uni2WinInnwa");

	//First, convert
	wcscpy(srcStr, src.c_str());
	wcscpy(destStr, L"");
	waitzar::convertFont(destStr, srcStr, Zawgyi_One, WinInnwa);

	//Next, fix a few special cases
	//TODO: Merge this code with WordBuilder's; or just write our own conversion function.
	if (wcscmp(srcStr, L"\u1009\u102C\u1025\u1039")==0) {
		wcscpy(destStr, L"123");
		destStr[0] = 211;
		destStr[1] = 79;
		destStr[2] = 102;
	} else if (wcscmp(srcStr, L"\u1015\u102B\u1094")==0) {
		wcscpy(destStr, L"123");
		destStr[0] = 121;
		destStr[1] = 103;
		destStr[2] = 104;
	}

	//Finally, save the value created.
	src = std::wstring(destStr);
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
