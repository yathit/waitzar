/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "OptionStack.h"



void OptionStack::backup()
{
	//We can actually have maxLevels+1, so check the size BEFORE adding this
	if (options.size()>maxLevels)
		throw std::runtime_error("OptionStack max levels exceeded.");

	//New "last entry"
	options.push_back(std::map<std::wstring, std::wstring>());
}


void OptionStack::persist()
{
	//Make sure we won't end up with nowhere to put data.
	if (options.size()==1)
		throw std::runtime_error("OptionStack attempted to persist() when no backup had been made.");

	//Save it
	options.erase(options.end()-2);
}


void OptionStack::restore()
{
	//Make sure we won't end up with nowhere to put data.
	if (options.size()==1)
		throw std::runtime_error("OptionStack attempted to restore() when no backup had been made.");

	//Ditch it
	options.pop_back();
}


std::wstring OptionStack::getOption(const std::wstring& key)
{
	assertNonEmpty();
	if (options.back().count(key)>0)
		return options.back()[key];
	return L"";
}


void OptionStack::setOption(const std::wstring& key, const std::wstring& val)
{
	assertNonEmpty();
	if (val.empty()) {
		//Remove it if it exists
		if (options.back().count(key)>0)
			options.back().erase(key);
	} else {
		options.back()[key] = val;
	}
}


const std::map<std::wstring, std::wstring>& OptionStack::getOptions()
{
	assertNonEmpty();
	return options.back();
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


