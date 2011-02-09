/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "WZFactory.h"


using std::map;
using std::wstring;
using std::string;
using std::vector;
using std::pair;
using waitzar::WordBuilder;
using waitzar::SentenceList;



//Static initializations
HINSTANCE WZFactory::hInst = HINSTANCE();
MyWin32Window* WZFactory::mainWindow = NULL;
MyWin32Window* WZFactory::sentenceWindow = NULL;
MyWin32Window* WZFactory::helpWindow = NULL;
MyWin32Window* WZFactory::memoryWindow = NULL;
OnscreenKeyboard* WZFactory::helpKeyboard = NULL;
std::vector< std::pair <int, unsigned short> > WZFactory::systemWordLookup = std::vector< std::pair <int, unsigned short> >();


//Build our system lookup
void WZFactory::buildSystemWordLookup()
{
	//Build our reverse lookup.
	for (size_t i=0; i<waitzar::WZSystemDefinedWords.size(); i++) {
		int letter_id = waitzar::WZSystemDefinedWords[i];
		WZFactory::systemWordLookup.push_back(pair<int, unsigned short>(letter_id, i));
	}
}



/**
 * Load the Wait Zar language model.
 */
WordBuilder* WZFactory::readModel() {
	//Load our embedded resource, the WaitZar model
	HGLOBAL     res_handle = NULL;
	HRSRC       res;
    char *      res_data;
    DWORD       res_size;

	WordBuilder* model = NULL;
	{
	//Load the resource as a byte array and get its size, etc.
	res = FindResource(hInst, MAKEINTRESOURCE(IDR_WAITZAR_MODEL), L"Model");
	if (!res)
		throw std::runtime_error("Couldn't find IDR_WAITZAR_MODEL");
	res_handle = LoadResource(NULL, res);
	if (!res_handle)
		throw std::runtime_error("Couldn't get a handle on IDR_WAITZAR_MODEL");
	res_data = (char*)LockResource(res_handle);
	res_size = SizeofResource(NULL, res);

	//Save our "model"
	model = new WordBuilder(res_data, res_size, false);

	}

	{
	//We also need to load our easy pat-sint combinations
	//Load the resource as a byte array and get its size, etc.
	res = FindResource(hInst, MAKEINTRESOURCE(IDR_WAITZAR_EASYPS), L"Model");
	if (!res)
		throw std::runtime_error("Couldn't find IDR_WAITZAR_EASYPS");
	res_handle = LoadResource(NULL, res);
	if (!res_handle)
		throw std::runtime_error("Couldn't get a handle on IDR_WAITZAR_EASYPS");
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
				throw std::runtime_error(waitzar::escape_wstr(model->getLastError(), false).c_str());

			}
		}
	}

	//Free memory
	delete [] uniData;

	//Done - This shouldn't matter, though, since the process only
	//       accesses it once and, fortunately, this is not an external file.
	//UnlockResource(res_handle);
	}

	return model;
}


void WZFactory::addWordsToModel(WordBuilder* model, string userWordsFileName) {
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
		throw std::runtime_error("waitzar user wordlist file contains invalid UTF-8 characters.");
	delete [] buffer;

	//Skip the BOM, if it exists
	size_t currPosition = 0;
	if (uniBuffer[currPosition] == UNICOD_BOM)
		currPosition++;
	else if (uniBuffer[currPosition] == BACKWARDS_BOM)
		throw std::runtime_error("waitzar user wordlist file appears to be encoded backwards.");

	//Read each line
	wchar_t* name = new wchar_t[100];
	char* value = new char[100];
	while (currPosition<numUniChars) {
		//Get the name/value pair using our nifty template function....
		waitzar::readLine(uniBuffer, currPosition, numUniChars, true, true, false, true/*model->isAllowNonBurmese()*/, true, false, false, false, name, value);

		//Make sure both name and value are non-empty
		if (strlen(value)==0 || lstrlen(name)==0)
			continue;

		//Add this romanization
		if (!model->addRomanization(name, value, true))
			throw std::runtime_error(string(string("Error adding romanisation: ") + waitzar::escape_wstr(model->getLastError(), false)).c_str());
	}

	//Reclaim memory
	delete [] uniBuffer;
	delete [] name;
	delete [] value;
}



std::map<std::wstring, RomanInputMethod*> WZFactory::cachedWBInputs;
std::map<std::wstring, RomanInputMethod*> WZFactory::cachedBGInputs;
std::map<std::wstring, DisplayMethod*> WZFactory::cachedDisplayMethods;
std::map<std::wstring, LetterInputMethod*> WZFactory::cachedLetterInputs;

