#include ".\pulpcorefont.h"

PulpCoreFont::PulpCoreFont(HRSRC resource, HGLOBAL dataHandle)
{
	//Init
	this->error = FALSE;
	lstrcpy(this->errorMsg, _T(""));

	//Get raw data
    char* res_data = (char*)LockResource(dataHandle);
    DWORD res_size = SizeofResource(NULL, resource);

	//Loop through all bytes...
	DWORD currPos = 0;
	
	//Read the png signature
	for (int i=0; i<8; i++) {
		if (res_data[currPos++] != PNG_SIGNATURE[i] && error==FALSE) {
			swprintf(errorMsg, _T("PNG_SIG[%i] is %02X not %02X"), i, res_data[currPos-1], PNG_SIGNATURE[i]);
			error = TRUE;
		}
	}

	//Read remaining data...
	while (currPos < res_size) {
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

