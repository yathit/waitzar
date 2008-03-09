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

//Image stuff
#define COLOR_TYPE_GRAYSCALE 0
#define COLOR_TYPE_RGB 2
#define COLOR_TYPE_PALETTE 3
#define COLOR_TYPE_GRAYSCALE_WITH_ALPHA 4
#define COLOR_TYPE_RGB_WITH_ALPHA 6


const int SAMPLES_PER_PIXEL[] = { 1, 0, 3, 1, 2, 0, 4 };


class PulpCoreFont
{
public:
	PulpCoreFont(HRSRC resource, HGLOBAL dataHandle, HDC currDC);
	PulpCoreFont(PulpCoreFont* copyFrom, HDC currDC);

	BOOL isInError();
	TCHAR* getErrorMsg();

	void drawString(HDC bufferDC, TCHAR* str, int xPos, int yPos);
	void tintSelf(UINT rgbColor);


private:
	//Data regarding the image
	int bitDepth;
	int colorType;
	int width;
	int height;
	bool isOpaque;
	//int* imgData;
	int hotspotX;
	int hotspotY;
	int* palette;
	int pal_length;

	//Drawing onto the bitmap's surface... ugh...
	UINT *directPixels;
	HBITMAP directBitmap;
	HDC directDC;
//	HGDIOBJ previousObject;
	BLENDFUNCTION blendFunc;
	BITMAPINFO bmpInfo;

	//Font-specific properties
	int firstChar;
	int lastChar;
	int tracking;
	int* charPositions;
	int num_char_pos;
	int* bearingLeft;
	int* bearingRight;
	bool uppercaseOnly;

	//High-level stuff
	BOOL error;
	TCHAR errorMsg[300]; //Change back to 100 later...

	//Private methods
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

	//Useful globals
	DWORD currPos;
	char* res_data;
	DWORD res_size;
};
