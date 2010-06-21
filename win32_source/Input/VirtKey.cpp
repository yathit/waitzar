/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "VirtKey.h"



//Construct from an lParam (presumably a WM_HOTKEY message)
VirtKey::VirtKey(LPARAM wmHotkeyLParam) {
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


//Constructor: extrapolate "shfit" and vk values from the "alphanum" parameter. 
VirtKey::VirtKey(char alphanum) : alphanum(alphanum), vkCode(0), modShift(false), modAlt(false), modCtrl(false) {
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
unsigned int VirtKey::toKeyMagicVal() {
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
LPARAM VirtKey::toLParam() {
	return MAKELPARAM((modShift?MOD_SHIFT:0)|(modAlt?MOD_ALT:0)|(modCtrl?MOD_CONTROL:0), vkCode);
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
