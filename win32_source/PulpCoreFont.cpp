/*
    Copyright (c) 2008, Interactive Pulp, LLC
    All rights reserved. (see the NOTICE file in the top directory for more information)

	NOTE: This source file is licensed under the New BSD License, unlike the remainder
	of the WaitZar project, which is licensed under the Apache License 2.0. This file is an
	amalgamation of several Java source files in the Pulp Core project, and is used primarily
	for reading an annotated version of the PNG format.

	Please see:
	http://code.google.com/p/pulpcore/
	...for the latest licensing information regarding the Pulp Core project.
*/

#include ".\pulpcorefont.h"


/**
 * Copy constructor.
 */
PulpCoreFont::PulpCoreFont(PulpCoreFont *copyFrom, HDC currDC)
{
	//Init
	this->error = copyFrom->error;
	lstrcpy(this->errorMsg, copyFrom->errorMsg);

	//Copy all relevant fields
	this->bitDepth = copyFrom->bitDepth;
	this->colorType = copyFrom->colorType;
	this->width = copyFrom->width;
	this->height = copyFrom->height;
	this->isOpaque = copyFrom->isOpaque;
	this->hotspotX = copyFrom->hotspotX;
	this->hotspotY = copyFrom->hotspotY;
	this->num_char_pos = copyFrom->num_char_pos;
	this->uppercaseOnly = copyFrom->uppercaseOnly;
	this->pal_length = copyFrom->pal_length;
	this->firstChar = copyFrom->firstChar;
	this->lastChar = copyFrom->lastChar;
	this->tracking = copyFrom->tracking;

	//We're probably safe passing a reference.
	this->palette = copyFrom->palette;
	this->charPositions = copyFrom->charPositions;
	this->bearingLeft = copyFrom->bearingLeft;
	this->bearingRight = copyFrom->bearingRight;


	initBmpInfo();

	//Create the DIB to copy pixels onto
	directDC = CreateCompatibleDC(currDC);
	directBitmap = CreateDIBSection(directDC, &bmpInfo,  DIB_RGB_COLORS, (void**) &directPixels, NULL, 0);
	SelectObject(directDC, directBitmap);
	if (directBitmap==NULL && error==FALSE) {
		lstrcpy(errorMsg, _T("Couldn't create font bitmap."));
		error = TRUE;
		return;
	}

	//Copy pixels
	for (int i=0; i<width*height; i++)  {
		this->directPixels[i] = copyFrom->directPixels[i];
	}
}



/**
 * Tint all pixels of this font to a certain color; preserve alpha.
 *  NOTE: Unlike in PulpCore, the proper way to make a duplicate font
 *   of a tinted color is to call the copy constructor FIRST, and then
 *   this method on the newly-created font.
 */
void PulpCoreFont::tintSelf(UINT rgbColor)
{
	for (int i=0; i<width*height; i++) {
		directPixels[i] = premultiply((directPixels[i]&0xff000000)|(rgbColor&0x00ffffff));
	} 
}




void PulpCoreFont::initBmpInfo() 
{
	//Create a simple bitmap descriptor
	ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth=width;
	bmpInfo.bmiHeader.biHeight=height;
	bmpInfo.bmiHeader.biPlanes=1;
	bmpInfo.bmiHeader.biBitCount=32;
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	bmpInfo.bmiHeader.biSizeImage = width * height & sizeof(UINT);

	//Create the blend function now...
	blendFunc.BlendOp = AC_SRC_OVER;
	blendFunc.BlendFlags = 0; //Must be zero
	blendFunc.SourceConstantAlpha = 0xFF; //Blend at the pixel leve only.
	blendFunc.AlphaFormat = AC_SRC_ALPHA;
}



/**
 * Create a PulpCoreFont.
 */
