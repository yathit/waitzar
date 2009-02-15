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

//Don't let Visual Studio warn us to use the _s functions
#define _CRT_SECURE_NO_WARNINGS

//Ironic that adding compliance now causes headaches compiling under VS2003
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include "Inflater.h"

//Magic number for .PNG header.
const char PNG_SIGNATURE[] = "\x89PNG\x0D\x0A\x1A\x0A";  //0x89504e470d0a1a0a

//PNG chunk IDs
#define CHUNK_IHDR 0x49484452
#define CHUNK_PLTE 0x504c5445
#define CHUNK_TRNS 0x74524e53
#define CHUNK_IDAT 0x49444154
#define CHUNK_IEND 0x49454e44

//PulpCore chunk IDs
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
 * Since win32 doesn't seem to like PNGs, we might as well just use Pulp Core's
 * image class, and then extend it later to create fonts
 */
class PulpCoreImage
{
public:
	//Empty constructor
	PulpCoreImage();

	//Initializer, copy initializer
	virtual void init(HRSRC resource, HGLOBAL dataHandle, HDC currDC);
	virtual void init(PulpCoreImage *copyFrom, HDC currDC);

	//Make a blank image
	virtual void init(int width, int height, int bkgrdARGB, HDC currDC, HDC &thisDC, HBITMAP &thisBmp);

	//Useful for making various color replicas of one image.
	// Particularly useful for fonts, but used in other places
	// for monochrome images as well
	void tintSelf(UINT rgbColor);

	//Basic drawing functionality
	void draw(HDC bufferDC, int xPos, int yPos);

	//We report errors, though they're unlikely.
	BOOL isInError();
	TCHAR* getErrorMsg();

	//Basic properties
	int getWidth();
	int getHeight();

protected:
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
	virtual void readChunk(int chunkType, int length, HDC currDC);
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

};
