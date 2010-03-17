/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _WZFACTORY
#define _WZFACTORY

//Don't let Visual Studio warn us to use the _s functions
//#define _CRT_SECURE_NO_WARNINGS

//Ironic that adding compliance now causes headaches compiling under VS2003
//#define _CRT_NON_CONFORMING_SWPRINTFS

#include <map>
#include <string>

#include "resource.h"

#include "Input/RomanInputMethod.h"
#include "Display/TtfDisplay.h"
#include "NGram/WordBuilder.h"
#include "NGram/BurglishBuilder.h"
#include "NGram/SentenceList.h"
#include "NGram/wz_utilities.h"
#include "Settings/Language.h"
#include "Transform/Zg2Uni.h"
#include "Transform/Uni2Zg.h"
#include "Transform/Uni2WinInnwa.h"
#include "Transform/Self2Self.h"
#include "Settings/ConfigManager.h"
#include "resource.h"


//Grr... notepad...
const unsigned int UNICOD_BOM = 0xFEFF;
const unsigned int BACKWARDS_BOM = 0xFFFE;


/**
 * Implementation of our factory interface: make input/display managers and transformers on demand
 */
template <class ModelType>
class WZFactory
{
public:
	WZFactory(void);
	~WZFactory(void);

	//Builders
	static InputMethod* makeInputMethod(const std::wstring& id, const Language& language, const std::map<std::wstring, std::wstring>& options);
	static Encoding makeEncoding(const std::wstring& id, const std::map<std::wstring, std::wstring>& options);
	static DisplayMethod* makeDisplayMethod(const std::wstring& id, const Language& language, const std::map<std::wstring, std::wstring>& options);
	static Transformation* makeTransformation(const std::wstring& id, const std::map<std::wstring, std::wstring>& options);

	//More specific builders/instances
	static RomanInputMethod<waitzar::WordBuilder>* getWaitZarInput(std::wstring langID);
	static RomanInputMethod<waitzar::BurglishBuilder>* getBurglishInput(std::wstring langID);
	static RomanInputMethod<waitzar::WordBuilder>* getWordlistBasedInput(std::wstring langID, std::wstring inputID, std::string wordlistFileName);

	//Display method builders
	static DisplayMethod* getZawgyiPngDisplay(std::wstring langID, std::wstring dispID, unsigned int dispResourceID);
	static DisplayMethod* getPadaukZawgyiTtfDisplay(std::wstring langID, std::wstring dispID);
	static DisplayMethod* getTtfDisplayManager(std::wstring langID, std::wstring dispID, std::wstring fontFileName, std::wstring fontFaceName, int pointSize);
	static DisplayMethod* getPngDisplayManager(std::wstring langID, std::wstring dispID, std::wstring fontFileName);

	//Init; load all special builders at least once
	static void InitAll(HINSTANCE& hInst, MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, OnscreenKeyboard* helpKeyboard);

	//Ugh
	static std::wstring sanitize_id(const std::wstring& str);
	static bool read_bool(const std::wstring& str);
	static int read_int(const std::wstring& str);

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

	//Instance Mappings, to save memory
	//Ugh.... templates are exploding!
	static std::map<std::wstring, RomanInputMethod<waitzar::WordBuilder>*> cachedWBInputs;
	static std::map<std::wstring, RomanInputMethod<waitzar::BurglishBuilder>*> cachedBGInputs;
	static std::map<std::wstring, DisplayMethod*> cachedDisplayMethods;

	//Helper methods
	static void buildSystemWordLookup();
	static waitzar::WordBuilder* readModel();
	static void addWordsToModel(waitzar::WordBuilder* model, std::string userWordsFileName);
};


using std::map;
using std::wstring;
using std::string;
using std::vector;
using std::pair;
using waitzar::WordBuilder;
using waitzar::SentenceList;


template <class ModelType>
WZFactory<ModelType>::WZFactory(void)
{
}


template <class ModelType>
WZFactory<ModelType>::~WZFactory(void)
{
}


//Static initializations
template <class ModelType> HINSTANCE WZFactory<ModelType>::hInst = HINSTANCE();
template <class ModelType> MyWin32Window* WZFactory<ModelType>::mainWindow = NULL;
template <class ModelType> MyWin32Window* WZFactory<ModelType>::sentenceWindow = NULL;
template <class ModelType> MyWin32Window* WZFactory<ModelType>::helpWindow = NULL;
template <class ModelType> MyWin32Window* WZFactory<ModelType>::memoryWindow = NULL;
template <class ModelType> OnscreenKeyboard* WZFactory<ModelType>::helpKeyboard = NULL;
template <class ModelType> std::vector< std::pair <int, unsigned short> > WZFactory<ModelType>::systemWordLookup = std::vector< std::pair <int, unsigned short> >();

