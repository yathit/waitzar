/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _INPUT_VIRT_KEY
#define _INPUT_VIRT_KEY

//Include windows libraries
#define _UNICODE
#define UNICODE

#include <windows.h>

//System includes
#include <map>
#include <vector>

//Normal includes
#include "Input/keymagic_vkeys.h"




//Class unified virtual key presses for virtual/physical/etc. hotkeys.
//   Masks the "scan code" property by simply generating a new key if instructed to.
class VirtKey {
//Static methods
public:
	//Change the locale for all future VKeys
	static void SetCurrLocale(WORD newLocale);

	//Is this locale capable enough for WZ? 
	static bool IsCurrLocaleInsufficient();


//Static data
private:
	//Lookup table: converting from physical scan-codes to VK_* codes in the en-US locale
	//NOTE: Both scancodes and VK_* codes are UINTs. 
	//NOTE: VK_*'s are Virtual Keys, NOT characters. Further conversion is necessary before using them.
	static std::map<unsigned int, unsigned int> scancode2VirtKey;

	//Reverse lookup table: converting from VK_* codes in the current locale to VK_* codes.
	//NOTE: Both scancodes and VK_* codes are UINTs. However, we will convert VK_* codes to wchar_t values for clarity.
	static std::map<unsigned int, unsigned int> localevkey2Scancode;

	//Current locale for all generated VirtKeys
	static WORD currLocale;

	//Does this locale contain all possible vkeys we might need to type?
	static bool currLocaleInsufficient;

public:
	//Handy lookup for the "en-US" locale.
	static const WORD en_usLocale;


//Public methods
public:
	//Simple constructor
	VirtKey(unsigned int vkCode, char alphanum, bool modShift, bool modAlt, bool modCtrl) 
		: vkCode(vkCode), alphanum(alphanum), modShift(modShift), modAlt(modAlt), modCtrl(modCtrl) {}

	//Construct from an lParam (presumably a WM_HOTKEY message)
	VirtKey(LPARAM wmHotkeyLParam);

	//Constructor: extrapolate "shfit" and vk values from the "alphanum" parameter. 
	VirtKey(char alphanum);


	//
	//Generally useful methods:
	//
	
	//Represent as a key magic integer.
	unsigned int toKeyMagicVal();

	//Convert back into a Windows message.
	LPARAM toLParam();

	//Convert this VKey to the en-US locale.
	//We do NOT convert "control"-style keys, like VK_BACK, since this will likely just confuse the user.
	//  TODO: Double-check if this is a good idea or not...
	void stripLocale();


//Private helper methods
private:
	//Given a valid VK_* code and set of modifiers, generate the alphanum value.
	// Named very obtusely to imply that this function should rarely be used. 
	//  (it'd be manually inlined, except I use it in two places.)
	void constructAlphanumFromVkcodeAndModifiers();



//Accessible properties of the virtual key
public:
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
};


#endif //_INPUT_VIRT_KEY

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

