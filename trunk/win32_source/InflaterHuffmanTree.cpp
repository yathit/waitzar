/* 
   InflaterHuffmanTree.cpp v0.0.7
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


#include ".\inflaterhuffmantree.h"


/** 
 *
 */
short bitReverse(int value)
{
    return (short) (bit4Reverse[value & 0xf] << 12
		    | bit4Reverse[doubleRightShift(value, 4) & 0xf] << 8
		    | bit4Reverse[doubleRightShift(value, 8) & 0xf] << 4
		    | bit4Reverse[doubleRightShift(value, 12)]);
}



/**
 * NOTE: Make sure this is never called with the default constructor!
 * Otherwise, the statics won't initialize.
 */
InflaterHuffmanTree::InflaterHuffmanTree(char* codeLens, int codeL_len)
{	
	lstrcpy(specialString, _T(""));
	debug_error_count = 0;

	buildTree(codeLens, codeL_len);

    //Compute a "checksum"
    long check = 0;
    for (int i=0; i<512; i++)
    	check += tree[i];
	check = 0;
}

InflaterHuffmanTree::~InflaterHuffmanTree(void)
{
	if (tree != NULL)
		delete [] tree;
}

void InflaterHuffmanTree::buildTree(char* codeLengths, int codeL_len)
{
	int* blCount = new int[MAX_BITLEN+1];
    int* nextCode = new int[MAX_BITLEN+1];

	//Java inits... gr....
	for (int i=0; i<MAX_BITLEN+1; i++) {
		blCount[i] = 0;
		nextCode[i] = 0;
	}

    for (int i = 0; i < codeL_len; i++) {
		int bits = codeLengths[i];
		if (bits > 0)
			blCount[bits]++;

		if (bits >= MAX_BITLEN+1)
			debug_error_count++;
    }

	//return;

	//DEBUG
	/*lstrcpy(specialString, _T("["));
	TCHAR debug_str[100];
	lstrcpy(debug_str, _T("["));*/


    unsigned int code = 0;
    int treeSize = 512;
    for (int bits = 1; bits <= MAX_BITLEN; bits++) {
		//DEBUG
		/*if (lstrlen(debug_str) < 70) {
			swprintf(specialString, _T("%s(%x,%x), "), debug_str, code, blCount[bits]);
			lstrcpy(debug_str, specialString);
		}*/

		nextCode[bits] = code;
		code += blCount[bits] << (16 - bits);
		if (bits >= 10) {
			// We need an extra table for bit lengths >= 10.
			int start = nextCode[bits] & 0x1ff80;
		    int end   = code & 0x1ff80;
			treeSize += doubleRightShift((end - start), (16 - bits));
		}
    }

	//DEBUG
	if (code == 65536) {
	//	swprintf(specialString, _T("code ok!"));
	} else {
	//	swprintf(specialString, _T("bad code! %i"), code);
		return;
	}
	//return;

    /*if (code != 65536)
      throw new DataFormatException("Code lengths don't add up properly.");*/

    // Now create and fill the extra tables from longest to shortest
    // bit len.  This way the sub trees will be aligned.
    tree = new short[treeSize];

	//Java inits....
	for (int i=0; i<treeSize; i++)
		tree[i] = 0;

    int treePtr = 512;
    for (int bits = MAX_BITLEN; bits >= 10; bits--) {
		int end   = code & 0x1ff80;
		code -= blCount[bits] << (16 - bits);
		int start = code & 0x1ff80;
		for (int i=start; i<end; i+=1<<7) {
			tree[bitReverse(i)] = (short) ((-treePtr << 4) | bits);

			if (bitReverse(i) >= treeSize)
				debug_error_count++;

			treePtr += 1 << (bits-9);
		}
    }

    for (int i = 0; i < codeL_len; i++) {
		int bits = codeLengths[i];
		if (bits == 0)
			continue;

		if (bits >= MAX_BITLEN+1)
			debug_error_count++;

		code = nextCode[bits];
		int revcode = bitReverse(code);
		if (bits <= 9) {
			do {
				tree[revcode] = (short) ((i << 4) | bits);

				if (revcode >= treeSize)
					debug_error_count++;

				revcode += 1 << bits;
			} while (revcode < 512);
		} else {
			if ((revcode & 511) >= treeSize)
				debug_error_count++;

			int subTree = tree[revcode & 511];
			int treeLen = 1 << (subTree & 15);
			subTree = -doubleRightShift(subTree, 4);
			do { 

				if ((subTree |doubleRightShift(revcode, 9)) >= treeSize)
					debug_error_count++;

				tree[subTree |doubleRightShift(revcode, 9)] = (short) ((i << 4) | bits);
				revcode += 1 << bits;
			} while (revcode < treeLen);
		}

		if (bits>=MAX_BITLEN+1)
			debug_error_count++;

		nextCode[bits] = code + (1 << (16 - bits));
    }

	delete [] blCount;
	delete [] nextCode;
}


/**
 * Reads the next symbol from input, as encoded by the huffman tree
 */
int InflaterHuffmanTree::getSymbol(StreamManipulator &input)
{
    int lookahead, symbol;
	lookahead = input.peekBits(9);
    if (lookahead >= 0) {
		symbol = tree[lookahead];
		if (symbol >= 0) {
			input.dropBits(symbol & 15);
			return doubleRightShift(symbol, 4);
		}
		int subtree = -doubleRightShift(symbol, 4);
		int bitlen = symbol & 15;
		lookahead = input.peekBits(bitlen);
		if ((lookahead) >= 0) {
			symbol = tree[subtree | doubleRightShift(lookahead, 9)];
			input.dropBits(symbol & 15);
			return doubleRightShift(symbol, 4);
		} else {
			int bits = input.getAvailableBits();
			lookahead = input.peekBits(bits);
			symbol = tree[subtree | doubleRightShift(lookahead, 9)];
			if ((symbol & 15) <= bits) {
				input.dropBits(symbol & 15);
				return doubleRightShift(symbol, 4);
			} else
				return -1;
		}
    } else {
		int bits = input.getAvailableBits();
		lookahead = input.peekBits(bits);
		symbol = tree[lookahead];
		if (symbol >= 0 && (symbol & 15) <= bits) {
			input.dropBits(symbol & 15);
			return doubleRightShift(symbol, 4);
		} else
			return -1;
	}
}