//A few more static initializers
template <class ModelType> const std::wstring WZFactory<ModelType>::systemDefinedWords = L"`~!@#$%^&*()-_=+[{]}\\|;:'\"<>/? 1234567890";
template <class ModelType> const int WZFactory<ModelType>::systemDefinedKeys[] = {HOTKEY_COMBINE, HOTKEY_SHIFT_COMBINE, HOTKEY_SHIFT_1, HOTKEY_SHIFT_2, HOTKEY_SHIFT_3, 
		HOTKEY_SHIFT_4, HOTKEY_SHIFT_5, HOTKEY_SHIFT_6, HOTKEY_SHIFT_7, HOTKEY_SHIFT_8, HOTKEY_SHIFT_9, HOTKEY_SHIFT_0, 
		HOTKEY_MINUS, HOTKEY_SHIFT_MINUS, HOTKEY_EQUALS, HOTKEY_SHIFT_EQUALS, HOTKEY_LEFT_BRACKET, 
		HOTKEY_SHIFT_LEFT_BRACKET, HOTKEY_RIGHT_BRACKET, HOTKEY_SHIFT_RIGHT_BRACKET, HOTKEY_SEMICOLON, 
		HOTKEY_SHIFT_SEMICOLON, HOTKEY_APOSTROPHE, HOTKEY_SHIFT_APOSTROPHE, HOTKEY_BACKSLASH, HOTKEY_SHIFT_BACKSLASH, 
		HOTKEY_SHIFT_COMMA, HOTKEY_SHIFT_PERIOD, HOTKEY_FORWARDSLASH, HOTKEY_SHIFT_FORWARDSLASH, HOTKEY_SHIFT_SPACE, 
		HOTKEY_1, HOTKEY_2, HOTKEY_3, HOTKEY_4, HOTKEY_5, HOTKEY_6, HOTKEY_7, HOTKEY_8, HOTKEY_9, HOTKEY_0};


//Build our system lookup
template <class ModelType>
void WZFactory<ModelType>::buildSystemWordLookup()
{
	//Check
	if (WZFactory<ModelType>::systemDefinedWords.length() != sizeof(systemDefinedKeys)/sizeof(int))
		throw std::exception("System words arrays of mismatched size.");

	//Build our reverse lookup.
	for (size_t i=0; i<WZFactory<ModelType>::systemDefinedWords.size(); i++) {
		int hotkey_id = WZFactory<ModelType>::systemDefinedKeys[i];
		WZFactory<ModelType>::systemWordLookup.push_back(pair<int, unsigned short>(hotkey_id, i));
	}
}



/**
 * Load the Wait Zar language model.
 */