PulpCoreFont::PulpCoreFont(HRSRC resource, HGLOBAL dataHandle, HDC currDC)
{
	//Init
	this->error = FALSE;
	lstrcpy(this->errorMsg, _T(""));

	//Get raw data
    res_data = (char*)LockResource(dataHandle);
    res_size = SizeofResource(NULL, resource);

	//Loop through all bytes...
	currPos = 0;

	//Read the png signature
	for (int i=0; i<8; i++) {
		if (res_data[currPos++] != PNG_SIGNATURE[i] && error==FALSE) {
			swprintf(errorMsg, _T("PNG_SIG[%i] is %02X not %02X"), i, res_data[currPos-1], PNG_SIGNATURE[i]);
			error = TRUE;
		}
	}

	//Read remaining "chunks"
    int length;
    int chunkType;
	while (error==FALSE) {
		length = readInt();
		chunkType = readInt();

		if (chunkType == CHUNK_IHDR)
			readHeader(currDC);
		else if (chunkType == CHUNK_HOTS) {
			hotspotX = readInt();
			hotspotY = readInt();
		} else if (chunkType == CHUNK_PLTE)
            readPalette(length);
        else if (chunkType == CHUNK_TRNS)
            readTransparency(length);
        else if (chunkType == CHUNK_IDAT)
            readData(length);
        else if (chunkType == CHUNK_ANIM)
            readAnimation();
        else if (chunkType == CHUNK_FONT)
            fontSet();
		else if (chunkType != CHUNK_IEND)
			currPos += length;  //Ignore this chunk

		//Ignore CRC
        readInt();

		//Continue?
        if (chunkType == CHUNK_IEND)
            break;
	}

	//Remaining data signifies an error
	if (currPos != res_size && error==FALSE) {
		swprintf(errorMsg, _T("Extraneous bytes: %l"), (res_size-currPos));
		error = TRUE;
	}


	//Why are bitmaps loading upside-down....?
	for (int x=0; x<width; x++) {
		for (int y=0; y<height/2; y++) {
			UINT temp = directPixels[y*width + x];
			directPixels[y*width + x] = directPixels[(height-1-y)*width + x];
			directPixels[(height-1-y)*width + x] = temp;
		}
	}

	//Note: Absolutely NEVER do this:
	//SelectObject(directDC, previousObject);
	//If you do, the bitmap will no longer be valid to write to.
}



void PulpCoreFont::readHeader(HDC currDC)
{
	//Read basic image information
	width = readInt();
    height = readInt();
	bitDepth = readByte();
    colorType = readByte();
    int compressionMethod = readByte();
    int filterMethod = readByte();
    int interlaceMethod = readByte();
    bool supportedBitDepth = (bitDepth == 8 || (bitDepth == 4 && colorType == COLOR_TYPE_PALETTE));

	//Simple error check
    if (compressionMethod != 0 || filterMethod != 0 || interlaceMethod != 0 || !supportedBitDepth
		|| ( colorType != COLOR_TYPE_GRAYSCALE && colorType != COLOR_TYPE_RGB &&  colorType != COLOR_TYPE_PALETTE &&
		     colorType != COLOR_TYPE_GRAYSCALE_WITH_ALPHA && colorType != COLOR_TYPE_RGB_WITH_ALPHA))
	{
		//It's an error...
		if (error==FALSE) {
			lstrcpy(errorMsg, _T("PNG header requires unsupported options"));
			error = TRUE;
		}
	}

	//Create a new image
    isOpaque = true;
    if (colorType == COLOR_TYPE_GRAYSCALE_WITH_ALPHA || colorType == COLOR_TYPE_RGB_WITH_ALPHA)
		isOpaque = false;



	initBmpInfo();


	//Create the DIB to copy pixels onto
	directDC = CreateCompatibleDC(currDC);
	
	//Make the bitmap to use....
	directBitmap = CreateDIBSection(directDC, &bmpInfo,  DIB_RGB_COLORS, (void**) &directPixels, NULL, 0);
	SelectObject(directDC, directBitmap);
	if (directBitmap==NULL && error==FALSE) {
		lstrcpy(errorMsg, _T("Couldn't create font bitmap."));
		error = TRUE;
		return;
	}
}


