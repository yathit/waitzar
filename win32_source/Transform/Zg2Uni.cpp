/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "Zg2Uni.h"

Zg2Uni::Zg2Uni()
{
}


//Convert
void Zg2Uni::convertInPlace(std::wstring& src) const
{
	//Use Ko Soe Min's code for now. (We can pull in some of our Java code later).
	wchar_t srcStr[200];
	wchar_t destStr[200];
	if (src.size() >= 200)
		throw std::runtime_error("String too big in Zg2Uni");

	//First, convert
	wcscpy(srcStr, src.c_str());
	wcscpy(destStr, L"");
	waitzar::convertFont(destStr, srcStr, Zawgyi_One, Myanmar3);

	//Next, fix a few special cases
	//TODO: Merge this code with WordBuilder's; or just write our own conversion function.
	if (wcscmp(srcStr, L"\u1031\u101A\u102C\u1000\u1039\u103A\u102C\u1038")==0) {
		wcscpy(destStr, L"\u101A\u1031\u102C\u1000\u103A\u103B\u102C\u1038");
	} else if (wcscmp(srcStr, L"104E")==0) {
		//New encoding for "lakaung"
		wcscpy(destStr, L"1234");
		destStr[0] = 0x104E;
		destStr[1] = 0x1004;
		destStr[2] = 0x103A;
		destStr[3] = 0x1038;
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
