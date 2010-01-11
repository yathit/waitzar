/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once

#define _UNICODE
#define UNICODE

//Don't let Visual Studio warn us to use the _s functions
#define _CRT_SECURE_NO_WARNINGS

//Ironic that adding compliance now causes headaches compiling under VS2003
#define _CRT_NON_CONFORMING_SWPRINTFS


#include <windows.h>
#include <string>


/**
 * Win32 top-level windows are notoriously difficult to manage in a distributed
 *  set of files. Hence, this class manages creation, deletion, repainting, and
 *  resizing of Win32 windows. 
 */
class MyWin32Window
{
public:
	//Constructor/destructor pair
	MyWin32Window(LPCWSTR windowClassName, LPCWSTR windowTitle, const HINSTANCE& hInstance, 
		int x=99, int y=99, int width=99, int height=99, bool useAlpha=true);
	~MyWin32Window();

	//Functionality
	void createDoubleBufferedSurface();


private:
	//Data members
	HWND window;
	HDC topDC;
	HDC underDC;
	HBITMAP topBitmap;

	//More bookkeeping
	RECT windowArea;
	RECT clientArea;


	
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

