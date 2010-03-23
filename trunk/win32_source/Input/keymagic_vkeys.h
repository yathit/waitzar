/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _KEYMAGIC_VKEYS
#define _KEYMAGIC_VKEYS

#include <string>


enum modifier_flags {
	KM_VKMOD_SHIFT = 1 * 0x10000,
	KM_VKMOD_ALT   = 2 * 0x10000,
	KM_VKMOD_CTRL  = 4 * 0x10000,
	KM_VKMOD_CAPS  = 8 * 0x10000,

	//For removing the mask
	KM_VKMOD_MASK  =      0xFFFF,
};


struct VKeyPair {
	const std::wstring keyName;
	const unsigned int keyValue;
}; 


const VKeyPair KeyMagicVKeys[] = {
	//Each virtual key string, int pair
	{L"VK_BACK", 0x0008},
	{L"VK_TAB", 0x0009},
	{L"VK_RETURN", 0x000D},
	{L"VK_ENTER", 0x000D},
	{L"VK_SHIFT", 0x0010},
	{L"VK_CONTROL", 0x0011},
	{L"VK_CTRL", 0x0011},
	{L"VK_ALT", 0x0012},
	{L"VK_MENU", 0x0012},
	{L"VK_PAUSE", 0x0013},
	{L"VK_CAPITAL", 0x0014},
	{L"VK_CAPSLOCK", 0x0014},
	{L"VK_KANJI", 0x0019},
	{L"VK_ESCAPE", 0x001B},
	{L"VK_SPACE", 0x0020},
	{L"VK_PRIOR", 0x0021},
	{L"VK_NEXT", 0x0022},
	{L"VK_DELETE", 0x002E},
	{L"VK_KEY_0", 0x0030},
	{L"VK_KEY_1", 0x0031},
	{L"VK_KEY_2", 0x0032},
	{L"VK_KEY_3", 0x0033},
	{L"VK_KEY_4", 0x0034},
	{L"VK_KEY_5", 0x0035},
	{L"VK_KEY_6", 0x0036},
	{L"VK_KEY_7", 0x0037},
	{L"VK_KEY_8", 0x0038},
	{L"VK_KEY_9", 0x0039},
	{L"VK_KEY_A", 0x0041},
	{L"VK_KEY_B", 0x0042},
	{L"VK_KEY_C", 0x0043},
	{L"VK_KEY_D", 0x0044},
	{L"VK_KEY_E", 0x0045},
	{L"VK_KEY_F", 0x0046},
	{L"VK_KEY_G", 0x0047},
	{L"VK_KEY_H", 0x0048},
	{L"VK_KEY_I", 0x0049},
	{L"VK_KEY_J", 0x004A},
	{L"VK_KEY_K", 0x004B},
	{L"VK_KEY_L", 0x004C},
	{L"VK_KEY_M", 0x004D},
	{L"VK_KEY_N", 0x004E},
	{L"VK_KEY_O", 0x004F},
	{L"VK_KEY_P", 0x0050},
	{L"VK_KEY_Q", 0x0051},
	{L"VK_KEY_R", 0x0052},
	{L"VK_KEY_S", 0x0053},
	{L"VK_KEY_T", 0x0054},
	{L"VK_KEY_U", 0x0055},
	{L"VK_KEY_V", 0x0056},
	{L"VK_KEY_W", 0x0057},
	{L"VK_KEY_X", 0x0058},
	{L"VK_KEY_Y", 0x0059},
	{L"VK_KEY_Z", 0x005A},
	{L"VK_NUMPAD0", 0x0060},
	{L"VK_NUMPAD1", 0x0061},
	{L"VK_NUMPAD2", 0x0062},
	{L"VK_NUMPAD3", 0x0063},
	{L"VK_NUMPAD4", 0x0064},
	{L"VK_NUMPAD5", 0x0065},
	{L"VK_NUMPAD6", 0x0066},
	{L"VK_NUMPAD7", 0x0067},
	{L"VK_NUMPAD8", 0x0068},
	{L"VK_NUMPAD9", 0x0069},
	{L"VK_MULTIPLY", 0x006A},
	{L"VK_ADD", 0x006B},
	{L"VK_SEPARATOR", 0x006C},
	{L"VK_SUBTRACT", 0x006D},
	{L"VK_DECIMAL", 0x006E},
	{L"VK_DIVIDE", 0x006F},
	{L"VK_F1", 0x0070},
	{L"VK_F2", 0x0071},
	{L"VK_F3", 0x0072},
	{L"VK_F4", 0x0073},
	{L"VK_F5", 0x0074},
	{L"VK_F6", 0x0075},
	{L"VK_F7", 0x0076},
	{L"VK_F8", 0x0077},
	{L"VK_F9", 0x0078},
	{L"VK_F10", 0x0079},
	{L"VK_F11", 0x007A},
	{L"VK_F12", 0x007B},
	{L"VK_LSHIFT", 0x00A0},
	{L"VK_RSHIFT", 0x00A1},
	{L"VK_LCONTROL", 0x00A2},
	{L"VK_LCTRL", 0x00A2},
	{L"VK_RCONTROL", 0x00A3},
	{L"VK_RCTRL", 0x00A3},
	{L"VK_LMENU", 0x00A4}, 
	{L"VK_LALT", 0x00A4},
	{L"VK_RMENU", 0x00A5},
	{L"VK_RALT", 0x00A5},
	{L"VK_OEM_1", 0x00BA},
	{L"VK_COLON", 0x00BA},
	{L"VK_OEM_PLUS", 0x00BB},
	{L"VK_OEM_COMMA", 0x00BC},
	{L"VK_OEM_MINUS", 0x00BD},
	{L"VK_OEM_PERIOD", 0x00BE},
	{L"VK_OEM_2", 0x00BF},
	{L"VK_QUESTION", 0x00BF},
	{L"VK_OEM_3", 0x00C0},
	{L"VK_CFLEX", 0x00C0},
	{L"VK_OEM_4", 0x00DB},
	{L"VK_LBRACKET", 0x00DB},
	{L"VK_OEM_5", 0x00DC},
	{L"VK_BACKSLASH", 0x00DC},
	{L"VK_OEM_6", 0x00DD},
	{L"VK_RBRACKET", 0x00DD},
	{L"VK_OEM_7", 0x00DE},
	{L"VK_QUOTE", 0x00DE},
	{L"VK_OEM_8", 0x00DF},
	{L"VK_EXCM", 0x00DF},
	{L"VK_OEM_102", 0x00E2},
	{L"VK_LESSTHEN", 0x00E2},

	{L"", 0} //How we know we're done
};


#endif //_KEYMAGIC_VKEYS


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