RomanInputMethod* WZFactory::getWaitZarInput(wstring langID, const wstring& extraWordsFileName, const wstring& userWordsFileName, InMethNode& node)
{
	wstring fullID = langID + L"." + L"waitzar";

	//Singleton init
	if (WZFactory::cachedWBInputs.count(fullID)==0) {
		//Load model; create sentence list
		//NOTE: These resources will not be reclaimed, but since they're
		//      contained within a singleton class, I don't see a problem.
		WordBuilder* model = WZFactory::readModel();
		SentenceList* sentence = new SentenceList();

		//Add extra words
		WZFactory::addWordsToModel(model, waitzar::escape_wstr(extraWordsFileName, false));

		//Add user words
		WZFactory::addWordsToModel(model, waitzar::escape_wstr(userWordsFileName, false));

		//One final check
		if (model->isInError())
			throw std::runtime_error(waitzar::escape_wstr(model->getLastError(), false).c_str());

		//Should probably build the reverse lookup now
		model->reverseLookupWord(0);

		//Create, init
		WZFactory::cachedWBInputs[fullID] = new RomanInputMethod();
		WZFactory::cachedWBInputs[fullID]->init(WZFactory::mainWindow, WZFactory::sentenceWindow, WZFactory::helpWindow, WZFactory::memoryWindow, WZFactory::systemWordLookup, WZFactory::helpKeyboard, waitzar::WZSystemDefinedWords, model, sentence, node.encoding, node.controlKeyStyle, node.typeBurmeseNumbers, node.typeNumeralConglomerates, node.suppressUppercase);
	}

	return WZFactory::cachedWBInputs[fullID];
}



LetterInputMethod* WZFactory::getMywinInput(std::wstring langID, InMethNode& node)
{
	wstring fullID = langID + L"." + L"mywin";

	//Singleton init
	if (WZFactory::cachedLetterInputs.count(fullID)==0) {
		//Create, init
		WZFactory::cachedLetterInputs[fullID] = new LetterInputMethod();
		WZFactory::cachedLetterInputs[fullID]->init(WZFactory::mainWindow, WZFactory::sentenceWindow, WZFactory::helpWindow, WZFactory::memoryWindow, WZFactory::systemWordLookup, WZFactory::helpKeyboard, waitzar::WZSystemDefinedWords, node.encoding, node.controlKeyStyle, node.typeBurmeseNumbers, node.typeNumeralConglomerates, node.suppressUppercase);
	}

	return WZFactory::cachedLetterInputs[fullID];
}




RomanInputMethod* WZFactory::getBurglishInput(wstring langID, InMethNode& node)
{
	wstring fullID = langID + L"." + L"burglish";

	//Singleton init
	if (WZFactory::cachedBGInputs.count(fullID)==0) {
		//Load model; create sentence list
		//NOTE: These resources will not be reclaimed, but since they're
		//      contained within a singleton class, I don't see a problem.
		waitzar::BurglishBuilder* model = new waitzar::BurglishBuilder();
		SentenceList* sentence = new SentenceList();

		//Create, init
		WZFactory::cachedBGInputs[fullID] = new RomanInputMethod();
		WZFactory::cachedBGInputs[fullID]->init(WZFactory::mainWindow, WZFactory::sentenceWindow, WZFactory::helpWindow, WZFactory::memoryWindow, WZFactory::systemWordLookup, WZFactory::helpKeyboard, waitzar::WZSystemDefinedWords, model, sentence, node.encoding, node.controlKeyStyle, node.typeBurmeseNumbers, node.typeNumeralConglomerates, node.suppressUppercase);
	}

	return WZFactory::cachedBGInputs[fullID];
}


//Get a keymagic input method

