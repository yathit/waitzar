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
 * "It is recommended that virtual function calls in constructors should be avoided for C++"
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
	//this->error = FALSE;
	//lstrcpy(this->errorMsg, _T(""));

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
	if (directBitmap==NULL)
		throw std::runtime_error("Couldn't create image bitmap.");

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
	//this->error = copyFrom->error;
	//lstrcpy(this->errorMsg, copyFrom->errorMsg);

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
	if (directBitmap==NULL)
		throw std::runtime_error("Couldn't create font bitmap.");

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

//Tint only part of the rectangle
void PulpCoreImage::tintSelf(UINT rgbColor, int sX, int sY, int w, int h)
{
	//Loop through all relevant pixels
	for (int y=sY; y<sY+h; y++) {
		for (int x=sX; x<sX+w; x++) {
			int i = y*width+x;
			directPixels[i] = premultiply((directPixels[i]&0xff000000)|(rgbColor&0x00ffffff));
		}
	}
}



//Draw a translucent pixel (from an anti-aliased line) on 
//  top of the image, blending it with the one underneath
/*void PulpCoreImage::drawAAPixel(int x, int y, unsigned int A, unsigned int RGB, float brightness)
{
	//Mirror vertical 
	int offset = (this->getHeight()-1-y)*this->getWidth() + x;

	//Some alpha stuff
	float newAlpha = (A/255.0F) * brightness;
	A =  (((int)(newAlpha*0xFF))&0xFF) << 24;
	int premultColor = premultiply(A|RGB);

	//Avoid memory errors (again~)
	if (offset>=0 && offset<this->getWidth()*this->getHeight())  {
		unsigned int oldARGB = directPixels[offset];
		//float oldAlpha = ((oldARGB>>24)&0xFF)/255.0F;
		unsigned int newRGB = RGB + (int)((1.0F - newAlpha)*(oldARGB&0xFFFFFF));
		//unsigned int newA = ((int)((newAlpha + oldAlpha * (1.0F - newAlpha))*0xFF))&0xFF;
		directPixels[offset] = 0xFF000000|newRGB; //Can only draw on opaque pixels, for now.
	}
}


int PulpCoreImage::roundPt5(float val)
{
	return (int)(val+0.5);
}

float PulpCoreImage::fractPart(float val)
{
	return val - ((int)val);
}

float PulpCoreImage::revfractPart(float val)
{
	return 1.0F - fractPart(val);
}


float PulpCoreImage::absFp(float val)
{
	return val>=0 ? val : -val;
}


//Draw an anti-aliased line
//Based on the Wikipedia article about Xiaolin Wu's algorithm
void PulpCoreImage::drawAALine(float startX, float startY, float endX, float endY, int ARGB)
{
	//Avoid corrupting memory
	if (startX < 0)
		startX = 0;
	if (startY < 0)
		startY = 0;
	if (endX >= this->getWidth())
		endX = (float)this->getWidth() - 1;
	if (endY >= this->getHeight())
		endY = (float)this->getHeight() - 1;

	//Separate components
	unsigned int A = ((ARGB>>24)&0xFF);
	unsigned int RGB = (ARGB&0xFFFFFF);

	//Compute deltas
	float deltaX = endX - startX;
	float deltaY = endY - startY;

	//Swap values so that this line extends horizontally
	if (absFp(deltaX) < absFp(deltaY)) {
		float temp = startX;
		startX = startY;
		startY = temp;
		temp = endX;
		endX = endY;
		endY = temp;
		temp = deltaX;
		deltaX = deltaY;
		deltaY = temp;
	}

	//Swap values if line extends leftwards
	if (endX < startX) {
		float temp = startX;
		startX = endX;
		endX = temp;
		temp = startY;
		startY = endY;
		endY = temp;
	}

	//Compute the gradient component
	float gradient = ((float)deltaY)/deltaX;

	//Paint the first endpoint
	int xpxl1 = 0;
	float intery = 0.0;
	{
		int xEnd = roundPt5(startX);
		float yEnd = startY + gradient * (xEnd - startX);
		float xGap = revfractPart(startX + 0.5F);
		xpxl1 = xEnd;
		int ypxl1 = (int)yEnd;
		drawAAPixel(xpxl1, ypxl1, A, RGB, revfractPart(yEnd)*xGap);
		drawAAPixel(xpxl1, ypxl1+1, A, RGB, fractPart(yEnd)*xGap);
		intery = yEnd + gradient;
	}

	//Paint the second end point
	int xpxl2 = 0;
	{
		int xEnd = roundPt5(endX);
		float yEnd = endY + gradient * (xEnd - endX);
		float xGap = fractPart(endX + 0.5F);
		xpxl2 = xEnd;
		int ypxl2 = (int)yEnd;
		drawAAPixel(xpxl2, ypxl2, A, RGB, revfractPart(yEnd)*xGap);
		drawAAPixel(xpxl2, ypxl2+1, A, RGB, fractPart(yEnd)*xGap);
	}

	//Paint all the points in the middle
	for (int x=xpxl1+1; x<xpxl2; x++) {
		drawAAPixel(x, (int)intery, A, RGB, revfractPart(intery));
		drawAAPixel(x, ((int)intery)+1, A, RGB, fractPart(intery));
		intery = intery + gradient;
	}
}*/



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


