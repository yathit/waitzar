/* 
   InflaterHuffmanTree.h v0.0.7
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

#include "StreamManipulator.h"

#define MAX_BITLEN 15

short bitReverse(int value);
const char bit4Reverse[] = "\000\010\004\014\002\012\006\016\001\011\005\015\003\013\007\017";

/**
 * Constructs a Huffman tree from the array of code lengths.
 */
class InflaterHuffmanTree
{
public:
	InflaterHuffmanTree(short* codeLens, int codeL_len);
	~InflaterHuffmanTree(void);

	int getSymbol(StreamManipulator &input);

	//Delete later:
	TCHAR specialString[500];
private:
	//Methods
	void buildTree(short* codeLengths, int codeL_len);

	//Fields
	short* tree;
};