template <class ModelType>
WordBuilder* WZFactory<ModelType>::readModel() {
	//Load our embedded resource, the WaitZar model
	HGLOBAL     res_handle = NULL;
	HRSRC       res;
    char *      res_data;
    DWORD       res_size;

	WordBuilder* model = NULL;
	{
	//Load the resource as a byte array and get its size, etc.
	res = FindResource(hInst, MAKEINTRESOURCE(WZ_MODEL), _T("Model"));
	if (!res)
		throw std::exception("Couldn't find WZ_MODEL");
	res_handle = LoadResource(NULL, res);
	if (!res_handle)
		throw std::exception("Couldn't get a handle on WZ_MODEL");
	res_data = (char*)LockResource(res_handle);
	res_size = SizeofResource(NULL, res);

	//Save our "model"
	model = new WordBuilder(res_data, res_size, false);

	//Done - This shouldn't matter, though, since the process only
	//       accesses it once and, fortunately, this is not an external file.
	UnlockResource(res_handle);
	}

	{
	//We also need to load our easy pat-sint combinations
	//Load the resource as a byte array and get its size, etc.
	res = FindResource(hInst, MAKEINTRESOURCE(WZ_EASYPS), _T("Model"));
	if (!res)
		throw std::exception("Couldn't find WZ_EASYPS");
	res_handle = LoadResource(NULL, res);
	if (!res_handle)
		throw std::exception("Couldn't get a handle on WZ_EASYPS");
	res_data = (char*)LockResource(res_handle);
	res_size = SizeofResource(NULL, res);

	//We, unfortunately, have to convert this to unicode now...
	wchar_t *uniData = new wchar_t[res_size];
	waitzar::mymbstowcs(uniData, res_data, res_size);
	DWORD uniSize = wcslen(uniData);

	//Now, read through each line and add it to the external words list.
	wchar_t pre[200];
	wchar_t curr[200];
	wchar_t post[200];
	size_t index = 0;

	while(index<uniSize) {
		//Left-trim
		while (uniData[index] == ' ')
			index++;

		//Comment? Empty line? If so, skip...
		if (uniData[index]=='#' || uniData[index]=='\n') {
			while (uniData[index] != '\n')
				index++;
			index++;
			continue;
		}

		//Init
		pre[0] = 0x0000;
		int pre_pos = 0;
		bool pre_done = false;
		curr[0] = 0x0000;
		int curr_pos = 0;
		bool curr_done = false;
		post[0] = 0x0000;
		int post_pos = 0;

		//Ok, look for pre + curr = post
		while (index<uniSize) {
			if (uniData[index] == '\n') {
				index++;
				break;
			} else if (uniData[index] == '+') {
				//Switch modes
				pre_done = true;
				index++;
			} else if (uniData[index] == '=') {
				//Switch modes
				pre_done = true;
				curr_done = true;
				index++;
			} else if (uniData[index] >= 0x1000 && uniData[index] <= 0x109F) {
				//Add this to the current string
				if (curr_done) {
					post[post_pos++] = uniData[index++];
				} else if (pre_done) {
					curr[curr_pos++] = uniData[index++];
				} else {
					pre[pre_pos++] = uniData[index++];
				}
			} else {
				//Ignore it; avoid weird errors
				index++;
			}
		}

		//Ok, seal the strings
		post[post_pos++] = 0x0000;
		curr[curr_pos++] = 0x0000;
		pre[pre_pos++] = 0x0000;

		//Do we have anything?
		if (wcslen(post)!=0 && wcslen(curr)!=0 && wcslen(pre)!=0) {
			//Ok, process these strings and store them
			if (!model->addShortcut(pre, curr, post)) {
				throw std::exception(waitzar::escape_wstr(model->getLastError(), false).c_str());

				/*if (isLogging) {
					for (size_t q=0; q<model->getLastError().size(); q++)
						fprintf(logFile, "%c", model->getLastError()[q]);
					fprintf(logFile, "\n  pre: ");
					for (unsigned int x=0; x<wcslen(pre); x++)
						fprintf(logFile, "U+%x ", pre[x]);
					fprintf(logFile, "\n");

					fprintf(logFile, "  curr: ");
					for (unsigned int x=0; x<wcslen(curr); x++)
						fprintf(logFile, "U+%x ", curr[x]);
					fprintf(logFile, "\n");

					fprintf(logFile, "  post: ");
					for (unsigned int x=0; x<wcslen(post); x++)
						fprintf(logFile, "U+%x ", post[x]);
					fprintf(logFile, "\n\n");
				}*/
			}
		}
	}

	//Free memory
	delete [] uniData;

	//Done - This shouldn't matter, though, since the process only
	//       accesses it once and, fortunately, this is not an external file.
	UnlockResource(res_handle);
	}

	return model;
}


template <class ModelType>
void WZFactory<ModelType>::addWordsToModel(WordBuilder* model, string userWordsFileName) {
	//Read our words file, if it exists.
	FILE* userFile = fopen(userWordsFileName.c_str(), "r");
	if (userFile == NULL)
		return;

	//Get file size
	fseek (userFile, 0, SEEK_END);
	long fileSize = ftell(userFile);
	rewind(userFile);

	//Read it all into an array, close the file.
	char* buffer = (char*) malloc(sizeof(char)*fileSize);
	size_t buff_size = fread(buffer, 1, fileSize, userFile);
	fclose(userFile);
	if (buff_size==0)
		return; //Empty file.

	//Finally, convert this array to unicode
	//TODO: Replace with our own custom function (which we trust more)
	TCHAR * uniBuffer;
	size_t numUniChars = MultiByteToWideChar(CP_UTF8, 0, buffer, (int)buff_size, NULL, 0);
	uniBuffer = (TCHAR*) malloc(sizeof(TCHAR)*numUniChars);
	if (!MultiByteToWideChar(CP_UTF8, 0, buffer, (int)buff_size, uniBuffer, (int)numUniChars))
		throw std::exception("mywords.txt contains invalid UTF-8 characters.");
	delete [] buffer;

	//Skip the BOM, if it exists
	size_t currPosition = 0;
	if (uniBuffer[currPosition] == UNICOD_BOM)
		currPosition++;
	else if (uniBuffer[currPosition] == BACKWARDS_BOM)
		throw std::exception("mywords.txt appears to be encoded backwards.");

	//Read each line
	wchar_t* name = new wchar_t[100];
	char* value = new char[100];
	while (currPosition<numUniChars) {
		//Get the name/value pair using our nifty template function....
		waitzar::readLine(uniBuffer, currPosition, numUniChars, true, true, false, model->isAllowNonBurmese(), true, false, false, false, name, value);

		//Make sure both name and value are non-empty
		if (strlen(value)==0 || lstrlen(name)==0)
			continue;

		//Add this romanization
		if (!model->addRomanization(name, value, true))
			throw std::exception(string(string("Error adding romanisation: ") + waitzar::escape_wstr(model->getLastError(), false)).c_str());
	}

	//Reclaim memory
	delete [] uniBuffer;
	delete [] name;
	delete [] value;
}