LetterInputMethod* WZFactory::getKeyMagicBasedInput(std::wstring langID, std::wstring inputID, std::string wordlistFileName, bool disableCache, InMethNode& node)
{
	wstring fullID = langID + L"." + inputID;

	if (WZFactory::cachedLetterInputs.count(fullID)==0) {
		//Prepare our binary path/name
		string fs = "\\";
		std::stringstream binaryName;
		if (!disableCache) {
			//Get the path
			wchar_t localAppPath[MAX_PATH];
			if (FAILED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, localAppPath)))
				disableCache = true;
			else {
				binaryName <<waitzar::escape_wstr(localAppPath, true) <<fs <<"WaitZar";

				//Create the directory if it doesn't exist
				std::wstringstream temp;
				temp << binaryName.str().c_str();
				if (!FileExists(temp.str()))
					CreateDirectory(temp.str().c_str(), NULL);

				//Get the name
				binaryName <<fs <<waitzar::escape_wstr(fullID, false) <<'.';
				size_t firstValidID = wordlistFileName.rfind('\\');
				if (firstValidID==std::string::npos)
					firstValidID = 0;
				else
					firstValidID++;
				size_t lastDotID = wordlistFileName.find('.');
				if (lastDotID!=std::string::npos) {
					for (size_t i=firstValidID; i<lastDotID; i++)
						binaryName <<wordlistFileName[i];
				}
				binaryName <<".bin";
			}
		}

		//Build our result
		KeyMagicInputMethod* res = new KeyMagicInputMethod();
		res->init(WZFactory::mainWindow, WZFactory::sentenceWindow, WZFactory::helpWindow, WZFactory::memoryWindow, WZFactory::systemWordLookup, WZFactory::helpKeyboard, waitzar::WZSystemDefinedWords, node.encoding, node.controlKeyStyle, node.typeBurmeseNumbers, node.typeNumeralConglomerates, node.suppressUppercase);
		res->loadRulesFile(wordlistFileName, binaryName.str(), disableCache/*, fileMD5Function*/);
		//res->disableCache = disableCache;

		WZFactory::cachedLetterInputs[fullID] = res;
	}

	return WZFactory::cachedLetterInputs[fullID];
}



//Build a model up from scratch.

RomanInputMethod* WZFactory::getWordlistBasedInput(wstring langID, wstring inputID, string wordlistFileName, InMethNode& node)
{
	wstring fullID = langID + L"." + inputID;

	if (WZFactory::cachedWBInputs.count(fullID)==0) {
		//Create a basically empty model (Nexus can't be empty)
		vector< vector<unsigned int> > nexus;
		nexus.push_back(vector<unsigned int>());
		WordBuilder* model = new WordBuilder(vector<wstring>(), nexus, vector< vector<unsigned int> >());
		SentenceList* sentence = new SentenceList();

		//Add user words (there's ONLY user words here)
		WZFactory::addWordsToModel(model, wordlistFileName);

		//Should probably build the reverse lookup now
		model->reverseLookupWord(0);

		//One final check
		if (model->isInError())
			throw std::runtime_error(waitzar::escape_wstr(model->getLastError(), false).c_str());

		//Now, build the romanisation method and return
		WZFactory::cachedWBInputs[fullID] = new RomanInputMethod();
		WZFactory::cachedWBInputs[fullID]->init(WZFactory::mainWindow, WZFactory::sentenceWindow, WZFactory::helpWindow, WZFactory::memoryWindow, WZFactory::systemWordLookup, WZFactory::helpKeyboard, waitzar::WZSystemDefinedWords, model, sentence, node.encoding, node.controlKeyStyle, node.typeBurmeseNumbers, node.typeNumeralConglomerates, node.suppressUppercase);
	}

	return WZFactory::cachedWBInputs[fullID];
}




DisplayMethod* WZFactory::getZawgyiPngDisplay(std::wstring langID, std::wstring dispID, unsigned int dispResourceID, DispMethNode& node)
{
	wstring fullID = langID + L"." + dispID;

	//Singleton init
	if (WZFactory::cachedDisplayMethods.count(fullID)==0) {
		//Load our internal font
		HRSRC fontRes = FindResource(hInst, MAKEINTRESOURCE(dispResourceID), L"COREFONT");
		if (!fontRes)
			throw std::runtime_error("Couldn't find IDR_(MAIN|SMALL)_FONT");

		//Get a handle from this resource.
		HGLOBAL res_handle = LoadResource(hInst, fontRes);
		if (!res_handle)
			throw std::runtime_error("Couldn't get a handle on IDR_(MAIN|SMALL)_FONT");

		//Create, init
		WZFactory::cachedDisplayMethods[fullID] = new PulpCoreFont();
		mainWindow->initDisplayMethod(WZFactory::cachedDisplayMethods[fullID], fontRes, res_handle, node.fontFaceName, node.pointSize, 0x000000);
	}

	return WZFactory::cachedDisplayMethods[fullID];
}




