/*
    Copyright (c) 2008, Interactive Pulp, LLC
    All rights reserved. (see the NOTICE file in the top directory for more information)

	NOTE: This source file is licensed under the New BSD License, unlike the remainder
	of the WaitZar project, which is licensed under the Apache License 2.0. This file is an
	amalgamation of several Java source files in the Pulp Core project, and is used primarily
	for reading an annotated version of the PNG format. 

	Please see:
	http://code.google.com/p/pulpcore/
	...for the latest licensing information regarding the Pulp Core project. 
*/

#pragma once

#define _UNICODE
#define UNICODE

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include "Inflater.h"

//Magic numbers for Pulp Core and .PNG headers.
const char PULP_MAGICNUM[] = "pulpfnt\x0B";              //0x70756c70666e740b
const char PNG_SIGNATURE[] = "\x89PNG\x0D\x0A\x1A\x0A";  //0x89504e470d0a1a0a

//PNG chunk IDs
#define CHUNK_IHDR 0x49484452
#define CHUNK_PLTE 0x504c5445
#define CHUNK_TRNS 0x74524e53
#define CHUNK_IDAT 0x49444154
#define CHUNK_IEND 0x49454e44

//Pulp Core chunk IDs
#define CHUNK_FONT 0x666f4e74
#define CHUNK_HOTS 0x686f5473
#define CHUNK_ANIM 0x616e496d

//Image encoding values
#define COLOR_TYPE_GRAYSCALE 0
#define COLOR_TYPE_RGB 2
#define COLOR_TYPE_PALETTE 3
#define COLOR_TYPE_GRAYSCALE_WITH_ALPHA 4
#define COLOR_TYPE_RGB_WITH_ALPHA 6

//Help convert bit-depth to bits-per-pixel
const int SAMPLES_PER_PIXEL[] = { 1, 0, 3, 1, 2, 0, 4 };


/**
 * PulpCore is a Java project; see the link in the license file. 
 *  Fonts used in PulpCore are accomplished by creating a PNG of 
 *   the Font's letters and then embedding additional "chunks" 
 *   specifying the advance width, kerning, and other specifics. 
 *  This creates a valid PNG file (the additional chunks are just
 *   ignored by any other image processing software) that can
 *   handle complex fonts like Zawgyi-One (used internally by WaitZar). 
 *  There are three good reasons to do it this way:
 *   1) PulpCore automatically anti-aliases a font's characters when
 *      converting them to PNG. So, Myanmar text in WaitZar looks good
 *      even if the user doesn't have Clear Type installed.
 *   2) Processing is slightly faster, and memory usage barely increases.
 *      Instead of rendering a true-type font, we're just blitting  pixels
 *      from one buffer to another (this is always fast.)
 *   3) If Zawgyi-One ever becomes obsolete, or changes its encoding order 
 *      (for whatever reason) then we can still display our Myanmar text
 *      without forcing the user to install the "old" version on his system.
 *      We could embed the Zawgyi-One.ttf file itself, but that's 10 times
 *      larger than our PNG (which uses internal compression, like all PNGs do.)
 */
class PulpCoreFont
{
public:
	//Contructors
	PulpCoreFont(HRSRC resource, HGLOBAL dataHandle, HDC currDC);
	PulpCoreFont(PulpCoreFont* copyFrom, HDC currDC);

	//Useful for making various color replicas of one font.
	void tintSelf(UINT rgbColor);

	//We report errors, though they're unlikely.
	BOOL isInError();
	TCHAR* getErrorMsg();

	//Drawing functionality
	void drawString(HDC bufferDC, TCHAR* str, int xPos, int yPos);
	
	//Other useful metrics
    int getStringWidth(TCHAR* str);
	int getStringWidth(TCHAR* str, int start, int length);
	int getHeight();


private:
	//Data regarding the image
	int bitDepth;
	int colorType;
	int width;
	int height;
	bool isOpaque;
	int hotspotX;
	int hotspotY;
	int* palette;
	int pal_length;

	//Drawing onto the bitmap's surface... ugh...
	UINT *directPixels;
	HBITMAP directBitmap;
	HDC directDC;
	BLENDFUNCTION blendFunc;
	BITMAPINFO bmpInfo;

	//PulpCoreFont-specific properties
	int firstChar;
	int lastChar;
	int tracking;
	int* charPositions;
	int num_char_pos;
	int* bearingLeft;
	int* bearingRight;
	bool uppercaseOnly;

	//Used by the primary constructor to hold resource-related information.
	DWORD currPos;
	char* res_data;
	DWORD res_size;

	//Error tracking
	BOOL error;
	TCHAR errorMsg[100];

	//Internal Methods
	void initBmpInfo();
	void readHeader(HDC currDC);
	void readPalette(int length);
	void readTransparency(int length);
	void fontSet();
	void readAnimation();
	void readData(int length);
	void decodeFilter(char* curr, int curr_len, char* prev, int filter, int bpp);
	int paethPredictor(int a, int b, int c);
	int premultiply(UINT arbg);
	void premultiply(UINT* arbg, int argb_len);
    int readInt();
    short readShort();
	char readByte();
	void inflateFully(Inflater* inflater, char* result, int res_length);
	int getCharIndex(TCHAR ch);
	int getKerning(TCHAR left, TCHAR right);
	int getKerning(int leftIndex, int rightIndex);
	bool shouldIgnoreTracking(int index);
};
