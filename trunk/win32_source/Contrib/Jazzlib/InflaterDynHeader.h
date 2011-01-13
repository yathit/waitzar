/* 
   InflaterDynHeader.h v0.0.7
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

#include <windows_wz.h>
//#include <stdio.h>

#include "StreamManipulator.h"
#include "InflaterHuffmanTree.h"

#define LNUM 0
#define DNUM 1
#define BLNUM 2
#define BLLENS 3
#define LENS 4
#define REPS 5


//Globals
const int repMin[] = { 3, 3, 11 };
const int BL_ORDER[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
const int repBits[] = { 2, 3,  7 };


/**
 * 
 */
class InflaterDynHeader
{
public:
	InflaterDynHeader();
	~InflaterDynHeader();

	bool decode(StreamManipulator &input);
	InflaterHuffmanTree* buildLitLenTree();
	InflaterHuffmanTree* buildDistTree();
private:
	char* blLens;
	char* litdistLens;
	
	InflaterHuffmanTree* blTree;
  
	int mode;
	int lnum, dnum, blnum, num;
	int repSymbol;
	char lastLen;
	int ptr;
};