DisplayMethod* WZFactory::getPadaukZawgyiTtfDisplay(std::wstring langID, std::wstring dispID)
{
	wstring fullID = langID + L"." + dispID;

	//Singleton init
	if (WZFactory::cachedDisplayMethods.count(fullID)==0) {
		//Init our internal font
		TtfDisplay* res = new TtfDisplay();

		//Get the Padauk embedded resource
		HRSRC fontRes = FindResource(hInst, MAKEINTRESOURCE(IDR_PADAUK_ZG), L"MODEL");
		if (!fontRes)
			throw std::runtime_error("Couldn't find IDR_PADAUK_ZG");
		HGLOBAL res_handle = LoadResource(NULL, fontRes);
		if (!res_handle)
			throw std::runtime_error("Couldn't get a handle on IDR_PADAUK_ZG");

		//Save, init
		WZFactory::cachedDisplayMethods[fullID] = res;
		mainWindow->initTtfMethod(WZFactory::cachedDisplayMethods[fullID], fontRes, res_handle, L"PdkZgWz", 10, 0x000000);
	}

	return WZFactory::cachedDisplayMethods[fullID];
}





DisplayMethod* WZFactory::getTtfDisplayManager(std::wstring langID, std::wstring dispID, std::wstring fontFileName, std::wstring fontFaceName, int pointSize)
{
	wstring fullID = langID + L"." + dispID;

	//Singleton init
	if (WZFactory::cachedDisplayMethods.count(fullID)==0) {
		//Init our internal font
		TtfDisplay* res = new TtfDisplay();
		//res->fontFaceName = fontFaceName;
		//res->pointSize = pointSize;

		//Save, init
		WZFactory::cachedDisplayMethods[fullID] = res;
		mainWindow->initTtfMethod(WZFactory::cachedDisplayMethods[fullID], fontFileName, fontFaceName, pointSize, 0x000000);
	}

	return WZFactory::cachedDisplayMethods[fullID];
}



DisplayMethod* WZFactory::getPngDisplayManager(std::wstring langID, std::wstring dispID, std::wstring fontFileName)
{
	wstring fullID = langID + L"." + dispID;

	//Singleton init
	if (WZFactory::cachedDisplayMethods.count(fullID)==0) {
		//Does it end in ".png"
		if (fontFileName.length()<5 || fontFileName[fontFileName.length()-4]!=L'.' || fontFileName[fontFileName.length()-3]!=L'p' || fontFileName[fontFileName.length()-2]!=L'n' || fontFileName[fontFileName.length()-1]!=L'g')
			throw std::runtime_error("PngFont file does not end in \".png\"");

		//Open it
		FILE* fontFile = fopen(waitzar::escape_wstr(fontFileName, false).c_str(), "rb");
		if (fontFile == NULL)
			throw std::runtime_error("PngFont file could not be opened for reading.");

		//Try to read its data into a char[] array; get the size, too
		fseek (fontFile, 0, SEEK_END);
		long fileSize = ftell(fontFile);
		rewind(fontFile);
		char * file_buff = new char[fileSize];
		size_t file_buff_size = fread(file_buff, 1, fileSize, fontFile);
		fclose(fontFile);
		if ((long)file_buff_size!=fileSize)
			throw std::runtime_error("PngFont file could not be read to the end of the file.");

		//Ok, load our font
		PulpCoreFont* res = new PulpCoreFont();
		try {
			mainWindow->initDisplayMethod(res, file_buff, file_buff_size, 0x000000);
		} catch (std::exception ex) {
			//Is our font in error? If so, load the embedded font
			std::stringstream msg;
			msg <<"Custom font didn't load correctly: " <<ex.what();
			delete res;
			throw std::runtime_error(msg.str().c_str());
		}

		//Save
		WZFactory::cachedDisplayMethods[fullID] = res;
	}

	return WZFactory::cachedDisplayMethods[fullID];
}




void WZFactory::InitAll(HINSTANCE& hInst, MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, OnscreenKeyboard* helpKeyboard)
{
	//Save
	WZFactory::hInst = hInst;
	WZFactory::mainWindow = mainWindow;
	WZFactory::sentenceWindow = sentenceWindow;
	WZFactory::helpWindow = helpWindow;
	WZFactory::memoryWindow = memoryWindow;
	WZFactory::helpKeyboard = helpKeyboard;

	//Initialize our system word lookup
	WZFactory::buildSystemWordLookup();
}





