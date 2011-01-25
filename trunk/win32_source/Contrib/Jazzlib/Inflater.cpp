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
	//NULLs
	input = NULL;
	outputWindow = NULL;
	dynHeader = NULL;
	litlenTree = NULL;
	distTree = NULL;
	adler = NULL;

    this->nowrap = noWrap;
    this->adler = new Adler32();
    input = new StreamManipulator();
    outputWindow = new OutputWindow();
    mode = nowrap ? DECODE_BLOCKS : DECODE_HEADER;

	//Init to Java defaults...
	this->isLastBlock = false;
	this->readAdler = 0;
	this->neededBits = 0;
	this->repLength = this->repDist = 0;
	this->uncomprLen = 0;
	this->isLastBlock = false;
	this->totalOut = 0;
	this->totalIn = 0;
}


void Inflater::end()
{
	//Nothing needed... delete the object instead.
}


bool Inflater::finished()
{
	return (mode == FINISHED) && (outputWindow->getAvailable() == 0);
}


int Inflater::getAdler()
{
	return needsDictionary() ? readAdler : (int) adler->getValue();
}


int Inflater::getRemaining()
{
	return input->getAvailableBytes();
}


int Inflater::getTotalIn()
{
	return totalIn - getRemaining();
}


int Inflater::getTotalOut()
{
	return totalOut;
}


int Inflater::inflate(char* buf, int buf_length)
{
	return inflate (buf, buf_length, 0, buf_length);
}


int Inflater::inflate(char* buf, int buf_length, int off, int len)
{
    //Special case: len may be zero
    if (len == 0)
		return 0;
    
	//Check for correct buff, off, len triple
    if (0 > off || off > off + len || off + len > buf_length)
		return -1;

    int count = 0;
    int more;
    do {
		if (mode != DECODE_CHKSUM) {
	    /* Don't give away any output, if we are waiting for the
	     * checksum in the input stream.
	     *
	     * With this trick we have always:
	     *   needsInput() and not finished() 
	     *   implies more output can be produced.  
	     */
			more = outputWindow->copyOutput(buf, off, len);

			adler->update(buf, off, more);

			off += more;
			count += more;
			totalOut += more;
			len -= more;
			if (len == 0) {
				return count;
			}
		}
    } while (decode() || (outputWindow->getAvailable() > 0 && mode != DECODE_CHKSUM));
    return count;
}


bool Inflater::needsDictionary()
{
	return mode == DECODE_DICT && neededBits == 0;
}


bool Inflater::needsInput()
{
	return input->needsInput();
}


void Inflater::reset()
{
    mode = nowrap ? DECODE_BLOCKS : DECODE_HEADER;
    totalIn = totalOut = 0;
    input->reset();
    outputWindow->reset();

	if (dynHeader!=NULL) {
		delete dynHeader;
		dynHeader = NULL;
	}
	if (litlenTree!=NULL) {
		delete litlenTree;
		litlenTree = NULL;
	}
	if (distTree!=NULL) {
		delete distTree;
		distTree = NULL;
	}
    isLastBlock = false;
    adler->reset();
}


void Inflater::setDictionary(char* buffer, int buf_length)
{
	setDictionary(buffer, 0, buf_length);
}

void Inflater::setDictionary (char* buffer, int off, int len)
{
    /*if (!needsDictionary())
      throw new IllegalStateException();*/

    adler->update(buffer, off, len);
	//long addTest = adler->getValue(); //from check below...
    /*if ((int) addTest != readAdler)
      throw new IllegalArgumentException("Wrong adler checksum");*/
    adler->reset();
    outputWindow->copyDict(buffer, off, len);
    mode = DECODE_BLOCKS;
}


void Inflater::setInput(char* buf, int buf_length)
{
	setInput (buf, 0, buf_length);
}


void Inflater::setInput(char* buf, int off, int len)
{
    input->setInput (buf, off, len);
    totalIn += len;
}


bool Inflater::decodeHeader()
{
    int header = input->peekBits(16);
    if (header < 0)
		return false;
    input->dropBits(16);
    
    //The header is written in "wrong" byte order
    header = ((header << 8) | doubleRightShift(header, 8)) & 0xffff;
    /*if (header % 31 != 0)
		throw new DataFormatException("Header checksum illegal");*/
    /*if ((header & 0x0f00) != (Deflater.DEFLATED << 8))
      throw new DataFormatException("Compression Method unknown");*/

    /* Maximum size of the backwards window in bits. 
     * We currently ignore this, but we could use it to make the
     * inflater window more space efficient. On the other hand the
     * full window (15 bits) is needed most times, anyway.
     int max_wbits = ((header & 0x7000) >> 12) + 8;
     */
	if ((header & 0x0020) == 0) { //Dictionary flag?
		mode = DECODE_BLOCKS;
    } else {
		mode = DECODE_DICT;
		neededBits = 32;      
    }

	

    return true;
}


bool Inflater::decodeDict()
{
	while (neededBits > 0) {
		int dictByte = input->peekBits(8);
		if (dictByte < 0)
			return false;
		input->dropBits(8);
		readAdler = (readAdler << 8) | dictByte;
		neededBits -= 8;
    }
    return false;
}


