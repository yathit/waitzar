/* 
   OutputWindow.h v0.0.7
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

#include <windows_wz.h>
//#include <stdio.h>
#include <algorithm>

#include "StreamManipulator.h"


//Useful defines
#define WINDOW_SIZE (1 << 15)
#define WINDOW_MASK (WINDOW_SIZE - 1)


/**
 * Contains the output from the Inflation process.
 * Needed so as to refer backwards to the output stream to repeat operations.
 */
class OutputWindow
{
public:
	OutputWindow();

	void write(int abyte);
	void repeat(int len, int dist);
	int copyStored(StreamManipulator &input, int len);
	void copyDict(char* dict, int offset, int len);
	int getFreeSpace();
	int getAvailable();
	int copyOutput(char* output, int offset, int len);
	void reset();

private:
	//Functions
	void slowRepeat(int rep_start, int len, int dist);

	//Fields
	char window[WINDOW_SIZE];
	int window_end;
	int window_filled;
};