template <class ModelType> std::map<std::wstring, RomanInputMethod<waitzar::WordBuilder>*> WZFactory<ModelType>::cachedWBInputs;
template <class ModelType> std::map<std::wstring, RomanInputMethod<waitzar::BurglishBuilder>*> WZFactory<ModelType>::cachedBGInputs;
template <class ModelType> std::map<std::wstring, DisplayMethod*> WZFactory<ModelType>::cachedDisplayMethods;

template <class ModelType>
RomanInputMethod<waitzar::WordBuilder>* WZFactory<ModelType>::getWaitZarInput(wstring langID) 
{
	wstring fullID = langID + L"." + L"waitzar";

	//Singleton init
	if (WZFactory<ModelType>::cachedWBInputs.count(fullID)==0) {
		//Load model; create sentence list
		//NOTE: These resources will not be reclaimed, but since they're 
		//      contained within a singleton class, I don't see a problem.
		WordBuilder* model = WZFactory<ModelType>::readModel();
		SentenceList<waitzar::WordBuilder>* sentence = new SentenceList<waitzar::WordBuilder>();

		//Add user words
		WZFactory<ModelType>::addWordsToModel(model, "mywords.txt");

		//One final check	
		if (model->isInError())
			throw std::exception(waitzar::escape_wstr(model->getLastError(), false).c_str());

		//Should probably build the reverse lookup now
		model->reverseLookupWord(0);

		//Create, init
		WZFactory<ModelType>::cachedWBInputs[fullID] = new RomanInputMethod<waitzar::WordBuilder>();
		WZFactory<ModelType>::cachedWBInputs[fullID]->init(WZFactory<ModelType>::mainWindow, WZFactory<ModelType>::sentenceWindow, WZFactory<ModelType>::helpWindow, WZFactory<ModelType>::memoryWindow, WZFactory<ModelType>::systemWordLookup, WZFactory<ModelType>::helpKeyboard, WZFactory<ModelType>::systemDefinedWords, model, sentence);
	}
	
	return WZFactory<ModelType>::cachedWBInputs[fullID];
}


template <class ModelType>
RomanInputMethod<waitzar::BurglishBuilder>* WZFactory<ModelType>::getBurglishInput(wstring langID) 
{
	wstring fullID = langID + L"." + L"burglish";

	//Singleton init
	if (WZFactory<ModelType>::cachedBGInputs.count(fullID)==0) {
		//Load model; create sentence list
		//NOTE: These resources will not be reclaimed, but since they're 
		//      contained within a singleton class, I don't see a problem.
		waitzar::BurglishBuilder* model = new waitzar::BurglishBuilder();
		SentenceList<waitzar::BurglishBuilder>* sentence = new SentenceList<waitzar::BurglishBuilder>();

		//Create, init
		WZFactory<ModelType>::cachedBGInputs[fullID] = new RomanInputMethod<waitzar::BurglishBuilder>();
		WZFactory<ModelType>::cachedBGInputs[fullID]->init(WZFactory<ModelType>::mainWindow, WZFactory<ModelType>::sentenceWindow, WZFactory<ModelType>::helpWindow, WZFactory<ModelType>::memoryWindow, WZFactory<ModelType>::systemWordLookup, WZFactory<ModelType>::helpKeyboard, WZFactory<ModelType>::systemDefinedWords, model, sentence);
	}
	
	return WZFactory<ModelType>::cachedBGInputs[fullID];
}


