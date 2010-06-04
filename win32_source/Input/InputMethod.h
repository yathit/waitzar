/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _INPUT_METHOD
#define _INPUT_METHOD

#include <string>
#include <vector>
#include "MyWin32Window.h"
#include "Hotkeys.h"
#include "OnscreenKeyboard.h"
#include "Settings/Types.h"
#include "Settings/Encoding.h"
#include "NGram/wz_utilities.h"
#include "Input/keymagic_vkeys.h"



enum CONTROL_KEY_STYLES {CK_CHINESE, CK_JAPANESE};

//Hilite styles
enum HILITE_FLAGS {HF_NOTHING=0, HF_PATSINT=1, HF_CURRSELECTION=2, HF_LABELTILDE=4};


//Structure for virtual key presses
struct VirtKey {
	//The virtual key code of the this keypress. Guaranteed.
	unsigned int vkCode;

	//The alpha-numeric value of this keypress. (Also: ascii value for some additional, common keys)
	//  Guaranteed to be 'a'-'z' for [a-zA-Z].
	//  Guaranteed to be '0'-'9' for any numeric (numpad, regular).
	//  Guaranteed to be "!@#$%^&*()" for the shifted number keys, respectively.
	//  Guaranteed to be '`', '~', '-', '_', '=', '+', for the other top-row keys.
	//  Guaranteed to be '[', '{', ']', '}', '\\', '|', for the other first-row keys.
	//  Guaranteed to be ';', ':', '\'', '"', for the other second-row keys.
	//  Guaranteed to be ',', '<', '.', '>', '/', '?', for the other third-row keys.
	//  Guaranteed to be '\0' for anything else.
	char alphanum; 

	//Modifiers. Guaranteed
	bool modShift;
	bool modAlt;
	bool modCtrl;

	//Possibly use more modifiers? 

	//Useful functions: constructor
	VirtKey(unsigned int vkCode, char alphanum, bool modShift, bool modAlt, bool modCtrl) 
		: vkCode(vkCode), alphanum(alphanum), modShift(modShift), modAlt(modAlt), modCtrl(modCtrl) {}

	//Useful functions: construct from an lParam (presumably a WM_HOTKEY message)
	VirtKey(LPARAM wmHotkeyLParam) {
		//Easy
		vkCode = HIWORD(wmHotkeyLParam);
		modShift = (LOWORD(wmHotkeyLParam)&MOD_SHIFT) != 0;
		modCtrl = (LOWORD(wmHotkeyLParam)&MOD_CONTROL) != 0;
		modAlt = (LOWORD(wmHotkeyLParam)&MOD_ALT) != 0;

		//Alphanum: letters & numbers
		if (vkCode>='a' && vkCode<='z')
			alphanum = vkCode;
		else if (vkCode>='A' && vkCode<='Z') 
			alphanum = (vkCode-'A')+'a';
		else if (vkCode>='0' && vkCode<='9' && !modShift)
			alphanum = vkCode;
		else if (vkCode>=VK_NUMPAD0 && vkCode<=VK_NUMPAD9)
			alphanum = (vkCode-VK_NUMPAD0)+'0';
		else {
			switch (vkCode) {
				case '1':
					alphanum = '!';  break;
				case '2':
					alphanum = '@';  break;
				case '3':
					alphanum = '#';  break;
				case '4':
					alphanum = '$';  break;
				case '5':
					alphanum = '%';  break;
				case '6':
					alphanum = '^';  break;
				case '7':
					alphanum = '&';  break;
				case '8':
					alphanum = '*';  break;
				case '9':
					alphanum = '(';  break;
				case '0':
					alphanum = ')';  break;
				case VK_OEM_3:
					alphanum = modShift ? '~' : '`';  break;
				case VK_OEM_MINUS:
					alphanum = modShift ? '_' : '-';  break;
				case VK_OEM_PLUS:
					alphanum = modShift ? '+' : '=';  break;
				case VK_OEM_4:
					alphanum = modShift ? '{' : '[';  break;
				case VK_OEM_6:
					alphanum = modShift ? '}' : ']';  break;
				case VK_OEM_5:
					alphanum = modShift ? '|' : '\\';  break;
				case VK_OEM_1:
					alphanum = modShift ? ':' : ';';  break;
				case VK_OEM_7:
					alphanum = modShift ? '"' : '\'';  break;
				case VK_OEM_COMMA:
					alphanum = modShift ? '<' : ',';  break;
				case VK_OEM_PERIOD:
					alphanum = modShift ? '>' : '.';  break;
				case VK_OEM_2:
					alphanum = modShift ? '?' : '/';  break;
				default:
					alphanum = '\0';
			}
		}
	}

	//Useful functions: represent as a key magic integer.
	unsigned int toKeyMagicVal() {
		//Some Japanese keyboards can generate very large keycode values. Ignore these for now.
		unsigned int ret = (vkCode & KM_VKMOD_MASK);
		if (ret != vkCode)
			return 0;

		//Add parameters
		if (modShift)
			ret |= KM_VKMOD_SHIFT;
		if (modCtrl)
			ret |= KM_VKMOD_CTRL;
		if (modAlt)
			ret |= KM_VKMOD_ALT;

		return ret;
	}

};