//
// TODO: A lot of our nodeset_exceptions are forced; we should really build them automatically.
//
Extension* WZFactory::makeAndVerifyExtension(const std::wstring& id, ExtendNode& ex)
{
	Extension* res;
	try {
		//Check required settings
		if (ex.libraryFilePath.empty())
			throw std::runtime_error("Cannot construct extension: no \"library-file-path\"");

		//Make it
		if (id == L"javascript") {
			res = new JavaScriptConverter();
		} else if (id == L"curl") {
			res = new HttpVersionChecker();
		} else {
			throw std::runtime_error(waitzar::glue(L"Unknown extension: ", id).c_str());
			//res = new Extension();
		}

		//Initialize it
		res->InitDLL(ex.enabled, ex.requireChecksum, ex.libraryFilePath, ex.libraryFileChecksum);

	} catch (std::exception& ex) {
		//Pack all exceptions into nodeset_exceptions
		throw nodeset_exception(ex.what(), id.c_str());
	}
	return res;
}




//
// TODO: A lot of our nodeset_exceptions are forced; we should really build them automatically.
//
InputMethod* WZFactory::makeAndVerifyInputMethod(const LangNode& lang, const std::wstring& id, InMethNode& im)
{
	InputMethod* res = NULL;

	try {
		//Check required settings; type is checked in the "switch" statement
		if (im.encoding.empty())
			throw std::runtime_error("Cannot construct input manager: no \"encoding\"");
		if (im.displayName.empty())
			throw std::runtime_error("Cannot construct input manager: no \"display-name\"");

		//Ensure that referenced encodings/etc. exist
		if (lang.encodings.count(im.encoding)==0)
			throw nodeset_exception(waitzar::glue(L"Input method \"" , id , L"\" references non-existent encoding: ", im.encoding).c_str(), wstring(L"languages."+lang.id+L".inputmethods."+id+L".encoding").c_str());

		//Ensure that at least one transformation exists
		if (im.encoding!=L"unicode") {
			//Sadly, our lookup has not been built yet, so we must manually scan every transformation
			bool transFound = false;
			for (auto it=lang.transformations.begin(); it!=lang.transformations.end(); it++) {
				if (it->second.fromEncoding==im.encoding && it->second.toEncoding==L"unicode") {
					transFound = true;
					break;
				}
			}
			if (!transFound)
				throw nodeset_exception(waitzar::glue(L"Input method \"" , id , L"\" uses an encoding with no corresponding transformation: ", im.encoding).c_str(), wstring(L"languages."+lang.id+L".inputmethods."+id+L".encoding").c_str());
		}

		//Make an object based on the type
		switch (im.type) {
			case INPUT_TYPE::BUILTIN:
				if (im.id==L"waitzar") {
					res = WZFactory::getWaitZarInput(lang.id, im.extraWordsFile, im.userWordsFile, im);
				} else if (im.id==L"mywin") {
					res = WZFactory::getMywinInput(lang.id, im);
				} else if (im.id==L"burglish") {
					res = WZFactory::getBurglishInput(lang.id, im);
				} else {
					throw std::runtime_error(waitzar::glue(L"Invalid \"builtin\" Input Manager: ", im.id).c_str());
				}
				break;
			case INPUT_TYPE::ROMAN:
				//Check required wordlist
				if (im.extraWordsFile.empty())
					throw std::runtime_error("Cannot construct \"roman\" input manager: no \"wordlist\".");

				//Test
				if (!FileExists(im.extraWordsFile))
					throw std::runtime_error(waitzar::glue(L"Wordlist file does not exist: ", im.extraWordsFile).c_str());

				//Get it, as a singleton
				res = WZFactory::getWordlistBasedInput(lang.id, im.id, waitzar::escape_wstr(im.extraWordsFile), im);
				break;
			case INPUT_TYPE::KEYBOARD:
				//Requires a keyboard file
				if (im.keyboardFile.empty())
					throw std::runtime_error("Cannot construct \"keymagic\" input manager: no \"keyboard-file\".");

				//Test
				if (!FileExists(im.keyboardFile))
					throw std::runtime_error(waitzar::glue(L"Keyboard file does not exist: ", im.keyboardFile).c_str());

				//Override disabling the cache, keymagic only.
				if (Logger::isLogging('K'))
					im.disableCache = true;

				//Get it, as a singleton
				res = WZFactory::getKeyMagicBasedInput(lang.id, im.id, waitzar::escape_wstr(im.keyboardFile, false), im.disableCache, im);
				break;
			default:
				throw std::runtime_error("Cannot construct input manager: no \"type\"");
		}
	} catch (std::exception& ex) {
		//Pack all exceptions into nodeset_exceptions
		throw nodeset_exception(ex.what(), id.c_str());
	}

	return res;
}