//Build a model up from scratch.
template <class ModelType>
RomanInputMethod<WordBuilder>* WZFactory<ModelType>::getWordlistBasedInput(wstring langID, wstring inputID, string wordlistFileName)
{
	wstring fullID = langID + L"." + inputID;

	if (WZFactory<ModelType>::cachedWBInputs.count(fullID)==0) {
		//Create a basically empty model (Nexus can't be empty)
		vector< vector<unsigned int> > nexus;
		nexus.push_back(vector<unsigned int>());
		WordBuilder* model = new WordBuilder(vector<wstring>(), nexus, vector< vector<unsigned int> >());
		SentenceList<waitzar::WordBuilder>* sentence = new SentenceList<waitzar::WordBuilder>();

		//Add user words (there's ONLY user words here)
		WZFactory<ModelType>::addWordsToModel(model, wordlistFileName);

		//Should probably build the reverse lookup now
		model->reverseLookupWord(0);

		//One final check
		if (model->isInError())
			throw std::exception(waitzar::escape_wstr(model->getLastError(), false).c_str());

		//Now, build the romanisation method and return
		WZFactory<ModelType>::cachedWBInputs[fullID] = new RomanInputMethod<waitzar::WordBuilder>();
		WZFactory<ModelType>::cachedWBInputs[fullID]->init(WZFactory<ModelType>::mainWindow, WZFactory<ModelType>::sentenceWindow, WZFactory<ModelType>::helpWindow, WZFactory<ModelType>::memoryWindow, WZFactory<ModelType>::systemWordLookup, WZFactory<ModelType>::helpKeyboard, WZFactory<ModelType>::systemDefinedWords, model, sentence);
	}

	return WZFactory<ModelType>::cachedWBInputs[fullID];
}



template <class ModelType>
DisplayMethod* WZFactory<ModelType>::getZawgyiPngDisplay(std::wstring langID, std::wstring dispID, unsigned int dispResourceID)
{
	wstring fullID = langID + L"." + dispID;

	//Singleton init
	if (WZFactory<ModelType>::cachedDisplayMethods.count(fullID)==0) {
		//Load our internal font
		HRSRC fontRes = FindResource(hInst, MAKEINTRESOURCE(dispResourceID), _T("COREFONT"));
		if (!fontRes)
			throw std::exception("Couldn't find WZ_FONT");

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(NULL, fontRes);
		if (!res_handle)
			throw std::exception("Couldn't get a handle on WZ_(SMALL?)_FONT");

		//Create, init
		WZFactory<ModelType>::cachedDisplayMethods[fullID] = new PulpCoreFont();
		mainWindow->initDisplayMethod(WZFactory<ModelType>::cachedDisplayMethods[fullID], fontRes, res_handle, 0x000000);
	}
	
	return WZFactory<ModelType>::cachedDisplayMethods[fullID];
}



template <class ModelType>
DisplayMethod* WZFactory<ModelType>::getPadaukZawgyiTtfDisplay(std::wstring langID, std::wstring dispID)
{
	wstring fullID = langID + L"." + dispID;

	//Singleton init
	if (WZFactory<ModelType>::cachedDisplayMethods.count(fullID)==0) {
		//Init our internal font
		TtfDisplay* res = new TtfDisplay();
		res->fontFaceName = L"PdkZgWz";
		res->pointSize = 10;

		//Get the Padauk embedded resource
		HRSRC fontRes = FindResource(hInst, MAKEINTRESOURCE(WZ_PADAUK_ZG), _T("COREFONT"));
		if (!fontRes)
			throw std::exception("Couldn't find WZ_PADAUK_ZG");
		HGLOBAL res_handle = LoadResource(NULL, fontRes);
		if (!res_handle)
			throw std::exception("Couldn't get a handle on WZ_PADAUK_ZG");

		//Save, init
		WZFactory<ModelType>::cachedDisplayMethods[fullID] = res;
		mainWindow->initTtfMethod(WZFactory<ModelType>::cachedDisplayMethods[fullID], fontRes, res_handle, 0x000000);
	}
	
	return WZFactory<ModelType>::cachedDisplayMethods[fullID];
}




template <class ModelType>
DisplayMethod* WZFactory<ModelType>::getTtfDisplayManager(std::wstring langID, std::wstring dispID, std::wstring fontFileName, std::wstring fontFaceName, int pointSize)
{
	wstring fullID = langID + L"." + dispID;

	//Singleton init
	if (WZFactory<ModelType>::cachedDisplayMethods.count(fullID)==0) {
		//Init our internal font
		TtfDisplay* res = new TtfDisplay();
		res->fontFaceName = fontFaceName;
		res->pointSize = pointSize;

		//Save, init
		WZFactory<ModelType>::cachedDisplayMethods[fullID] = res;
		mainWindow->initTtfMethod(WZFactory<ModelType>::cachedDisplayMethods[fullID], fontFileName, 0x000000);
	}
	
	return WZFactory<ModelType>::cachedDisplayMethods[fullID];
}