//Hackish, but obvious
void PulpCoreImage::flipSelfVertical()
{
	int yMax = this->getHeight()/2 + this->getHeight()%2;
	for (int y=0; y<yMax; y++) {
		for (int x=0; x<this->getWidth(); x++) {
			int src  = y*this->getWidth()+x;
			int dest = (this->getHeight()-1-y)*this->getWidth()+x;

			//Swap
			UINT temp = directPixels[src];
			directPixels[src] = directPixels[dest];
			directPixels[dest] = temp;
		}
	}
}


//Same, same
void PulpCoreImage::sepiaizeSelf()
{
	for (int y=0; y<this->getHeight(); y++) {
		for (int x=0; x<this->getWidth(); x++) {
			int id  = y*this->getWidth()+x;

			//Get RGB, alpha.
			int a, r, g, b;
			unpremultiply(directPixels[id], a, r, g, b);

			//Compute "luminosity" (more accurate grayscale version). Apply to each pixel, bound.
			// Also, compute the "depth" based on luminosity; greater depth adds more sepia.
			unsigned int lum = (2126*r + 7152*g + 722*b)/10000;
			unsigned int depth = (lum * 20)/255 + 10;
			r = std::min<unsigned int>(255, lum+(depth*2));
			g = std::min<unsigned int>(255, lum+depth);
			b = lum;

			//Save final result
			directPixels[id] = premultiply(a, r, g, b);
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
 * Specific resource initializer
 */
void PulpCoreImage::init(char *data, DWORD size, HDC currDC)
{
	//Init
	//this->error = FALSE;
	//lstrcpy(this->errorMsg, _T(""));

	//Copy reference...
	this->res_data = data;
	this->res_size = size;

	//Loop through all bytes...
	currPos = 0;

	//Read the png signature
	for (int i=0; i<8; i++) {
		if (res_data[currPos++] != PNG_SIGNATURE[i]) {
			std::stringstream msg;
			msg <<"PNG_SIG[" <<i <<"] is " <<res_data[currPos-1] <<" not " <<PNG_SIGNATURE[i];
			throw std::runtime_error(msg.str().c_str());
		}
	}

	//Read remaining "chunks"
    int length;
    int chunkType;
	for (;;) {
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
	if (currPos != res_size) {
		std::stringstream msg;
		msg <<"Extraneous bytes: %l" <<(res_size-currPos);
		throw std::runtime_error(msg.str().c_str());
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




/**
 * Resource initializer
 */
void PulpCoreImage::init(HRSRC resource, HGLOBAL dataHandle, HDC currDC)
{
	//Get raw data
    res_data = (char*)LockResource(dataHandle);
    res_size = SizeofResource(NULL, resource);

	//Loop through and read this data
	this->init(res_data, res_size, currDC);

	//Unlock this resource for later use
	//Apparently not needed anymore? Odd...
	//UnLockResource(dataHandle);
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


//NOTE: We won't merge this with draw() yet, since too much stuff relies on the standard draw.
//      TODO: 1.9, just have draw() call this function with 0,0,0,0
void PulpCoreImage::draw(HDC bufferDC, int xPos, int yPos, unsigned int cropLeft, unsigned int cropRight, unsigned int cropTop, unsigned int cropBottom) 
{
	int newW = getWidth()-cropRight-cropLeft;
	int newH = getHeight()-cropBottom-cropTop;
	AlphaBlend(
		bufferDC, xPos, yPos, newW, newH,   //Destination
		directDC, cropLeft, cropTop, newW, newH,    //Source
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
		throw std::runtime_error("PNG header requires unsupported options");
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
	if (directBitmap==NULL) 
		throw std::runtime_error("Couldn't create font bitmap.");
}


void PulpCoreImage::readPalette(int length)
{
	//Palettes must be %3
	if (length%3!=0) {
		std::stringstream msg;
		msg <<"Palette length " <<length <<" is not mod 3";
		throw std::runtime_error(msg.str().c_str());
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
	if ((palette == NULL || length > pal_length)) 
		throw std::runtime_error("Pallete is null or too short.");

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
		inflateFully(inflater, currScanline, bytesPerScanline);
        
		//Apply filter
		int filter = filterBuffer[0];
        if (filter > 0 && filter < 5)
			decodeFilter(currScanline, bytesPerScanline, prevScanline, filter, bytesPerPixel);
		else if (filter!=0) {
			std::stringstream msg;
			msg <<"Illegal filter type: " <<filter;
			throw std::runtime_error(msg.str().c_str());
        }

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
		if (inflater->needsInput())
			throw std::runtime_error("Inflater ran out of input");

		int res = inflater->inflate(result, res_length, bytesRead, res_length - bytesRead);
		if (res==-1)
			throw std::runtime_error("0 > off || off > off + len || off + len > buf_length");

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

	return premultiply(a, r, g, b);
}


int PulpCoreImage::premultiply(int a, int r, int g, int b)
{
	r = (a * r + 127) / 255;
    g = (a * g + 127) / 255;
    b = (a * b + 127) / 255;

    return (a << 24) | (r << 16) | (g << 8) | b;
}


void PulpCoreImage::unpremultiply(UINT arbg, int& a, int& r, int& g, int& b)
{
	a = doubleRightShift(arbg, 24) & 0xFF;
	r = doubleRightShift(arbg, 16) & 0xFF;
	g = doubleRightShift(arbg, 8) & 0xFF;
    b = arbg & 0xFF;

	//Undo~
	if (a!=0) {
		r = (255*r - 127) / a;
		g = (255*g - 127) / a;
		b = (255*b - 127) / a;
	}
}


int PulpCoreImage::readInt()
{
	unsigned int retVal = (((0xFF&res_data[currPos])<<24)  | ((0xFF&res_data[currPos+1])<<16) | ((0xFF&res_data[currPos+2])<<8) | ((0xFF&res_data[currPos+3]))) ;
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

unsigned int PulpCoreImage::readUnsignedInt()
{
	unsigned int retVal = (((0xFF&res_data[currPos])<<24)  | ((0xFF&res_data[currPos+1])<<16) | ((0xFF&res_data[currPos+2])<<8) | ((0xFF&res_data[currPos+3]))) ;
	currPos += 4;
	return retVal;
}

unsigned short PulpCoreImage::readUnsignedShort()
{
	int retVal = (((0xFF&res_data[currPos])<<8)  | ((0xFF&res_data[currPos+1])));
	currPos += 2;
	return retVal;
}

unsigned char PulpCoreImage::readUnsignedByte()
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
/*BOOL PulpCoreImage::isInError()
{
	return error;
}

TCHAR* PulpCoreImage::getErrorMsg()
{
	return errorMsg;
}*/








