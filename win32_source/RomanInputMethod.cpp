/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "RomanInputMethod.h"


void RomanInputMethod::init(WordBuilder* model, SentenceList* sentence)
{
	this->model = model;
	this->sentence = sentence;
}



void RomanInputMethod::handleEsc()
{
	//Escape out of the main window or the sentence, depending
	if (!mainWindow->isVisible()) {
		//Kill the entire sentence.
		sentence.clear();
		patSintIDModifier = 0;  //Note: this should def. be in some kind of sub-class.
		model.reset(true);
		turnOnControlkeys(false);

		mainWindow->showWindow(false);
		(sentenceWindow!=NULL) && sentenceWindow->showWindow(false);
	} else {
		patSintIDModifier = 0;
		model.reset(false);

		//Are we using advanced input?
		if (!typePhrases) {
			//Turn off control keys
			turnOnControlkeys(false);
			
			mainWindow->showWindow(false);
			(sentenceWindow!=NULL) && sentenceWindow->showWindow(false);
		} else {
			//Just hide the typing window for now.
			mainWindow->showWindow(false);

			if (sentence.size()==0) {
				//Kill the entire sentence.
				sentence.clear();

				mainWindow->showWindow(false);
				(sentenceWindow!=NULL) && sentenceWindow->showWindow(false);

				turnOnControlkeys(false);
			} else
				recalculate();
		}
	}
}



void RomanInputMethod::handleBackspace()
{
	if (!mainWindow->isVisible()) {
		//Delete the previous word
		if (sentence.deletePrev(model))
			recalculate();
		if (sentence.size()==0) {
			//Kill the entire sentence.
			sentence.clear();
			turnOnControlkeys(false);
			ShowBothWindows(SW_HIDE);
		}
	} else {
		if (model.backspace()) {
			//Truncate...
			currStr.erase(currStr.length()-1);
			recalculate();
		} else {
			//No more numerals.
			//if (typeBurmeseNumbers==FALSE)
			//	turnOnNumberkeys(FALSE);

			//Are we using advanced input?
			if (!typePhrases) {
				//Turn off control keys
				turnOnControlkeys(false);

				ShowBothWindows(SW_HIDE);
			} else {
				//Just hide the typing window for now.
				mainWindow->showWindow(false);
				//ShowMainWindow(SW_HIDE);

				if (sentence.size()==0) {
					//Kill the entire sentence.
					sentence.clear();
					turnOnControlkeys(false);

					sentenceWindow->showWindow(false);
					//ShowSubWindow(SW_HIDE);
				}
			}
		}
	}
}


void RomanInputMethod::handleDelete()
{
	if (!mainWindow->isVisible()) {
		//Delete the next word
		if (sentence.deleteNext())
			recalculate();
		if (sentence.size()==0) {
			//Kill the entire sentence.
			sentence.clear();
			turnOnControlkeys(false);
			ShowBothWindows(SW_HIDE);
		}
	}
}


void RomanInputMethod::handleRight()
{
	if (mainWindow->isVisible()) {
		//Move right/left within the current word.
		if (patSintIDModifier==-1) {
			patSintIDModifier = 0;
			recalculate();
		} else if (model.moveRight(1) == TRUE)
			recalculate();
	} else {
		//Move right/left within the current phrase.
		if (sentence.moveCursorRight(1, model))
			recalculate();
	}
}


void RomanInputMethod::handleLeft()
{
	if (mainWindow->isVisible()) {
		if (model.moveRight(-1) == TRUE)
			recalculate();
		else if (model.hasPostStr() && patSintIDModifier==0) {
			//Move left to our "pat-sint shortcut"
			patSintIDModifier = -1;
			recalculate();
		}
	} else {
		//Move right/left within the current phrase.
		if (sentence.moveCursorRight(-1, model))
			recalculate();
	}
}


