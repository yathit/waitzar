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

#include ".\pulpcoreimage.h"


/** 
 * From Wikipedia:
 * "While it is recommended that virtual function calls in constructors should be avoided for C++"
 */
PulpCoreImage::PulpCoreImage()
{
}


/**
 * Create a blank image. Pass in an un-initialized HDC and HBITMAP to initalize them
 */
void PulpCoreImage::init(int width, int height, int bkgrdARGB, HDC currDC, HDC &thisDC, HBITMAP &thisBmp)
{
	//Init
	this->error = FALSE;
	lstrcpy(this->errorMsg, _T(""));

	//Init all relevant fields
	/*this->bitDepth = copyFrom->bitDepth;
	this->colorType = copyFrom->colorType;
	this->isOpaque = copyFrom->isOpaque;
	this->hotspotX = copyFrom->hotspotX;
	this->hotspotY = copyFrom->hotspotY;
	this->pal_length = copyFrom->pal_length;
	this->palette = copyFrom->palette;*/
	this->width = width;
	this->height = height;
	this->hotspotX = 0;
	this->hotspotY = 0;

	//Initialize our bitmap-related data structures
	initBmpInfo();

	//Create the DIB to copy pixels onto
	directDC = thisDC = CreateCompatibleDC(currDC);
	directBitmap = thisBmp = CreateDIBSection(directDC, &bmpInfo,  DIB_RGB_COLORS, (void**) &directPixels, NULL, 0);
	SelectObject(directDC, directBitmap);
	if (directBitmap==NULL && error==FALSE) {
		lstrcpy(errorMsg, _T("Couldn't create image bitmap."));
		error = TRUE;
		return;
	}

	//Init with background color
	for (int i=0; i<width*height; i++)  {
		this->directPixels[i] = premultiply(bkgrdARGB);
	}
}



/**
 * Copy initializer.
 */
void PulpCoreImage::init(PulpCoreImage *copyFrom, HDC currDC)
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
	this->pal_length = copyFrom->pal_length;

	//We're probably safe passing a reference.
	this->palette = copyFrom->palette;

	//Initialize our bitmap-related data structures
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


void PulpCoreImage::initBmpInfo() 
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
	blendFunc.SourceConstantAlpha = 0xFF; //Blend at the pixel level only.
	blendFunc.AlphaFormat = AC_SRC_ALPHA;
}


/**
 * Tint all pixels of this font to a certain color; preserve alpha.
 *  NOTE: Unlike in PulpCore, the proper way to make a duplicate font
 *   of a tinted color is to call the copy constructor FIRST, and then
 *   this method on the newly-created font.
 */
void PulpCoreImage::tintSelf(UINT rgbColor)
{
	for (int i=0; i<width*height; i++) {
		directPixels[i] = premultiply((directPixels[i]&0xff000000)|(rgbColor&0x00ffffff));
	} 
}


/**
 * This function is a bit of a hack, since it accesses the image's pixels directly.
 * However, we have no choice, since layered windows require premultiplied alphas.
 */ 
void PulpCoreImage::fillRectangle(int startX, int startY, int width, int height, int ARGB)
{
	//Avoid corrupting memory
	if (startX < 0)
		startX = 0;
	if (startY < 0)
		startY = 0;
	if (width > this->getWidth())
		width = this->getWidth();
	if (height > this->getHeight())
		height = this->getHeight();

	//Convert our color
	int premultColor = premultiply(ARGB);
	for (int y=startY; y<startY+height; y++) {
		for (int x=startX; x<startX+width; x++) {
			//Mirror vertical 
			int offset = (this->getHeight()-1-y)*this->getWidth() + x;

			//Avoid memory errors (again~)
			if (offset>=0 && offset<this->getWidth()*this->getHeight()) 
				directPixels[offset] = premultColor;
		}
	}
}


/**
 * Also slightly hackish. Used to rotate an image by 90 degrees. This is used in the "corner" image file,
 *  to save a tiny amount of space and (more importantly) for consistency.
 */