bool Inflater::decodeHuffman()
{
	int free = outputWindow->getFreeSpace();

    while (free >= 258) {
		int symbol;
		switch (mode)
		{
			case DECODE_HUFFMAN:
				//This is the inner loop so it is optimized a bit
				
				for(;;) {
					symbol = litlenTree->getSymbol(*input);
					if (((symbol) & ~0xff) != 0)
						break;

					outputWindow->write(symbol);
					if (--free < 258)
						return true;
				} 
				if (symbol < 257) {
					if (symbol < 0)
						return false;
					else {
						//symbol == 256: end of block
						if (distTree!=NULL) {
							delete distTree;
							distTree = NULL;
						}
						if (litlenTree != NULL) {
							delete litlenTree;
							litlenTree = NULL;
						}
						mode = DECODE_BLOCKS;
						return true;
					}
				}		
				//try {
				repLength = CPLENS[symbol - 257];
				neededBits = CPLEXT[symbol - 257];
				//} catch (ArrayIndexOutOfBoundsException ex) {
				//	throw new DataFormatException("Illegal rep length code");
				//}
				//Fall-through
			case DECODE_HUFFMAN_LENBITS:
				if (neededBits > 0) {
					mode = DECODE_HUFFMAN_LENBITS;
					int i = input->peekBits(neededBits);
					if (i < 0)
						return false;
					input->dropBits(neededBits);
					repLength += i;
				}
				mode = DECODE_HUFFMAN_DIST;
				//fall through
			case DECODE_HUFFMAN_DIST:
				symbol = distTree->getSymbol(*input);
				if (symbol < 0)
					return false;
				//try 
				//{
				repDist = CPDIST[symbol];
				neededBits = CPDEXT[symbol];
				//} catch (ArrayIndexOutOfBoundsException ex) {
				//	throw new DataFormatException("Illegal rep dist code");
				//}
				// fall through 
			case DECODE_HUFFMAN_DISTBITS:
				if (neededBits > 0) {
					mode = DECODE_HUFFMAN_DISTBITS;
					int i = input->peekBits(neededBits);
					if (i < 0)
						return false;
					input->dropBits(neededBits);
					repDist += i;
				}
				outputWindow->repeat(repLength, repDist);
				free -= repLength;
				mode = DECODE_HUFFMAN;
				break;
			/*default:
				throw new IllegalStateException();*/
		}
	}
    return true;
}


bool Inflater::decodeChksum()
{
	while (neededBits > 0) {
		int chkByte = input->peekBits(8);
		if (chkByte < 0)
			return false;
		input->dropBits(8);
		readAdler = (readAdler << 8) | chkByte;
		neededBits -= 8;
    }
    /*if ((int) adler->getValue() != readAdler)
		throw new DataFormatException("Adler chksum doesn't match: "
				    +Integer.toHexString((int)adler.getValue())
				    +" vs. "+Integer.toHexString(readAdler)); */
    mode = FINISHED;
    return false;
}


bool Inflater::decode()
{
    switch (mode) 
	{
		case DECODE_HEADER:
			return decodeHeader();
		case DECODE_DICT:
			return decodeDict();
		case DECODE_CHKSUM:
			return decodeChksum();
		case DECODE_BLOCKS:
		{
			if (isLastBlock) {
				if (nowrap) {
					mode = FINISHED;
					return false;
				} else {
					input->skipToByteBoundary();
					neededBits = 32;
					mode = DECODE_CHKSUM;
					return true;
				}
			}

			int type = input->peekBits(3);
			if (type < 0)
				return false;
			input->dropBits(3);

			if ((type & 1) != 0)
				isLastBlock = true;
			switch (doubleRightShift(type, 1))
			{
				case DEFLATE_STORED_BLOCK:
					input->skipToByteBoundary();
					mode = DECODE_STORED_LEN1;
					break;
				case DEFLATE_STATIC_TREES:
					litlenTree = createLitlenTree();
					distTree = createDistTree();
					mode = DECODE_HUFFMAN;
					break;
				case DEFLATE_DYN_TREES:
					dynHeader = new InflaterDynHeader();
					mode = DECODE_DYN_HEADER;
					break;
				/*default:
					throw new DataFormatException("Unknown block type "+type); */
			}

			//Second call returns here...

			return true;
		}
		case DECODE_STORED_LEN1:
		{
			if ((uncomprLen = input->peekBits(16)) < 0)
				return false;
			input->dropBits(16);
			mode = DECODE_STORED_LEN2;
		} //fall through
		case DECODE_STORED_LEN2:
		{
			int nlen = input->peekBits(16);
			if (nlen < 0)
				return false;
			input->dropBits(16);
			/*if (nlen != (uncomprLen ^ 0xffff))
				throw new DataFormatException("broken uncompressed block");*/
			mode = DECODE_STORED;
		} //fall through
		case DECODE_STORED:
		{
			int more = outputWindow->copyStored(*input, uncomprLen);
			uncomprLen -= more;
			if (uncomprLen == 0) {
				mode = DECODE_BLOCKS;
				return true;
			}
			return !input->needsInput();
		}
		case DECODE_DYN_HEADER:
			if (!dynHeader->decode(*input))
				return false;

			litlenTree = dynHeader->buildLitLenTree();

			distTree = dynHeader->buildDistTree();
			mode = DECODE_HUFFMAN;
			//fall through
		case DECODE_HUFFMAN:
		case DECODE_HUFFMAN_LENBITS:
		case DECODE_HUFFMAN_DIST:
		case DECODE_HUFFMAN_DISTBITS:
			return decodeHuffman();
		case FINISHED:
			return false;
		default:
			//throw new IllegalStateException();
			return false;
	}
}


InflaterHuffmanTree* Inflater::createLitlenTree()
{
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

	InflaterHuffmanTree* retVal = new InflaterHuffmanTree(codeLengths, 288);
	delete [] codeLengths;
	return retVal;
}



InflaterHuffmanTree* Inflater::createDistTree()
{
	char* codeLengths= new char[32];
	int i = 0;
	while (i < 32)
		codeLengths[i++] = 5;
	InflaterHuffmanTree* retVal = new InflaterHuffmanTree(codeLengths, 32);
	delete [] codeLengths;
	return retVal;
}

