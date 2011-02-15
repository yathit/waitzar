/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "VirtKey.h"

using std::map;


//Initialize static data
HKL VirtKey::currLocale = NULL;//MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
bool VirtKey::currLocaleInsufficient = false;
const HKL VirtKey::en_usLocale = LoadKeyboardLayout(L"00000409", 0); //Should represent the "default layout"
map<unsigned int, unsigned int> VirtKey::localevkey2Scancode;
map<unsigned int, unsigned int> VirtKey::scancode2CurrLocale;
map<unsigned int, unsigned int> VirtKey::enusvkey2Scancode;


//Initialize our en-us lookup table directly.
map<unsigned int, unsigned int> VirtKey::scancode2VirtKey = {
	//Row 1, Tilde  through Plus
	{0x29, VK_OEM_3},
	{0x02, '1'},
	{0x03, '2'},
	{0x04, '3'},
	{0x05, '4'},
	{0x06, '5'},
	{0x07, '6'},
	{0x08, '7'},
	{0x09, '8'},
	{0x0A, '9'},
	{0x0B, '0'},
	{0x0C, VK_OEM_MINUS},
	{0x0D, VK_OEM_PLUS},

	//Row 2, 'Q' through Back-Slash
	{0x10, 'Q'},
	{0x11, 'W'},
	{0x12, 'E'},
	{0x13, 'R'},
	{0x14, 'T'},
	{0x15, 'Y'},
	{0x16, 'U'},
	{0x17, 'I'},
	{0x18, 'O'},
	{0x19, 'P'},
	{0x1A, VK_OEM_4},
	{0x1B, VK_OEM_6},
	{0x2B, VK_OEM_5},

	//Row 3, 'A' through Single Quote
	{0x1E, 'A'},
	{0x1F, 'S'},
	{0x20, 'D'},
	{0x21, 'F'},
	{0x22, 'G'},
	{0x23, 'H'},
	{0x24, 'J'},
	{0x25, 'K'},
	{0x26, 'L'},
	{0x27, VK_OEM_1},
	{0x28, VK_OEM_7},

	//Row 4, 'Z' through Forward Slash
	{0x2C, 'Z'},
	{0x2D, 'X'},
	{0x2E, 'C'},
	{0x2F, 'V'},
	{0x30, 'B'},
	{0x31, 'N'},
	{0x32, 'M'},
	{0x33, VK_OEM_COMMA},
	{0x34, VK_OEM_PERIOD},
	{0x35, VK_OEM_2},
};


//Do we contain enough keys to type this locale? (Always true for en_US)
bool VirtKey::IsCurrLocaleInsufficient()
{
	return (currLocale!=VirtKey::en_usLocale && VirtKey::currLocaleInsufficient);
}


//Convert from currLocale to en_US
void VirtKey::SetCurrLocale(HKL newLocale)
{
	//Initialize our us-2-scancode lookup
	if (VirtKey::enusvkey2Scancode.empty()) {
		for (map<unsigned int, unsigned int>::iterator it=VirtKey::scancode2VirtKey.begin(); it!=VirtKey::scancode2VirtKey.end(); it++) {
			VirtKey::enusvkey2Scancode[it->second] = it->first;
		}
	}
	

	//Do nothing if there's no change in locale.
	if (newLocale==VirtKey::currLocale)
		return;

	//Save the current locale.
	VirtKey::currLocale = newLocale;
	VirtKey::currLocaleInsufficient = false;

	//Re-generate the current locale if not en_US
	//NOTE: Need to use MapVirtualKeyEx to handle this properly.
	if (VirtKey::currLocale != VirtKey::en_usLocale) {
		VirtKey::localevkey2Scancode.clear();

		//Load the locale
		//NOTE: We can just use the HKL entry returned by GetKeyboardLayout()
		//HKL nonUSLayout = LoadKeyboardLayout(test, KLF_NOTELLSHELL);

		//Reset the keys if this layout exists.
		if (VirtKey::currLocale!=NULL) {
			for (map<unsigned int, unsigned int>::iterator mapval=VirtKey::scancode2VirtKey.begin(); mapval!=VirtKey::scancode2VirtKey.end(); mapval++) {
				unsigned int scancode = mapval->first;
				unsigned int vkey_in_locale = MapVirtualKeyEx(scancode, MAPVK_VSC_TO_VK, VirtKey::currLocale);
				if (vkey_in_locale != 0) {
					VirtKey::localevkey2Scancode[vkey_in_locale] = scancode;
					VirtKey::scancode2CurrLocale[scancode] = vkey_in_locale;
				}
			}
		}

		//Does this locale contain enough keys?
		VirtKey::currLocaleInsufficient = (VirtKey::scancode2VirtKey.size() != VirtKey::localevkey2Scancode.size());
	}
}



