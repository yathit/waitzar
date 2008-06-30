#include <stdio.h>

#include "../win32_source/fontconv.h"
#include "../win32_source/WordBuilder.h"


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
int main(int argc, const char* argv[])
{
	//Switch to wide-character mode
	if (fwide(stdout, 0) == 0) {
		if (fwide(stdout, 1) <= 0) {
			printf("Could not switch to wide char mode!\n");
			return 0;
		} else {
			wprintf(L"Switched to wide mode.\n");
		}
	}
	
	//Create your model as an object.
	//NOTE: You should not mix printf() and wprintf() (it might behave unexpectedly). So, for unicode programs, always use wprintf().
	WordBuilder *model = new WordBuilder("../win32_source/Myanmar.model", "../win32_source/mywords.txt");
	wprintf(L"Model loaded correctly.\n\n");


	//////////////////////////////////////////////////////////////////////
	//Use case 1: List all Burmese words for "kote"
	//////////////////////////////////////////////////////////////////////
	
	//First, we reset the model; this should occur before the first keypress of every NEW word.
	//  Note that "true" will reset the trigrams as well, "false" just resets everything else.
	model->reset(true);
	
	//Now, we must type each letter individually. Since we know that "kote" is in our dictionary, 
	//  we just call typeLetter() four times. If we are not sure if the word exists, we should check
	//  the return value of typeLetter() each time we call it. 
	model->typeLetter('k'); //Transition -->'k'
	model->typeLetter('o'); //Transition (k) --> 'o'
	model->typeLetter('t'); //Transition (k-->o) --> 't'
	model->typeLetter('e'); //Transition (k-->o-->t) --> 'e'
	
	//We have arrived at "kote", so we now just list all possible values.
	wchar_t* currWord;
	std::vector<unsigned int> possWords = model->getPossibleWords();
	wprintf(L"\"kote\" can be one of the following %i words \n", possWords.size());
	for (int i=0; i<possWords.size(); i++) {
		currWord = model->getWordString(possWords[i]);
		
		for (int x=0; x<wcslen(currWord); x++) {
			wprintf(L" %x", currWord[x]);
		}
		
		
		wprintf(L"  (%i)\n", wcslen(currWord));
	}
	wprintf(L"\n");
	



	wprintf(L"Done\n" );
	return 0;
}
