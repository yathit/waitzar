#pragma once

#define _UNICODE
#define UNICODE

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#define MAGIC_NUMBER 0x70756c70666e740b

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
