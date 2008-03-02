/* 
   Inflater.h v0.0.7
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
#include "OutputWindow.h"
#include "InflaterDynHeader.h"
#include "InflaterHuffmanTree.h"
#include "Adler32.h"


#define DECODE_HEADER 0
#define DECODE_DICT 1
#define DECODE_BLOCKS 2
#define DECODE_STORED_LEN1 3
#define DECODE_STORED_LEN2 4
#define DECODE_STORED 5
#define DECODE_DYN_HEADER 6
#define DECODE_HUFFMAN 7
#define DECODE_HUFFMAN_LENBITS 8
#define DECODE_HUFFMAN_DIST 9
#define DECODE_HUFFMAN_DISTBITS 10
#define DECODE_CHKSUM 11
#define FINISHED 12

#define DEFLATE_STORED_BLOCK 0
#define DEFLATE_STATIC_TREES 1
#define DEFLATE_DYN_TREES 2

//Copy lengths for literal codes 257..285
const int CPLENS[] =  { 
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};

//Extra bits for literal codes 257..285 
const int CPLEXT[] = { 
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};

//Copy offsets for distance codes 0..29
const int CPDIST[] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
    8193, 12289, 16385, 24577
};

//Extra bits for distance codes
const int CPDEXT[] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 
    12, 12, 13, 13
};


/**
 *  De-compresses data which was deflated according to rfc1950
 */
class Inflater
{
public:
	Inflater();
	Inflater(bool noWrap);
	~Inflater();

	void end();
	bool finished();
	int getAdler();
	int getRemaining();
	int getTotalIn();
	int getTotalOut();
	int inflate (char* buf, int buf_length);
	int inflate (char* buf, int buf_length, int off, int len);
	bool needsDictionary();
	bool needsInput();
	void reset();
	void setDictionary(char* buffer, int buf_length);
	void setDictionary (char* buffer, int off, int len);
	void setInput(char* buf, int buf_length);
	void setInput(char* buf, int off, int len);
	bool decodeHeader();
	bool decodeDict();
	bool decodeHuffman();
	bool decodeChksum();
	bool decode();

	//Move back to private later...
	int mode;
	TCHAR specialMessage[500]; //Delete later
private:
	void init(bool noWrap);
	InflaterHuffmanTree* createLitlenTree();
	InflaterHuffmanTree* createDistTree();

	
	int readAdler;
	int neededBits;
	int repLength, repDist;
	int uncomprLen;
	bool isLastBlock;
	int totalOut;
	int totalIn;
	bool nowrap;

	StreamManipulator* input;
	OutputWindow* outputWindow;
	InflaterDynHeader* dynHeader;
	InflaterHuffmanTree* litlenTree;
	InflaterHuffmanTree* distTree;
	Adler32* adler;
};