//
// TODO: A lot of our nodeset_exceptions are forced; we should really build them automatically.
//
DisplayMethod* WZFactory::makeAndVerifyDisplayMethod(const LangNode& lang, const std::wstring& id, DispMethNode& dm)
{
	DisplayMethod* res = NULL;

	try {
		//Check some required settings
		if (dm.encoding.empty())
			throw std::runtime_error("Cannot construct display method: no \"encoding\"");

		//Ensure its encoding exists, etc.
		if (lang.encodings.count(dm.encoding)==0)
			throw nodeset_exception(waitzar::glue(L"Display Method (", id, L") references non-existent encoding: ", dm.encoding).c_str(), wstring(L"languages."+lang.id+L".displaymethods."+id+L".encoding").c_str());

		//Ensure that at least one transformation exists
		if (dm.encoding!=L"unicode") {
			//Sadly, our lookup has not been built yet, so we must manually scan every transformation
			bool transFound = false;
			for (auto it=lang.transformations.begin(); it!=lang.transformations.end(); it++) {
				if (it->second.fromEncoding==L"unicode" && it->second.toEncoding==dm.encoding) {
					transFound = true;
					break;
				}
			}
			if (!transFound)
				throw nodeset_exception(waitzar::glue(L"Display method \"" , id , L"\" uses an encoding with no corresponding transformation: ", dm.encoding).c_str(), wstring(L"languages."+lang.id+L".inputmethods."+id+L".encoding").c_str());
		}

		//First, generate an actual object, based on the type.
		switch (dm.type) {
			case DISPLAY_TYPE::BUILTIN:
				//Built-in types are known entirely by our core code
				if (id==L"zawgyibmp")
					res = WZFactory::getZawgyiPngDisplay(lang.id, id, IDR_MAIN_FONT, dm);
				else if (id==L"zawgyibmpsmall")
					res = WZFactory::getZawgyiPngDisplay(lang.id, id, IDR_SMALL_FONT, dm);
				else if (id==L"pdkzgwz")
					res = WZFactory::getPadaukZawgyiTtfDisplay(lang.id, id);
				else
					throw std::runtime_error(waitzar::glue(L"Invalid \"built-in\" Display Method: ", id).c_str());
				break;

			case DISPLAY_TYPE::PNG:
				//Enforce that a valid font-file is given
				if (dm.fontFile.empty())
					throw std::runtime_error("Cannot construct \"png\" display method: no \"font-file\".");
				if (!FileExists(dm.fontFile))
					throw std::runtime_error(waitzar::glue(L"Font file file does not exist: ", dm.fontFile).c_str());

				//Get it, as a singleton
				res = WZFactory::getPngDisplayManager(lang.id, id, dm.fontFile);
				break;
			case DISPLAY_TYPE::TTF:
				//Enforce that a font-face-name and point-size are given
				if (dm.fontFaceName.empty())
					throw std::runtime_error(waitzar::glue(L"Cannot construct \"ttf\" display method: no \"font-face-name\" for: ", id).c_str());
				if (dm.pointSize==0)
					throw std::runtime_error("Cannot construct \"ttf\" display method: no \"point-size\".");

				//Ensure that a valid "font-file" was given, or none
				if (!dm.fontFile.empty() && !FileExists(dm.fontFile))
					throw std::runtime_error(waitzar::glue(L"Font file file does not exist: ", dm.fontFile).c_str());

				//Get it, as a singleton
				res = WZFactory::getTtfDisplayManager(lang.id, id, dm.fontFile, dm.fontFaceName, dm.pointSize);
				break;
			default:
				throw std::runtime_error("Cannot construct display method: no \"type\"");
		}
	} catch (std::exception& ex) {
		//Pack all exceptions into nodeset_exceptions
		throw nodeset_exception(ex.what(), id.c_str());
	}

	return res;
}