//Construct from an lParam (presumably a WM_HOTKEY message)
VirtKey::VirtKey(LPARAM wmHotkeyLParam) 
{
	//Easy
	vkCode = HIWORD(wmHotkeyLParam);
	modShift = (LOWORD(wmHotkeyLParam)&MOD_SHIFT) != 0;
	modCtrl = (LOWORD(wmHotkeyLParam)&MOD_CONTROL) != 0;
	modAlt = (LOWORD(wmHotkeyLParam)&MOD_ALT) != 0;

	//Make the alphanum
	constructAlphanumFromVkcodeAndModifiers();
}

VirtKey::VirtKey(unsigned int vkCode, bool modShift, bool modAlt, bool modCtrl) : vkCode(vkCode), modShift(modShift), modAlt(modAlt), modCtrl(modCtrl)
{
	//Make the alphanum
	constructAlphanumFromVkcodeAndModifiers();
}

//Helper method for the above-detailed constructor
void VirtKey::constructAlphanumFromVkcodeAndModifiers()
{
	//Alphanum: letters & numbers & some symbols
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


//Constructor: extrapolate "shfit" and vk values from the "alphanum" parameter. 
VirtKey::VirtKey(char alphanum) : vkCode(0), alphanum(alphanum), modShift(false), modAlt(false), modCtrl(false)
{
	//Primary goal: figure out the vkCode. It remains at "0" if there was any error.
	if (alphanum>='a' && alphanum<='z') {
		vkCode = alphanum;
	} else if (alphanum>='A' && alphanum<='Z') {
		vkCode = (alphanum-'A')+'a';
		modShift = true;
	} else if (alphanum>='0' && alphanum<='9') {
		//We can represent these as number keys or NUMPAD vkeys. We'll choose the former.
		vkCode = alphanum;
	} else {
		switch (alphanum) {
			case '!':
				vkCode = '1';
				modShift = true;
				break;
			case '@':
				vkCode = '2';
				modShift = true;
				break;
			case '#':
				vkCode = '3';
				modShift = true;
				break;
			case '$':
				vkCode = '4';
				modShift = true;
				break;
			case '%':
				vkCode = '5';
				modShift = true;
				break;
			case '^':
				vkCode = '6';
				modShift = true;
				break;
			case '&':
				vkCode = '7';
				modShift = true;
				break;
			case '*':
				vkCode = '8';
				modShift = true;
				break;
			case '(':
				vkCode = '9';
				modShift = true;
				break;
			case ')':
				vkCode = '0';
				modShift = true;
				break;
			case '~':  modShift = true; //Fall-through
			case '`':
				vkCode = VK_OEM_3;
				break;
			case '_':  modShift = true; //Fall-through
			case '-':
				vkCode = VK_OEM_MINUS;
				break;
			case '+':  modShift = true; //Fall-through
			case '=':
				vkCode = VK_OEM_PLUS;
				break;
			case '{':  modShift = true; //Fall-through
			case '[':
				vkCode = VK_OEM_4;
				break;
			case '}':  modShift = true; //Fall-through
			case ']':
				vkCode = VK_OEM_6;
				break;
			case '|':  modShift = true; //Fall-through
			case '\\':
				vkCode = VK_OEM_5;
				break;
			case ':':  modShift = true; //Fall-through
			case ';':
				vkCode = VK_OEM_1;
				break;
			case '"':  modShift = true; //Fall-through
			case '\'':
				vkCode = VK_OEM_7;
				break;
			case '<':  modShift = true; //Fall-through
			case ',':
				vkCode = VK_OEM_COMMA;
				break;
			case '>':  modShift = true; //Fall-through
			case '.':
				vkCode = VK_OEM_PERIOD;
				break;
		case '?':  modShift = true; //Fall-through
			case '/':
				vkCode = VK_OEM_2;
				break;
		}
	}
}



//////////////////////////////////
//Useful functions
//////////////////////////////////

//Represent as a key magic integer.
unsigned int VirtKey::toKeyMagicVal() 
{
	//Some Japanese keyboards can generate very large keycode values. Ignore these for now.
	unsigned int ret = (vkCode & KM_VKMOD_MASK);
	if (ret != vkCode)
		return 0;

	//Deal with the fact that letters are upper-case in KeyMagic VK codes.
	if (ret>='a' && ret<='z') {
		ret = (ret-'a') + 0x041; //TODO: Add some inline function for finding "VK_KEY_A" more easily...
	}

	//Add parameters
	if (modShift)
		ret |= KM_VKMOD_SHIFT;
	if (modCtrl)
		ret |= KM_VKMOD_CTRL;
	if (modAlt)
		ret |= KM_VKMOD_ALT;

	return ret;
}


//Represent as a Windows message
LPARAM VirtKey::toLParam() 
{
	return MAKELPARAM((modShift?MOD_SHIFT:0)|(modAlt?MOD_ALT:0)|(modCtrl?MOD_CONTROL:0), vkCode);
}


void VirtKey::updateLocale()
{
	//Test: perform automatically.
	//Not working; we might actually need to check the "focus" window.
	//...or possibly just the ThreadID of the top-most window's windowing thread.
	HWND foreWnd = GetForegroundWindow();
	{
		//Get the associated IMEWnd ("Default IM"). 
		// This remains accurate even with console apps
		HWND immWnd = ImmGetDefaultIMEWnd(foreWnd);
		if (immWnd!=NULL)
			foreWnd = immWnd;
	}
	DWORD foreID = GetWindowThreadProcessId(foreWnd, NULL);
	HKL newLocale = GetKeyboardLayout(foreID);

	//Just because I'm superstitious, fall back on the Desktop's Layout if all else fails.
	if (newLocale==NULL) {
		foreWnd = GetDesktopWindow();
		foreID = GetWindowThreadProcessId(foreWnd, NULL);
		newLocale = GetKeyboardLayout(foreID); 
	}

	//Set the layout, if it even exists.
	if (newLocale!=NULL) {
		if (newLocale != VirtKey::currLocale)
			VirtKey::SetCurrLocale(newLocale);
	}
}



//Convert this virtual key into its equivalent in the en_US locale.
//  Will not change the virtual key if it has no en_US equivalent (e.g., accented letters in French),
//    since this might be useful for keyboard layotu designers.
void VirtKey::stripLocale()
{
	//Make sure our locale is up-to-date.
	VirtKey::updateLocale();

	//No change if currently in en_US; avoid spurious errors
	if (VirtKey::currLocale==VirtKey::en_usLocale)
		return;

	//Only vkcode and alphanum change; the modifiers remain. Vkcode must be changed first.
	// Abort the conversion whenever we encounter trouble.
	unsigned int oldVKCode = vkCode;
	map<unsigned int, unsigned int>::iterator it1 = VirtKey::localevkey2Scancode.find(vkCode);
	if (it1==VirtKey::localevkey2Scancode.end())
		return;
	map<unsigned int, unsigned int>::iterator it2 = VirtKey::scancode2VirtKey.find(it1->second);
	if (it2==VirtKey::scancode2VirtKey.end())
		return;
	vkCode = it2->second;

	//Now, re-generate the alphanum (unless nothing changed).
	if (vkCode != oldVKCode) 
		constructAlphanumFromVkcodeAndModifiers();
}


void VirtKey::considerLocale()
{
	//Make sure our locale is up-to-date.
	VirtKey::updateLocale();

	//No change if currently in en_US; avoid spurious errors
	if (VirtKey::currLocale==VirtKey::en_usLocale)
		return;

	//Only vkcode and alphanum change; the modifiers remain. Vkcode must be changed first.
	// Abort the conversion whenever we encounter trouble.
	unsigned int oldVKCode = vkCode;
	map<unsigned int, unsigned int>::iterator it1 = VirtKey::enusvkey2Scancode.find(vkCode);
	if (it1==VirtKey::enusvkey2Scancode.end())
		return;
	map<unsigned int, unsigned int>::iterator it2 = VirtKey::scancode2CurrLocale.find(it1->second);
	if (it2==VirtKey::scancode2CurrLocale.end())
		return;
	vkCode = it2->second;

	//Now, re-generate the alphanum (unless nothing changed).
	if (vkCode != oldVKCode) 
		constructAlphanumFromVkcodeAndModifiers();
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