void PulpCoreFont::readPalette(int length)
{
	//Palettes must be %3
	if (length%3!=0 && error==FALSE) {
		swprintf(errorMsg, _T("Palette length %i is not mod 3"), length);
		error = TRUE;
		return;
	}

	//Init palette array
	pal_length = length/3;
	palette = new int[pal_length];

	//Fill palette array
    for (int i=0; i<pal_length; i++) {
		int a = 0xff;
        int r = readByte();
        int g = readByte();
        int b = readByte();

        palette[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
}


void PulpCoreFont::readTransparency(int length)
{
	//Was the palette valid
	if ((palette == NULL || length > pal_length) && error==FALSE) {
		swprintf(errorMsg, _T("Pallete is null or too short."));
		error = TRUE;
		return;
	}

	//Read data
    for (int i=0; i<length; i++) {
		int a = readByte();
        if (a<0xFF)
			isOpaque = false;

		//CoreGraphics.PREMULTIPLIED_ALPHA is always true...
        palette[i] = premultiply((a << 24) | (palette[i] & 0xffffff));
    }
}


void PulpCoreFont::readAnimation()
{
	//Init
	int numFramesAcross = readShort();
    int numFramesDown = readShort();
    bool loop = (readByte()!=0);
    int frames = readShort();
    int* frameSequence = new int[frames];
    int* frameDuration = new int[frames];

	//Read sequences & durations
    for (int i=0; i<frames; i++)
		frameSequence[i] = readShort();
    for (int i=0; i<frames; i++)
        frameDuration[i] = readShort();

	//That's all... we don't support animated fonts for now...
	delete [] frameSequence;
	delete [] frameDuration;
}


void PulpCoreFont::readData(int length)
{
	Inflater* inflater = new Inflater();
	inflater->setInput(res_data, currPos, length);

    int bitsPerPixel = bitDepth * SAMPLES_PER_PIXEL[colorType];
    int bytesPerPixel = (bitsPerPixel + 7) / 8;
    int bytesPerScanline = (width * bitsPerPixel + 7) / 8;
    char* prevScanline = new char[bytesPerScanline];
	char* currScanline = new char[bytesPerScanline];
    char* filterBuffer = new char[1];
    int index = 0;

	//Java inits...
	filterBuffer[0] = 0;
	for (int i=0; i<bytesPerScanline; i++)
		currScanline[i] = 0;

    for (int i=0; i<height; i++) {
		inflateFully(inflater, filterBuffer, 1);

		if (error==TRUE)
			return;

		inflateFully(inflater, currScanline, bytesPerScanline);
        
		//Apply filter
		int filter = filterBuffer[0];
        if (filter > 0 && filter < 5)
			decodeFilter(currScanline, bytesPerScanline, prevScanline, filter, bytesPerPixel);
        else if (filter!=0 && error==FALSE) {
			error = TRUE;
			swprintf(errorMsg, _T("Illegal filter type: %i"), filter);
        }


		if (error==TRUE)
			return;

        //Convert bytes into ARGB pixels
        int srcIndex = 0;
        switch (colorType) {
			default: case COLOR_TYPE_GRAYSCALE:
				for (int j = 0; j < width; j++) {
					int v = currScanline[j] & 0xff;
                    directPixels[index++] = (0xff << 24) | (v << 16) | (v << 8) | v;
                }
                break;
			case COLOR_TYPE_RGB:
				for (int j = 0; j < width; j++) {
                    int r = currScanline[srcIndex++] & 0xff;
                    int g = currScanline[srcIndex++] & 0xff;
                    int b = currScanline[srcIndex++] & 0xff;
                    directPixels[index++] = (0xff << 24) | (r << 16) | (g << 8) | b;
                }
                break;
            case COLOR_TYPE_PALETTE:
                if (bitDepth == 8) {
                    for (int j = 0; j < width; j++)
                        directPixels[index++] = palette[currScanline[j] & 0xff];
                } else {
                    //Assume bitDepth == 4
                    bool isOdd = (width & 1) == 1;
                    int s = width & ~1;
                    for (int j = 0; j < s; j+=2) {
                        int b = currScanline[srcIndex++] & 0xff;
                        directPixels[index++] = palette[doubleRightShift(b, 4)];
                        directPixels[index++] = palette[b & 0x0f];
                    }
                    if (isOdd) {
                        int b = currScanline[srcIndex++] & 0xff;
                        directPixels[index++] = palette[doubleRightShift(b, 4)];
                    }
                }
                break;
            case COLOR_TYPE_GRAYSCALE_WITH_ALPHA:
                for (int j = 0; j < width; j++) {
                    int v = currScanline[srcIndex++] & 0xff;
                    int a = currScanline[srcIndex++] & 0xff;
                    directPixels[index++] = (a << 24) | (v << 16) | (v << 8) | v;
                }
                break;
            case COLOR_TYPE_RGB_WITH_ALPHA:
                for (int j = 0; j < width; j++) {
                    int r = currScanline[srcIndex++] & 0xff;
                    int g = currScanline[srcIndex++] & 0xff;
                    int b = currScanline[srcIndex++] & 0xff;
                    int a = currScanline[srcIndex++] & 0xff;
                    directPixels[index++] = (a << 24) | (r << 16) | (g << 8) | b;
                }
                break;
        }

        //Swap curr and prev scanlines
        char* temp = currScanline;
        currScanline = prevScanline;
        prevScanline = temp;
    }

    if (/*CoreGraphics.PREMULTIPLIED_ALPHA &&*/
        (colorType == COLOR_TYPE_GRAYSCALE_WITH_ALPHA ||
        colorType == COLOR_TYPE_RGB_WITH_ALPHA))
    {
        premultiply(directPixels, width*height);
    }

    inflater->end();
	currPos += length;

	delete [] prevScanline;
	delete [] currScanline;
    delete [] filterBuffer;

	delete inflater;
}


void PulpCoreFont::inflateFully(Inflater* inflater, char* result, int res_length)
{
	int bytesRead = 0;

    while (bytesRead < res_length) {
		if (inflater->needsInput() && error==FALSE) {
			swprintf(errorMsg, _T("Inflater ran out of input"));
			error = TRUE;
			return;
        }

		int res = inflater->inflate(result, res_length, bytesRead, res_length - bytesRead);
		if (res==-1 && error==FALSE) {
			lstrcpy(errorMsg, _T("0 > off || off > off + len || off + len > buf_length"));
			error = TRUE;
			return;
        }

        bytesRead += res;
    }
}

void PulpCoreFont::fontSet()
{
	//Read PulpCore's magic number
	for (int i=0; i<8; i++) {
		if (res_data[currPos++] != PULP_MAGICNUM[i] && error==FALSE) {
			swprintf(errorMsg, _T("PULP_MAGICNUM[%i] is %02X not %02X"), i, res_data[currPos-1], PULP_MAGICNUM[i]);
			error = TRUE;
		}
	}

	//"shorts" are always BIG_ENDIAN
	firstChar = readShort();
    lastChar = readShort();
    tracking = readShort();
	bool hasBearing = (readByte()!=0);

	//Initialize arrays
    int numChars = lastChar - firstChar + 1;
	num_char_pos = numChars + 1;
    charPositions = new int[num_char_pos];
    bearingLeft = new int[numChars];
    bearingRight = new int[numChars];

	//Java inits...
	for (int i=0; i<num_char_pos; i++) 
		charPositions[i] = 0;

	//Read character positions
    for (int i=0; i<num_char_pos; i++) {
		charPositions[i] = readShort();
    }

	//Read bearings
    for (int i = 0; i < numChars; i++) {
		if (hasBearing) {
			bearingLeft[i] = readShort();
			bearingRight[i] = readShort();
		} else {
			bearingLeft[i] = 0;
			bearingRight[i] = 0;
        }
	}
	uppercaseOnly = (lastChar < 'a');
}


void PulpCoreFont::decodeFilter(char* curr, int curr_len, char* prev, int filter, int bpp)
{
    int length = curr_len;

    if (filter == 1) {
        // Input = Sub
        // Raw(x) = Sub(x) + Raw(x-bpp)
        // For all x < 0, assume Raw(x) = 0.
        for (int i = bpp; i < length; i++) {
            curr[i] = (char)(curr[i] + curr[i - bpp]);
        }
    } else if (filter == 2) {
        // Input = Up
        // Raw(x) = Up(x) + Prior(x)
        for (int i = 0; i < length; i++) {
            curr[i] = (char)(curr[i] + prev[i]);
        }
    } else if (filter == 3) {
        // Input = Average
        // Raw(x) = Average(x) + floor((Raw(x-bpp)+Prior(x))/2)
        for (int i = 0; i < bpp; i++) {
			curr[i] = (char)(curr[i] + doubleRightShift((prev[i] & 0xff), 1));
        }
        for (int i = bpp; i < length; i++) {
            curr[i] = (char)(curr[i] + doubleRightShift(((curr[i - bpp] & 0xff) + (prev[i] & 0xff)), 1));
        }
    } else if (filter == 4) {
        // Input = Paeth
        // Raw(x) = Paeth(x) + PaethPredictor(Raw(x-bpp), Prior(x), Prior(x-bpp))
        for (int i = 0; i < bpp; i++) {
            curr[i] = (char)(curr[i] + prev[i]);
        }
        for (int i = bpp; i < length; i++) {
            curr[i] = (char)(curr[i] +
                paethPredictor(curr[i - bpp] & 0xff, prev[i] & 0xff, prev[i - bpp] & 0xff));
        }
    }
}


// a = left, b = above, c = upper left
int PulpCoreFont::paethPredictor(int a, int b, int c)
{
    // Initial estimate
    int p = a + b - c;

    // Distances to a, b, c
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);

    // Return nearest of a,b,c, breaking ties in order a,b,c.
    if (pa <= pb && pa <= pc) {
        return a;
    } else if (pb <= pc) {
        return b;
    } else {
        return c;
    }
}


void PulpCoreFont::premultiply(UINT* arbg, int argb_len)
{
    for (int i=0; i<argb_len; i++)
		arbg[i] = premultiply(arbg[i]);
}


int PulpCoreFont::premultiply(UINT arbg)
{
	int a = doubleRightShift(arbg, 24) & 0xFF;
	int r = doubleRightShift(arbg, 16) & 0xFF;
	int g = doubleRightShift(arbg, 8) & 0xFF;
    int b = arbg & 0xFF;

	r = (a * r + 127) / 255;
    g = (a * g + 127) / 255;
    b = (a * b + 127) / 255;

    return (a << 24) | (r << 16) | (g << 8) | b;
}


int PulpCoreFont::readInt()
{
	int retVal = (((0xFF&res_data[currPos])<<24)  | ((0xFF&res_data[currPos+1])<<16) | ((0xFF&res_data[currPos+2])<<8) | ((0xFF&res_data[currPos+3]))) ;
	currPos += 4;
	return retVal;
}

short PulpCoreFont::readShort()
{
	int retVal = (((0xFF&res_data[currPos])<<8)  | ((0xFF&res_data[currPos+1])));
	currPos += 2;
	return retVal;
}

char PulpCoreFont::readByte()
{
	return (0xFF&res_data[currPos++]);
}



void PulpCoreFont::drawString(HDC bufferDC, TCHAR* str, int xPos, int yPos)
{
	//Don't loop through null or zero-lengthed strings
	int numChars = lstrlen(str);
	if (str==NULL || numChars==0 || directPixels==NULL)
		return;


	//Loop through all letters...
	int nextIndex = getCharIndex(str[0]);
	int startX = xPos;
    for (int i=0; i<numChars; i++) {
		int index = nextIndex;
        int pos = charPositions[index];
        int charWidth = charPositions[index+1] - pos;

		//Draw this letter
		AlphaBlend(
			   bufferDC, startX, yPos, charWidth, height,   //Destination
			   directDC, pos, 0, charWidth, height,    //Source
			   blendFunc				   //Method
		);

		//Prepare next character.... if any
        if (i < numChars-1) {
            nextIndex = getCharIndex(str[i + 1]);
            int dx = charWidth + getKerning(index, nextIndex);
			startX += dx;
        }
    }
}


int PulpCoreFont::getCharIndex(TCHAR ch)
{
	//Special-case (not in WZ)
	if (uppercaseOnly && ch>='a' &&ch<= 'z')
		ch += 'A' - 'a';

	//Bound
	if (ch<firstChar || ch>lastChar)
		ch = lastChar;

	return ch - firstChar;
}


int PulpCoreFont::getKerning(int leftIndex, int rightIndex)
{
	// Future versions of this method might handle kerning pairs, like "WA" and "Yo"
	if (tracking!=0 && (shouldIgnoreTracking(rightIndex) || shouldIgnoreTracking(leftIndex)))
        return bearingRight[leftIndex] + bearingLeft[rightIndex];
	else
		return bearingRight[leftIndex] + tracking + bearingLeft[rightIndex];
}


bool PulpCoreFont::shouldIgnoreTracking(int index)
{
	int width = (charPositions[index+1] - charPositions[index]);
	int lsb = bearingLeft[index];
    int rsb = bearingRight[index];
    int advance = width + lsb + rsb;
    return advance < width/2;
}


int PulpCoreFont::getStringWidth(TCHAR* str)
{
	return getStringWidth(str, 0, lstrlen(str));
}


int PulpCoreFont::getStringWidth(TCHAR* str, int start, int end)
{
        if (end <= start) {
            return 0;
        }
        int stringWidth = 0;
        
        int lastIndex = -1;
        for (int i=start; i<end; i++) {
            int index = getCharIndex(str[i]);
            int charWidth = charPositions[index+1] - charPositions[index];
            
            if (lastIndex!=-1)
                stringWidth += getKerning(lastIndex, index);
            stringWidth += charWidth;
            lastIndex = index;
        }
        return stringWidth;
}


int PulpCoreFont::getHeight()
{
	return height;
}



/////////////////////
//Properties
/////////////////////
BOOL PulpCoreFont::isInError()
{
	return error;
}

TCHAR* PulpCoreFont::getErrorMsg()
{
	return errorMsg;
}

