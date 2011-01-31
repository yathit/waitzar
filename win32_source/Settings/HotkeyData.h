/*
 * Copyright 2011 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */


#pragma once


#include <string>



//Hotkey wrapper
class HotkeyData {
public:
	std::wstring hotkeyStrRaw;
	std::wstring hotkeyStrFormatted;
	int hotkeyID;
	unsigned int hkModifiers;
	unsigned int hkVirtKeyCode;

	HotkeyData(const std::wstring& srcStr=L"") {
		//Save raw string
		this->hotkeyStrRaw = srcStr;

		//Simple case
		if (srcStr==L"") {
			hotkeyID = hkModifiers = hkVirtKeyCode = 0;
			return;
		}

		//Parse the raw string; build up a vector of substrings in reverse
		std::vector<std::wstring> hkSubs;
		const std::wstring& src = srcStr;
		std::wstringstream segment;
		for (size_t i=0; i<src.size(); i++) {
			//Skip non-ascii
			char c = (char)src[i];
			if (src[i]>0x7F)
				c = '\0'; //Needed to allow for the final string to be added...

			//Capitalize
			if (c>='a'&&c<='z')
				c = (c-'a')+'A';

			//Valid?
			if((c>='A'&&c<='Z')||(c>='0'&&c<='9'))
				segment <<c;

			//Done?
			if ((c=='+'||i==src.size()-1) && !segment.str().empty()) {
				hkSubs.insert(hkSubs.begin(), segment.str());
				segment.str(L"");
			}
		}

		//Build up the hotkey data: key code
		this->hotkeyID = LANGUAGE_HOTKEY;

		//Enough?
		if (hkSubs.empty())
			throw std::runtime_error("Invalid hotkey: empty string");

		//Build up the hotkey data: vk
		unsigned int vk = 0;
		if (hkSubs[0]==L"SPACE")
			vk = VK_SPACE;
		else if (hkSubs[0]==L"CTRL")
			vk = VK_CONTROL;
		else if (hkSubs[0]==L"SHIFT")
			vk = VK_SHIFT;
		else if (hkSubs[0]==L"ALT")
			vk = VK_MENU; //VK_MENU == VK_ALT
		else if (hkSubs[0]==L"WIN")
			throw std::runtime_error("Invalid hotkey: WIN can't be the main hotkey, only a modifier.");
		else if (hkSubs[0].length()==1) {
			if ((hkSubs[0][0]>='A'&&hkSubs[0][0]<='Z')||(hkSubs[0][0]>='0'&&hkSubs[0][0]<='9'))
				vk = hkSubs[0][0];   //Capital letters (or numbers) for vk codes
			else
				throw std::runtime_error(waitzar::glue(L"Invalid hotkey letter: ", hkSubs[0]).c_str());
		} else
			throw std::runtime_error(waitzar::glue(L"Invalid hotkey letter: ", hkSubs[0]).c_str());
		this->hkVirtKeyCode = vk;

		//Build up the hotkey data: modifiers
		unsigned int mod = 0;
		for (size_t i=1; i<hkSubs.size(); i++) {
			if (hkSubs[i]==L"CTRL")
				mod |= MOD_CONTROL;
			else if (hkSubs[i]==L"SHIFT")
				mod |= MOD_SHIFT;
			else if (hkSubs[i]==L"ALT")
				mod |= MOD_ALT;
			else if (hkSubs[i]==L"WIN")
				throw std::runtime_error("Invalid hotkey modifier: WIN is reserved for use by the Windows Operating System.");
			else
				throw std::runtime_error(waitzar::glue(L"Invalid hotkey modifier: ", hkSubs[i]).c_str());
		}
		//Add modifiers for the keys themselves
		if (vk==VK_SHIFT)
			mod |= MOD_SHIFT;
		else if (vk==VK_CONTROL)
			mod |= MOD_CONTROL;
		else if (vk==VK_MENU)
			mod |= MOD_ALT;
		this->hkModifiers = mod;

		//Build up the formatted string (at this point, we know all arguments are correct)
		std::wstring fmt = L"";
		for (int i=hkSubs.size()-1; i>=0; i--) {
			if (hkSubs[i]==L"CTRL")
				fmt += L"Ctrl";
			else if (hkSubs[i]==L"SHIFT")
				fmt += L"Shift";
			else if (hkSubs[i]==L"ALT")
				fmt += L"Alt";
			else if (hkSubs[i]==L"SPACE")
				fmt += L"Space";
			else if (hkSubs[i]==L"WIN")
				fmt += L"Win";
			else
				fmt += hkSubs[i];

			if (i!=0)
				fmt += L"+";
		}
		this->hotkeyStrFormatted = fmt;

		//Final check
		if (((vk>='A'&&vk<='Z')||(vk>='0'&&vk<='9')||vk==VK_SPACE)&&(mod==MOD_SHIFT||mod==0))
			throw std::runtime_error(waitzar::glue(L"Invalid hotkey: ", fmt, L" --overlaps existing typable letter or space.").c_str());
	}
};



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
