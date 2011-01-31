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
#include "Settings/Node.h"


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
	TransformNode(const std::wstring& mp=L"",
			const std::function<TNode& (const Node& src, TNode& dest)>& om=std::function<TNode& (const Node& src, TNode& dest)>()) : matchPattern(mp), OnMatch(om) {
	}

	//Check properties about this node
	bool isLeaf() const {
		return childrenByName.empty();
	}

	//Getters & setters
	const std::function<TNode& (const Node& src, TNode& dest)>&  getMatchAction() const {
		return OnMatch;
	}


	//Get/set children
	const std::map<std::wstring, TransformNode>& getChildNodes() const {
		return childrenByName;
	}
	void addChild(const std::wstring& rkey, const std::function<TNode& (const Node& src, TNode& dest)> onMatch) {
		std::wstring key = (rkey==L"*") ? rkey : waitzar::sanitize_id(rkey);
		if (childrenByName.count(key)>0)
			throw std::runtime_error(waitzar::glue(L"Child already exists for: ", key).c_str());
		childrenByName[key] = TransformNode(key, onMatch);
	}


	//Used to access child elements
	TransformNode& operator[] (const std::wstring& rkey) {
		std::wstring key = (rkey==L"*") ? rkey : waitzar::sanitize_id(rkey);
		if (childrenByName.count(key)>0)
			return childrenByName.find(key)->second;
		else if (childrenByName.count(L"*")>0)
			return childrenByName.find(L"*")->second;
		throw std::runtime_error((std::string("Node contains no key: ")+waitzar::escape_wstr(key)).c_str());
    }
	const TransformNode& operator[] (const std::wstring& rkey) const {
		std::wstring key = (rkey==L"*") ? rkey : waitzar::sanitize_id(rkey);
		if (childrenByName.count(key)>0)
			return childrenByName.find(key)->second;
		else if (childrenByName.count(L"*")>0)
			return childrenByName.find(L"*")->second;
		throw std::runtime_error((std::string("Node contains no key: ")+waitzar::escape_wstr(key)).c_str());
    }


private:
	//Const data
	mutable std::wstring matchPattern; //What we match
	mutable std::function<TNode& (const Node& src, TNode& dest)> OnMatch; //What happens when we match it (return next node)

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
