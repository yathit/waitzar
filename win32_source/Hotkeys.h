/*
 * Copyright 2007 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _HOTKEYS
#define _HOTKEYS

/**
 * This file contains our hotkey definitions. There's a significant number
 * of them (>75) so they've been shuffled out of MainFile.cpp in order to increase its
 * readability.
 */
enum {
	//Capital letters
	HOTKEY_A = 65,
	HOTKEY_B = 66,
	HOTKEY_C = 67,
	HOTKEY_D = 68,
	HOTKEY_E = 69,
	HOTKEY_F = 70,
	HOTKEY_G = 71,
	HOTKEY_H = 72,
	HOTKEY_I = 73,
	HOTKEY_J = 74,
	HOTKEY_K = 75,
	HOTKEY_L = 76,
	HOTKEY_M = 77,
	HOTKEY_N = 78,
	HOTKEY_O = 79,
	HOTKEY_P = 80,
	HOTKEY_Q = 81,
	HOTKEY_R = 82,
	HOTKEY_S = 83,
	HOTKEY_T = 84,
	HOTKEY_U = 85,
	HOTKEY_V = 86,
	HOTKEY_W = 87,
	HOTKEY_X = 88,
	HOTKEY_Y = 89,
	HOTKEY_Z = 90,

	//Lowercase letters
	HOTKEY_A_LOW = 97,
	HOTKEY_B_LOW = 98,
	HOTKEY_C_LOW = 99,
	HOTKEY_D_LOW = 100,
	HOTKEY_E_LOW = 101,
	HOTKEY_F_LOW = 102,
	HOTKEY_G_LOW = 103,
	HOTKEY_H_LOW = 104,
	HOTKEY_I_LOW = 105,
	HOTKEY_J_LOW = 106,
	HOTKEY_K_LOW = 107,
	HOTKEY_L_LOW = 108,
	HOTKEY_M_LOW = 109,
	HOTKEY_N_LOW = 110,
	HOTKEY_O_LOW = 111,
	HOTKEY_P_LOW = 112,
	HOTKEY_Q_LOW = 113,
	HOTKEY_R_LOW = 114,
	HOTKEY_S_LOW = 115,
	HOTKEY_T_LOW = 116,
	HOTKEY_U_LOW = 117,
	HOTKEY_V_LOW = 118,
	HOTKEY_W_LOW = 119,
	HOTKEY_X_LOW = 120,
	HOTKEY_Y_LOW = 121,
	HOTKEY_Z_LOW = 122,

	//Control keys
	HOTKEY_SPACE = 32,
	HOTKEY_LEFT = 17,
	HOTKEY_RIGHT = 18,
	HOTKEY_UP = 19,
	HOTKEY_DOWN = 20,
	HOTKEY_ESC = 21,
	HOTKEY_BACK = 22,
	HOTKEY_ENTER = 23,
	HOTKEY_DELETE = 24,

	//Numbers as control keys
	HOTKEY_NUM0 = 38,
	HOTKEY_NUM1 = 39,
	HOTKEY_NUM2 = 40,
	HOTKEY_NUM3 = 41,
	HOTKEY_NUM4 = 42,
	HOTKEY_NUM5 = 43,
	HOTKEY_NUM6 = 44,
	HOTKEY_NUM7 = 45,
	HOTKEY_NUM8 = 46,
	HOTKEY_NUM9 = 47,

	//And numbers as characters
	HOTKEY_0 = 48,
	HOTKEY_1 = 49,
	HOTKEY_2 = 50,
	HOTKEY_3 = 51,
	HOTKEY_4 = 52,
	HOTKEY_5 = 53,
	HOTKEY_6 = 54,
	HOTKEY_7 = 55,
	HOTKEY_8 = 56,
	HOTKEY_9 = 57,

	//Super control keys...
	HOTKEY_COMMA = 61,
	HOTKEY_PERIOD = 62,

	//Special help-related keys
	HOTKEY_HELP = 63,
	HOTKEY_VIRT_LSHIFT = 64,
	HOTKEY_VIRT_RSHIFT = 91,
	HOTKEY_SHIFT = 92,

