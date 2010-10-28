/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include ".\MyWin32Window.h"

using std::vector;
using std::pair;


//Useful constants
namespace {
	POINT PT_ORIGIN = { 0 , 0 };
	BLENDFUNCTION BLEND_FULL = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA }; //NOTE: This requires premultiplied pixel values
}

//NOTE: This is mildly dangerous (static initialization)
std::map< std::wstring, MyWin32Window* > MyWin32Window::WndMap;
/*{
	static std::map< LPCWSTR, MyWin32Window* > *res = new std::map< LPCWSTR, MyWin32Window* >();
	return *res;
}*/

HCURSOR MyWin32Window::ArrowCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));


MyWin32Window::MyWin32Window(LPCWSTR windowClassName)
{
	//Save in advance; needed for our "onallcreated" trick
	//Add this class name and the "this" pointer to the global Wnd() array
	this->windowClassName = windowClassName;
	WndMap[windowClassName] = this;

	//Avoid errors
	this->window = NULL;
	this->skipRegionIndexUpdate = false;
	//this->lastActiveRegionID = -1;

	//Prepare for updating our "linked" windows
	skipNextUpdate = 0;
	for (int i=0; i<4; i++)
		linkedWindows[i] = NULL;
}

void MyWin32Window::init(LPCWSTR windowTitle, WNDPROC userWndProc, HBRUSH& bkgrdClr, const HINSTANCE& hInstance, int x, int y, int width, int height, void (*onShowFunction)(void), void (*onAllCreatedFunction)(void), bool useAlpha)
{
	//Save callbacks
	this->onShowFunction = onShowFunction;
	this->onAllCreatedFunction = onAllCreatedFunction;
	this->userWndProc = userWndProc;

	//Init
	this->is_visible = false;
	this->useAlpha = useAlpha;
	this->isDragging = false;

	//Save width and height as default
	this->setDefaultSize(width, height);

	//Manually track the window's width/height
	windowArea.left = x;
	windowArea.top = y;
	windowArea.right = x + width;
	windowArea.bottom = y + height;

	//Does this window class already exist?
	if (WndMap.count(windowClassName)>0 && WndMap[windowClassName]!=this) {
		std::stringstream err;
		err << "Window class already exists " << waitzar::escape_wstr(windowClassName, false);
		throw std::runtime_error(err.str().c_str());
	}

	//Create the window class
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = MyWin32Window::StaticWndProc; //NOTE: This can NOT be set to a non-static member function.
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_SIZEALL);
	wc.hbrBackground = bkgrdClr;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = windowClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if(!RegisterClassEx(&wc)) {
		std::stringstream err;
		err << "Window class registration failed for " << waitzar::escape_wstr(windowClassName);
		throw std::runtime_error(err.str().c_str());
	}


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

	//NOTE: No code below CreateWindowEx; this will IMMEDIATELY trigger a WM_CREATE
}



MyWin32Window::~MyWin32Window()
{
	//Send WM_DESTROY message.
	if (window!=NULL)
		DestroyWindow(window);
}



//General message management
LRESULT CALLBACK MyWin32Window::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//Get the window's class name
	TCHAR wndClassName[256];
	if (GetClassName(hwnd, wndClassName, 256)==0)
		throw std::runtime_error("Window class name doesn't exist for the current hwnd");

	//Get the class that created this window
	if (WndMap.count(wndClassName)==0) {
		std::stringstream err;
		err << "Window class not known: " << waitzar::escape_wstr(wndClassName, false);
		throw std::runtime_error(err.str().c_str());
	}
	MyWin32Window* caller = WndMap[wndClassName];

	//Call the WndProc function for this item
	return caller->MyWndProc(hwnd, msg, wParam, lParam);
}


//Some helpful stuff
MyWin32Window* MyWin32Window::lastMouseFocus = NULL;
void MyWin32Window::UpdateMouseMove(MyWin32Window* currMouseFocus)
{
	//Clear all mouse highlights when mouse-moving onto a new window
	if (currMouseFocus!=lastMouseFocus) {
		if (lastMouseFocus!=NULL) {
			vector<unsigned int> emptyList;
			lastMouseFocus->checkRegionTriggersAndCursor(emptyList);
		}
		if (currMouseFocus!=NULL) {
			//Register a TrackMouseEvent() message to handle leaving this window
			TRACKMOUSEEVENT tev;
			tev.cbSize = sizeof(TRACKMOUSEEVENT);
			tev.dwFlags = TME_LEAVE;
			currMouseFocus->trackMouseEvent(tev);
		}
		lastMouseFocus = currMouseFocus;
	}
}