template <class ModelType>
DisplayMethod* WZFactory<ModelType>::getPngDisplayManager(std::wstring langID, std::wstring dispID, std::wstring fontFileName)
{
	wstring fullID = langID + L"." + dispID;

	//Singleton init
	if (WZFactory<ModelType>::cachedDisplayMethods.count(fullID)==0) {
		//Does it end in ".png"
		if (fontFileName.length()<5 || fontFileName[fontFileName.length()-4]!=L'.' || fontFileName[fontFileName.length()-3]!=L'p' || fontFileName[fontFileName.length()-2]!=L'n' || fontFileName[fontFileName.length()-1]!=L'g')
			throw std::exception("PngFont file does not end in \".png\"");

		//Open it
		FILE* fontFile = fopen(waitzar::escape_wstr(fontFileName, false).c_str(), "rb");
		if (fontFile == NULL)
			throw std::exception("PngFont file could not be opened for reading.");

		//Try to read its data into a char[] array; get the size, too
		fseek (fontFile, 0, SEEK_END);
		long fileSize = ftell(fontFile);
		rewind(fontFile);
		char * file_buff = new char[fileSize];
		size_t file_buff_size = fread(file_buff, 1, fileSize, fontFile);
		fclose(fontFile);
		if (file_buff_size!=fileSize)
			throw std::exception("PngFont file could not be read to the end of the file.");
		
		//Ok, load our font
		PulpCoreFont* res = new PulpCoreFont();
		try {
			mainWindow->initDisplayMethod(res, file_buff, file_buff_size, 0x000000);
		} catch (std::exception ex) {
			//Is our font in error? If so, load the embedded font
			std::stringstream msg;
			msg <<"Custom font didn't load correctly: " <<ex.what();
			delete res;
			throw std::exception(msg.str().c_str());
		}

		//Save
		WZFactory<ModelType>::cachedDisplayMethods[fullID] = res;
	}
	
	return WZFactory<ModelType>::cachedDisplayMethods[fullID];
}



template <class ModelType>
void WZFactory<ModelType>::InitAll(HINSTANCE& hInst, MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, OnscreenKeyboard* helpKeyboard)
{
	//Save
	WZFactory<ModelType>::hInst = hInst;
	WZFactory<ModelType>::mainWindow = mainWindow;
	WZFactory<ModelType>::sentenceWindow = sentenceWindow;
	WZFactory<ModelType>::helpWindow = helpWindow;
	WZFactory<ModelType>::memoryWindow = memoryWindow;
	WZFactory<ModelType>::helpKeyboard = helpKeyboard;

	//Call all singleton classes
	//TODO: Is this needed here?
	//WZFactory<ModelType>::getWaitZarInput();
}


template <class ModelType>
InputMethod* WZFactory<ModelType>::makeInputMethod(const std::wstring& id, const Language& language, const std::map<std::wstring, std::wstring>& options)
{
	InputMethod* res = NULL;

	//Check some required settings 
	if (options.count(L"type")==0)
		throw std::exception("Cannot construct input manager: no \"type\"");
	if (options.count(L"encoding")==0)
		throw std::exception("Cannot construct input manager: no \"encoding\"");
	if (options.count(sanitize_id(L"display-name"))==0)
		throw std::exception("Cannot construct input manager: no \"display-name\"");

	//First, generate an actual object, based on the type.
	if (sanitize_id(options.find(L"type")->second) == L"builtin") {
		//Built-in types are known entirely by our core code
		if (id==L"waitzar")
			res = WZFactory<ModelType>::getWaitZarInput(language.id);
		else if (id==L"mywin")
			res = WZFactory<ModelType>::getWaitZarInput(language.id);
		else if (id==L"burglish")
			res = WZFactory<ModelType>::getBurglishInput(language.id);
		else
			throw std::exception(ConfigManager::glue(L"Invalid \"builtin\" Input Manager: ", id).c_str());
		res->type = BUILTIN;
	} else if (sanitize_id(options.find(L"type")->second) == L"roman") {
		//These use a wordlist, for now.
		if (options.count(L"wordlist")==0)
			throw std::exception("Cannot construct \"roman\" input manager: no \"wordlist\".");
		if (options.count(sanitize_id(L"current-folder"))==0)
			throw std::exception("Cannot construct \"roman\" input manager: no \"current-folder\" (should be auto-added).");

		//Build the wordlist file; we will need the respective directory, too.
		std::wstring langConfigDir = options.find(sanitize_id(L"current-folder"))->second;
		std::wstring wordlistFile = langConfigDir + options.find(L"wordlist")->second;

		//TODO: Test in a cleaner way.
		WIN32_FILE_ATTRIBUTE_DATA InfoFile;
		std::wstringstream temp;
		temp << wordlistFile.c_str();
		if (GetFileAttributesEx(temp.str().c_str(), GetFileExInfoStandard, &InfoFile)==FALSE)
			throw std::exception(ConfigManager::glue(L"Wordlist file does not exist: ", wordlistFile).c_str());

		//Get it, as a singleton
		res = WZFactory<ModelType>::getWordlistBasedInput(language.id, id, waitzar::escape_wstr(wordlistFile, false));
		res->type = IME_ROMAN;
	} else {
		throw std::exception(ConfigManager::glue(L"Invalid type (",options.find(L"type")->second, L") for Input Manager: ", id).c_str());
	}

	//Now, add general settings
	res->id = id;
	res->displayName = options.find(sanitize_id(L"display-name"))->second;
	res->encoding.id = sanitize_id(options.find(L"encoding")->second);

	res->suppressUppercase = true;
	if (options.count(sanitize_id(L"suppress-uppercase"))>0)
		res->suppressUppercase = read_bool(options.find(sanitize_id(L"suppress-uppercase"))->second);

	//Extra for RomanInputMethod
	res->controlKeyStyle = CK_CHINESE;
	if (options.count(sanitize_id(L"control-keys"))>0) {
		std::wstring prop = options.find(sanitize_id(L"control-keys"))->second;
		if (prop == L"chinese")
			res->controlKeyStyle = CK_CHINESE;
		else if (prop == L"japanese")
			res->controlKeyStyle = CK_JAPANESE;
		else
			throw std::exception("Invalid option for \"control keys\"");
	}

	//Also
	res->typeNumeralConglomerates = false;
	if (options.count(sanitize_id(L"numeral-conglomerate"))>0)
		res->typeNumeralConglomerates = read_bool(options.find(sanitize_id(L"numeral-conglomerate"))->second);

	//Return our resultant IM
	return res;
}


