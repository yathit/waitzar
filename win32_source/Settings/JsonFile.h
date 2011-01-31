/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once


#include <string>

#include "Json CPP/value.h"
#include "Json CPP/reader.h"


/**
 * Simple class to help us load json files easier
 * We define this purely in its header file since ConfigManager is the only
 *   class to use it.
 */
class JsonFile {
public:
	JsonFile(const std::string& path="", bool fileIsStream=false) //Confusing, I know. (TODO: Make a better way of loading a file OR a string)
	{
		if (fileIsStream) {
			//The "path" represents a stream of data
			this->path = "";
			this->folderPath = L"";
			this->text = path;
			this->hasReadFile = true;
			this->hasParsed = false;
		} else {
			//The "path" represents an actual file path
			this->path = path;
			this->folderPath = L"";
			this->text = "";
			this->hasReadFile = false;
			this->hasParsed = false;

			//Set the folder path
			int fwIndex = path.rfind("/");
			int bwIndex = path.rfind("\\");
			int index = std::max<int>(fwIndex, bwIndex);
			if (index!=-1) {
				std::wstringstream temp;
				temp <<path.substr(0, index+1).c_str();
				folderPath = temp.str();
			}
		}
	}
	Json::Value json() const
	{
		if (!hasParsed) {
			if (!this->hasReadFile) {
				text = waitzar::ReadBinaryFile(path);
				this->hasReadFile = true;
			}

			//First, try to just read it. If there's an error, then try "read or throw" and get a better error message.
			Json::Reader reader;
			if (!reader.parse(text, root)) {
				//Now, throw the error.
				std::stringstream errMsg;
				errMsg << "Invalid json config file: " << path;
				errMsg << std::endl << "  Problem: " << reader.getFormatedErrorMessages();
				throw std::runtime_error(errMsg.str().c_str());
			}

			//Save space
			text = "";

			hasParsed = true;
		}
		return root;
	}
	bool isEmpty() const
	{
		return this->path.empty() && this->text.empty();
	}
	bool isSet() const //Should be a better way of automating this... maybe a singleton JSON object to return by default?
	{
		return this->path.length() > 0;
	}
	const std::wstring& getFolderPath() const
	{
		return this->folderPath;
	}
	const std::string& getFilePath() const
	{
		return this->path;
	}
	//For map indexing:
	bool operator<(const JsonFile& j) const
	{
		return this->path < j.path;
	}
private:
	std::string path;
	std::wstring folderPath;
	mutable std::string text;
	mutable Json::Value root;
	mutable bool hasReadFile;
	mutable bool hasParsed;
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
