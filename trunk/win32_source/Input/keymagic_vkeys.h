/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _KEYMAGIC_VKEYS
#define _KEYMAGIC_VKEYS

#include <string>


enum modifier_flags {
	KM_VKMOD_SHIFT = 1 * 0x1000,
	KM_VKMOD_ALT   = 2 * 0x1000,
	KM_VKMOD_CTRL  = 4 * 0x1000,
	KM_VKMOD_CAPS  = 8 * 0x1000,

	//For removing the mask
	KM_VKMOD_MASK  =      0xFFF,
};


struct VKeyPair {
	const std::wstring keyName;
	const unsigned int keyValue;
}; 


const VKeyPair KeyMagicVKeys[] = {
	//Each virtual key string, int pair
	{L"VK_BACK", 0x008},
	{L"VK_TAB", 0x009},
	{L"VK_RETURN", 0x00D},
	{L"VK_ENTER", 0x00D},
	{L"VK_SHIFT", 0x010},
	{L"VK_CONTROL", 0x011},
	{L"VK_CTRL", 0x011},
	{L"VK_ALT", 0x012},
	{L"VK_MENU", 0x012},
	{L"VK_PAUSE", 0x013},
	{L"VK_CAPITAL", 0x014},
	{L"VK_CAPSLOCK", 0x014},
	{L"VK_KANJI", 0x019},
	{L"VK_ESCAPE", 0x01B},
	{L"VK_SPACE", 0x020},
	{L"VK_PRIOR", 0x021},
	{L"VK_NEXT", 0x022},
	{L"VK_DELETE", 0x02E},
	{L"VK_KEY_0", 0x030},
	{L"VK_KEY_1", 0x031},
	{L"VK_KEY_2", 0x032},
	{L"VK_KEY_3", 0x033},
	{L"VK_KEY_4", 0x034},
	{L"VK_KEY_5", 0x035},
	{L"VK_KEY_6", 0x036},
	{L"VK_KEY_7", 0x037},
	{L"VK_KEY_8", 0x038},
	{L"VK_KEY_9", 0x039},
	{L"VK_KEY_A", 0x041},
	{L"VK_KEY_B", 0x042},
	{L"VK_KEY_C", 0x043},
	{L"VK_KEY_D", 0x044},
	{L"VK_KEY_E", 0x045},
	{L"VK_KEY_F", 0x046},
	{L"VK_KEY_G", 0x047},
	{L"VK_KEY_H", 0x048},
	{L"VK_KEY_I", 0x049},
	{L"VK_KEY_J", 0x04A},
	{L"VK_KEY_K", 0x04B},
	{L"VK_KEY_L", 0x04C},
	{L"VK_KEY_M", 0x04D},
	{L"VK_KEY_N", 0x04E},
	{L"VK_KEY_O", 0x04F},
	{L"VK_KEY_P", 0x050},
	{L"VK_KEY_Q", 0x051},
	{L"VK_KEY_R", 0x052},
	{L"VK_KEY_S", 0x053},
	{L"VK_KEY_T", 0x054},
	{L"VK_KEY_U", 0x055},
	{L"VK_KEY_V", 0x056},
	{L"VK_KEY_W", 0x057},
	{L"VK_KEY_X", 0x058},
	{L"VK_KEY_Y", 0x059},
	{L"VK_KEY_Z", 0x05A},
	{L"VK_NUMPAD0", 0x060},
	{L"VK_NUMPAD1", 0x061},
	{L"VK_NUMPAD2", 0x062},
	{L"VK_NUMPAD3", 0x063},
	{L"VK_NUMPAD4", 0x064},
	{L"VK_NUMPAD5", 0x065},
	{L"VK_NUMPAD6", 0x066},
	{L"VK_NUMPAD7", 0x067},
	{L"VK_NUMPAD8", 0x068},
	{L"VK_NUMPAD9", 0x069},
	{L"VK_MULTIPLY", 0x06A},
	{L"VK_ADD", 0x06B},
	{L"VK_SEPARATOR", 0x06C},
	{L"VK_SUBTRACT", 0x06D},
	{L"VK_DECIMAL", 0x06E},
	{L"VK_DIVIDE", 0x06F},
	{L"VK_F1", 0x070},
	{L"VK_F2", 0x071},
	{L"VK_F3", 0x072},
	{L"VK_F4", 0x073},
	{L"VK_F5", 0x074},
	{L"VK_F6", 0x075},
	{L"VK_F7", 0x076},
	{L"VK_F8", 0x077},
	{L"VK_F9", 0x078},
	{L"VK_F10", 0x079},
	{L"VK_F11", 0x07A},
	{L"VK_F12", 0x07B},
	{L"VK_LSHIFT", 0x0A0},
	{L"VK_RSHIFT", 0x0A1},
	{L"VK_LCONTROL", 0x0A2},
	{L"VK_LCTRL", 0x0A2},
	{L"VK_RCONTROL", 0x0A3},
	{L"VK_RCTRL", 0x0A3},
	{L"VK_LMENU", 0x0A4}, 
	{L"VK_LALT", 0x0A4},
	{L"VK_RMENU", 0x0A5},
	{L"VK_RALT", 0x0A5},
	{L"VK_OEM_1", 0x0BA},
	{L"VK_COLON", 0x0BA},
	{L"VK_OEM_PLUS", 0x0BB},
	{L"VK_OEM_COMMA", 0x0BC},
	{L"VK_OEM_MINUS", 0x0BD},
	{L"VK_OEM_PERIOD", 0x0BE},
	{L"VK_OEM_2", 0x0BF},
	{L"VK_QUESTION", 0x0BF},
	{L"VK_OEM_3", 0x0C0},
	{L"VK_CFLEX", 0x0C0},
	{L"VK_OEM_4", 0x0DB},
	{L"VK_LBRACKET", 0x0DB},
	{L"VK_OEM_5", 0x0DC},
	{L"VK_BACKSLASH", 0x0DC},
	{L"VK_OEM_6", 0x0DD},
	{L"VK_RBRACKET", 0x0DD},
	{L"VK_OEM_7", 0x0DE},
	{L"VK_QUOTE", 0x0DE},
	{L"VK_OEM_8", 0x0DF},
	{L"VK_EXCM", 0x0DF},
	{L"VK_OEM_102", 0x0E2},
	{L"VK_LESSTHEN", 0x0E2},

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