//
// TODO: A lot of our nodeset_exceptions are forced; we should really build them automatically.
//
Transformation* WZFactory::makeAndVerifyTransformation(ConfigRoot& conf, const LangNode& lang, const std::wstring& id, TransNode& tm)
{
	Transformation* res = NULL;

	try {
		//Check some required settings
		if (tm.fromEncoding.empty())
			throw std::runtime_error("Cannot construct transformation: no \"from-encoding\"");
		if (tm.toEncoding.empty())
			throw std::runtime_error("Cannot construct transformation: no \"to-encoding\"");

		//Ensure we have valid encodings
		if (lang.encodings.count(tm.fromEncoding)==0)
			throw nodeset_exception(waitzar::glue(L"Transformation \"" , id , L"\" references non-existent from-encoding: ", tm.fromEncoding).c_str(), wstring(L"languages."+lang.id+L".transformations."+id+L".fromencoding").c_str());
		if (lang.encodings.count(tm.toEncoding)==0)
			throw nodeset_exception(waitzar::glue(L"Transformation \"" , id , L"\" references non-existent to-encoding: ", tm.toEncoding).c_str(), wstring(L"languages."+lang.id+L".transformations."+id+L".toencoding").c_str());


		//First, generate an actual object, based on the type.
		switch (tm.type) {
			case TRANSFORM_TYPE::BUILTIN:
				//Built-in types are known entirely by our core code
				if (id==L"self2self")
					res = new Self2Self();
				else if (id==L"uni2zg")
					res = new Uni2Zg();
				else if (id==L"uni2wi")
					res = new Uni2WinInnwa();
				else if (id==L"zg2uni")
					res = new Zg2Uni();
				else if (id==L"uni2ayar")
					res = new Uni2Ayar();
				else if (id==L"ayar2uni")
					res = new Ayar2Uni();
				else
					throw std::runtime_error(waitzar::glue(L"Invalid \"builtin\" Transformation: ", id).c_str());
				break;

			case TRANSFORM_TYPE::JAVASCRIPT:
				//Ensure a valid source file
				if (tm.sourceFile.empty())
					throw std::runtime_error("Cannot construct transformation: no javascript \"source-file\"");
				if (!FileExists(tm.sourceFile))
					throw std::runtime_error("Cannot construct transformation: \"source-file\" references a file that does not exist.");

				//Make sure our interpreter is actually running.
				if ((conf.extensions.count(L"javascript")==0) || !conf.extensions[L"javascript"].enabled)
					throw std::runtime_error("Cannot construct a \"javscript\" Transformation: interpreter DLL failed to load.");

				res = new JSTransform(waitzar::escape_wstr(tm.sourceFile, false), *(JavaScriptConverter*)conf.extensions[L"javascript"].impl);
				break;

			default:
				throw std::runtime_error("Cannot construct transformation: no \"type\"");
		}
	} catch (std::exception& ex) {
		//Pack all exceptions into nodeset_exceptions
		throw nodeset_exception(ex.what(), id.c_str());
	}

	return res;
}



void WZFactory::verifyEncoding(const std::wstring& id, EncNode& enc)
{
	try {
		//Necessary properties
		if (enc.displayName.empty())
			throw std::runtime_error("Cannot construct encoding: no \"display-name\"");
	} catch (std::exception& ex) {
		//Pack all exceptions into nodeset_exceptions
		throw nodeset_exception(ex.what(), id.c_str());
	}
}


/**
 * Don't ask.
 */
wstring WZFactory::InterpretFlashSave(const map<wstring, vector<wstring> >& flashSave, const std::wstring& langID, size_t resIndex)
{
	auto res = flashSave.find(langID);
	if (res==flashSave.end())
		return L"";
	return res->second[resIndex];
}
wstring WZFactory::MatchAvoidBackwards(const vector<wstring>& vec, const wstring& avoidStr)
{
	for (auto it=vec.rbegin(); it!=vec.rend(); it++) {
		if (*it != avoidStr)
			return *it;
	}
	return L"";
}



//
// TODO: A lot of our nodeset_exceptions are forced; we should really build them automatically.
//
void WZFactory::verifySettings(ConfigRoot& cfg, SettingsNode& set, const std::wstring& lastUsedLangID)
{
	//Deal with "last-used" languages
	if (set.defaultLanguage==L"lastused") {
		//First, try to set it to the last used language (which may not exist)
		wstring backupDefLang = WZFactory::MatchAvoidBackwards(set.defaultLanguageStack, L"lastused");
		if (!lastUsedLangID.empty() && cfg.languages.count(lastUsedLangID)>0)
			set.defaultLanguage = lastUsedLangID;
		else if (!backupDefLang.empty())
			set.defaultLanguage = backupDefLang;
		else
			throw nodeset_exception("Settings chooses default-language of \"last-used\" without specifying a backup.", L"settings.defaultlanguage");
	}

	//Verify our default language
	if (cfg.languages.count(set.defaultLanguage)==0)
		throw nodeset_exception(waitzar::glue(L"Settings references non-existent default-language: ", set.defaultLanguage).c_str(), L"settings.defaultlanguage");
}



