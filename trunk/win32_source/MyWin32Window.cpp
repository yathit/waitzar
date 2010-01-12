/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include ".\MyWin32Window.h"


//Useful constants
namespace {
	POINT PT_ORIGIN = { 0 , 0 };
	BLENDFUNCTION BLEND_FULL = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA }; //NOTE: This requires premultiplied pixel values
}

MyWin32Window::MyWin32Window()
{
	//Avoid errors
	this->window = NULL;
}

void MyWin32Window::init(LPCWSTR windowClassName, LPCWSTR windowTitle, const HINSTANCE& hInstance, int x, int y, int width, int height, void (*onShowFunction)(void), bool useAlpha)
{
	//Save callback
	this->onShowFunction = onShowFunction;

	//Init
	this->is_visible = false;
	this->useAlpha = useAlpha;

	//Manually track the window's width/height
	windowArea.left = x;
	windowArea.top = y;
	windowArea.right = x + width;
	windowArea.bottom = y + height;

	//Create the window
	// We have to use NOACTIVATE because, otherwise, typing text into a box that "selects all on refresh"
	// (like IE's address bar) is almost impossible. Unfortunately, this means our window will
	// receive very few actual events
	CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_NOACTIVATE | (useAlpha?WS_EX_LAYERED:0), //Keep this window on top, never activate it.
		windowClassName, windowTitle,
		WS_POPUP, //No border or title bar
		windowArea.left, windowArea.top, windowArea.right-windowArea.left, windowArea.bottom-windowArea.top, //Default x,y,width,height (most windows will resize later)
		NULL, NULL, hInstance, NULL); 

	//Save the device context; this should prevent some errors.
	//NOTE: Can't do this; the WM_CREATE message may be posted before this...
	//if (window != NULL)
	//	topDC = GetDC(window);
}



MyWin32Window::~MyWin32Window()
{
	//Send WM_DESTROY message.
	if (window!=NULL)
		DestroyWindow(window);
}

void MyWin32Window::saveHwnd(HWND &hwnd)
{
	this->window = hwnd;
	this->topDC = GetDC(hwnd);
}


bool MyWin32Window::isInvalid()
{
	return window==NULL;
}


//WARNING: This is used in OnscreenKeyboard.cpp for something specific.
//    Need to remove this as soon as possible; for now, we're keeping it
//    since I don't want to break too much on this commit.
HDC MyWin32Window::WARNINGgetUnderDC()
{
	return underDC;
}


void MyWin32Window::createDoubleBufferedSurface()
{
	//Create all our buffering objects
	GetClientRect(window, &clientArea);
	topDC = GetDC(window);
	underDC = CreateCompatibleDC(topDC);
	topBitmap = CreateCompatibleBitmap(topDC, windowArea.right-windowArea.left, windowArea.bottom-windowArea.top);
	SelectObject(underDC, topBitmap);
}


void MyWin32Window::initShellNotifyIconData(NOTIFYICONDATA& toInit)
{
	toInit.cbSize = sizeof(NOTIFYICONDATA); //Init size
	toInit.hWnd = window; //Bind to this window
}



bool MyWin32Window::getTextMetrics(LPTEXTMETRICW res)
{
	if (GetTextMetrics(topDC, res)==FALSE)
		return false;
	return true;
}

bool MyWin32Window::registerHotKey(int id, UINT fsModifiers, UINT vk)
{
	if (RegisterHotKey(window, id, fsModifiers, vk)==FALSE)
		return false;
	return true;
}

bool MyWin32Window::unregisterHotKey(int id)
{
	if (UnregisterHotKey(window, id)==FALSE)
		return false;
	return true;
}



//Do not resize.
//Do not repaint.
bool MyWin32Window::moveWindow(int newX, int newY)
{
	int width = getWidth();
	int height = getHeight();
	int c_width = getClientWidth();
	int c_height = getClientHeight();
	bool res = (MoveWindow(window, newX, newY, width, height, FALSE)==TRUE);

	//Bookkeeping
	windowArea.left = newX;
	windowArea.top = newY;
	windowArea.right = windowArea.left + width;
	windowArea.bottom = windowArea.top + height;
	clientArea.left = newX;
	clientArea.top = newY;
	clientArea.right = clientArea.left + c_width;
	clientArea.bottom = clientArea.top + c_height;

	return res;
}

