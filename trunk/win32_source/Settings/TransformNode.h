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

#include "NGram/wz_utilities.h"


/**
 * A "transform node" is used in conjunction with a "node" to
 *   convert the tree of maps and strings into an actual series of
 *   classes. This is just a storage class (hence its header-only implementation);
 *   we expect the ConfigManager to populate a tree of TransformNodes with the
 *   relevant pattern matching devices and lambda functions
 */
class TransformNode {
public:
	//Constructor
	TransformNode(const std::wstring& matchPattern=L"", std::function<void, (Node& src, TNode& dest)>& onMatch) : matchPattern(matchPattern), OnMatch(onMatch) {

	}

	//Check properties about this node
	bool isLeaf() const {
		return 0; //TEMP
	}

	//Getters & setters


private:
	//Const data
	const std::wstring matchPattern; //What we match
	const std::function<void, (Node& src, TNode& dest)> OnMatch; //What happens when we match it

	//Children
	//NOTE: The key "*" is special; it represents "always match", and is used for identifiers
	std::map<std::wstring, TransformNode> childrenByName;
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
