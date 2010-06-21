/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "VirtKey.h"

using std::map;


//Initialize static data
WORD VirtKey::currLocale = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
bool VirtKey::currLocaleInsufficient = false;
const WORD VirtKey::en_usLocale = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
map<unsigned int, unsigned int> VirtKey::scancode2VirtKey;
map<unsigned int, unsigned int> VirtKey::localevkey2Scancode;


//Do we contain enough keys to type this locale? (Always true for en_US)
bool VirtKey::IsCurrLocaleInsufficient()
{
	return (currLocale!=VirtKey::en_usLocale && VirtKey::currLocaleInsufficient);
}


//Convert from currLocale to en_US
void VirtKey::SetCurrLocale(WORD newLocale)
{
	//Only occurs once: initialize the en-us lookup table:
	//We only add "single-case" (virtual key) values.
	if (VirtKey::scancode2VirtKey.empty()) {
		//Row 1, Tilde  through Plus
		VirtKey::scancode2VirtKey[0x29] = VK_OEM_3;
		VirtKey::scancode2VirtKey[0x02] =  '1';
		VirtKey::scancode2VirtKey[0x03] =  '2';
		VirtKey::scancode2VirtKey[0x04] =  '3';
		VirtKey::scancode2VirtKey[0x05] =  '4';
		VirtKey::scancode2VirtKey[0x06] =  '5';
		VirtKey::scancode2VirtKey[0x07] =  '6';
		VirtKey::scancode2VirtKey[0x08] =  '7';
		VirtKey::scancode2VirtKey[0x09] =  '8';
		VirtKey::scancode2VirtKey[0x0A] =  '9';
		VirtKey::scancode2VirtKey[0x0B] =  '0';
		VirtKey::scancode2VirtKey[0x0C] =  VK_OEM_MINUS;
		VirtKey::scancode2VirtKey[0x0D] =  VK_OEM_PLUS;

		//Row 2, 'Q' through Back-Slash
		VirtKey::scancode2VirtKey[0x10] = 'Q';
		VirtKey::scancode2VirtKey[0x11] =  'W';
		VirtKey::scancode2VirtKey[0x12] =  'E';
		VirtKey::scancode2VirtKey[0x13] =  'R';
		VirtKey::scancode2VirtKey[0x14] =  'T';
		VirtKey::scancode2VirtKey[0x15] =  'Y';
		VirtKey::scancode2VirtKey[0x16] =  'U';
		VirtKey::scancode2VirtKey[0x17] =  'I';
		VirtKey::scancode2VirtKey[0x18] =  'O';
		VirtKey::scancode2VirtKey[0x19] =  'P';
		VirtKey::scancode2VirtKey[0x1A] =  VK_OEM_4;
		VirtKey::scancode2VirtKey[0x1B] =  VK_OEM_6;
		VirtKey::scancode2VirtKey[0x2B] =  VK_OEM_5;

		//Row 3, 'A' through Single Quote
		VirtKey::scancode2VirtKey[0x1E] = 'A';
		VirtKey::scancode2VirtKey[0x1F] =  'S';
		VirtKey::scancode2VirtKey[0x20] =  'D';
		VirtKey::scancode2VirtKey[0x21] =  'F';
		VirtKey::scancode2VirtKey[0x22] =  'G';
		VirtKey::scancode2VirtKey[0x23] =  'H';
		VirtKey::scancode2VirtKey[0x24] =  'J';
		VirtKey::scancode2VirtKey[0x25] =  'K';
		VirtKey::scancode2VirtKey[0x26] =  'L';
		VirtKey::scancode2VirtKey[0x27] =  VK_OEM_1;
		VirtKey::scancode2VirtKey[0x28] =  VK_OEM_7;

		//Row 4, 'Z' through Forward Slash
		VirtKey::scancode2VirtKey[0x2C] = 'Z';
		VirtKey::scancode2VirtKey[0x2D] =  'X';
		VirtKey::scancode2VirtKey[0x2E] =  'C';
		VirtKey::scancode2VirtKey[0x2F] =  'V';
		VirtKey::scancode2VirtKey[0x30] =  'B';
		VirtKey::scancode2VirtKey[0x31] =  'N';
		VirtKey::scancode2VirtKey[0x32] =  'M';
		VirtKey::scancode2VirtKey[0x33] =  VK_OEM_COMMA;
		VirtKey::scancode2VirtKey[0x34] =  VK_OEM_PERIOD;
		VirtKey::scancode2VirtKey[0x35] =  VK_OEM_2;		
	}

	//Do nothing if there's no change in locale.
	if (newLocale==VirtKey::currLocale)
		return;

	//Save the current locale.
	VirtKey::currLocale = newLocale;
	VirtKey::currLocaleInsufficient = false;

	//Re-generate the current locale if not en_US
	if (VirtKey::currLocale != VirtKey::en_usLocale) {
		VirtKey::localevkey2Scancode.clear();
		for (map<unsigned int, unsigned int>::iterator mapval=VirtKey::scancode2VirtKey.begin(); mapval!=VirtKey::scancode2VirtKey.end(); mapval++) {
			unsigned int scancode = mapval->first;
			unsigned int vkey_in_locale = MapVirtualKey(scancode, MAPVK_VSC_TO_VK);
			if (vkey_in_locale != 0)
				VirtKey::localevkey2Scancode[vkey_in_locale] = scancode;
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
VirtKey::VirtKey(char alphanum) : alphanum(alphanum), vkCode(0), modShift(false), modAlt(false), modCtrl(false) 
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



//Convert this virtual key into its equivalent in the en_US locale.
//  Will not change the virtual key if it has no en_US equivalent (e.g., accented letters in French),
//    since this might be useful for keyboard layotu designers.
void VirtKey::stripLocale()
{
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
