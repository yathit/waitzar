/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "GenerativeLookup.h"


using std::wstring;
using std::string;
using std::vector;
using std::map;


namespace waitzar
{

GenerativeLookup::GenerativeLookup(const string& onsetsJson, const string& rhymesJson, const string& specialsJson, bool isStream)
{
	//First, optionally convert our files into actual streams
	string onsetBuffer   = isStream ? onsetsJson   : waitzar::ReadBinaryFile(onsetsJson);
	string rhymeBuffer   = isStream ? rhymesJson   : waitzar::ReadBinaryFile(rhymesJson);
	string specialBuffer = isStream ? specialsJson : waitzar::ReadBinaryFile(specialsJson);

	//Create, read, save our onsets and rhymes.
	Json::Value onsetRoot;
	Json::Value rhymeRoot;
	Json::Value specialRoot;
	{
		Json::Reader reader;
		if (!(reader.parse(onsetBuffer, onsetRoot) && reader.parse(rhymeBuffer, rhymeRoot) && reader.parse(specialBuffer, specialRoot)))
			throw std::runtime_error("Generative: onsets, rhymes, or specials contains parse errors --this should never happen in release mode.");
	}

	//Read into each map, convert to wstrings
	{
		onsetPairs.clear();
		Json::Value::Members keys = onsetRoot.getMemberNames();
		for (auto it=keys.begin(); it!=keys.end(); it++) {
			onsetPairs[waitzar::mbs2wcs(*it)] = waitzar::mbs2wcs(onsetRoot[*it].asString());
		}
	}
	{
		rhymePairs.clear();
		Json::Value::Members keys = rhymeRoot.getMemberNames();
		for (auto it=keys.begin(); it!=keys.end(); it++) {
			rhymePairs[waitzar::mbs2wcs(*it)] = waitzar::mbs2wcs(rhymeRoot[*it].asString());
		}
	}
	{
		specialWords.clear();
		Json::Value::Members keys = specialRoot.getMemberNames();
		for (auto it=keys.begin(); it!=keys.end(); it++) {
			specialWords[waitzar::mbs2wcs(*it)] = waitzar::mbs2wcs(specialRoot[*it].asString());
		}
	}
}


bool GenerativeLookup::continueLookup(const string& roman)
{
	for (auto ch=roman.begin(); ch!=roman.end(); ch++) {
		string candidateRoman = typedRoman + string(1, *ch);
		vector<wstring> candidateMatch = regenerateWordlist(candidateRoman);
		if (candidateMatch.empty())
			return false;

		//Save
		typedRoman = candidateRoman;
		cacheDirty = true;  //We might be able to save "candidateMatch" here... depending on how we organize our cache.
	}
}





} //End waitzar namespace



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

