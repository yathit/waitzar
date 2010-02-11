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


WZFactory::WZFactory(void)
{
}


WZFactory::~WZFactory(void)
{
}


//Something static
HINSTANCE WZFactory::hInst = HINSTANCE();



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

	for (;index<uniSize;) {
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
		if (!model->addRomanization(name, value))
			throw std::exception(string(string("Error adding romanisation: ") + waitzar::escape_wstr(model->getLastError(), false)).c_str());
	}

	//Reclaim memory
	delete [] uniBuffer;
	delete [] name;
	delete [] value;
}



RomanInputMethod* WZFactory::wz_input = NULL;
RomanInputMethod* WZFactory::getWaitZarInput() 
{
	//Singleton init
	if (WZFactory::wz_input==NULL) {
		//Load model; create sentence list
		//NOTE: These resources will not be reclaimed, but since they're 
		//      contained within a singleton class, I don't see a problem.
		WordBuilder* model = WZFactory::readModel();
		SentenceList* sentence = new SentenceList();

		//Add user words
		WZFactory::addWordsToModel(model, "mywords.txt");

		//One final check	
		if (model->isInError())
			throw std::exception(waitzar::escape_wstr(model->getLastError(), false).c_str());

		//Should probably build the reverse lookup now
		model->reverseLookupWord(0);

		//Create, init
		//TODO: Actually hook up the windows, etc.
		vector< pair <int, unsigned short> > temp;
		WZFactory::wz_input = new RomanInputMethod(NULL, NULL, NULL, NULL, temp, NULL, L"");
		WZFactory::wz_input->init(model, sentence);
	}
	
	return WZFactory::wz_input;
}

//Build a model up from scratch.
RomanInputMethod* WZFactory::getWordlistBasedInput(string wordlistFileName)
{
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
		throw std::exception(waitzar::escape_wstr(model->getLastError(), false).c_str());

	//Now, build the romanisation method and return
	//TODO: Actually hook up the windows, etc.
	vector< pair <int, unsigned short> > temp;
	RomanInputMethod* res = new RomanInputMethod(NULL, NULL, NULL, NULL, temp, NULL, L"");
	res->init(model, sentence);
	return res;
}





void WZFactory::InitAll(HINSTANCE& hInst)
{
	//Save
	WZFactory::hInst = hInst;

	//Call all singleton classes
	WZFactory::getWaitZarInput();
}



InputMethod* WZFactory::makeInputMethod(const std::wstring& id, const std::map<std::wstring, std::wstring>& options)
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
		if (id==L"waitzar") {
			return WZFactory::getWaitZarInput();
		}
		else if (id==L"mywin")
			res = WZFactory::getWaitZarInput();
		else
			throw std::exception(ConfigManager::glue(L"Invalid \"builtin\" Input Manager: ", id).c_str());
		res->type = BUILTIN;
	} else {
		throw std::exception(ConfigManager::glue(L"Invalid type (",options.find(L"type")->second, L") for Input Manager: ", id).c_str());
	}

	//Now, add general settings
	res->id = id;
	res->displayName = options.find(sanitize_id(L"display-name"))->second;
	res->encoding.id = sanitize_id(options.find(L"encoding")->second);

	//Return our resultant IM
	return res;
}


DisplayMethod* WZFactory::makeDisplayMethod(const std::wstring& id, const std::map<std::wstring, std::wstring>& options)
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
			res = new PngFont();
		else
			throw std::exception(ConfigManager::glue(L"Invalid \"builtin\" Display Method: ", id).c_str());
		res->type = BUILTIN;
	} else {
		throw std::exception(ConfigManager::glue(L"Invalid type (",options.find(L"type")->second, L") for Display Method: ", id).c_str());
	}

	//Now, add general settings
	res->id = id;
	res->encoding.id = sanitize_id(options.find(L"encoding")->second);

	//Return our resultant DM
	return res;
}


Transformation* WZFactory::makeTransformation(const std::wstring& id, const std::map<std::wstring, std::wstring>& options)
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
			res = new Uni2Uni();   //TODO: Implement
		else if (id==L"uni2wi")
			res = new Uni2Uni();   //TODO: Implement
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

Encoding WZFactory::makeEncoding(const std::wstring& id, const std::map<std::wstring, std::wstring>& options)
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
std::wstring WZFactory::sanitize_id(const std::wstring& str)
{
	return ConfigManager::sanitize_id(str);
}
bool WZFactory::read_bool(const std::wstring& str)
{
	return ConfigManager::read_bool(str);
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
