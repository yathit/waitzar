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