void PulpCoreImage::rotateSelf90DegreesClockwise()
{
	//Right now, we'll only allow rotation of squre images
	if (this->getWidth() != this->getHeight())
		return;

	//Make a new array, copy each row into its appropriate column
	UINT *newDirect = new UINT[this->getWidth()*this->getHeight()];
	for (int y=0; y<this->getHeight(); y++) {
		for (int x=0; x<this->getWidth(); x++) {
			//Let's not corrupt anything, hmm?
			int destX = (this->getHeight()-1-x)*this->getWidth()+y;
			int srcX = y*this->getWidth()+x;

			//Set it (flip the y axis, of course)
			if (destX>=0 && destX<this->getWidth()*this->getHeight() && srcX>=0 && srcX<this->getWidth()*this->getHeight())
				newDirect[destX] = directPixels[srcX];
		}
	}

	//Sadly, we cannot access directPixels to delete it, so we have to re-copy
	for (int y=0; y<this->getHeight(); y++) {
		for (int x=0; x<this->getWidth(); x++) {
			int dX = y*this->getWidth()+x;
			directPixels[dX] = newDirect[dX];
		}
	}
	delete [] newDirect;
}



/**
 * Resource initializer
 */
void PulpCoreImage::init(HRSRC resource, HGLOBAL dataHandle, HDC currDC)
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

		//Delegate
		this->readChunk(chunkType, length, currDC); 

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


void PulpCoreImage::readChunk(int chunkType, int length, HDC currDC)
{
	//Switch on chunk type:
	if (chunkType == CHUNK_IHDR)
		readHeader(currDC);
	else if (chunkType == CHUNK_HOTS) {
		this->hotspotX = readInt();
		this->hotspotY = readInt();
	} else if (chunkType == CHUNK_PLTE)
        readPalette(length);
    else if (chunkType == CHUNK_TRNS)
        readTransparency(length);
    else if (chunkType == CHUNK_IDAT)
        readData(length);
    else if (chunkType == CHUNK_ANIM)
        readAnimation();
	else if (chunkType != CHUNK_IEND)
		currPos += length;  //Ignore this chunk
}



void PulpCoreImage::draw(HDC bufferDC, int xPos, int yPos) 
{
	AlphaBlend(
		bufferDC, xPos, yPos, getWidth(), getHeight(),   //Destination
		directDC, 0, 0, getWidth(), getHeight(),    //Source
		blendFunc				   //Method
	);
}



void PulpCoreImage::readHeader(HDC currDC)
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


void PulpCoreImage::readPalette(int length)
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


void PulpCoreImage::readTransparency(int length)
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


void PulpCoreImage::readAnimation()
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


void PulpCoreImage::readData(int length)
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


void PulpCoreImage::inflateFully(Inflater* inflater, char* result, int res_length)
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


void PulpCoreImage::decodeFilter(char* curr, int curr_len, char* prev, int filter, int bpp)
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
int PulpCoreImage::paethPredictor(int a, int b, int c)
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


void PulpCoreImage::premultiply(UINT* arbg, int argb_len)
{
    for (int i=0; i<argb_len; i++)
		arbg[i] = premultiply(arbg[i]);
}


int PulpCoreImage::premultiply(UINT arbg)
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


int PulpCoreImage::readInt()
{
	int retVal = (((0xFF&res_data[currPos])<<24)  | ((0xFF&res_data[currPos+1])<<16) | ((0xFF&res_data[currPos+2])<<8) | ((0xFF&res_data[currPos+3]))) ;
	currPos += 4;
	return retVal;
}

short PulpCoreImage::readShort()
{
	int retVal = (((0xFF&res_data[currPos])<<8)  | ((0xFF&res_data[currPos+1])));
	currPos += 2;
	return retVal;
}

char PulpCoreImage::readByte()
{
	return (0xFF&res_data[currPos++]);
}

int PulpCoreImage::getWidth()
{
	return width;
}

int PulpCoreImage::getHeight()
{
	return height;
}

/////////////////////
//Properties
/////////////////////
BOOL PulpCoreImage::isInError()
{
	return error;
}

TCHAR* PulpCoreImage::getErrorMsg()
{
	return errorMsg;
}








