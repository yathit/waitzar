/*
 * Copyright 2007 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#define _WIN32_WINNT 0x0501
#define _UNICODE
#define UNICODE
//#define _WIN32_WINNT 0x0410 //Run on Windows 98+

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

//Unique IDs
#define IDC_CURR_WORD_LBL 101
#define LANG_HOTKEY 142

//Brushes
HBRUSH g_WhiteBkgrd;
HBRUSH g_BlackBkgrd;

HWND currWordLbl;

//Singletons
INPUT inputItem;
KEYBDINPUT keyInput;

//Global stuff
TCHAR currStr[50];

/*BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
	DWORD dwThreadId, dwProcessId;
	HINSTANCE hInstance;
	char String[255];
	HANDLE hProcess;

	if (!hWnd)
		return TRUE;		// Not a window
	if (!::IsWindowVisible(hWnd))
		return TRUE;		// Not visible
	if (!SendMessage(hWnd, WM_GETTEXT, sizeof(String), (LPARAM)String))
		return TRUE;		// No window title
	hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	//dwThreadId = GetWindowThreadProcessId(hWnd, &dwProcessId);
	//hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	PWINDOWINFO info;
	GetWindowInfo(hwnd, info);

	MessageBox(hwnd, String, "Window Info", MB_OK|MB_ICONHAND);
	//cout << hWnd << ' ' << dwProcessId << '\t' << String << '\t';
	// GetModuleFileNameEx uses psapi, which works for NT only!
	//if (GetModuleFileNameEx(hProcess, hInstance, String, sizeof(String)))
		//MessageBox(hwnd, String, "Window Info", MB_OK|MB_ICONHAND);
		//cout << String << endl;
	//else
		//MessageBox(hwnd, "None", "Window Info", MB_OK|MB_ICONHAND);
		//cout << "(None)\n";
	CloseHandle(hProcess);
	return TRUE;
}*/


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//Handle callback
	switch(msg) {
		case WM_CREATE:
		{
			//Change its color
			//NOTE: not correct.
			/*HDC hdcStatic = GetDC(currWordLbl);

			if (hdcStatic == NULL)
				MessageBox(hwnd, "Can't get HDC", "Error", MB_OK|MB_ICONERROR);

			SetTextColor(hdcStatic, RGB(0, 128, 0));

			ReleaseDC(currWordLbl, hdcStatic);*/

//			SendDlgItemMessage(hwnd, IDC_CURR_WORD_LBL, WM_CTLCOLORSTATIC, (WPARAM)RGB(0, 255, 0), TRUE);
//			SendMessage(hwnd, WM_CTLCOLORDLG, (WPARAM)RGB(0, 0, 0), TRUE);

			break;
		}
		case WM_CTLCOLORSTATIC:
		{
			int staticID = GetDlgCtrlID((HWND)lParam);
			if (staticID == IDC_CURR_WORD_LBL)
			{
				HDC hdcStatic = (HDC)wParam;
				SetTextColor(hdcStatic, RGB(0, 128, 0));
				SetBkMode(hdcStatic, TRANSPARENT);
				return (LONG)g_WhiteBkgrd;

				//Can't do this without returning a brush color anyways...
				//SetBkColor(hdcStatic, RGB(255, 255, 255));
			}
			break;
		}
		case WM_HOTKEY:
			if( wParam == LANG_HOTKEY)
			{
				//Show/hide the window. This allows us to "debug", and will later be used for
				// the actual language switch.
				if (IsWindowVisible(hwnd))
					ShowWindow(hwnd, SW_HIDE);
				else {
					ShowWindow(hwnd, SW_SHOW);
					if (GetForegroundWindow() != hwnd)
						SetForegroundWindow(hwnd);
				}
			}
			break;
		case WM_CTLCOLORDLG:
				return (LONG)g_BlackBkgrd;
			break;
		case WM_CHAR:
			//Only handle ASCII keys
			/*if (wParam&0xFF != wParam)
				break;*/
			{
				//Capital to lowercase
				TCHAR key = wParam;
				TCHAR keyStr[5];
				if (key >= 'A' && key <= 'Z')
					key += 32;

				if (key >= 'a' && key <= 'z') { //Handle letter keys
					swprintf(currStr, _T("%s%c"), keyStr, key);
					//StrCatW(currStr, keyStr);
					//TCHAR str = _T(currStr);
					SetDlgItemText(hwnd, IDC_CURR_WORD_LBL, currStr);
				} else if (key == ' ') {        //Handle space
					//Send key presses to the top-level program.
	//				EnumWindows(EnumWindowsProc, NULL);
					HWND fore = GetNextWindow(hwnd, GW_HWNDNEXT);
					while (fore != NULL) {
						if (IsWindowVisible(fore)) {
							TCHAR res[100];
							if (GetWindowText(fore, res, 100) > -1) {
								SetForegroundWindow(fore);

								fore = GetForegroundWindow();
								//SendMessage(fore, WM_CHAR, 'K', 0);

								//Try SendInput() instead of SendMessage()
								inputItem.type=INPUT_KEYBOARD;
								keyInput.wVk=0;
								keyInput.wScan=_T('\u1000');
								keyInput.dwFlags=KEYEVENTF_UNICODE;
								keyInput.time=0;
								keyInput.dwExtraInfo=0;
								inputItem.ki=keyInput;
								if(!SendInput(1,&inputItem,sizeof(INPUT)))
								{
									MessageBox(hwnd, _T("Couldn't send input"), _T("Error"), MB_OK|MB_ICONERROR);
								}
								/*inputItem.type = INPUT_KEYBOARD;
								inputItem.ki.wVk = 'k';
								inputItem.ki.dwFlags = 0;
								inputItem.ki.time = 0;
								inputItem.ki.wScan = 0;
								inputItem.ki.dwExtraInfo = 0;
								SendInput(1,inputItem,sizeof(INPUT));*/

								//MessageBox(hwnd, res, "Sent to window:", MB_OK|MB_ICONHAND);
								break;
							}
						}
						fore = GetNextWindow(fore, GW_HWNDNEXT);
					}
					/*int i;
					for (i=0; i<50; i++) {
						if (currStr[i] == '\0')
							break;

						//Send this key to the foreground window.

					}*/

					//For now...
					//PostQuitMessage(0);
					ShowWindow(hwnd, SW_HIDE);
				}
			}


			//char res[20];
			//char buf[10];
			//MessageBox(hwnd, strcat(strcpy(res, "Key: "), itoa(wParam, buf, 10)), "Color", MB_OK|MB_ICONASTERISK);
			break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	//Stuffz
	LPCWSTR g_szClassName = _T("myWindowClass");

	//Create a white/black brush
	g_WhiteBkgrd = CreateSolidBrush(RGB(255, 255, 255));
	g_BlackBkgrd = CreateSolidBrush(RGB(0, 0, 0));

	//Set window's class parameters
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = g_BlackBkgrd;//(HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL, _T("Window Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//Create a handle to the window
	hwnd = CreateWindowEx(
		0, //No style
		g_szClassName,
		_T("WaitZar"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
		NULL, NULL, hInstance, NULL
	);

	//Set our hotkey
	if(RegisterHotKey(hwnd, LANG_HOTKEY, MOD_ALT | MOD_SHIFT, VK_SHIFT)==0 )
		MessageBox(NULL, _T("Hotkey Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);

	//Create a label
	LPTSTR str = TEXT("ko");
	currWordLbl = CreateWindowEx(
		0, //No style
		_T("STATIC"),
		str,
		WS_CHILD | WS_VISIBLE ,
		0, 0, 100, 30,
		hwnd, (HMENU)IDC_CURR_WORD_LBL, GetModuleHandle(NULL),
		NULL);


	//Initialize our romanisation string
	lstrcpy(currStr, _T(""));


//	HDC hdcStatic = GetDC(currWordLbl);
//	SetTextColor(hdcStatic, RGB(0, 255, 0));
//	SetBkColor(hdcStatic, RGB(255, 255, 255));

	//Success?
	if(hwnd == NULL || currWordLbl == NULL)
	{
		MessageBox(NULL, _T("Window Creation Failed!"), _T("Error!"),
		MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//Show it
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	//Main message handling loop
	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	//Cleanup
	UnregisterHotKey(hwnd, LANG_HOTKEY);

	//Done
	return (int)Msg.wParam;
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