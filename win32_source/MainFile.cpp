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
#include <string>
//#include <vector>
//#include <hash_map>
//#include "google\sparse_hash_map"
#include "WordBuilder.h"
#include "resource.h"

//Convenience namespace
//using google::sparse_hash_map;

//Resources
//ICON_WZ ICON WZ.ico

//Prototypes
BOOL turnOnHotkeys(HWND hwnd, BOOL on);
void switchToLanguage(HWND hwnd, BOOL toMM);
BOOL loadModel(HINSTANCE hInst);
UINT32 HsiehHash ( std::string *str );

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
HICON mmIcon;
HICON engIcon;
WordBuilder *model;

//Global stuff
TCHAR currStr[50];
BOOL mmOn;


BOOL loadModel(HINSTANCE hInst) {
	//Load our embedded resource, the WaitZar model
	HGLOBAL     res_handle = NULL;
	HRSRC       res;
    char *      res_data;
    DWORD       res_size;

	//NOTE: Store MM segments as Vector<WORD>, since we need to pass individual SendInput() events,
	//      and we need the size (can't just use WORD[])
	//  -update: just use short ints; they'll scale
	//NOTE2: Because the above increases memory usage, we store all "words" as integers pointing
	//      to the dictionary vector.
	//std::vector< std::vector < WORD > > *dictionary = new std::vector< std::vector < WORD > > ;
	//std::vector< sparse_hash_map< std::string, int,  stringhasher, stringhasher> > *nexus = new std::vector< sparse_hash_map< std::string, int,  stringhasher, stringhasher> > ;
	//std::vector< std::pair <sparse_hash_map <int, int>, std::vector<int> > > *prefix = new std::vector< std::pair <sparse_hash_map <int, int>, std::vector<int> > > ;
	WORD **dictionary;
	UINT32 **nexus;
	UINT32 **prefix;

	//Find the resource and get its size, etc.
	res = FindResource(hInst, MAKEINTRESOURCE(WZ_MODEL), _T("Model")); 
	if (!res) {
		MessageBox(NULL, _T("Couldn't find WZ_MODEL"), _T("Error"), MB_ICONERROR | MB_OK);
        return FALSE;
	}
    res_handle = LoadResource(NULL, res);
	if (!res_handle) {
		MessageBox(NULL, _T("Couldn't get a handle on WZ_MODEL"), _T("Error"), MB_ICONERROR | MB_OK);
        return FALSE;
	}
    res_data = (char*)LockResource(res_handle);
    res_size = SizeofResource(NULL, res);

	//Loop through all this
	DWORD currLineStart = 0;
	char currLetter[] = "1000";
	//char currNumber[50];
	int count = 0;
	int mode = 0;
	int lastCommentedNumber = 0;
	int currDictionaryID = 0;
	UINT32 newWord[1000];
	int newWordSz;
	while (currLineStart < res_size) {
		//LTrim
		while (res_data[currLineStart] == ' ')
			currLineStart++;
		//Is this an empty line?
		if (res_data[currLineStart] == '\n') {
			currLineStart++;
			continue;
		}
		//Is this a comment
		else if (res_data[currLineStart] == '#') {
			count = 0;
			mode++;

			//Skip to the end of the line
			lastCommentedNumber = 0;
			for (;;) {
				char curr = res_data[currLineStart++];
				if (curr == '\n')
					break;
				else if (curr >= '0' && curr <= '9') {
					lastCommentedNumber *= 10;
					lastCommentedNumber += (curr-'0');
				}
			}
			switch (mode) {
				case 1: //Words
					//Initialize our dictionary
					dictionary = (WORD **)malloc(lastCommentedNumber * sizeof(WORD *));
					currDictionaryID = 0;
					break;
				case 2: //Nexi
					//Initialize our nexus list
					nexus = (UINT32 **)malloc(lastCommentedNumber * sizeof(UINT32 *));
					currDictionaryID = 0;
					break;
				case 3: //Prefixes
					//Initialize our prefixes list
					prefix = (UINT32 **)malloc(lastCommentedNumber * sizeof(UINT32 *));
					currDictionaryID = 0;
					break;
			}

			continue;
		}

		switch (mode) {
			case 1: //Words
			{
				//Skip until the first number inside the bracket
				while (res_data[currLineStart] != '[')
					currLineStart++;
				currLineStart++;

				//Keep reading until the terminating bracket. 
				//  Each "word" is of the form DD(-DD)*,
				newWordSz = 0;

				for(;;) {
					//Read a "pair"
					currLetter[2] = res_data[currLineStart++];
					currLetter[3] = res_data[currLineStart++];

					//Translate/Add this letter
					newWord[newWordSz++] = (WORD)strtol(currLetter, NULL, 16);

					//Continue?
					char nextChar = res_data[currLineStart++];
					if (nextChar == ',' || nextChar == ']') {
						//Finangle & add this word
						dictionary[currDictionaryID] = (WORD *)malloc((newWordSz+1) * sizeof(WORD));
						dictionary[currDictionaryID][0] = (WORD)newWordSz;
						for (int i=0; i<newWordSz; i++) {
							dictionary[currDictionaryID][i+1] = newWord[i];
						}
						currDictionaryID++;

						if (nextChar == ']')
							break;
						else
							newWordSz = 0;
					}
				}
				break;
			}
			case 2: //Mappings (nexi)
			{
				//Skip until the first letter inside the bracket
				while (res_data[currLineStart] != '{')
					currLineStart++;
				currLineStart++;

				//A new hashtable for this entry. Use sparse_hash_map to keep memory usage down.
				newWordSz=0;
				while (res_data[currLineStart] != '}') {
					//Read a hashed mapping: character
					int nextInt = 0;
					char nextChar = 0;
					while (res_data[currLineStart] != ':')
						nextChar = res_data[currLineStart++];
					currLineStart++;
					
					//Read a hashed mapping: number
					while (res_data[currLineStart] != ',' && res_data[currLineStart] != '}') {
						nextInt *= 10;
						nextInt += (res_data[currLineStart++] - '0');
					}
					
					//Add that entry to the hash
					newWord[newWordSz++] = ((nextInt<<8) | (0xFF&nextChar));

					//Continue
					if (res_data[currLineStart] == ',')
						currLineStart++;
				}

				//Add this entry to the current vector collection
				nexus[currDictionaryID] = (UINT32 *)malloc((newWordSz+1) * sizeof(UINT32));
				nexus[currDictionaryID][0] = (UINT32)newWordSz;
				for (int i=0; i<newWordSz; i++) {
					nexus[currDictionaryID][i+1] = newWord[i];
				}
				currDictionaryID++;

				break;
			}
			case 3: //Prefixes (mapped)
			{
				//Skip until the first letter inside the bracket
				while (res_data[currLineStart] != '{')
					currLineStart++;
				currLineStart++;

				//A new hashtable for this entry. Use sparse_hash_map to keep memory usage down.
				newWordSz = 0;
				int nextVal;
				while (res_data[currLineStart] != '}') {
					//Read a hashed mapping: number
					nextVal = 0;
					while (res_data[currLineStart] != ':') {
						nextVal *= 10;
						nextVal += (res_data[currLineStart++] - '0');
					}
					currLineStart++;

					//Store: key
					newWord[newWordSz++] = nextVal;
					
					//Read a hashed mapping: number
					nextVal = 0;
					while (res_data[currLineStart] != ',' && res_data[currLineStart] != '}') {
						nextVal *= 10;
						nextVal += (res_data[currLineStart++] - '0');
					}
					//Store: val
					newWord[newWordSz++] = nextVal;

					//Continue
					if (res_data[currLineStart] == ',')
						currLineStart++;
				}

				//Used to mark our "halfway" boundary.
				lastCommentedNumber = newWordSz;

				//Skip until the first letter inside the square bracket
				while (res_data[currLineStart] != '[')
					currLineStart++;
				currLineStart++;

				//Add a new vector for these
				while (res_data[currLineStart] != ']') {
					//Read a hashed mapping: number
					nextVal = 0;
					while (res_data[currLineStart] != ',' && res_data[currLineStart] != ']') {
						nextVal *= 10;
						nextVal += (res_data[currLineStart++] - '0');
					}

					//Add it
					newWord[newWordSz++] = nextVal;

					//Continue
					if (res_data[currLineStart] == ',')
						currLineStart++;
				}

				//Add this entry to the current vector collection
				prefix[currDictionaryID] = (UINT32 *)malloc((newWordSz+2) * sizeof(UINT32));
				prefix[currDictionaryID][0] = (UINT32)lastCommentedNumber/2;
				prefix[currDictionaryID][1] = (UINT32)(newWordSz - lastCommentedNumber);
				for (int i=0; i<lastCommentedNumber; i++) {
					prefix[currDictionaryID][i+2] = newWord[i];
				}
				for (UINT32 i=0; i<prefix[currDictionaryID][1]; i++) {
					prefix[currDictionaryID][i+lastCommentedNumber+2] = newWord[i+lastCommentedNumber];
				}
				currDictionaryID++;

				break; 
			}
			default:
				MessageBox(NULL, _T("Too many comments."), _T("Error"), MB_ICONERROR | MB_OK);
				return FALSE;
		}		

		//Assume all processing is done, and read until the end of the line
		while (res_data[currLineStart] != '\n')
			currLineStart++;
		currLineStart++;
	}


	//Do some testing...
	//{
		//Test the dictionary
		/*for (int i=0; i<100; i++) {
			int len = dictionary[i][0];
			
			TCHAR temp[200];
			TCHAR msg[200];
			lstrcpy(msg, _T(""));
			
			for (int j=0; j<len; j++) {
				wsprintf(temp, _T("%s%x "), msg, dictionary[i][j+1]);
				lstrcpy(msg, temp);
			}
			
			MessageBox(NULL, msg, _T("Yo!"), MB_ICONINFORMATION | MB_OK);
		}*/

		//Test the nexus list
		/*for (int i=0; i<100; i++) {
			int len = nexus[i][0];
			
			TCHAR msg[200];
			for (int j=0; j<len && j<3; j++) {
				int val = nexus[i][j+1];
				wsprintf(msg, _T("%c --> %i"), (val&0xFF), (val>>8));

				MessageBox(NULL, msg, _T("Yo!"), MB_ICONINFORMATION | MB_OK);
			}
		}*/

		//Test the prefix list
		/*for (int i=0; i<100; i++) {
			int len1 = prefix[i][0];
			int len2 = prefix[i][1];
			
			TCHAR msg[200];
			TCHAR temp[200];
			lstrcpy(msg, _T(""));
			for (int j=0; j<len1 && j<3; j++) {
				int val1 = prefix[i][j*2 + 2];
				int val2 = prefix[i][j*2 + 3];
				wsprintf(temp, _T("%s%i-->%i "), msg, val1, val2);
				lstrcpy(msg, temp);
			}
			lstrcat(msg, _T("\n"));
			for (int j=0; j<len2 && j<3; j++) {
				int val = prefix[i][2+len1*2+j];
				wsprintf(temp, _T("%s%i "), msg, val);
				lstrcpy(msg, temp);
			}

			MessageBox(NULL, msg, _T("Yo!"), MB_ICONINFORMATION | MB_OK);
		}*/
	//}

	//Save our "model"
	model = new WordBuilder(dictionary, nexus, prefix);

	//Done - This shouldn't matter, though, since the process only 
	//       accesses it once and, fortunately, this is not an external file.
	UnlockResource(res_handle);

	return TRUE;
}