template <class ModelType>
DisplayMethod* WZFactory<ModelType>::makeDisplayMethod(const std::wstring& id, const Language& language, const std::map<std::wstring, std::wstring>& options)
{
	DisplayMethod* res = NULL;

	//Check some required settings 
	if (options.count(L"type")==0)
		throw std::exception("Cannot construct display method: no \"type\"");
	if (options.count(L"encoding")==0)
		throw std::exception("Cannot construct display method: no \"encoding\"");

	//First, generate an actual object, based on the type.
	if (sanitize_id(options.find(L"type")->second) == L"builtin") {
		//Built-in types are known entirely by our core code
		if (id==L"zawgyibmp")
			res = WZFactory<ModelType>::getZawgyiPngDisplay(language.id, id, WZ_FONT);
		else if (id==L"zawgyibmpsmall")
			res = WZFactory<ModelType>::getZawgyiPngDisplay(language.id, id, WZ_SMALL_FONT);
		else if (id==L"pdkzgwz")
			res = WZFactory<ModelType>::getPadaukZawgyiTtfDisplay(language.id, id);
		else
			throw std::exception(ConfigManager::glue(L"Invalid \"builtin\" Display Method: ", id).c_str());
		res->type = BUILTIN;
	} else if (sanitize_id(options.find(L"type")->second) == L"ttf") {
		//Enforce that a font-face-name is given
		if (options.count(sanitize_id(L"font-face-name"))==0)
			throw std::exception(ConfigManager::glue(L"Cannot construct \"ttf\" display method: no \"font-face-name\" for: ", id).c_str());
		std::wstring fontFaceName = options.find(sanitize_id(L"font-face-name"))->second;

		//Enforce that a point-size is given
		if (options.count(sanitize_id(L"point-size"))==0)
			throw std::exception("Cannot construct \"ttf\" display method: no \"point-size\".");
		int pointSize = read_int(options.find(sanitize_id(L"point-size"))->second);

		if (options.count(sanitize_id(L"current-folder"))==0)
			throw std::exception("Cannot construct \"roman\" input manager: no \"current-folder\" (should be auto-added).");
		
		//Get the local directory; add the font-file:
		std::wstring langConfigDir = options.find(sanitize_id(L"current-folder"))->second;
		std::wstring fontFile = (options.count(L"font-file")>0) ? langConfigDir + options.find(L"font-file")->second : L"";

		//TODO: Test in a cleaner way.
		if (!fontFile.empty()) {
			WIN32_FILE_ATTRIBUTE_DATA InfoFile;
			std::wstringstream temp;
			temp << fontFile.c_str();
			if (GetFileAttributesEx(temp.str().c_str(), GetFileExInfoStandard, &InfoFile)==FALSE)
				throw std::exception(ConfigManager::glue(L"Font file file does not exist: ", fontFile).c_str());
		}

		//Get it, as a singleton
		res = WZFactory<ModelType>::getTtfDisplayManager(language.id, id, fontFile, fontFaceName, pointSize);
		res->type = DISPM_TTF;
	} else if (sanitize_id(options.find(L"type")->second) == L"png") {
		//Enforce that a font-file is given
		if (options.count(sanitize_id(L"font-file"))==0)
			throw std::exception("Cannot construct \"png\" display method: no \"font-file\".");
		std::wstring fontFile = options.find(sanitize_id(L"font-file"))->second;

		if (options.count(sanitize_id(L"current-folder"))==0)
			throw std::exception("Cannot construct \"roman\" input manager: no \"current-folder\" (should be auto-added).");
		
		//Get the local directory; add the font-file:
		std::wstring langConfigDir = options.find(sanitize_id(L"current-folder"))->second;
		fontFile = langConfigDir + fontFile;

		//TODO: Test in a cleaner way.
		WIN32_FILE_ATTRIBUTE_DATA InfoFile;
		std::wstringstream temp;
		temp << fontFile.c_str();
		if (GetFileAttributesEx(temp.str().c_str(), GetFileExInfoStandard, &InfoFile)==FALSE)
			throw std::exception(ConfigManager::glue(L"Font file file does not exist: ", fontFile).c_str());

		//Get it, as a singleton
		res = WZFactory<ModelType>::getPngDisplayManager(language.id, id, fontFile);
		res->type = DISPM_PNG;
	} else {
		throw std::exception(ConfigManager::glue(L"Invalid type (",options.find(L"type")->second, L") for Display Method: ", id).c_str());
	}

	//Now, add general settings
	res->id = id;
	res->encoding.id = sanitize_id(options.find(L"encoding")->second);

	//Return our resultant DM
	return res;
}


