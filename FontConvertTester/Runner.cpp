
#define _UNICODE
#define UNICODE

#define UNICOD_BOM 0xFEFF

#include <stdio.h>
#include <tchar.h>
#include "..\\win32_source\\fontconv.h"

#include <windows.h>
#include <string>


bool LineToFile(FILE* f, const std::wstring& s)
{
    // write the string to the file

    size_t n = fwrite(s.c_str(), sizeof(wchar_t), s.size(), f);
            
    // write line break to the file

    //fputwc(L'\n', f);
            
    // return whether the write operation was successful

    return (n == s.size());
};



void main(int argc, char* argv[])
{
	//Open output files...
	FILE* outWinInnwa = fopen("words.wininnwa.txt", "wb");
	FILE* outUnicode = fopen("words.unicode.txt", "wb");

	/*FILE* f = fopen("file.txt", "wb");
	LineToFile(f, L"\u1000string 3");
	fclose(f);*/

	//Read the list of zawgyi-encoded words in our model...
	FILE* file = fopen("words.zawgyi.txt", "r");
	if (file != NULL && outWinInnwa!=NULL && outUnicode!=NULL) {
		//Get file size
		fseek (file, 0, SEEK_END);
		long fileSize = ftell(file);
		rewind(file);

		//Read it all into an array, close the file.
		char * buffer = (char*) malloc(sizeof(char)*fileSize);
		size_t buff_size = fread(buffer, 1, fileSize, file);
		fclose(file);
		if (buff_size==0) {
			return; //Empty file.
		}

		//Finally, convert this array to unicode
		TCHAR * uniBuffer;
		size_t numUniChars = MultiByteToWideChar(CP_UTF8, 0, buffer, (int)buff_size, NULL, 0);
		uniBuffer = (TCHAR*) malloc(sizeof(TCHAR)*numUniChars);
		if (!MultiByteToWideChar(CP_UTF8, 0, buffer, (int)buff_size, uniBuffer, (int)numUniChars)) {
			printf("Invalid UTF-8 characters!");
			return;
		}
		delete [] buffer;

		//Skip the BOM, if it exists
		size_t currPosition = 0;
		if (uniBuffer[currPosition] == UNICOD_BOM)
			currPosition++;

		//Read each line
		TCHAR* word = new TCHAR[150];
		TCHAR* temp = new TCHAR[150];
		TCHAR* nlStr = _T("\n");
		lstrcpy(word, _T(""));
		for (;currPosition<numUniChars;currPosition++) {
			//Read a line
			TCHAR currChar = uniBuffer[currPosition];
			if (currChar=='\n' || currPosition==numUniChars-1) {
				if (lstrlen(word)>0) {
					printf("Line: %i\n", lstrlen(word));

					convertFont(temp, word, Zawgyi_One, WinInnwa);
					LineToFile(outWinInnwa, temp);
					LineToFile(outWinInnwa, L"\r\n");

					convertFont(temp, word, Zawgyi_One, Myanmar3);
					LineToFile(outUnicode, temp);
					LineToFile(outUnicode, L"\r\n");

					lstrcpy(word, _T(""));
				}
			} else {
				//Else, append
				swprintf(temp, _T("%c"), currChar);
				lstrcat(word, temp);
			}
		}
		delete [] uniBuffer;
		
		fclose(outWinInnwa);
		fclose(outUnicode);

		printf("Done.\n");
	}
}