void switchToLanguage(HWND hwnd, BOOL toMM) {
	//Don't do anything if we are switching to the SAME language.
	if (toMM == mmOn)
		return;

	//Ok, switch
	BOOL res;
	if (toMM==TRUE)
		res = turnOnHotkeys(hwnd, TRUE);
	else
		res = turnOnHotkeys(hwnd, FALSE);
	if (res==FALSE)
		MessageBox(NULL, _T("Some hotkeys could not be set..."), _T("Warning"), MB_ICONERROR | MB_OK);

	//Any windows left?
	if (mmOn==FALSE)
		ShowWindow(hwnd, SW_HIDE);
}


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
				return (LONG_PTR)g_WhiteBkgrd;

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
				if (mmOn==TRUE)
					switchToLanguage(hwnd, FALSE);
				else
					switchToLanguage(hwnd, TRUE);
				
				//Reset the model
				model->reset(true);
			}

			//Handle our individual keystrokes as hotkeys (less registering that way...)
			int keyCode = (int)wParam;
			if (wParam >= HOTKEY_A && wParam <= HOTKEY_Z)
				keyCode += 32;
			if (wParam >= HOTKEY_A_LOW && wParam <= HOTKEY_Z_LOW) 
			{
				//Run this keypress into the model. Accomplish anything?
				if (!model->typeLetter(keyCode))
					break;

				//Get list of chars from here
				/*std::vector<UINT32> words =  model->getPossibleWords();
				TCHAR msg[1000];
				TCHAR temp[1000];
				wsprintf(msg, _T("Num words: %i\n"), words.size());
				for (size_t i=0; i<words.size(); i++) {
					std::vector<WORD> thisWord = model->getWordKeyStrokes(words[i]);
					for (size_t x=0; x<thisWord.size(); x++) {
						wsprintf(temp, _T("%s%04x "), msg, thisWord[x]);
						lstrcpy(msg, temp);
					}
					wsprintf(temp, _T("%s\n"), msg);
					//wsprintf(temp, _T("%s%i (%i)\n"), msg, words[i], thisWord.size());
					lstrcpy(msg, temp);
				}
				MessageBox(NULL, msg, _T("Words"), MB_OK|MB_ICONINFORMATION);*/

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
			LRESULT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if(uHitTest == HTCLIENT)
				return HTCAPTION;
			else
				return uHitTest;
			break;
		}
		case UWM_SYSTRAY: //Custom callback for our system tray icon
		{
			POINT pt;
			HMENU hmenu, hpopup;
			HINSTANCE hInst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

			if (lParam==WM_RBUTTONUP || lParam==WM_LBUTTONUP) {
				//Make a popup menu
				GetCursorPos(&pt);
				hmenu = LoadMenu(hInst, MAKEINTRESOURCE(WZ_MENU));
				hpopup = GetSubMenu(hmenu, 0);

				//Cause our popup to appear in front of any other window.
				SetForegroundWindow(hwnd);

				//Force a track on this menu.
				int retVal = TrackPopupMenu(hpopup, //Which menu to track
                                 TPM_RETURNCMD |    //This code specifies that we return the ID of the selected menu item.
                                 TPM_RIGHTBUTTON,   //Track right mouse button
                                 pt.x, pt.y,        //Specifies the menu's location.
                                 0,                 //Reserved (MSDN: _Must_ be zero)
                                 hwnd,              //Owner
                                 NULL);            //MSDN: Ignored
				if (retVal == IDM_HELP) {
					MessageBox(NULL, _T("WaitZar version 1.0 - for more information, see: http://code.google.com/p/waitzar/\n\nAlt+Shift - Switch between Myanmar and English\nType Burmese words like they sound, and press \"space\".\n\nWaitZar users should have Zawgyi-One installed. It works without it, but you'll be \"typing blind\"."), _T("About"), MB_ICONINFORMATION | MB_OK);
				} else if (retVal == IDM_ENGLISH) {
					switchToLanguage(hwnd, FALSE);
				} else if (retVal == IDM_MYANMAR) {
					switchToLanguage(hwnd, TRUE);
				} else if (retVal == IDM_EXIT) {
					DestroyWindow(hwnd);
				}

				//Fixes a bug re: MSKB article: Q135788
				PostMessage(hwnd, 0, 0, 0);

				//Reclaim resources
				DestroyMenu(hmenu);
			}
			break;
		}
		case WM_CTLCOLORDLG:
				return (LONG_PTR)g_BlackBkgrd;
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
								keyInput.wScan=0x1000;//_T('\u1000');
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

	//Change icon in the tray
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = STATUS_NID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //States that the callback message, icon, and size tip are used.
	nid.uCallbackMessage = UWM_SYSTRAY; //Message to send to our window
	lstrcpy(nid.szTip, _T("WaitZar Myanmar Input System")); //Set tool tip text...
	if (mmOn)
		nid.hIcon = mmIcon;
	else
		nid.hIcon = engIcon;

	if (Shell_NotifyIcon(NIM_MODIFY, &nid) == FALSE)
		MessageBox(NULL, _T("Can't switch icon..."), _T("Warning"), MB_ICONERROR | MB_OK);

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

	//Load some icons...
	mmIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ICON_WZ_MM), IMAGE_ICON,
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); //"Small Icons" are 16x16
	engIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ICON_WZ_ENG), IMAGE_ICON,
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); //"Small Icons" are 16x16

	//Make our "notify icon" data structure
	nid.cbSize = sizeof(NOTIFYICONDATA); //natch
	nid.hWnd = hwnd; //Cauess OUR window to receive notifications for this icon.
	nid.uID = STATUS_NID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //States that the callback message, icon, and size tip are used.
	nid.uCallbackMessage = UWM_SYSTRAY; //Message to send to our window
	nid.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ICON_WZ_LOADING), IMAGE_ICON,
                        GetSystemMetrics(SM_CXSMICON),
                        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); //"Small Icons" are 16x16
	lstrcpy(nid.szTip, _T("WaitZar Myanmar Input System")); //Set tool tip text...

	//Error checking..
	if (mmIcon == NULL || engIcon==NULL)
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

	//If we got this far, let's try to load our file.
	if (loadModel(hInstance) == FALSE) {
		DestroyWindow(hwnd);
	}

	//For now...
//	DestroyWindow(hwnd);


	//Show it's ready by changing the shell icon
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = STATUS_NID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; //States that the callback message, icon, and size tip are used.
	nid.uCallbackMessage = UWM_SYSTRAY; //Message to send to our window
	lstrcpy(nid.szTip, _T("WaitZar Myanmar Input System")); //Set tool tip text...
	nid.hIcon = engIcon;
	if (Shell_NotifyIcon(NIM_MODIFY, &nid) == FALSE)
		MessageBox(NULL, _T("Can't switch icon..."), _T("Warning"), MB_ICONERROR | MB_OK);



	//DEBUG
	//ShowWindow(hwnd, nCmdShow);
	//UpdateWindow(hwnd);

	//Hide it
	//ShowWindow(hwnd, SW_HIDE);
	//END DEBUG

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