//
// TODO: A lot of our nodeset_exceptions are forced; we should really build them automatically.
//
void WZFactory::verifyLanguage(const std::wstring& id, LangNode& lang, const std::wstring& lastUsedInMethID, const std::wstring& lastUsedOutEncID)
{
	try {
		//Necessary properties
		if (lang.displayName.empty())
			throw std::runtime_error("Cannot construct language: no \"display-name\"");
		if (lang.encodings.count(L"unicode")==0)
			throw std::runtime_error(waitzar::glue(L"Language \"" , lang.id , L"\" does not include \"unicode\" as an encoding.").c_str());

		//Deal with "last-used" input method
		if (lang.defaultInputMethod==L"lastused") {
			//First, try to set it to the last used language (which may not exist)
			wstring backupDefIM = WZFactory::MatchAvoidBackwards(lang.defaultInMethStack, L"lastused");
			if (!lastUsedInMethID.empty() && lang.inputMethods.count(lastUsedInMethID)>0)
				lang.defaultInputMethod = lastUsedInMethID;
			else if (!backupDefIM.empty())
				lang.defaultInputMethod = backupDefIM;
			else
				throw nodeset_exception("Language chooses default-input-method of \"last-used\" without specifying a backup.", wstring(L"languages."+lang.id+L".defaultinputmethod").c_str());
		}

		//Deal with "last-used" display method
		if (lang.defaultOutputEncoding==L"lastused") {
			//First, try to set it to the last used language (which may not exist)
			wstring backupDefOE = WZFactory::MatchAvoidBackwards(lang.defaultOutEncStack, L"lastused");
			if (!lastUsedOutEncID.empty() && lang.encodings.count(lastUsedOutEncID)>0)
				lang.defaultOutputEncoding = lastUsedOutEncID;
			else if (!backupDefOE.empty())
				lang.defaultOutputEncoding = backupDefOE;
			else
				throw nodeset_exception("Language chooses default-output-encoding of \"last-used\" without specifying a backup.", wstring(L"languages."+lang.id+L".defaultoutputencoding").c_str());
		}

		//Ensure dependencies are met
		if (lang.encodings.count(lang.defaultOutputEncoding)==0)
			throw nodeset_exception(waitzar::glue(L"Language \"" , lang.id , L"\" references non-existant default output encoding: ", lang.defaultOutputEncoding).c_str(), wstring(L"languages."+lang.id+L".defaultoutputencoding").c_str());
		if (lang.inputMethods.count(lang.defaultInputMethod)==0)
			throw nodeset_exception(waitzar::glue(L"Language \"" , lang.id , L"\" references non-existant default input method: ", lang.defaultInputMethod).c_str(), wstring(L"languages."+lang.id+L".defaultinputmethod").c_str());
		if (lang.displayMethods.count(lang.defaultDisplayMethodReg)==0)
			throw nodeset_exception(waitzar::glue(L"Language \"" , lang.id , L"\" references non-existant default (regular) display method: ", lang.defaultDisplayMethodReg).c_str(), wstring(L"languages."+lang.id+L".defaultdisplaymethod").c_str());
		if (lang.displayMethods.count(lang.defaultDisplayMethodSmall)==0)
			throw nodeset_exception(waitzar::glue(L"Language \"" , lang.id , L"\" references non-existant default (small) display method: ", lang.defaultDisplayMethodSmall).c_str(), wstring(L"languages."+lang.id+L".defaultdisplaymethodsmall").c_str());
		if (!lang.encodings[lang.defaultOutputEncoding].canUseAsOutput)
			throw nodeset_exception(waitzar::glue(L"Language \"" , lang.id , L"\" uses a default output encoding which does not support output.").c_str(), wstring(L"languages."+lang.id+L".defaultoutputencoding").c_str());
		if (lang.displayMethods[lang.defaultDisplayMethodReg].encoding != lang.displayMethods[lang.defaultDisplayMethodSmall].encoding)
			throw nodeset_exception(waitzar::glue(L"Language \"" , lang.id , L"\" uses two display methods with two different encodings.").c_str(), wstring(L"languages."+lang.id+L".defaultdisplaymethod").c_str());
	} catch (std::exception& ex) {
		//Pack all exceptions into nodeset_exceptions
		throw nodeset_exception(ex.what(), id.c_str());
	}
}




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
