/* 
   Inflater.cpp v0.0.7
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


#include ".\inflater.h"


/**
 * 
 */
Inflater::Inflater()
{
	init(false);
}

/**
 * 
 */
Inflater::Inflater(bool noWrap)
{
	init(noWrap);
}


/**
 * Cleanup, where possible...
 */
Inflater::~Inflater()
{
	if (input!=NULL)
		delete input;
	if (outputWindow!=NULL)
		delete outputWindow;
	if (dynHeader!=NULL)
		delete dynHeader;
	if (litlenTree!=NULL)
		delete litlenTree;
	if (distTree!=NULL)
		delete distTree;
	if (adler!=NULL)
		delete adler;
}


void Inflater::init(bool noWrap)
{
    this->nowrap = noWrap;
    this->adler = new Adler32();
    input = new StreamManipulator();
    outputWindow = new OutputWindow();
    mode = nowrap ? DECODE_BLOCKS : DECODE_HEADER;

	//Try this:
	char* codeLengths = new char[288];
	int i = 0;
	while (i < 144)
		codeLengths[i++] = 8;
	while (i < 256)
		codeLengths[i++] = 9;
	while (i < 280)
		codeLengths[i++] = 7;
	while (i < 288)
		codeLengths[i++] = 8;
	litlenTree = new InflaterHuffmanTree(codeLengths, 288);
	delete [] codeLengths;
	
	codeLengths = new char[32];
	i = 0;
	while (i < 32)
		codeLengths[i++] = 5;
	distTree = new InflaterHuffmanTree(codeLengths, 32);
	delete [] codeLengths;
}