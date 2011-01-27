/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once


#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>


/**
 * A "node" in our javascript tree contains either a
 *   String value or a Map to other nodes via string keys.
 * (The String value is stored in a stack to allow multiple overrides)
 * Note that this class is only included in ConfigManager.h, so we define the
 *   entire thing in the header. (Include guards are there just in case).
 */
class Node {
public:
	//Constructors
	Node() {
	}
	Node(const std::string& val) {
		this->setString(val);
	}
	Node(const char* val) {
		this->setString(val);
	}

	//Check properties about this node
	bool isLeaf() const {
		return !textValue.empty();
	}
	bool isEmpty() const {
		return textValue.empty() && childList.empty();
	}

	//Get/Set string value
	const std::string& getString() const {
		return textValue;
	}
	void setString(const std::string& val) {
		textValue = val;
		assertValid();
	}

	//Get/Set children
	const std::map<std::string, Node>& getChildNodes() const {
		return childList;
	}
	void addChild(const std::string& key, const Node& val) {
		childList[key] = val;
		assertValid();
	}

	//Used to access child elements
	Node& operator[] (const std::string& key) {
		if (childList.count(key)>0)
			return childList[key];
		throw std::runtime_error((std::string("Node contains no key: ")+key).c_str());
    }


private:
	//Data
	std::map<std::string, Node> childList;
	std::string textValue;

	//Helper: throw an exception if we're in an invalid state.
	void assertValid() const {
		if (!textValue.empty() && !childList.empty())
			throw std::runtime_error("Non-leaf node contains value.");
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
