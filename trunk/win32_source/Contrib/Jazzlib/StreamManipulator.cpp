/* 
   StreamManipulator.cpp v0.0.7
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


#include ".\streammanipulator.h"


/**
 * Clean-room implementation of System.arraycopy() from the JRE
 */
void copyArray(const char* source, int srcPos, char* dest, int destPos, int length)
{
	for (int i=0; i<length; i++) {
		dest[destPos+i] = source[srcPos+i];
	}
}


/**
 * >>> is UNSIGNED shift in java
 */
int tripleRightShift(int value, int shiftBy)
{
	unsigned int retVal = (unsigned int) value;
	return (retVal >> shiftBy);
}


/**
 * >> is SIGNED shift in java
 */
int doubleRightShift(int value, int shiftBy)
{
	return value >> shiftBy;
}


StreamManipulator::StreamManipulator()
{
    window_start = 0;
    window_end = 0;
    buffer = 0;
    bits_in_buffer = 0;
}


/**
 * Get the next n bits without incrementing the pointer
 */
int StreamManipulator::peekBits(int n) 
{
	if (bits_in_buffer < n) {
		//Are enough bits available?
		if (window_start == window_end)
			return -1;
		
		//Get bits
		buffer |= ((window[window_start] & 0xff) | (window[window_start+1] & 0xff) << 8) << bits_in_buffer;
		window_start+=2;
		bits_in_buffer += 16;
    }

    return buffer & ((1 << n) - 1);
}


/**
 * Drop the next n bits
 */
void StreamManipulator::dropBits(int n)
{
	buffer = tripleRightShift(buffer, n);
    bits_in_buffer -= n;
}


/**
 * Get the next n bits and increase the input pointer 
 */
int StreamManipulator::getBits(int n)
{
    int bits = peekBits(n);
    if (bits >= 0)
      dropBits(n);
    return bits;
}


/**
 * Gets the number of available bits in the buffer.
 */
int StreamManipulator::getAvailableBits()
{
	return bits_in_buffer;
}


/**
 * Get the number of available bytes
 */
int StreamManipulator::getAvailableBytes()
{
	return window_end - window_start + doubleRightShift(bits_in_buffer, 3);
}


/**
 * Skip to the next byte boundary
 */
void StreamManipulator::skipToByteBoundary()
{
	buffer = doubleRightShift(buffer, (bits_in_buffer & 7));
    bits_in_buffer &= ~7;
}


/**
 * 
 */
bool StreamManipulator::needsInput()
{
	return window_start == window_end;
}


/**
 * Copy "length" number of bytes from the input to the output buffer.
 * Returns the number of bytes copied (may be <length)
 * Returns -1 for error
 */
int StreamManipulator::copyBytes(char* output, int offset, int length)
{
	//lenght must be a natural number
    if (length < 0)
      return -1;

	// bits_in_buffer may only be 0 or 8 
    if ((bits_in_buffer & 7) != 0)  
      return -1;

	//Start copying
    int count = 0;
    while (bits_in_buffer > 0 && length > 0) {
		output[offset++] = (char) buffer;
		buffer = tripleRightShift(buffer, 8);
		bits_in_buffer -= 8;
		length--;
		count++;
    }
    if (length == 0)
      return count;

    int avail = window_end - window_start;
    if (length > avail)
      length = avail;
    copyArray(window, window_start, output, offset, length);
    window_start += length;

	// We always want an even number of bytes in input, see peekBits 
    if (((window_start - window_end) & 1) != 0) {
		buffer = (window[window_start++] & 0xff);
		bits_in_buffer = 8;
    }

    return count + length;
}


/**
 * 
 */
void StreamManipulator::reset()
{
	window_start = window_end = buffer = bits_in_buffer = 0;
}


/**
 *
 */
void StreamManipulator::setInput(const char* buf, int off, int len)
{
	/*
    if (window_start < window_end)
      throw new IllegalStateException
	("Old input was not completely processed");*/

    int end = off + len;

    // We want to throw an ArrayIndexOutOfBoundsException early.  The
    // check is very tricky: it also handles integer wrap around.  
    /*if (0 > off || off > end || end > buf.length)
      throw new ArrayIndexOutOfBoundsException();*/
    
	// We always want an even number of bytes in input, see peekBits 
    if ((len & 1) != 0) {
		buffer |= (buf[off++] & 0xff) << bits_in_buffer;
		bits_in_buffer += 8;
    }
    
    window = buf;
    window_start = off;
    window_end = end;
}