template <class ModelType>
Transformation* WZFactory<ModelType>::makeTransformation(const std::wstring& id, const std::map<std::wstring, std::wstring>& options)
{
	Transformation* res = NULL;

	//Check some required settings 
	if (options.count(L"type")==0)
		throw std::exception("Cannot construct transformation: no \"type\"");
	if (options.count(sanitize_id(L"from-encoding"))==0)
		throw std::exception("Cannot construct transformation: no \"from-encoding\"");
	if (options.count(sanitize_id(L"to-encoding"))==0)
		throw std::exception("Cannot construct transformation: no \"to-encoding\"");

	//First, generate an actual object, based on the type.
	if (sanitize_id(options.find(L"type")->second) == L"builtin") {
		//Built-in types are known entirely by our core code
		if (id==L"uni2zg")
			res = new Uni2Zg();
		else if (id==L"uni2wi")
			res = new Uni2WinInnwa();
		else if (id==L"zg2uni")
			res = new Zg2Uni();
		else
			throw std::exception(ConfigManager::glue(L"Invalid \"builtin\" Transformation: ", id).c_str());
		res->type = BUILTIN;
	} else {
		throw std::exception(ConfigManager::glue(L"Invalid type (",options.find(L"type")->second, L") for Transformation: ", id).c_str());
	}

	//Now, add general settings
	res->id = id;
	res->fromEncoding.id = sanitize_id(options.find(sanitize_id(L"from-encoding"))->second);
	res->toEncoding.id = sanitize_id(options.find(sanitize_id(L"to-encoding"))->second);

	//Optional settings
	res->hasPriority = false;
	if (options.count(sanitize_id(L"has-priority"))>0)
		res->hasPriority = read_bool(options.find(sanitize_id(L"has-priority"))->second);

	//Return our resultant Transformation
	return res;
}


template <class ModelType>
Encoding WZFactory<ModelType>::makeEncoding(const std::wstring& id, const std::map<std::wstring, std::wstring>& options)
{
	Encoding res;

	//Check some required settings 
	if (options.count(sanitize_id(L"display-name"))==0)
		throw std::exception("Cannot construct encoding: no \"display-name\"");

	//General Settings
	res.id = id;
	res.displayName = options.find(sanitize_id(L"display-name"))->second;

	//Optional settings
	res.canUseAsOutput = false;
	if (options.count(L"image")>0)
		res.imagePath = options.find(L"image")->second;
	if (options.count(sanitize_id(L"use-as-output"))>0)
		res.canUseAsOutput = read_bool(options.find(sanitize_id(L"use-as-output"))->second);
	if (options.count(L"initial")>0)
		res.initial = options.find(L"initial")->second;

	//Return our resultant DM
	return res;
}

//Move these later
template <class ModelType>
std::wstring WZFactory<ModelType>::sanitize_id(const std::wstring& str)
{
	return ConfigManager::sanitize_id(str);
}
template <class ModelType>
bool WZFactory<ModelType>::read_bool(const std::wstring& str)
{
	return ConfigManager::read_bool(str);
}
template <class ModelType>
int WZFactory<ModelType>::read_int(const std::wstring& str)
{
	return ConfigManager::read_int(str);
}




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

