/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "InputMethod.h"

InputMethod::InputMethod(MyWin32Window* mainWindow, MyWin32Window* sentenceWindow, MyWin32Window* helpWindow, MyWin32Window* memoryWindow)
{
	//Init
	isHelpInput = false;

	//Save
	this->mainWindow = mainWindow;
	this->sentenceWindow = sentenceWindow;
	this->helpWindow = helpWindow;
	this->memoryWindow = memoryWindow;
}

InputMethod::~InputMethod()
{
}


void InputMethod::treatAsHelpKeyboard(bool val)
{
	this->isHelpInput = val;
}



//Handle system keys
void InputMethod::handleKeyPress(WPARAM wParam)
{
	//Get an adjusted numcode.
	int base = (wParam>=HOTKEY_0 && wParam<=HOTKEY_9) ? HOTKEY_0 : (wParam>=HOTKEY_NUM0 && wParam<=HOTKEY_NUM9) ? HOTKEY_NUM0 : -1;
	int numCode = (base==-1) ? wParam : HOTKEY_0 + (int)wParam - base;

	//Check system keys
	if (!helpWindow->isVisible() && !mainWindow->isVisible() && !keyWasUsed) {
		int newID = -1;
		for (size_t i=0; i<systemWordLookup.size(); i++) {
			if (systemWordLookup[i].first==numCode) {
				newID = i;
				break;
			}
		}

		//Did we get anything?
		if (newID!=-1) {
			newID = -1-newID;

			//Try to type this word
			BOOL typed = selectWord(newID, true);
			if (typed==TRUE && typePhrases) {
				if (!sentenceWindow->isVisible()) {
					turnOnControlkeys(true);

					sentenceWindow->showWindow(true);
					//ShowSubWindow(SW_SHOW);
				}

				recalculate();
			}

			//We need to reset the trigrams here...
			sentence.updateTrigrams(model);
		}
	}
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