	//More help-related keys
	HOTKEY_COMBINE = 93,
	HOTKEY_SHIFT_COMBINE = 94,
	HOTKEY_LEFT_BRACKET = 95,
	HOTKEY_RIGHT_BRACKET = 96,
	HOTKEY_SHIFT_0 = 123,
	HOTKEY_SHIFT_1 = 124,
	HOTKEY_SHIFT_2 = 125,
	HOTKEY_SHIFT_3 = 126,
	HOTKEY_SHIFT_4 = 127,
	HOTKEY_SHIFT_5 = 128,
	HOTKEY_SHIFT_6 = 129,
	HOTKEY_SHIFT_7 = 130,
	HOTKEY_SHIFT_8 = 131,
	HOTKEY_SHIFT_9 = 132,
	HOTKEY_BACKSLASH = 133,
	HOTKEY_SHIFT_LEFT_BRACKET = 134,
	HOTKEY_SHIFT_RIGHT_BRACKET = 135,
	HOTKEY_SHIFT_BACKSLASH = 136,
	HOTKEY_SEMICOLON = 137,
	HOTKEY_APOSTROPHE = 138,
	HOTKEY_SHIFT_SEMICOLON = 139,
	HOTKEY_SHIFT_APOSTROPHE = 140,
	HOTKEY_MINUS = 141,
	//Hotkey 142 somehow crashes our virtual keyboard. Needs looking into... //HAHA, that's because we used
	//   it as the language hotkey. Stupid us....
	HOTKEY_SHIFT_MINUS = 143,
	HOTKEY_SHIFT_EQUALS = 144,
	HOTKEY_EQUALS = 145,
	HOTKEY_FORWARDSLASH = 146,
	HOTKEY_SHIFT_FORWARDSLASH = 147,
	HOTKEY_SHIFT_COMMA = 148,
	HOTKEY_SHIFT_PERIOD = 149,
	HOTKEY_SHIFT_SPACE = 150,
	HOTKEY_SHIFT_ENTER = 151,

	//More hotkeys
	HOTKEY_PAGEUP = 152,
	HOTKEY_PAGEDOWN = 153,
	HOTKEY_TAB = 154,
	HOTKEY_SHIFT_TAB = 155,

	LANGUAGE_HOTKEY = 156,
}; 

//Easier way of storing hotkeys
struct Hotkey {
	int hotkeyID;
	bool useShift; 
	UINT virtKey;
}; 


//////////////////////////////////////////
//Save our various classes of hotkeys
//////////////////////////////////////////


const Hotkey NumberHotkeys[] = {
	//One special additional case: the Combiner (unshifted)
	{HOTKEY_COMBINE, false, VK_OEM_3},

	//Register the numpad numerals
	{HOTKEY_NUM0, false, VK_NUMPAD0},
	{HOTKEY_NUM1, false, VK_NUMPAD1},
	{HOTKEY_NUM2, false, VK_NUMPAD2},
	{HOTKEY_NUM3, false, VK_NUMPAD3},
	{HOTKEY_NUM4, false, VK_NUMPAD4},
	{HOTKEY_NUM5, false, VK_NUMPAD5},
	{HOTKEY_NUM6, false, VK_NUMPAD6},
	{HOTKEY_NUM7, false, VK_NUMPAD7},
	{HOTKEY_NUM8, false, VK_NUMPAD8},
	{HOTKEY_NUM9, false, VK_NUMPAD9},

	//Register the number key numerals
	{HOTKEY_0, false, '0'},
	{HOTKEY_1, false, '1'},
	{HOTKEY_2, false, '2'},
	{HOTKEY_3, false, '3'},
	{HOTKEY_4, false, '4'},
	{HOTKEY_5, false, '5'},
	{HOTKEY_6, false, '6'},
	{HOTKEY_7, false, '7'},
	{HOTKEY_8, false, '8'},
	{HOTKEY_9, false, '9'},
};


const Hotkey PunctuationHotkeys[] = {
	//Period and comma, unshifted
	{HOTKEY_COMMA,  false, VK_OEM_COMMA},
	{HOTKEY_PERIOD, false, VK_OEM_PERIOD},
};