//Expected interface: "Input Method"
class InputMethod {
public:
	InputMethod();
	virtual ~InputMethod();

	//Cascading init
	virtual void init(MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow, const std::vector< std::pair <int, unsigned short> > &systemWordLookup, OnscreenKeyboard *helpKeyboard, std::wstring systemDefinedWords);

	//Allow map comparison 
	bool operator<(const InputMethod& other) const {
		return id < other.id;
	}

	//Allow logical equals and not equals
	bool operator==(const InputMethod &other) const {
		return id == other.id;
	}
	bool operator!=(const InputMethod &other) const {
		return id != other.id;
	}

	//Allow eq/neq on strings, too
	bool operator==(const std::wstring& other) const {
		return id == other;
	}
	bool operator!=(const std::wstring& other) const {
		return id != other;
	}

public:
	//Struct-like properties
	std::wstring id;
	std::wstring displayName;
	Encoding encoding;
	TYPES type;
	bool suppressUppercase;
	bool typeNumeralConglomerates;
	bool disableCache;
	CONTROL_KEY_STYLES controlKeyStyle;

	//Useful functionality
	virtual void treatAsHelpKeyboard(InputMethod* providingHelpFor);
	bool isHelpInput();
	void forceViewChanged();
	bool getAndClearViewChanged();
	bool getAndClearRequestToTypeSentence();
	std::pair <std::string, std::wstring> getAndClearMostRecentRomanizationCheck();

	//Keypress handlers (abstract virtual)
	virtual void handleEsc() = 0;
	virtual void handleBackspace(VirtKey& vkey) = 0;
	virtual void handleDelete() = 0;
	virtual void handleLeftRight(bool isRight, bool loopToZero) = 0;
	virtual void handleUpDown(bool isDown) = 0;
	virtual void handleCommit(bool strongCommit) = 0;
	virtual void handleNumber(VirtKey& vkey, bool typeBurmeseNumbers) = 0;
	virtual void handleStop(bool isFull, VirtKey& vkey) = 0;
	virtual void handleTab() = 0;

	virtual void handleKeyPress(VirtKey& vkey);


	//Need to move later...
	virtual void typeHelpWord(std::string roman, std::wstring myanmar, int currStrDictID) = 0;

	//Returns its id (-1 for failure) and its romanization
	virtual std::pair<int, std::string> lookupWord(std::wstring typedWord) = 0;

protected:
	std::wstring systemDefinedWords;
	std::vector< std::pair <int, unsigned short> > systemWordLookup;
	std::vector<std::wstring> userDefinedWords;

	//Additional character to print
	wchar_t typedStopChar;

	//Additional entry
	std::pair <std::string, std::wstring> mostRecentRomanizationCheck;


protected:
	//Window control
	MyWin32Window* mainWindow;
	MyWin32Window* sentenceWindow;
	MyWin32Window* helpWindow;
	MyWin32Window* memoryWindow;

	//For now... may move later
	OnscreenKeyboard *helpKeyboard;

	//Helper typing control
	InputMethod* providingHelpFor;

	//Repaint after this?
	bool viewChanged;
	bool requestToTypeSentence;

	//Must be maintained by the subclass
	std::wstringstream typedRomanStr;


public:  //Abstract methods

	//Is this class a placeholder, or is it a real IM?
	//   There are other ways to do this, but it's nice to 
	//   have a way of double-checking.
	//virtual bool isPlaceholder() = 0; 

	//Get strings to print, always in unicode
	//The current typed string (sentence string). 
	//This will be typed to the output program if "Enter" is pressed.
	//All sub-classes must ensure that this remains valid outside of any function call, and
	//  before any calls to the base class's methods.
	//Returns 3 strings:
	//  1) The string before the highlight.
	//  2) The string before the cursor (highlighted string).
	//  3) The string after the cursor.
	// In addition, a fourth string is returned containing the entire typed string.
	//TODO: Eventually redo with tuples
	virtual std::vector< std::wstring > getTypedSentenceStrings() = 0;
	virtual void appendToSentence(wchar_t letter, int id) = 0; //Used for system letters only, for perf. reasons. (id is optional)

	//The current "candidate" string, which will be displayed in the top
	//  window. It will not be entered until it also appears in the sentence string.
	//The same warnings apply as to the typedSentenceString.
	//NOTE: for now, the "int" part is just the highlight level: 1 for red and 2 for green (3 for both, which means green)
	//      TODO: Make this cleaner.
	virtual std::vector< std::pair<std::wstring, unsigned int> > getTypedCandidateStrings() = 0;


	//Get the typed romanized string. This consists ONLY of all typed valid letters
	//Not abstract
	virtual std::wstring getTypedRomanString(bool asLowercase);

	//Get the status of paging
	// Returns <currIndex, maxPages>
	virtual std::pair<int, int> getPagingInfo() const;

	//Called periodically
	virtual void reset(bool resetCandidates, bool resetRoman, bool resetSentence, bool performFullReset) = 0;

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

