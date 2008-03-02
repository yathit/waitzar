/* 
   StreamManipulator.h v0.0.7
   Copyright (C) 2001 Free Software Foundation, Inc.
   Written by John Leuner for the jazzlib project.

   All rights reserved. (see the NOTICE file in the top directory for more information)

   NOTE: For those unfamiliar with the GNU Classpath project's license, this code
   falls under the "special exception" to the GNU GPL which allows one 
         "to link this library... to produce an executable,... and to copy and distribute 
		  [the .exe] under terms of your choice..."
   The WaitZar project chooses to license the resultant EXE under the general licensing
   terms for the WaitZar project (currently the Apache License 2.0) and to distribute the 
   relevant jazzlib source (as required) under the terms of the GNU Classpath license.

   Please see:
   http://jazzlib.sourceforge.net/
   ...for the latest licensing information regarding the Jazzlib
*/

#pragma once

#define _UNICODE
#define UNICODE

#include <windows.h>
#include <tchar.h>
#include <stdio.h>


//Useful methods
void copyArray(char* source, int srcPos, char* dest, int destPos, int length);
int tripleRightShift(int value, int shiftBy);
int doubleRightShift(int value, int shiftBy);


/**
 * General-purpose input getter, but optimized for the Inflator class.
 */
class StreamManipulator
{
public:
	StreamManipulator();

	int peekBits(int n);
	void dropBits(int n);
	int getBits(int n);
	int getAvailableBits();
	int getAvailableBytes();
	void skipToByteBoundary();
	bool needsInput();
	int copyBytes(char* output, int offset, int length);
	void reset();
	void setInput(char* buf, int off, int len);

	//Delete later...
	TCHAR specialMessage[300];
private:
	//Fields
	char* window;
    int window_start;
    int window_end;
    int buffer;
    int bits_in_buffer;
};
