/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "Uni2Zg.h"

Uni2Zg::Uni2Zg()
{
	Logger::resetLogFile('Z');
}


//Convert
void Uni2Zg::convertInPlace(std::wstring& src) const
{
	//Use our code, from the utilities package.
	Logger::writeLogLine('Z', std::wstring(L"Unicode: {") + src + L"}");
	src = waitzar::sortMyanmarString(src);
	src = waitzar::renderAsZawgyi(src);
	Logger::writeLogLine('Z', std::wstring(L"Zawgyi1: {") + src + L"}");
	Logger::writeLogLine('Z');
	//src = waitzar::removeZWS(src, L"-"); //Remove hyphens
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
