/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once


//We define permissions here too
class CfgPerm {
public:
	bool chgSettings;

	bool chgLanguage;
	bool addLanguage;

	bool chgLangInputMeth;
	bool addLangInputMeth;

	bool chgLangDispMeth;
	bool addLangDispMeth;

	bool chgLangTransform;
	bool addLangTransform;

	bool chgLangEncoding;
	bool addLangEncoding;

	bool chgExtension;
	bool addExtension;

	//All false by default
	CfgPerm(bool chgSettings=false, bool chgLanguage=false, bool addLanguage=false,
			bool chgLangInputMeth=false, bool addLangInputMeth=false,
			bool chgLangDispMeth=false, bool addLangDispMeth=false,
			bool chgLangTransform=false, bool addLangTransform=false,
			bool chgLangEncoding=false, bool addLangEncoding=false,
			bool chgExtension=false, bool addExtension=false) :
		chgSettings(chgSettings), chgLanguage(chgLanguage), addLanguage(addLanguage),
		chgLangInputMeth(chgLangInputMeth), addLangInputMeth(addLangInputMeth),
		chgLangDispMeth(chgLangDispMeth), addLangDispMeth(addLangDispMeth),
		chgLangTransform(chgLangTransform), addLangTransform(addLangTransform),
		chgLangEncoding(chgLangEncoding), addLangEncoding(addLangEncoding),
		chgExtension(chgExtension), addExtension(addExtension)
		{}
};


//The embedded config file and the main (root) config are allowed to make any changes except extensions
class PrimaryCfgPerm : public CfgPerm {
public:
	PrimaryCfgPerm() : CfgPerm(true, true, true,
			true, true, true, true,
			true, true, true, true,
			false, false) {}
};

//The language (and subdir) configs can make most changes, but not settings or extensions
class LangLevelCfgPerm :  public CfgPerm {
public:
	LangLevelCfgPerm() : CfgPerm(false, true, true,
			true, true, true, true,
			true, true, true, true,
			false, false) {}
};

//The extension subdir can only change extensions
class ExtendCfgPerm :  public CfgPerm {
public:
	ExtendCfgPerm() : CfgPerm(false, false, false,
			false, false, false, false,
			false, false, false, false,
			true, true) {}
};

//The local/user config files can change most things, but not add to them.
class UserLocalCfgPerm : public CfgPerm {
public:
	UserLocalCfgPerm() : CfgPerm(true, true, false,
			true, false, true, false,
			true, false, true, false,
			false, false) {}
};


//When we override, we can only affect "settings.*"
class OverrideSettingCfgPerm : public CfgPerm {
public:
	OverrideSettingCfgPerm() : CfgPerm(true, false, false,
			false, false, false, false,
			false, false, false, false,
			false, false) {}
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

