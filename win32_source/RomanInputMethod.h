/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _ROMAN_INPUT_METHOD
#define _ROMAN_INPUT_METHOD

#include "InputMethod.h"
#include "NGram/WordBuilder.h"
#include "NGram/SentenceList.h"

class RomanInputMethod : public InputMethod {

public:
	//Needed to add Roman-specific stuff
	void init(WordBuilder* model, SentenceList* sentence);

	//Abstract implementation - keypresses
	void handleEsc();
	void handleBackspace();
	void handleDelete();
	void handleRight();
	void handleLeft();
	void handleEnter();
	void handleSpace();
	void handleNumber(int numCode, WPARAM wParam);
	void handleStop(bool isFull);
	void handleKeyPress(WPARAM wParam);


	//Abstract implementation - simple
	bool isPlaceholder() { return false; }


private:
	//Romanization model
	WordBuilder* model;
	SentenceList* sentence;

	//For now, we track the shortcut pat-sint keys directly. Later, we'll integrate this into the model (if people like it)
	int patSintIDModifier = 0;
}


#endif //_ROMAN_INPUT_METHOD

/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

