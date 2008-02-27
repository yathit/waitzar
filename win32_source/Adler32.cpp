/* 
   Adler32.cpp v0.0.7
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


#include ".\adler32.h"


/**
 * Checksum starts with a value of 1
 */
Adler32::Adler32()
{
	reset();
}


/**
 * Reset to initial value
 */
void Adler32::reset()
{
	checksum = 1;
}


/**
 * Updates the checksum with the byte b. Ignores high byte
 */
void Adler32::update(int bval)
{
    //Re-implement to avoid creating a new int array
    int s1 = checksum & 0xffff;
    int s2 = tripleRightShift(checksum, 16);
    
    s1 = (s1 + (bval & 0xFF)) % BASE;
    s2 = (s1 + s2) % BASE;
    
    checksum = (s2 << 16) + s1;
}


/**
 * Updates the checksum with an array of bytes
 */
void Adler32::update(char* buffer, int buffer_length)
{
	update(buffer, 0, buffer_length);
}


/**
 * Updates the checksum with an array of bytes
 */
void Adler32::update(char* buf, int off, int len)
{
    //(By Per Bothner)
    int s1 = checksum & 0xffff;
	int s2 = tripleRightShift(checksum, 16);

    while (len > 0) {
		// We can defer the modulo operation:
		// s1 maximally grows from 65521 to 65521 + 255 * 3800
		// s2 maximally grows by 3800 * median(s1) = 2090079800 < 2^31
		int n = 3800;
		if (n > len)
			n = len;
		len -= n;
		while (--n >= 0) {
			s1 = s1 + (buf[off++] & 0xFF);
			s2 = s2 + s1;
		}
		s1 %= BASE;
		s2 %= BASE;
    }

    checksum = (s2 << 16) | s1;
}


/**
 * Returns the checksum computed so far
 */
long Adler32::getValue()
{
	return (long) (checksum & 0xffffffff);
}
