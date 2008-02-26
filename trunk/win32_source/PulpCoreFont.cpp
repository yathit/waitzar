#include ".\pulpcorefont.h"

PulpCoreFont::PulpCoreFont(HRSRC resource, HGLOBAL dataHandle)
{
	//Init
	this->error = FALSE;

	//Get raw data
    char* res_data = (char*)LockResource(dataHandle);
    DWORD res_size = SizeofResource(NULL, resource);

	//Loop through all bytes...
	DWORD currPos = 0;
	while (currPos < res_size) {
		if (currPos==0 && (res_data[currPos]&0xFF) != 0x89) {
			swprintf(errorMsg, _T("First Int: %i"), (res_data[currPos]&0xFF));
			error = TRUE;
		}

		//Increment
		currPos++;
	}

}

BOOL PulpCoreFont::isInError() 
{
	return error;
}

TCHAR* PulpCoreFont::getErrorMsg() 
{
	return errorMsg;
}

