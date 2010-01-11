/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include ".\MyWin32Window.h"

MyWin32Window::MyWin32Window(LPCWSTR windowClassName, LPCWSTR windowTitle, const HINSTANCE& hInstance, int x, int y, int width, int height, bool useAlpha)
{
	//Manually track the window's width/height
	windowArea.left = x;
	windowArea.top = y;
	windowArea.right = x + width;
	windowArea.bottom = y + height;

	//Create the window
	// We have to use NOACTIVATE because, otherwise, typing text into a box that "selects all on refresh"
	// (like IE's address bar) is almost impossible. Unfortunately, this means our window will
	// receive very few actual events
	window = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_NOACTIVATE | (useAlpha?WS_EX_LAYERED:0), //Keep this window on top, never activate it.
		windowClassName, windowTitle,
		WS_POPUP, //No border or title bar
		windowArea.left, windowArea.top, windowArea.right-windowArea.left, windowArea.bottom-windowArea.top, //Default x,y,width,height (most windows will resize later)
		NULL, NULL, hInstance, NULL); 
}


MyWin32Window::~MyWin32Window()
{
	//Send WM_DESTROY message.
	DestroyWindow(window);
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


bool MyWin32Window::getTextMetrics(LPTEXTMETRICW res)
{
	if (GetTextMetrics(topDC, res)==FALSE)
		return false;
	return true;
}

bool MyWin32Window::postMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (PostMessage(window, msg, wParam, lParam)==FALSE)
		return false;
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

