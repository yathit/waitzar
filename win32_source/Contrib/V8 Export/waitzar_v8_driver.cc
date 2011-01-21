/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */


//
// This code drives the Wait Zar string conversion process.
// The function ConvertString() is the only thing that needs to
//    be called. Later we might improve the efficiency of the engine
//    by keeping an instance running; for now, the overhead in
//    reloading a script each time is minor.
// Sample code included at the bottom of this file.
//


//
//  TODO: Figure out how V8 include memcpy/wcscpy and use that
//        (Actually, this doesn't really affect performance much)
//


#include "../include/v8.h"

using namespace v8;


//Input string
wchar_t* WAITZAR_V8_INPUT_STR_REF = L"";
v8::Handle<Value> InGetter(const v8::Arguments& args) {
	//Will this manage its own memory?
	return String::New((uint16_t*)WAITZAR_V8_INPUT_STR_REF);
}


//Helper
static int MyWcsLen(const wchar_t* data) {
  int length = 0;
  while (data[length] != '\0') length++;
  return length;
}
static void MyMemCpy(uint16_t* dest, const uint16_t* const src, const uint16_t size) {
  for (uint16_t i=0; i<size; i++)
  	dest[i] = src[i];
}


//Helper
static void createReturnStr(uint16_t* inoutStr, uint16_t errorCode, const wchar_t* const message)
{
	inoutStr[0] = errorCode;
	//wcscpy((wchar_t*)&inoutStr[1], message);
	MyMemCpy(&inoutStr[1], (uint16_t*)message, sizeof(uint16_t)*(MyWcsLen(message)+1));
}



//The function our DLL will use to handle data conversion
//TODO: Later we might consider keeping the context in place, and loading source scripts once only
//      (but how to deal with memory leaks or bad scripts?)
__declspec(dllexport) void ConvertString(uint16_t* scriptSource, uint16_t* inoutStr, uint16_t MAX_LENGTH)
{
	//Common error codes
	const uint16_t ERROR_NONE = 0;
	const uint16_t ERROR_SCRIPT_COMPILE = 1;
	const uint16_t ERROR_RUNTIME = 2;
	const uint16_t ERROR_RETURN_ERROR = 3;

	//Save our function argument so that it will actually be accesible
	WAITZAR_V8_INPUT_STR_REF = (wchar_t*)inoutStr;

	// Create a stack-allocated handle scope.
	HandleScope handle_scope;

	//Make an object template for accessing the string to convert
	Handle<ObjectTemplate> global = ObjectTemplate::New();
	global->Set(String::New("getInput"), FunctionTemplate::New(InGetter));

	// Create a new context, prepare for error handling
	Handle<Context> context = Context::New(NULL, global);
	TryCatch trycatch;

	//Enter the created context
	Context::Scope context_scope(context);

	// Create a string containing the JavaScript source code.
	Handle<String> source = String::New(scriptSource);

	// Compile the source code.
	Handle<Script> script = Script::Compile(source);
	if (script.IsEmpty()) {
		//Get the exception
		Handle<Value> exception = trycatch.Exception();
		String::Value exception_str(exception);
		wchar_t* excTxt = (wchar_t*)*exception_str;

		//Create a return value
		createReturnStr(inoutStr, ERROR_SCRIPT_COMPILE, excTxt);
		return;
	}

	// Run the script to get the result.
	Handle<Value> result = script->Run();
	if (result.IsEmpty()) {
		//Get the exception
		Handle<Value> exception = trycatch.Exception();
		String::Value exception_str(exception);
		wchar_t* excTxt = (wchar_t*)*exception_str;

		//Create a return value
		createReturnStr(inoutStr, ERROR_RUNTIME, excTxt);
		return;
	}

	//Did we get a null or undefined result?
	if (result->IsUndefined() || result->IsNull()) {
		//Error
		createReturnStr(inoutStr, ERROR_RETURN_ERROR, L"Script returned undefined or null");
		return;
	}

	//Convert the result to a string
	String::Value myres(result);
	wchar_t* myResTxt = (wchar_t*)*myres;
	if (myres.length()+2>MAX_LENGTH) {
		//Error
		createReturnStr(inoutStr, ERROR_RETURN_ERROR, L"Script return value exceeds maximum size.");
		return;
	}

	//Correct execution
	createReturnStr(inoutStr, ERROR_NONE, myResTxt);
}


//////////////////////////////////////
// Sample usage
//////////////////////////////////////



/*std::string escapeUni(const std::wstring& src)
{
	std::stringstream res;
	for (size_t i=0; i<src.length(); i++) {
		if (src[i] < 0x80) {
			res <<((char)src[i]);
		} else {
			res <<"\\u" <<std::hex <<src[i] <<std::dec;
		}
	}
	return res.str();
}




int main(int argc, char* argv[]) {
	using std::string;
	using std::wstring;
	using std::cout;

	//Basics
	wchar_t* scriptSource = L"getInput().replace(/(.)/g, '($1)')";
	size_t scriptSourceLen = wcslen(scriptSource);
	wchar_t* inputString = L"\u1019\u103C\u1014\u103A\u1019\u102C\u1005\u102C";
	size_t inputStringLen = wcslen(inputString);

	//Copy the inputString into a much larger array
	const size_t MAX_SIZE = 300;
	wchar_t* large_array = new wchar_t[MAX_SIZE];
	wcscpy(large_array, inputString);

	//Run our code
	ConvertString((uint16_t*)scriptSource, (uint16_t*)large_array, MAX_SIZE);

	//Get back the result
	uint16_t errorCode = large_array[0];
	wstring retStr = wstring(&large_array[1]);

	//Print it
	if (errorCode!=0) {
		switch (errorCode) {
			case 1:
				cout <<"Script compile error:" <<std::endl; break;
			case 2:
				cout <<"Script runtime error:" <<std::endl; break;
			case 3:
				cout <<"Script return error:" <<std::endl;  break;
			default:
				cout <<"Unknown error code:" <<std::endl;
		}
	}
	cout <<escapeUni(retStr) <<std::endl;

	//Reclaim memory
	delete [] large_array;

	return 0;
}*/



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