bool MyWin32Window::setWindowPosition(int x, int y, int cx, int cy, UINT uFlags)
{
	int width = getWidth();
	int height = getHeight();
	int c_width = getClientWidth();
	int c_height = getClientHeight();
	bool res = (SetWindowPos(window, HWND_TOPMOST, x, y, cx, cy, uFlags)==TRUE);

	//Bookkeeping
	windowArea.left = x;
	windowArea.top = y;
	windowArea.right = windowArea.left + width;
	windowArea.bottom = windowArea.top + height;
	clientArea.left = x;
	clientArea.top = y;
	clientArea.right = clientArea.left + c_width;
	clientArea.bottom = clientArea.top + c_height;

	return res;
}


//Do not reposition.
//DO repaint.
bool MyWin32Window::resizeWindow(int newWidth, int newHeight)
{
	RECT r;
	GetWindowRect(window, &r);
	bool res = (MoveWindow(window, r.left, r.top, newWidth, newHeight, TRUE)==TRUE);

	//Bookkeeping
	windowArea.right = windowArea.left + newWidth;
	windowArea.bottom = windowArea.top + newHeight;
	GetClientRect(window, &clientArea);

	return res;
}

//Need to merge this with resize eventually...
bool MyWin32Window::expandWindow(int newX, int newY, int newWidth, int newHeight, bool dontMove)
{
	//Resize the current window; use SetWindowPos() since it's easier...
	int flags = SWP_NOZORDER | SWP_NOACTIVATE;
	int x = newX;
	int y = newY;
	if (dontMove) {
		flags |= SWP_NOMOVE;
		x = windowArea.left;
		y = windowArea.top;
	}
	bool res = (SetWindowPos(window, NULL, newX, newY, newWidth, newHeight, flags)==TRUE);

	//Update client area, window area
	windowArea.left = x;
	windowArea.top = y;
	windowArea.right = windowArea.left + newWidth;
	windowArea.bottom = windowArea.top + newHeight;
	GetClientRect(window, &clientArea);

	//We also have to set our graphics contexts correctly. Also, throw out the old ones.
	DeleteDC(underDC);
	DeleteObject(topBitmap);
	topDC = GetDC(window);
	underDC = CreateCompatibleDC(topDC);
	topBitmap = CreateCompatibleBitmap(topDC, this->getClientWidth(), this->getClientHeight());
	SelectObject(underDC, topBitmap);

	return res;
}


bool MyWin32Window::expandWindow(int newWidth, int newHeight)
{
	return this->expandWindow(0, 0, newWidth, newHeight, true);
}



bool MyWin32Window::showWindow(bool show)
{
	//Avoid duplicate commands
	if (show == is_visible)
		return true;

	//Re-position?
	if (!is_visible && this->onShowFunction!=NULL)
		onShowFunction();

	//Set flags, perform show/hide
	bool res = (ShowWindow(window, show?SW_SHOW:SW_HIDE)==TRUE);
	is_visible = show;

	return res;
}

bool MyWin32Window::repaintWindow(RECT blitArea)
{
	bool res = false;
	if (this->useAlpha) {
		//Use the "Alpha" command (No rectangle at the moment...)
		SIZE sz;
		sz.cx = this->getClientWidth();
		sz.cy = this->getClientWidth();
		res = (UpdateLayeredWindow(window, GetDC(NULL), NULL, &sz, underDC, &PT_ORIGIN, 0, &BLEND_FULL, ULW_ALPHA)==TRUE);
	} else {
		//Use the "Blit" command
		res = (BitBlt(topDC,blitArea.left,blitArea.top,blitArea.right-blitArea.left,blitArea.bottom-blitArea.top,underDC,blitArea.left,blitArea.top,SRCCOPY)==TRUE);
	}
	return res;
}

