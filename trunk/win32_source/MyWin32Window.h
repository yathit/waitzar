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
#include "Pulp Core/PulpCoreFont.h"


/**
 * Win32 top-level windows are notoriously difficult to manage in a distributed
 *  set of files. Hence, this class manages creation, deletion, repainting, and
 *  resizing of Win32 windows. 
 * What this class doesn't manage:
 *   - Window Classes, and thus:
 *   - Window Message Handling
 */
class MyWin32Window
{
public:
	//Constructor/destructor pair
	MyWin32Window(LPCWSTR windowClassName, LPCWSTR windowTitle, const HINSTANCE& hInstance, 
		int x=99, int y=99, int width=99, int height=99, void (*onShowFunction)(void)=NULL, bool useAlpha=true);
	~MyWin32Window();

	//Required Inits (Hope to phase these out eventually)
	void createDoubleBufferedSurface();
	NOTIFYICONDATA getShellNotifyIconData();

	//Functionality forwarding to Win32
	bool getTextMetrics(LPTEXTMETRICW res);
	bool registerHotKey(int id, UINT fsModifiers, UINT vk);
	bool unregisterHotKey(int id);

	//Functionality similar to Win32, with minor differences
	bool moveWindow(int newX, int newY); //Preserves width/height
	bool resizeWindow(int newWidth, int newHeight); //Preserve x/y
	bool expandWindow(int newX, int newY, int newWidth, int newHeight, bool dontMove); //Ugh...
	bool expandWindow(int newWidth, int newHeight);
	bool showWindow(bool show); //Handle callbacks for cursor repositioning, etc.
	int getWidth(); //We track this ourselves
	int getHeight(); //We track this ourselves
	int getClientWidth(); //We track this ourselves
	int getClientHeight(); //We track this ourselves
	bool isVisible(); //We track this ourselves
	bool repaintWindow(); //Blit or UpdateLayer depending
	bool repaintWindow(RECT blitArea); //Blit or UpdateLayer depending

	//A new property
	void setDefaultSize(int width, int height);
	int getDefaultWidth();
	int getDefaultHeight();

	//Fonts (even DIB ones like Pulp Core fonts) must be tied to some compatible DC somewhere.
	void initPulpCoreImage(PulpCoreImage* img, HRSRC resource, HGLOBAL dataHandle);
	void initPulpCoreImage(PulpCoreImage* img, PulpCoreImage* copyFromImg);
	void initPulpCoreImage(PulpCoreImage* img, char *data, DWORD size);

	//Post fake messages
	bool postMessage(UINT msg, WPARAM wParam, LPARAM lParam);


private:
	//Data members
	HWND window;
	HDC topDC;
	HDC underDC;
	HBITMAP topBitmap;

	//More bookkeeping
	RECT windowArea;
	RECT clientArea;
	RECT defaultArea;
	bool is_visible;
	bool useAlpha;

	//Ugh
	POINT PT_ORIGIN;
	BLENDFUNCTION BLEND_FULL;

	//Used to update the caret position
	void (*onShowFunction)(void);


	
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

