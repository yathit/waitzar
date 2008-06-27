#include <stdio.h>

#include "..\\win32_source\\fontconv.h"
#include "..\\win32_source\\wordbuilder.h"


/**
 * This file demonstrates how to use Wait Zar technology in your own projects. 
 * In particular, we show how the WordBuilder class can be used to disambiguate words.
 * You will need the following to run this sample:
 *   1) Compile or link in fontmap.[cpp|h], fonconv.[cpp|h], lib.[cpp|h], regex.[cpp|h], and WordBuilder.[cpp|h]
 *   2) In WordBuilder.h, uncomment the line "#define __STDC_ISO_10646__  200104L"
 *   3) Have Myanmar.model and mywords.txt on hand (if you plan to use Wait Zar's current model). 
 *   4) Note that the WordBuilder constructor can also be called with detailed information about the internals
 *      of the model. This is a very advanced usage of WordBuilder; you should probably avoid it.
 */
void main(int argc, const char* argv[])
{
	//Create your object.
	WordBuilder *model = new WordBuilder(L"..\\win32_source\\Myanmar.model", L"..\\win32_source\\mywords.txt");


	printf( "Model Loaded\n" );
}
