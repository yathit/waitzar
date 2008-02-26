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

#pragma once

#define _UNICODE
#define UNICODE

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

const char PULP_MAGICNUM[] = "pulpfnt\x0B";              //0x70756c70666e740b
const char PNG_SIGNATURE[] = "\x89PNG\x0D\x0A\x1A\x0A";  //0x89504e470d0a1a0a

class PulpCoreFont
{
public:
	PulpCoreFont(HRSRC resource, HGLOBAL dataHandle);

	BOOL isInError();
	TCHAR* getErrorMsg();

private:
	BOOL error;
	TCHAR errorMsg[50];
};
