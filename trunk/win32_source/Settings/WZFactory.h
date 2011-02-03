/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once


#include <map>
#include <string>
#include <stdexcept>

#include "windows_wz.h"
#include "resource.h"

#include "Input/RomanInputMethod.h"
#include "Input/LetterInputMethod.h"
#include "Input/KeyMagicInputMethod.h"
#include "Display/TtfDisplay.h"
#include "NGram/WordBuilder.h"
#include "NGram/BurglishBuilder.h"
#include "NGram/SentenceList.h"
#include "NGram/wz_utilities.h"
#include "Settings/ConfigTreeContainers.h"
#include "Settings/Types.h"
#include "Transform/Zg2Uni.h"
#include "Transform/Uni2Zg.h"
#include "Transform/Uni2Ayar.h"
#include "Transform/Ayar2Uni.h"
#include "Transform/Uni2WinInnwa.h"
#include "Transform/Self2Self.h"
#include "Transform/JSTransform.h"



//Grr... notepad...
const unsigned int UNICOD_BOM = 0xFEFF;
const unsigned int BACKWARDS_BOM = 0xFFFE;


/**
 * Implementation of our factory interface: make input/display managers and transformers on demand
 */
class WZFactory
{
public:
	//New builders
	static Extension* makeAndVerifyExtension(const std::wstring& id, ExtendNode& ex);
	static InputMethod* makeAndVerifyInputMethod(const LangNode& lang, const std::wstring& id, InMethNode& im);
	static DisplayMethod* makeAndVerifyDisplayMethod(const LangNode& lang, const std::wstring& id, DispMethNode& dm);
	static Transformation* makeAndVerifyTransformation(ConfigRoot& conf, const LangNode& lang, const std::wstring& id, TransNode& tm);

	//Used to "verify" things which don't need to be built
	static void verifyEncoding(const std::wstring& id, EncNode& enc);
	static void verifyLanguage(const std::wstring& id, LangNode& lang, const std::wstring& lastUsedInMethID, const std::wstring& lastUsedOutEncID);
	static void verifySettings(ConfigRoot& cfg, SettingsNode& set, const std::wstring& lastUsedLangID);

	//Helper for that crazy Flash-save format
	static std::wstring InterpretFlashSave(const std::map<std::wstring, std::vector<std::wstring> >& flashSave, const std::wstring& langID, size_t resIndex);
	static std::wstring MatchAvoidBackwards(const std::vector<std::wstring>& vec, const std::wstring& avoidStr);

	//More specific builders/instances
	static RomanInputMethod<waitzar::WordBuilder>* getWaitZarInput(std::wstring langID, const std::wstring& extraWordsFileName, const std::wstring& userWordsFileName, InMethNode& node);
	static RomanInputMethod<waitzar::BurglishBuilder>* getBurglishInput(std::wstring langID, InMethNode& node);
	static RomanInputMethod<waitzar::WordBuilder>* getWordlistBasedInput(std::wstring langID, std::wstring inputID, std::string wordlistFileName, InMethNode& node);
	static LetterInputMethod* getKeyMagicBasedInput(std::wstring langID, std::wstring inputID, std::string wordlistFileName, bool disableCache, InMethNode& node);
	static LetterInputMethod* getMywinInput(std::wstring langID, InMethNode& node);

	//Display method builders
	static DisplayMethod* getZawgyiPngDisplay(std::wstring langID, std::wstring dispID, unsigned int dispResourceID, DispMethNode& node);
	static DisplayMethod* getPadaukZawgyiTtfDisplay(std::wstring langID, std::wstring dispID);
	static DisplayMethod* getTtfDisplayManager(std::wstring langID, std::wstring dispID, std::wstring fontFileName, std::wstring fontFaceName, int pointSize);
	static DisplayMethod* getPngDisplayManager(std::wstring langID, std::wstring dispID, std::wstring fontFileName);

	//Init; load all special builders at least once
	static void InitAll(HINSTANCE& hInst, MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, OnscreenKeyboard* helpKeyboard);

	//Parallel data structures for constructing systemWordLookup
	static const std::wstring systemDefinedWords;

	//Helper
	static bool FileExists(const wstring& fileName) {
		WIN32_FILE_ATTRIBUTE_DATA InfoFile;
		return (GetFileAttributesEx(fileName.c_str(), GetFileExInfoStandard, &InfoFile)==TRUE);
	}


private:
	//For loading
	static HINSTANCE hInst;
	static MyWin32Window* mainWindow;
	static MyWin32Window* sentenceWindow;
	static MyWin32Window* helpWindow;
	static MyWin32Window* memoryWindow;
	static OnscreenKeyboard* helpKeyboard;

	//Special "words" used in our keyboard, like "(" and "`"
	static std::vector< std::pair <int, unsigned short> > systemWordLookup;

	//Instance Mappings, to save memory
	//Ugh.... templates are exploding!
	static std::map<std::wstring, RomanInputMethod<waitzar::WordBuilder>*> cachedWBInputs;
	static std::map<std::wstring, RomanInputMethod<waitzar::BurglishBuilder>*> cachedBGInputs;
	static std::map<std::wstring, LetterInputMethod*> cachedLetterInputs;
	static std::map<std::wstring, DisplayMethod*> cachedDisplayMethods;

	//Helper methods
	static void buildSystemWordLookup();
	static waitzar::WordBuilder* readModel();
	static void addWordsToModel(waitzar::WordBuilder* model, std::string userWordsFileName);
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