const Hotkey HelpHotkeys[] = {
	//Only "shift", for highlighting purposes
	{HOTKEY_SHIFT, true, VK_SHIFT},
};


const Hotkey ControlHotkeys[] = {
	//Main control keys for the sentence window
	{HOTKEY_SPACE,   false, VK_SPACE},
	{HOTKEY_ENTER,   false, VK_RETURN},
	{HOTKEY_LEFT,    false, VK_LEFT},
	{HOTKEY_ESC,     false, VK_ESCAPE},
	{HOTKEY_BACK,    false, VK_BACK},
	{HOTKEY_DELETE,  false, VK_DELETE},
	{HOTKEY_RIGHT,   false, VK_RIGHT},
	{HOTKEY_UP,      false, VK_UP},
	{HOTKEY_DOWN,    false, VK_DOWN},
	{HOTKEY_PAGEUP,  false, VK_PRIOR},
	{HOTKEY_PAGEDOWN,false, VK_NEXT},
	{HOTKEY_TAB,     false, VK_TAB},

	//Migrated from "Extended Keys" --makes more sense here
	{HOTKEY_SHIFT_SPACE,  true,  HOTKEY_SPACE},
	{HOTKEY_SHIFT_ENTER,  true,  VK_RETURN},
};


const Hotkey ExtendedHotkeys[] = {
	//The shift+combine key, to avoid confusing new users
	{HOTKEY_SHIFT_COMBINE, true, VK_OEM_3},

	//Additional keyboard hotkeys for typing
	{HOTKEY_LEFT_BRACKET,         false, VK_OEM_4},
	{HOTKEY_SHIFT_LEFT_BRACKET,   true,  VK_OEM_4},
	{HOTKEY_RIGHT_BRACKET,        false, VK_OEM_6},
	{HOTKEY_SHIFT_RIGHT_BRACKET,  true,  VK_OEM_6},
	{HOTKEY_BACKSLASH,            false, VK_OEM_5},
	{HOTKEY_SHIFT_BACKSLASH,      true,  VK_OEM_5},
	{HOTKEY_SEMICOLON,            false, VK_OEM_1},
	{HOTKEY_SHIFT_SEMICOLON,      true,  VK_OEM_1},
	{HOTKEY_APOSTROPHE,           false, VK_OEM_7},
	{HOTKEY_SHIFT_APOSTROPHE,     true,  VK_OEM_7},
	{HOTKEY_FORWARDSLASH,         false, VK_OEM_2},
	{HOTKEY_SHIFT_FORWARDSLASH,   true,  VK_OEM_2},
	{HOTKEY_SHIFT_COMMA,          true,  VK_OEM_COMMA},
	{HOTKEY_SHIFT_PERIOD,         true,  VK_OEM_PERIOD},
	{HOTKEY_SHIFT_TAB,            true,  VK_TAB},

	//Keys that appear on our virtual keyboard which aren't, by default, used
	{HOTKEY_MINUS,         false, VK_OEM_MINUS},
	{HOTKEY_SHIFT_MINUS,   true,  VK_OEM_MINUS},
	{HOTKEY_EQUALS,        false, VK_OEM_PLUS},
	{HOTKEY_SHIFT_EQUALS,  true,  VK_OEM_PLUS},

	//Number keys shifted
	{HOTKEY_SHIFT_0, true, HOTKEY_0},
	{HOTKEY_SHIFT_1, true, HOTKEY_1},
	{HOTKEY_SHIFT_2, true, HOTKEY_2},
	{HOTKEY_SHIFT_3, true, HOTKEY_3},
	{HOTKEY_SHIFT_4, true, HOTKEY_4},
	{HOTKEY_SHIFT_5, true, HOTKEY_5},
	{HOTKEY_SHIFT_6, true, HOTKEY_6},
	{HOTKEY_SHIFT_7, true, HOTKEY_7},
	{HOTKEY_SHIFT_8, true, HOTKEY_8},
	{HOTKEY_SHIFT_9, true, HOTKEY_9},
};

#endif //_HOTKEYS


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
