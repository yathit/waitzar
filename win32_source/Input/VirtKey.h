/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _INPUT_VIRT_KEY
#define _INPUT_VIRT_KEY

#include <windows_wz.h>

//System includes
#include <map>
#include <vector>

//Normal includes
#include "Input/keymagic_vkeys.h"




//Class unified virtual key presses for virtual/physical/etc. hotkeys.
//   Masks the "scan code" property by simply generating a new key if instructed to.
class VirtKey {


//////////////////////////////////////////////
//Static methods for dealing with the locale
//////////////////////////////////////////////
private:
	//Change the locale for all future VKeys
	static void SetCurrLocale(HKL newLocale);

	//Is this locale capable enough for WZ? 
	static bool IsCurrLocaleInsufficient();

	//Double-check that the locale is up-to-date.
	static void updateLocale();

	//Given a valid VK_* code and set of modifiers, generate the alphanum value.
	// Named very obtusely to imply that this function should rarely be used.
	//  (it'd be manually inlined, except I use it in two places.)
	static char RetrieveAlphanumFromVkcodeAndModifiers(unsigned int vkCode, bool modShift, bool modAlt, bool modCtrl);


//////////////////////////////////////////////
//Static data for the locale-handling methods
//////////////////////////////////////////////
private:
	//Lookup table: converting from physical scan-codes to VK_* codes in the en-US locale
	//NOTE: Both scancodes and VK_* codes are UINTs. 
	//NOTE: VK_*'s are Virtual Keys, NOT characters. Further conversion is necessary before using them.
	static std::map<unsigned int, unsigned int> scancode2VirtKey;

	//Reverse lookup table: converting from VK_* codes in the current locale to VK_* codes.
	//NOTE: Both scancodes and VK_* codes are UINTs. However, we will convert VK_* codes to wchar_t values for clarity.
	static std::map<unsigned int, unsigned int> localevkey2Scancode;

	//Similar maps for converting BACK to the current locale
	static std::map<unsigned int, unsigned int> scancode2CurrLocale;
	static std::map<unsigned int, unsigned int> enusvkey2Scancode;

	//Current locale for all generated VirtKeys
	static HKL currLocale;

	//Does this locale contain all possible vkeys we might need to type?
	static bool currLocaleInsufficient;

	//Handy lookup for the "en-US" locale.
	static const HKL en_usLocale;



//////////////////////////////////////////////
//Constructors, converters, and "consider" methods
//////////////////////////////////////////////
public:
	//Construct from an lParam (presumably a WM_HOTKEY message, but 0 also works for a "null" VirtKey)
	VirtKey(LPARAM wmHotkeyLParam);

	//Construct from a vkCode and modifiers. It's assumed that these are in the current locale.
	//TODO: Trace calls to this constructor and see if it's ever used in scancode mode)
	VirtKey(unsigned int vkCode, bool modShift, bool modAlt, bool modCtrl);

	
	//Represent as a key magic integer.
	unsigned int toKeyMagicVal();

	//Convert back into a Windows message.
	LPARAM toLParam();

	//vkCode() and alphanum() will return based on the scancode value of the key pressed.
	void considerByScancode() {
		vkCode_p = &vkCodeByScancode;
		alphanum_p = &alphanumByScancode;
	}

	//vkCode() and alphanum() will return based on the locale-dependent value of the key pressed.
	void considerByLocale() {
		vkCode_p = &vkCodeByLocale;
		alphanum_p = &alphanumByLocale;
	}


//////////////////////////////////////////////
//Private data and helper methods
//////////////////////////////////////////////
private:
	//Hiden data
	unsigned int* vkCode_p;
	char* alphanum_p;

	//Helper
	void buildScancodeEntries();


//////////////////////////////////////////////
//Publicly-accessible properties and functions affected by "consider"
//////////////////////////////////////////////
public:
	//The virtual key code of the this keypress. Guaranteed. Affected by locale
	unsigned int vkCodeByScancode;
	unsigned int vkCodeByLocale;
	unsigned int vkCode() { return *vkCode_p; }

	//The alpha-numeric value of this keypress. (Also: ascii value for some additional, common keys)
	//  Guaranteed to be 'a'-'z' for [a-zA-Z].
	//  Guaranteed to be '0'-'9' for any numeric (numpad, regular).
	//  Guaranteed to be "!@#$%^&*()" for the shifted number keys, respectively.
	//  Guaranteed to be '`', '~', '-', '_', '=', '+', for the other top-row keys.
	//  Guaranteed to be '[', '{', ']', '}', '\\', '|', for the other first-row keys.
	//  Guaranteed to be ';', ':', '\'', '"', for the other second-row keys.
	//  Guaranteed to be ',', '<', '.', '>', '/', '?', for the other third-row keys.
	//  Guaranteed to be '\0' for anything else.
	// Affected by locale.
	char alphanumByScancode;
	char alphanumByLocale;
	char alphanum() { return *alphanum_p; }

	//Modifiers. Guaranteed. Not affected by locale.
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

