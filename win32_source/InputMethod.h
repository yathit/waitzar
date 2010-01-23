/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _INPUT_METHOD
#define _INPUT_METHOD

#include <string>
#include "MyWin32Window.h"


//Expected interface: "Input Method"
class InputMethod {
public:
	InputMethod(MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow);
	virtual ~InputMethod();

	//Struct-like properties
	Option<std::wstring> displayName;
	Option<std::wstring> encoding;
	Option<TYPES> type;

	//Useful functionality
	void treatAsHelpKeyboard(bool val);

	//Keypress handlers (abstract virtual)
	virtual void handleEsc() = 0;
	virtual void handleBackspace() = 0;
	virtual void handleDelete() = 0;
	virtual void handleRight() = 0;
	virtual void handleLeft() = 0;
	virtual void handleEnter() = 0;
	virtual void handleSpace() = 0;
	virtual void handleNumber(int numCode, WPARAM wParam) = 0;
	virtual void handleStop(bool isFull) = 0;

	virtual void handleKeyPress(WPARAM wParam) = 0;


private:
	bool isHelpInput;


protected:
	//Main and sub strings. These should generally be maintained by sub-classes
	std::wstring mainStr;
	std::wstring subStr;

	//Window control
	MyWin32Window* mainWindow;
	MyWin32Window* sentenceWindow;
	MyWin32Window* helpWindow;
	MyWin32Window* memoryWindow;



public:  //Abstract methods

	//Is this class a placeholder, or is it a real IM?
	//   There are other ways to do this, but it's nice to 
	//   have a way of double-checking.
	virtual bool isPlaceholder() = 0; 

	//Get strings to print, always in unicode
	virtual std::wstring getMainString() = 0; //The string we're typing now
	virtual std::wstring getSubString() = 0;  //The sentence string, usually
};


#endif //_INPUT_METHOD

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

