/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once

#include <vector>
#include <map>
#include <string>
#include <stdexcept>

/**
 * An "Option Stack" is just like a map, except that it can
 *     "save" and "reload" entire sets of options (by pushing them to a stack)
 */
class OptionStack {
public:
	OptionStack(size_t maxLevels=0) : maxLevels(maxLevels) {
		//Init the first frame
		backup();
	}

	//Stack manipulation
	void backup();
	void restore();
	void persist();

	//Option manipulation
	const std::map<std::wstring, std::wstring>& getOptions();
	std::wstring getOption(const std::wstring& key);
	void setOption(const std::wstring& key, const std::wstring& val);
	void clearOption(const std::wstring& key) {
		setOption(key, L"");
	}

private:
	//Data!
	std::vector<std::map<std::wstring, std::wstring>> options;

	//Used as a sanity check by WZ
	size_t maxLevels;

	//Helper
	void assertNonEmpty() {
		if (options.empty())
			throw std::runtime_error("OptionStack has not been properly initialized.");
	}

};








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