bool MyWin32Window::repaintWindow()
{
	bool res = false;
	if (this->useAlpha) {
		//Use the "Alpha" command
		SIZE sz;
		sz.cx = this->getClientWidth();
		sz.cy = this->getClientWidth();
		res = (UpdateLayeredWindow(window, GetDC(NULL), NULL, &sz, underDC, &PT_ORIGIN, 0, &BLEND_FULL, ULW_ALPHA)==TRUE);
	} else {
		//Use the "Blit" command
		res = (BitBlt(topDC,0,0,this->getClientWidth(),this->getClientHeight(),underDC,0,0,SRCCOPY)==TRUE);
	}
	return res;
}


int MyWin32Window::getWidth()
{
	return windowArea.right-windowArea.left;
}

int MyWin32Window::getHeight()
{
	return windowArea.bottom-windowArea.top;
}

int MyWin32Window::getClientWidth()
{
	return clientArea.right-clientArea.left;
}

int MyWin32Window::getClientHeight()
{
	return clientArea.bottom-clientArea.top;
}

void MyWin32Window::setDefaultSize(int width, int height)
{
	this->defaultArea.left = 0;
	this->defaultArea.top = 0;
	this->defaultArea.right = width;
	this->defaultArea.bottom = height;
}

int MyWin32Window::getDefaultWidth()
{
	return defaultArea.right-clientArea.left;
}

int MyWin32Window::getDefaultHeight()
{
	return defaultArea.bottom-defaultArea.top;
}

bool MyWin32Window::isVisible()
{
	return is_visible;
}

void MyWin32Window::showMessageBox(std::wstring msg, std::wstring title, UINT flags)
{
	MessageBox(window, msg.c_str(), title.c_str(), flags);
}


bool MyWin32Window::postMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (PostMessage(window, msg, wParam, lParam)==FALSE)
		return false;
	return true;
}

void MyWin32Window::initPulpCoreImage(PulpCoreImage* img, HRSRC resource, HGLOBAL dataHandle)
{
	img->init(resource, dataHandle, topDC);
}

void MyWin32Window::initPulpCoreImage(PulpCoreImage* img, PulpCoreImage* copyFromImage)
{
	img->init(copyFromImage, topDC);
}

void MyWin32Window::initPulpCoreImage(PulpCoreImage* img, char *data, DWORD size)
{
	img->init(data, size, topDC);
}

void MyWin32Window::initPulpCoreImage(PulpCoreImage* img, int width, int height, int bkgrdARGB)
{
	img->init(width, height, bkgrdARGB, topDC, underDC, topBitmap);
}



//Painting
bool MyWin32Window::selectObject(HPEN &obj)
{
	bool res = (SelectObject(underDC, obj)!=NULL);
	return res;
}

bool MyWin32Window::selectObject(HBRUSH &obj)
{
	bool res = (SelectObject(underDC, obj)!=NULL);
	return res;
}

bool MyWin32Window::moveTo(int x, int y)
{
	bool res = (MoveToEx(underDC, x, y, NULL) != 0);
	return res;
}

bool MyWin32Window::drawLineTo(int x, int y)
{
	bool res = (LineTo(underDC, x, y)==TRUE);
	return res;
}


bool MyWin32Window::drawRectangle(int left, int top, int right, int bottom)
{
	bool res = (Rectangle(underDC, left, top, right, bottom)==TRUE);
	return res;
}

bool MyWin32Window::drawImage(PulpCoreImage* img, int x, int y)
{
	img->draw(underDC, x, y);
	return true;
}

bool MyWin32Window::drawString(PulpCoreFont* font, const std::string& str, int x, int y)
{
	font->drawString(underDC, str, x, y);
	return true;
}


bool MyWin32Window::drawString(PulpCoreFont* font, const std::wstring& str, int x, int y)
{
	font->drawString(underDC, str, x, y);
	return true;
}

bool MyWin32Window::drawChar(PulpCoreFont* font, char letter, int xPos, int yPos)
{
	font->drawChar(underDC, letter, xPos, yPos);
	return true;
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

