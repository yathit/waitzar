/* 
   OutputWindow.cpp v0.0.7
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


#include ".\outputwindow.h"

OutputWindow::OutputWindow()
{
	window_end = 0;
	window_filled = 0;
}


void OutputWindow::write(int abyte)
{
    /*if (window_filled++ == WINDOW_SIZE)
      throw new IllegalStateException("Window full");*/
    window[window_end++] = (char) abyte;
    window_end &= WINDOW_MASK;
}


void OutputWindow::slowRepeat(int rep_start, int len, int dist)
{
	while (len-- > 0) {
		window[window_end++] = window[rep_start++];
		window_end &= WINDOW_MASK;
		rep_start &= WINDOW_MASK;
    }
}


void OutputWindow::repeat(int len, int dist)
{
	/*if ((window_filled += len) > WINDOW_SIZE)
      throw new IllegalStateException("Window full");*/

    int rep_start = (window_end - dist) & WINDOW_MASK;
    int border = WINDOW_SIZE - len;
    if (rep_start <= border && window_end < border) {
		if (len <= dist) {
			copyArray(window, rep_start, window, window_end, len);
			window_end += len;
		} else {

	    // We have to copy manually, since the repeat pattern overlaps.
	    while (len-- > 0)
	      window[window_end++] = window[rep_start++];
		}
	} else
		slowRepeat(rep_start, len, dist);
}


int OutputWindow::copyStored(StreamManipulator &input, int len)
{
	int copied;
    len = min(min(len, WINDOW_SIZE - window_filled), input.getAvailableBytes());
    
	int tailLen = WINDOW_SIZE - window_end;
    if (len > tailLen) {
		copied = input.copyBytes(window, window_end, tailLen);
		if (copied == tailLen)
			copied += input.copyBytes(window, 0, len - tailLen);
    } else
		copied = input.copyBytes(window, window_end, len);

    window_end = (window_end + copied) & WINDOW_MASK;
    window_filled += copied;
    return copied;
}


void OutputWindow::copyDict(char* dict, int offset, int len)
{
    /*if (window_filled > 0)
      throw new IllegalStateException();*/

    if (len > WINDOW_SIZE) {
		offset += len - WINDOW_SIZE;
		len = WINDOW_SIZE;
    }

    copyArray(dict, offset, window, 0, len);
    window_end = len & WINDOW_MASK;
}


int OutputWindow::getFreeSpace()
{
	return WINDOW_SIZE - window_filled;
}

int OutputWindow::getAvailable()
{
	return window_filled;
}

void OutputWindow::reset()
{
	window_filled = window_end = 0;
}


int OutputWindow::copyOutput(char* output, int offset, int len)
{
	int copy_end = window_end;
    if (len > window_filled)
      len = window_filled;
    else
      copy_end = (window_end - window_filled + len) & WINDOW_MASK;

    int copied = len;
    int tailLen = len - copy_end;

    if (tailLen > 0) {
		copyArray(window, WINDOW_SIZE - tailLen, output, offset, tailLen);
		offset += tailLen;
		len = copy_end;
    }
    copyArray(window, copy_end - len, output, offset, len);
    window_filled -= copied;
    /*if (window_filled < 0)
      throw new IllegalStateException();*/
    return copied;
}