//Ignore wParam
void RomanInputMethod::handleNumber(int numCode, WPARAM wParam)
{
	stopChar=0;
	if (numCode>-1 || wParam==HOTKEY_COMBINE || (wParam==HOTKEY_SHIFT_COMBINE&&mainWindow->isVisible())) {
		if (mainWindow->isVisible()) {
			//Convert 1..0 to 0..9
			if (--numCode<0)
				numCode = 9;

			//Mangle as usual...
			if (wParam==HOTKEY_COMBINE || wParam==HOTKEY_SHIFT_COMBINE) {
				numCode = -1;
				patSintIDModifier = -1;
			} else
				patSintIDModifier = 0;

			//The model is visible: select that word
			BOOL typed = selectWord(numCode, helpWindow->isVisible());
			if (typed==TRUE && typePhrases) {
				mainWindow->showWindow(false);
				//ShowMainWindow(SW_HIDE);

				currStr.clear();
				patSintIDModifier = 0;
				model.reset(false);
				recalculate();
			} else
				patSintIDModifier = 0;

			keyWasUsed = true;
		} else if (typeBurmeseNumbers) {
			if (!typePhrases) {
				sentence.clear();
				sentence.insert(numCode);
				typeCurrentPhrase();
			} else {
				//Just type that number directly.
				sentence.insert(numCode);
				sentence.moveCursorRight(0, true, model);

				//Is our window even visible?
				if (!sentenceWindow->isVisible()) {
					turnOnControlkeys(true);

					sentenceWindow->showWindow(true);
					//ShowSubWindow(SW_SHOW);
				}

				recalculate();
			}
		}
	}
}


void RomanInputMethod::handleStop(bool isFull)
{
	unsigned short stopChar = model.getStopCharacter(isFull);
	if (!mainWindow->isVisible()) {
		if (!sentenceWindow->isVisible()) {
			//This should be cleared already, but let's be safe...
			sentence.clear();
		}
		//Otherwise, we perform the normal "enter" routine.
		typeCurrentPhrase();
	}
}



void RomanInputMethod::handleCommit(bool strongCommit)
{
	if (mainWindow->isVisible()) {
		//The model is visible: select that word
		if (selectWord(-1, helpWindow->isVisible())==TRUE) {
			//Hide the main window
			mainWindow->showWindow(false);

			//Reset
			patSintIDModifier = 0;
			model.reset(false);
			typedRomanStr.str(L"");

			//Recalc
			recalculate();
		}
	} else {
		//A bit tricky here. If the cursor's at the end, we'll
		//  do HOTKEY_ENTER. But if not, we'll just advance the cursor.
		//Hopefully this won't confuse users so much.
		//Note: ENTER overrides this behavior.
		if (strongCommit) {
			//Type the entire sentence
			typeCurrentPhrase();
		} else {
			if (sentence.getCursorIndex()==-1 || sentence.getCursorIndex()<((int)sentence.size()-1)) {
				sentence.moveCursorRight(1, model);
				recalculate();
			} else {
				//Type the entire sentence
				typeCurrentPhrase();
			}
		}
	}
}





void RomanInputMethod::handleKeyPress(WPARAM wParam)
{
	//Handle regular letter-presses (as lowercase)
	//NOTE: ONLY handle letters
	int keyCode = (wParam >= HOTKEY_A && wParam <= HOTKEY_Z) ? (int)wParam+32 : (int)wParam;
	if (keyCode >= HOTKEY_A_LOW && keyCode <= HOTKEY_Z_LOW) {
		//Run this keypress into the model. Accomplish anything?
		if (!model.typeLetter(keyCode))
			return;

		//Reset pat-sint choice
		patSintIDModifier = 0;

		//Is this the first keypress of a romanized word? If so, the window is not visible...
		if (!mainWindow->isVisible()) {
			//Reset it...
			typedRomanStr.str(L"");

			//Optionally turn on numerals
			if (!numberKeysOn)
				turnOnNumberkeys(true);

			//Show it
			mainWindow->showWindow(true);

			//First word in a sentence?
			if (!sentenceWindow->isVisible()) {
				//Turn on control keys
				turnOnControlkeys(true);
				(sentenceWindow!=NULL) && sentenceWindow->showWindow(true);
			}
		}

		//Now, handle the keypress as per the usual...
		typedRomanStr <<(char)keyCode;
		recalculate();
	} else {
		//Check for system keys
		InputMethod::handleKeyPress(wParam);
	}
}



std::wstring RomanInputMethod::getTypedSentenceString()
{
	//TODO
}


std::wstring RomanInputMethod::getTypedCandidateString()
{
	//TODO
}


void RomanInputMethod::appendToSentence(wchar_t letter, int id)
{
	//Type it
	selectWord(newID, true;

	//We need to reset the trigrams here...
	sentence.updateTrigrams(model);
}




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
