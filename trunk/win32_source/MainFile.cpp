/*
 * Copyright 2007 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#define _WIN32_WINNT 0x0500 //Run on Windows 2000+
#define _UNICODE
#define UNICODE
//#define _WIN32_WINNT 0x0410 //Run on Windows 98+, fails for KEYBOARD_INPUT

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"

//Resources
//ICON_WZ ICON WZ.ico

//Prototypes
BOOL turnOnHotkeys(HWND hwnd, BOOL on);

//Unique IDs
#define LANG_HOTKEY 142
#define IDC_CURR_WORD_LBL 143
#define STATUS_NID 144
//#define ICON_WZ 145

//Custom message IDs
#define UWM_SYSTRAY (WM_USER + 1)

//Our obnoxious hotkey shortcuts
#define HOTKEY_A 65
#define HOTKEY_B 66
#define HOTKEY_C 67
#define HOTKEY_D 68
#define HOTKEY_E 69
#define HOTKEY_F 70
#define HOTKEY_G 71
#define HOTKEY_H 72
#define HOTKEY_I 73
#define HOTKEY_J 74
#define HOTKEY_K 75
#define HOTKEY_L 76
#define HOTKEY_M 77
#define HOTKEY_N 78
#define HOTKEY_O 79
#define HOTKEY_P 80
#define HOTKEY_Q 81
#define HOTKEY_R 82
#define HOTKEY_S 83
#define HOTKEY_T 84
#define HOTKEY_U 85
#define HOTKEY_V 86
#define HOTKEY_W 87
#define HOTKEY_X 88
#define HOTKEY_Y 89
#define HOTKEY_Z 90
//Lowercase
#define HOTKEY_A_LOW 97
#define HOTKEY_B_LOW 98
#define HOTKEY_C_LOW 99
#define HOTKEY_D_LOW 100
#define HOTKEY_E_LOW 101
#define HOTKEY_F_LOW 102
#define HOTKEY_G_LOW 103
#define HOTKEY_H_LOW 104
#define HOTKEY_I_LOW 105
#define HOTKEY_J_LOW 106
#define HOTKEY_K_LOW 107
#define HOTKEY_L_LOW 108
#define HOTKEY_M_LOW 109
#define HOTKEY_N_LOW 110
#define HOTKEY_O_LOW 111
#define HOTKEY_P_LOW 112
#define HOTKEY_Q_LOW 113
#define HOTKEY_R_LOW 114
#define HOTKEY_S_LOW 115
#define HOTKEY_T_LOW 116
#define HOTKEY_U_LOW 117
#define HOTKEY_V_LOW 118
#define HOTKEY_W_LOW 119
#define HOTKEY_X_LOW 120
#define HOTKEY_Y_LOW 121
#define HOTKEY_Z_LOW 122

//Brushes
HBRUSH g_WhiteBkgrd;
HBRUSH g_BlackBkgrd;

HWND currWordLbl;

//Singletons
INPUT inputItem;
KEYBDINPUT keyInput;

//Global stuff
TCHAR currStr[50];
BOOL mmOn;

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
		{
			//Handle our main language hotkey
			if(wParam == LANG_HOTKEY)
			{
				//Switch language
				BOOL res;
				if (mmOn==TRUE)
					res = turnOnHotkeys(hwnd, FALSE);
				else
					res = turnOnHotkeys(hwnd, TRUE);
				if (res==FALSE)
					MessageBox(NULL, _T("Some hotkeys could not be set..."), _T("Warning"), MB_ICONERROR | MB_OK);

				//Any windows left?
				if (mmOn==FALSE)
					ShowWindow(hwnd, SW_HIDE);
			}

			//Handle our individual keystrokes as hotkeys (less registering that way...)
			int keyCode = wParam;
			if (wParam >= HOTKEY_A && wParam <= HOTKEY_Z)
				keyCode += 32;
			if (wParam >= HOTKEY_A_LOW && wParam <= HOTKEY_Z_LOW) 
			{
				//Is this the first keypress of a romanized word? If so, the window is not visible...
				if (IsWindowVisible(hwnd) == FALSE)
				{
					//Reset it...
					lstrcpy(currStr, _T(""));
					SetDlgItemText(hwnd, IDC_CURR_WORD_LBL, currStr);

					//Show it
					ShowWindow(hwnd, SW_SHOW);
					if (GetForegroundWindow() != hwnd)
						SetForegroundWindow(hwnd);
				}

				//Now, handle the keypress as per the usual...
				TCHAR keyStr[50];
				lstrcpy(keyStr, currStr);
				swprintf(currStr, _T("%s%c"), keyStr, keyCode);
				SetDlgItemText(hwnd, IDC_CURR_WORD_LBL, currStr);
			}

			break;
		}
		case WM_NCHITTEST: //Allow dragging of the client area...
		{
			UINT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if(uHitTest == HTCLIENT)
				return HTCAPTION;
			else
				return uHitTest;
			break;
		}
		case WM_CTLCOLORDLG:
				return (LONG)g_BlackBkgrd;
			break;
		case WM_CHAR:
			{
				if (wParam == ' ') {        //Handle space
					//Send key presses to the top-level program.
					HWND fore = GetNextWindow(hwnd, GW_HWNDNEXT);
					while (fore != NULL) {
						if (IsWindowVisible(fore)) {
							TCHAR res[100];
							if (GetWindowText(fore, res, 100) > -1) {
								SetForegroundWindow(fore);
								fore = GetForegroundWindow();

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
								break;
							}
						}
						fore = GetNextWindow(fore, GW_HWNDNEXT);
					}

					//For now...
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
			//Cleanup
			if (UnregisterHotKey(hwnd, LANG_HOTKEY) == FALSE) 
				MessageBox(NULL, _T("Main Hotkey remains..."), _T("Warning"), MB_ICONERROR | MB_OK);
			if (mmOn==TRUE)
			{
				if (turnOnHotkeys(hwnd, FALSE) == FALSE)
					MessageBox(NULL, _T("Some hotkeys remain..."), _T("Warning"), MB_ICONERROR | MB_OK);
			}

			//Remove systray icon
			NOTIFYICONDATA nid;
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = hwnd;
			nid.uID = STATUS_NID;
			nid.uFlags = NIF_TIP; //??? Needed ???
			Shell_NotifyIcon(NIM_DELETE, &nid);

			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}


BOOL turnOnHotkeys(HWND hwnd, BOOL on) 
{
	int low_code;
	int high_code;
	BOOL retVal = TRUE;

	for (low_code=HOTKEY_A_LOW; low_code<=HOTKEY_Z_LOW; low_code++)  
	{
		high_code = low_code - 32;
		if (on==TRUE) 
		{
			//Register this as an uppercase/lowercase letter
			if (RegisterHotKey(hwnd, high_code, MOD_SHIFT, high_code)==FALSE)
				retVal = FALSE;
			if (RegisterHotKey(hwnd, low_code, NULL, high_code)==FALSE)
				retVal = FALSE;
		} 
		else 
		{
			//De-register this as an uppercase/lowercase letter
			if (UnregisterHotKey(hwnd, high_code)==FALSE)
				retVal = FALSE;
			if (UnregisterHotKey(hwnd, low_code)==FALSE)
				retVal = FALSE;
		}
	}

	mmOn = on;

	return retVal;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;
	NOTIFYICONDATA nid;

	//Stuffz
	LPCWSTR g_szClassName = _T("myWindowClass");

	//Give this process a low background priority
	SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);

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
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
		NULL, NULL, hInstance, NULL
	);

	//Make our "notify icon" data structure
	nid.cbSize = sizeof(NOTIFYICONDATA); //natch
	nid.hWnd = hwnd; //Cauess OUR window to receive notifications for this icon.
	nid.uID = STATUS_NID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //States that the callback message, icon, and size tip are used.
	nid.uCallbackMessage = UWM_SYSTRAY; //Message to send to our window
	nid.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ICON_WZ), IMAGE_ICON,
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); //"Small Icons" are 16x16
	lstrcpy(nid.szTip, _T("WaitZar Myanmar Input System")); //Set tool tip text...

	//Error checking..
	if (nid.hIcon == NULL)
		MessageBox(NULL, _T("Unable to load Icon!"), _T("Warning"), MB_ICONWARNING | MB_OK);

	//Add our icon to the tray
	Shell_NotifyIcon(NIM_ADD, &nid);
	

	//Set our hotkey
	if( RegisterHotKey(hwnd, LANG_HOTKEY, MOD_ALT | MOD_SHIFT, VK_SHIFT)==0 )
		MessageBox(NULL, _T("Hotkey Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
	mmOn = FALSE;	

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
		MessageBox(NULL, _T("Window Creation Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//Show it
	//ShowWindow(hwnd, nCmdShow);
	//UpdateWindow(hwnd);

	//Hide it
	//ShowWindow(hwnd, SW_HIDE);

	//Main message handling loop
	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

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