/* 
   Adler32.h v0.0.7
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
#undef min
#undef max
#include <tchar.h>
#include <stdio.h>

#include "StreamManipulator.h"

// largest prime smaller than 65536
#define BASE 65521

/**
 * Computes Adler32 checksum for a stream of data.
 */
class Adler32
{
public:
	Adler32();

	void reset();
	void update(int bval);
	void update(char* buffer, int buffer_length);
	void update(char* buf, int off, int len);
	long getValue();

private:
	int checksum;
};