//Our processing loop for messages
LRESULT CALLBACK MyWin32Window::MyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//Process our own messages
	bool processed = true;
	switch(msg) {
		case WM_CREATE:
		{
			//Save the window, get the DC
			this->topDC = GetDC(hwnd);
			this->window = hwnd;

			//Save logical pixel information
			this->deviceLogPixelsY = GetDeviceCaps(topDC, LOGPIXELSY);

			//Perform checks for "on all created"
			bool allDone = true;
			for (std::map< std::wstring, MyWin32Window* >::iterator item=WndMap.begin(); item!=WndMap.end(); item++) {
				if (!item->second->isWindowCreated()) {
					allDone = false;
					break;
				}
			}
			if (allDone && onAllCreatedFunction!=NULL)
				onAllCreatedFunction();
			break;
		}
		case WM_LBUTTONDOWN:
		{
			//Only drag if we're not over a button. Else, activate that button
			vector<unsigned int> matchedHndls = getRegionAtPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			if (!matchedHndls.empty()) {
				//Pass the handle to the function too.
				for (size_t i=0; i<matchedHndls.size(); i++) {
					regionHandles[matchedHndls[i]].second.OnClick(matchedHndls[i]);
				}
				checkRegionTriggersAndCursor(matchedHndls);
			} else {
				//Thanks to dr. Carbon for suggesting this method.
				if (SetCapture(window)!=NULL)
					break;

				//Drag the mosue
				isDragging = true;
				GetCursorPos(&dragFrom);
			}
			break;
		}
		case WM_MOUSEMOVE:
		{ 
			if (isDragging) {
				//Allow dragging of the mouse by its client area. Reportedly more accurate than NCHIT_TEST
				RECT rect;
				POINT dragTo;
				GetWindowRect(window, &rect);
				GetCursorPos(&dragTo);

				//Constantly update its position
				MoveWindow(window, (dragTo.x - dragFrom.x) + rect.left,
					(dragTo.y - dragFrom.y) + rect.top,
					rect.right - rect.left, rect.bottom - rect.top, FALSE);

				dragFrom = dragTo;
			} else {
				//Handle any subscriptions; change the cursor if the mouse is over them.
				checkRegionTriggersAndCursor(getRegionAtPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

				//Handle subscriptions across multiple windows
				MyWin32Window::UpdateMouseMove(this);
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			if (isDragging) {
				isDragging = false;
				ReleaseCapture();
			} else {
				//We might have to reset the cursor once the mouse is released. 
				// (Tested: it's necessary, unless we choose to remove the class cursor entirely)
				checkRegionTriggersAndCursor(getRegionAtPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
			}
			break;
		}
		case WM_MOUSELEAVE: //Only works if we are "tracking mouse events"
		{
			vector<unsigned int> emptyVec;
			checkRegionTriggersAndCursor(emptyVec);
			MyWin32Window::UpdateMouseMove(NULL); //No longer tracking
			break;
		}
		case WM_MOVE:
		{
			//Move any linked windows
			for (int i=0; i<4; i++) {
				//Nothing here?
				if (linkedWindows[i]==NULL)
					continue;

				//Do we skip this update?
				if (skipNextUpdate>0) {
					skipNextUpdate--;
					continue;
				}

				//Is the other window visible?
				if (!linkedWindows[i]->isVisible())
					continue;

				//Skip this window's next update step
				// Note: This must be done before we move the window, obviously.
				linkedWindows[i]->skipMoveUpdates();

				//Finally, move it. Make sure it doesn't leave the desktop, either
				RECT myRect;
				GetWindowRect(window, &myRect);
				RECT deskRect;
				GetWindowRect(GetDesktopWindow(), &deskRect);

				//NOTE: This is not generic yet; it just works for our special case. Maybe change later?
				if (i==SOUTH)
					linkedWindows[i]->setWindowPosition(min(max(myRect.left, 0), deskRect.right-linkedWindows[i]->getClientWidth()), min(myRect.top+this->getClientHeight(), deskRect.bottom-linkedWindows[i]->getClientHeight()), 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
				else if (i==NORTH)
					linkedWindows[i]->setWindowPosition(min(max(myRect.left, 0), deskRect.right-linkedWindows[i]->getClientWidth()), max(myRect.top-linkedWindows[i]->getClientHeight(), 0), 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
				else
					throw std::runtime_error("Cannot move windows linked EAST/WEST (Todo...)");
			}

			break;
		}
		case WM_PAINT:
		{
			//Update only if there's an area which needs updating (e.g., a higher-level
			//  window has dragged over this one's client area... it can happen only with popups,
			//  but let's do it just to be safe.
			RECT updateRect;
			if (GetUpdateRect(window, &updateRect, FALSE) != 0)
			{
				//Blitting every tick will slow us down... we should validate the
				//  rectangle after drawing it.
				this->repaintWindow(updateRect);

				//Validate the client area
				ValidateRect(window, NULL);
			}

			break;
		}
		case WM_CLOSE:
		{
			DestroyWindow(window);
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		default:
			processed = false;
	}

	//Process user messages
	if (this->userWndProc!=NULL)
		processed = (this->userWndProc(hwnd, msg, wParam, lParam)==0) || processed;

	//Return zero if processed
	if (processed)
		return 0;

	//Else, return the default process code
	return DefWindowProc(hwnd, msg, wParam, lParam);
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
	topBitmap = CreateCompatibleBitmap(topDC, this->getClientWidth(), this->getClientHeight());
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
//DO? repaint.
bool MyWin32Window::resizeWindow(int newWidth, int newHeight, bool doRepaint)
{
	RECT r;
	GetWindowRect(window, &r);
	bool res = (MoveWindow(window, r.left, r.top, newWidth, newHeight, (doRepaint?TRUE:FALSE))==TRUE);

	//Bookkeeping
	windowArea.right = windowArea.left + newWidth;
	windowArea.bottom = windowArea.top + newHeight;
	GetClientRect(window, &clientArea);

	return res;
}

//Need to merge this with resize eventually...
bool MyWin32Window::expandWindow(int newX, int newY, int newWidth, int newHeight, bool dontMove)
{
	//Don't expand if the new width/height are the same, and we're not moving it.
	if ((dontMove || (newX==this->getXPos() && newY==this->getYPos())) && newWidth==this->getWidth() && newHeight==this->getHeight())
		return false;

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


bool MyWin32Window::hiliteMenu(const HMENU& containerMenu, unsigned int hilightID, bool highlightON)
{
	return (HiliteMenuItem(window, containerMenu, hilightID, MF_BYCOMMAND|(highlightON?MF_HILITE:MF_UNHILITE))==TRUE);
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

	//If we're hiding the window, de-highlight all regions
	if (!show) {
		vector<unsigned int> emptyList;
		checkRegionTriggersAndCursor(emptyList);
	}

	//Set flags, perform show/hide
	ShowWindow(window, show?SW_SHOW:SW_HIDE);
	is_visible = show;

	//ShowWindow() returns a BOOL, but that represents whether or not it WAS visible.
	// So... it always succeeds? 
	return true; 
}

bool MyWin32Window::repaintWindow(RECT blitArea)
{
	bool res = false;
	if (this->useAlpha) {
		//Use the "Alpha" command (No rectangle at the moment...)
		SIZE sz;
		sz.cx = this->getClientWidth();
		sz.cy = this->getClientHeight();
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
		//Use the "UpdateLayeredWindow" command
		SIZE sz;
		sz.cx = this->getClientWidth();
		sz.cy = this->getClientHeight();
		res = (UpdateLayeredWindow(window, GetDC(NULL), NULL, &sz, underDC, &PT_ORIGIN, 0, &BLEND_FULL, ULW_ALPHA)==TRUE);
	} else {
		//Use the "Blit" command
		res = (BitBlt(topDC,0,0,this->getClientWidth(),this->getClientHeight(),underDC,0,0,SRCCOPY)==TRUE);
	}
	return res;
}

int MyWin32Window::getXPos()
{
	return windowArea.left;
}

int MyWin32Window::getYPos()
{
	return windowArea.top;
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
	return defaultArea.right-defaultArea.left;
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

void MyWin32Window::initDisplayMethod(DisplayMethod* img, HRSRC resource, HGLOBAL dataHandle, unsigned int defaultColor)
{
	img->init(resource, dataHandle, topDC, deviceLogPixelsY, defaultColor);
}

void MyWin32Window::initDisplayMethod(DisplayMethod* img, char *data, DWORD size, unsigned int defaultColor)
{
	img->init(data, size, topDC, defaultColor);
}

void MyWin32Window::initTtfMethod(DisplayMethod* img, HRSRC resource, HGLOBAL dataHandle, unsigned int defaultColor)
{
	img->init(resource, dataHandle, topDC, deviceLogPixelsY, defaultColor);
}

void MyWin32Window::initTtfMethod(DisplayMethod* img, const std::wstring& fontFileName, unsigned int defaultColor)
{
	img->init(fontFileName, topDC, defaultColor, deviceLogPixelsY);
}


void MyWin32Window::initPulpCoreImage(PulpCoreImage* img, HRSRC resource, HGLOBAL dataHandle)
{
	img->init(resource, dataHandle, topDC);
}

void MyWin32Window::initPulpCoreImage(PulpCoreImage* img, PulpCoreImage* copyFromImage)
{
	img->init(copyFromImage, topDC);
}

void MyWin32Window::initPulpCoreImage(PulpCoreFont* font, PulpCoreFont* copyFromFont, unsigned int defaultColor)
{
	font->init(copyFromFont, topDC, defaultColor);
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

bool MyWin32Window::drawPolygon(const POINT *points, int numPoints)
{
	bool res = (Polygon(underDC, points, numPoints)==TRUE);
	return res;
}

bool MyWin32Window::drawImage(PulpCoreImage* img, int x, int y)
{
	img->draw(underDC, x, y);
	return true;
}


bool MyWin32Window::drawImage(PulpCoreImage* img, int x, int y, unsigned int cropLeft, unsigned int cropRight, unsigned int cropTop, unsigned int cropBottom)
{
	img->draw(underDC, x, y, cropLeft, cropRight, cropTop, cropBottom);
	return true;
}

unsigned int MyWin32Window::getStringWidth(DisplayMethod* font, const std::wstring& str, const std::wstring& filterStr, size_t filterLetterWidth)
{
	return font->getStringWidth(str, underDC, filterStr, filterLetterWidth);
}


bool MyWin32Window::drawString(DisplayMethod* font, const std::string& str, int x, int y)
{
	font->drawString(underDC, str, x, y);
	return true;
}


bool MyWin32Window::drawString(DisplayMethod* font, const std::wstring& str, int x, int y, const std::wstring& filterStr, size_t filterLetterWidth)
{
	font->drawString(underDC, str, x, y, filterStr, filterLetterWidth);
	return true;
}

bool MyWin32Window::drawChar(DisplayMethod* font, char letter, int xPos, int yPos)
{
	font->drawChar(underDC, letter, xPos, yPos);
	return true;
}


bool MyWin32Window::isWindowCreated()
{
	return this->window != NULL;
}


bool MyWin32Window::trackMouseEvent(TRACKMOUSEEVENT& mev)
{
	mev.hwndTrack = this->window;
	return (TrackMouseEvent(&mev)==TRUE);
}



//Linking
void MyWin32Window::linkToWindow(MyWin32Window* other, ATTACH_DIRECTION linkAt)
{
	//Get the alternating direction from the one we're linking to.
	ATTACH_DIRECTION linkFrom = (linkAt==NORTH ? SOUTH : linkAt==SOUTH ? NORTH : linkAt==EAST ? WEST : EAST);

	//Check if it's already attached
	if (this->linkedWindows[linkAt]!=NULL || other->linkedWindows[linkFrom]!=NULL)
		throw std::runtime_error("Cannot link: one of these windows is already linked");

	//Attach.
	this->linkedWindows[linkAt] = other;
	other->linkedWindows[linkFrom] = this;
}


//Return by ref. to allow assigning and saving
void MyWin32Window::skipMoveUpdates()
{
	this->skipNextUpdate++;
}


/////////////////////////////////////////////////
// Region-based code, to simulate button presses
/////////////////////////////////////////////////

//Used to test if we're indexing our region.
bool MyWin32Window::requireRegionIndex()
{
	return regionHandles.size() >= regionIndexThreshhold;
}

//Index consists of a vector of positions, with each position containing the "start" or "end" of a region (by ID)
void MyWin32Window::insertIndexValue(std::vector<IndexEntry> &vec, unsigned int id, unsigned int val, bool valIsStart)
{
	//First, find the place to insert this value
	std::vector<IndexEntry>::iterator it = vec.begin();
	for (;it!=vec.end(); it++) {
		if (it->currVal >= val)
			break;
	}

	//Insert if the value is greater (or if we're at the end)
	if (it==vec.end() || it->currVal!=val) {
		IndexEntry newEntry;
		newEntry.currVal = val;
		it = vec.insert(it, newEntry);
	}

	//Now, add to it
	if (valIsStart)
		it->startRect.push_back(id);
	else
		it->endRect.push_back(id);
}

//Used to build a lookup table for the regions, to handle an excessively large number of region subscriptions.
// In WZ, this only happens for the Help Keyboard.
void MyWin32Window::recalcRegionIndex()
{
	//Only for large subscriptions
	if (!requireRegionIndex())
		return;

	//Update the index
	//TO-DO: This only works well if there's not a lot of inserts/deletes.
	//       For our virtual keyboard, that's fine, since many keys share the 
	//       same X/Y positions. But we should really re-build this with a linnked 
	//       list for the general case.
	sortedXIndex.clear();
	sortedYIndex.clear();
	maxWidth = 0;
	maxHeight = 0;
	for (size_t i=0; i<regionHandles.size(); i++) {
		//Update index
		insertIndexValue(sortedXIndex, i, regionHandles[i].first.left, true);
		insertIndexValue(sortedXIndex, i, regionHandles[i].first.right, false);
		insertIndexValue(sortedYIndex, i, regionHandles[i].first.top, true);
		insertIndexValue(sortedYIndex, i, regionHandles[i].first.bottom, false);

		//Update max width/height
		size_t width = regionHandles[i].first.right - regionHandles[i].first.left + 1;
		maxWidth = (width>maxWidth) ? width : maxWidth;
		size_t height = regionHandles[i].first.bottom - regionHandles[i].first.top + 1;
		maxHeight = (height>maxHeight) ? height : maxHeight;
	}
}



void MyWin32Window::searchRegIndexAxis(vector<int>& resByID, const vector<IndexEntry>& sortIndex, size_t searchVal, unsigned int maxSearchW)
{
	//Find the nearest <= value
	//TO-DO: Use a binary search (later). For now linear is fast enough
	int currIndexID = -1;
	for (size_t nextIndexID=0;nextIndexID<sortIndex.size(); nextIndexID++) {
		if (sortIndex[nextIndexID].currVal > searchVal)
			break;
		currIndexID = nextIndexID;
	}

	//Now, search backwards; adding and removing as needed.
	for (;currIndexID!=-1;) {
		//Update
		for (vector<unsigned int>::const_iterator it=sortIndex[currIndexID].startRect.begin(); it!=sortIndex[currIndexID].startRect.end(); it++)
			resByID[*it]++;
		if (sortIndex[currIndexID].currVal != searchVal) {
			for (vector<unsigned int>::const_iterator it=sortIndex[currIndexID].endRect.begin(); it!=sortIndex[currIndexID].endRect.end(); it++)
				resByID[*it]--;
		}

		//Break? Increment. Break?
		if (currIndexID==0)
			break;
		currIndexID--;
		if (searchVal-sortIndex[currIndexID].currVal > maxSearchW)
			break;
	}
}


//Return the region at a given client location.
std::vector<unsigned int> MyWin32Window::getRegionAtPoint(size_t clientX, size_t clientY)
{
	std::vector<unsigned int> res;

	if (!requireRegionIndex()) {
		//Just do a simple iterative search.
		for (size_t i=0; i<regionHandles.size(); i++) {
			if (   (int)clientX>=regionHandles[i].first.left && (int)clientX<=regionHandles[i].first.right
				&& (int)clientY>=regionHandles[i].first.top && (int)clientY<=regionHandles[i].first.bottom)
				res.push_back(i);
		}
	} else {
		//The way this works is simple. First, do a binary search for the index <= the current value.
		//   Then, go back MAX_WIDTH/HEIGHT (or to 0), adding 1 for each "start" value found and -1 for each "end" value
		//   Do this for each axis; any IDs that accumulate a value of 2 have been "matched".
		//NOTE: We ignore end-points on the first point if it matches the value exactly.
		vector<int> results(regionHandles.size(), 0); //CHECK: Is this how to init properly?
		searchRegIndexAxis(results, sortedXIndex, clientX, maxWidth);
		searchRegIndexAxis(results, sortedYIndex, clientY, maxHeight);

		//Covnert our arrays of [-2..2] to an array containing only "passed" values.
		for (size_t i=0; i<results.size(); i++) {
			if (results[i]==2)
				res.push_back(i);
		}
	}

	return res;
}

//Suppress updates
void MyWin32Window::beginMassSubscription()
{
	skipRegionIndexUpdate = true;
}

//Stop suppressing updates.
void MyWin32Window::endMassSubscription()
{
	skipRegionIndexUpdate = false;

	//Rebuild the index. Not necessary in all cases, but we'll do it anyway
	recalcRegionIndex();
}

//Add a rectangle to the list of subscriptions. Update the index.
unsigned int MyWin32Window::subscribeRect(const RECT& area, void(*onclick)(unsigned int), void(*onover)(unsigned int), void(*onout)(unsigned int))
{
	if (area.right<=area.left || area.bottom<=area.top)
		throw std::runtime_error("Error: invalid rectangle in subscribeRect().");

	RegionActions reg = {onclick, onover, onout};
	regionHandles.push_back(std::pair<RECT, RegionActions>(area, reg));
	if (!skipRegionIndexUpdate)
		recalcRegionIndex();
	return regionHandles.size()-1;
}
unsigned int MyWin32Window::subscribeRect(const RECT& area, void(*onclick)(unsigned int))
{
	return subscribeRect(area, onclick, NULL, NULL);
}

//Update a rectangle's subscription area. Re-compute the index
void MyWin32Window::updateRect(unsigned int handle, const RECT& area)
{
	if (handle<regionHandles.size())
	{
		regionHandles[handle].first = area;
		if (!skipRegionIndexUpdate)
			recalcRegionIndex();
	}
}


void MyWin32Window::checkRegionTriggersAndCursor(const vector<unsigned int>& matchedRegions)
{
	//For each region that has been removed, fire an "OnOut" message.
	for (vector<unsigned int>::const_iterator it=prevActiveRegIDs.begin(); it!=prevActiveRegIDs.end(); it++) {
		vector<unsigned int>::const_iterator it2=matchedRegions.begin();
		for (; it2!=matchedRegions.end() && (*it2)!=(*it); it2++);

		//Not found?
		if (it2==matchedRegions.end() && regionHandles[*it].second.OnOut!=NULL)
			regionHandles[*it].second.OnOut(*it);
	}


	//For each region that has been added, fire a "OnOver" message.
	for (vector<unsigned int>::const_iterator it=matchedRegions.begin(); it!=matchedRegions.end(); it++) {
		vector<unsigned int>::const_iterator it2=prevActiveRegIDs.begin();
		for (; it2!=prevActiveRegIDs.end() && (*it2)!=(*it); it2++);

		//Not found?
		if (it2==prevActiveRegIDs.end() && regionHandles[*it].second.OnOver!=NULL)
			regionHandles[*it].second.OnOver(*it);
	}	

	//Save
	prevActiveRegIDs = matchedRegions;

	//Set the cursor to an arrow if we're at least over ONE thing.
	if (!matchedRegions.empty())
		SetCursor(ArrowCursor);
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

