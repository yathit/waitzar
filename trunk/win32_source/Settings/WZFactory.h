/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _WZFACTORY
#define _WZFACTORY

//Don't let Visual Studio warn us to use the _s functions
#define _CRT_SECURE_NO_WARNINGS

#include <map>
#include <string>

#include "Interfaces.h"
#include "Input/RomanInputMethod.h"
#include "NGram/WordBuilder.h"
#include "NGram/SentenceList.h"
#include "NGram/wz_utilities.h"
#include "Display/PngFont.h"
#include "Transform/Zg2Uni.h"
#include "Transform/Uni2Uni.h"
#include "Settings/ConfigManager.h"
#include "resource.h"


//Grr... notepad...
const unsigned int UNICOD_BOM = 0xFEFF;
const unsigned int BACKWARDS_BOM = 0xFFFE;


/**
 * Implementation of our factory interface: make input/display managers and transformers on demand
 */
class WZFactory
{
public:
	WZFactory(void);
	~WZFactory(void);

	//Builders
	static InputMethod* makeInputMethod(const std::wstring& id, const std::map<std::wstring, std::wstring>& options);
	static Encoding makeEncoding(const std::wstring& id, const std::map<std::wstring, std::wstring>& options);
	static DisplayMethod* makeDisplayMethod(const std::wstring& id, const std::map<std::wstring, std::wstring>& options);
	static Transformation* makeTransformation(const std::wstring& id, const std::map<std::wstring, std::wstring>& options);

	//More specific builders/instances
	static RomanInputMethod* getWaitZarInput();
	static RomanInputMethod* getWordlistBasedInput(std::string wordlistFileName);

	//Init; load all special builders at least once
	static void InitAll(HINSTANCE& hInst, MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, OnscreenKeyboard* helpKeyboard);

	//Ugh
	static std::wstring sanitize_id(const std::wstring& str);
	static bool read_bool(const std::wstring& str);

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

	//Parallel data structures for constructing systemWordLookup
	static const std::wstring systemDefinedWords;
	static const int systemDefinedKeys[];

	//Instances
	static RomanInputMethod* wz_input;

	//Helper methods
	static void buildSystemWordLookup();
	static waitzar::WordBuilder* readModel();
	static void addWordsToModel(waitzar::WordBuilder* model, std::string userWordsFileName);
};


#endif //_WZFACTORY

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

