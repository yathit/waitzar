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
#include <sstream>
#include <map>
#include "Pulp Core/PulpCoreFont.h"
#include "NGram/wz_utilities.h"


enum ATTACH_DIRECTION {
	NORTH=0,
	SOUTH,
	EAST,
	WEST
};


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
	//Constructor/destructor pair. And an init, since CreateWindowEx() causes problems in a constructor... ugh.
	MyWin32Window(LPCWSTR windowClassName);
	void init(LPCWSTR windowTitle, WNDPROC userWndProc, HBRUSH& bkgrdClr, const HINSTANCE& hInstance, int x=99, int y=99, 
		int width=99, int height=99, void (*onShowFunction)(void)=NULL, void (*onAllCreatedFunction)(void)=NULL, bool useAlpha=true);
	~MyWin32Window();

	//Required Stuff (Hope to phase these out eventually)
	void createDoubleBufferedSurface();
	void initShellNotifyIconData(NOTIFYICONDATA& toInit);
	bool setWindowPosition(int x, int y, int cx, int cy, UINT uFlags);
	bool isInvalid();
	HDC WARNINGgetUnderDC();

	//Process some messages ourselves
	WNDPROC userWndProc;
	LRESULT CALLBACK MyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Functionality forwarding to Win32
	bool getTextMetrics(LPTEXTMETRICW res);
	bool registerHotKey(int id, UINT fsModifiers, UINT vk);
	bool unregisterHotKey(int id);

	//Functionality similar to Win32, with minor differences
	bool moveWindow(int newX, int newY); //Preserves width/height
	bool resizeWindow(int newWidth, int newHeight, bool doRepaint); //Preserve x/y
	bool expandWindow(int newX, int newY, int newWidth, int newHeight, bool dontMove); //Ugh...
	bool expandWindow(int newWidth, int newHeight);
	bool hiliteMenu(const HMENU& containerMenu, unsigned int hilightID, bool highlightON);
	bool showWindow(bool show); //Handle callbacks for cursor repositioning, etc.
	int getXPos(); //We track this ourselves
	int getYPos(); //We track this ourselves
	int getWidth(); //We track this ourselves
	int getHeight(); //We track this ourselves
	int getClientWidth(); //We track this ourselves
	int getClientHeight(); //We track this ourselves
	bool isVisible(); //We track this ourselves
	bool repaintWindow(); //Blit or UpdateLayer depending
	bool repaintWindow(RECT blitArea); //Blit or UpdateLayer depending
	void showMessageBox(std::wstring msg, std::wstring title, UINT flags); //Use STL strings
	bool isWindowCreated(); //After WM_CREATE

	//GDI functionality, always draws to the underDC
	bool selectObject(HPEN &obj);
	bool selectObject(HBRUSH &obj);
	bool moveTo(int x, int y);
	bool drawLineTo(int x, int y);
	bool drawRectangle(int left, int top, int right, int bottom);
	bool drawPolygon(const POINT *points, int numPoints);
	bool drawImage(PulpCoreImage* img, int x, int y);
	bool drawString(PulpCoreFont* font, const std::string& str, int x, int y);
	bool drawString(PulpCoreFont* font, const std::wstring& str, int x, int y);
	bool drawChar(PulpCoreFont* font, char letter, int xPos, int yPos);

	//A new property
	void setDefaultSize(int width, int height);
	int getDefaultWidth();
	int getDefaultHeight();

	//Fonts (even DIB ones like Pulp Core fonts) must be tied to some compatible DC somewhere.
	void initPulpCoreImage(PulpCoreImage* img, HRSRC resource, HGLOBAL dataHandle);
	void initPulpCoreImage(PulpCoreImage* img, PulpCoreImage* copyFromImg);
	void initPulpCoreImage(PulpCoreFont* font, PulpCoreFont* copyFromFont); //Needed for a tiny fluke w/ OOP
	void initPulpCoreImage(PulpCoreImage* img, char *data, DWORD size);
	void initPulpCoreImage(PulpCoreImage* img, int width, int height, int bkgrdARGB);

	//Post fake messages
	bool postMessage(UINT msg, WPARAM wParam, LPARAM lParam);

	//Link two windows
	void linkToWindow(MyWin32Window* other, ATTACH_DIRECTION linkAt);


private:
	//Data members
	HWND window;
	HDC topDC;
	HDC underDC;
	HBITMAP topBitmap;

	//Used to handle movement and locking windows
	MyWin32Window* linkedWindows[4];
	int skipNextUpdate;

	//Sets the "skip updates" flag
	void skipMoveUpdates();

	//More bookkeeping
	LPCWSTR windowClassName;
	RECT windowArea;
	RECT clientArea;
	RECT defaultArea;
	bool is_visible;
	bool useAlpha;
	POINT dragFrom;
	bool isDragging;

	//Used to update the caret position
	void (*onShowFunction)(void);

	//Used to perform initialization after all windows are created
	void (*onAllCreatedFunction)(void);

	//STATIC
	//Obnoxious WindowClass-to-Window mapping
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static std::map< std::wstring, MyWin32Window* > WndMap;


	
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

