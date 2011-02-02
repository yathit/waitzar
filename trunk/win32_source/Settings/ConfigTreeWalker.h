/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once

#include <string>
#include "Settings/ConfigTreeContainers.h"
#include "Settings/TransformNode.h"
#include "Settings/CfgPerm.h"
#include "Settings/StringNode.h"


/**
 * Simple class for getting a tree walker for our config tree
 *   This is essentially our parser; anything here is used to convert
 *   our tree of Simple Nodes to one of Ghost Nodes.
 * I also didn't want a giant chunk of loader code in ConfigManager.cpp
 */
class ConfigTreeWalker {
private:
	//Data
	static TransformNode verifyTree;

	//Helper method
	template <typename T>
	static T& AddOrCh(std::map<std::wstring, T>& existing, const StringNode& node, bool addAllowed, bool chgAllowed);

public:
	//Singleton get!
	static TransformNode& GetWalkerRoot() {
		if (ConfigTreeWalker::verifyTree.isLeaf()) {
			buildVerifyTree();
		}
		return ConfigTreeWalker::verifyTree;
	}

private:
	//Primary functionality
	static void buildVerifyTree();
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
