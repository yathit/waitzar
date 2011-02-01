/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _DISPLAY_METHOD
#define _DISPLAY_METHOD

#include <windows_wz.h>
#include <string>
#include "Settings/Types.h"
#include "Settings/Encoding.h"

class DisplayMethod {
public:
	virtual ~DisplayMethod(){}

	//Struct-like properties
	std::wstring id;
	Encoding encoding;
	TYPES type;

	//Allow map comparison 
	bool operator<(const DisplayMethod& other) const {
		return id < other.id;
	}

	//Allow logical equals and not equals
	bool operator==(const DisplayMethod &other) const {
		return id == other.id;
	}
	bool operator!=(const DisplayMethod &other) const {
		return id != other.id;
	}

	//Allow eq/neq on strings, too
	bool operator==(const std::wstring& other) const {
		return id == other;
	}
	bool operator!=(const std::wstring& other) const {
		return id != other;
	}


	//Initialization
	virtual void init(char *data, unsigned long size, HDC currDC, unsigned int defaultColor) = 0;
	virtual void init(HRSRC resource, HGLOBAL dataHandle, HDC currDC, int devLogPixelsY, unsigned int defaultColor) = 0;
	virtual void init(const std::wstring& fileName, HDC currDC, unsigned int defaultColor, int devLogPixelsY) = 0;

	//Functionality
	virtual void drawString(HDC bufferDC, const std::wstring &str, int xPos, int yPos, const std::wstring& filterStr, size_t filterLetterWidth) = 0;
	virtual void drawString(HDC bufferDC, const std::string &str, int xPos, int yPos) = 0;
	virtual void drawChar(HDC bufferDC, char letter, int xPos, int yPos) = 0;
	virtual int getStringWidth(const std::wstring &str, HDC currDC, const std::wstring& filterStr=L"", size_t filterLetterWidth=0) const = 0;
	virtual int getHeight(HDC currDC) const = 0;
	virtual void setColor(unsigned int red, unsigned int green, unsigned int blue) { 
		this->currColorRGB = RGB(red, green, blue); 
		this->currColor = ((red*0x10000) + (green*0x100) + blue)&0xFFFFFF;
	}

protected:
	unsigned currColor;
	COLORREF currColorRGB;

};



#endif //_DISPLAY_METHOD

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

