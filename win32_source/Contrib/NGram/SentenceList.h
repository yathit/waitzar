/*
 * Copyright 2008 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _SENTENCELIST
#define _SENTENCELIST

//Necessary libraries
#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include <list>
#include <vector>


namespace waitzar 
{


/**
 * Intended to encapsulate the cursor & the list of possible words into a single class,
 *   and to fix the reverse-insertion error at the same time.
 */
template <class ModelType>
class SentenceList
{
public:
	//Constructor/destructor
	SentenceList();
	~SentenceList();

	//Extracted interface
	void clear();
	void insert(int val);
	int getCursorIndex() const;
	bool moveCursorRight(int amt, bool allowSameIndex, ModelType &model);
	bool moveCursorRight(int amt, ModelType &model);
	size_t size() const;
	bool deleteNext();
	bool deletePrev(ModelType &model);
	std::wstring getPrevTypedWord(ModelType &model, const std::vector<std::wstring>& userDefinedWords) const;

	//Iterating
	std::list<int>::const_iterator begin() const;
	std::list<int>::const_iterator end() const;

	//Consistency
	void updateTrigrams(ModelType &model);


private:
	//Main wrapped variables
	int cursorIndex;
	std::list<int> prevTypedWords;
	std::list<int>::iterator cursor;

	//Helper
	std::list<int>::iterator newCursor;
};




template <class ModelType>
SentenceList<ModelType>::SentenceList ()
{
	this->clear();
}

template <class ModelType>
SentenceList<ModelType>::~SentenceList()
{
}


template <class ModelType>
void SentenceList<ModelType>::clear()
{
	prevTypedWords.clear();
	cursor = prevTypedWords.begin();
	cursorIndex = -1;
}

template <class ModelType>
void SentenceList<ModelType>::insert(int val)
{
	prevTypedWords.insert(cursor, val);
	cursorIndex++;
}

template <class ModelType>
int SentenceList<ModelType>::getCursorIndex() const
{
	return cursorIndex;
}

template <class ModelType>
size_t SentenceList<ModelType>::size() const
{
	return prevTypedWords.size();
}

template <class ModelType>
bool SentenceList<ModelType>::moveCursorRight(int amt, bool allowSameIndex, ModelType &model)
{
	//Any words?
	if (prevTypedWords.size()==0)
		return false;

	//Are we in bounds?
	int newCursorIndex = cursorIndex + amt;
	newCursor = cursor;
	while (amt!=0) {
		if (amt>0 && newCursor!=prevTypedWords.end()) {
			newCursor++;
			amt--;
		} else if (amt<0 && newCursor!=prevTypedWords.begin()) {
			newCursor--;
			amt++;
		} else {
			//Ignore this update
			return false;
		}
	}

	//Did we make any change?
	if (newCursor==cursor && !allowSameIndex)
		return false;

	//Update our model
	cursor = newCursor;
	cursorIndex = newCursorIndex;

	//Set the trigram
	this->updateTrigrams(model);
	
	//Success indicator
	return true;
}


template <class ModelType>
bool SentenceList<ModelType>::moveCursorRight(int amt, ModelType &model)
{
	return this->moveCursorRight(amt, false, model);
}


template <class ModelType>
bool SentenceList<ModelType>::deleteNext()
{
	//No words?
	if (prevTypedWords.size()==0)
		return false;

	//At end?
	if (cursor == prevTypedWords.end())
		return false;

	//Ok, delete it. No need to advance the cursor
	cursor = prevTypedWords.erase(cursor);
	return true;
}


template <class ModelType>
bool SentenceList<ModelType>::deletePrev(ModelType &model)
{
	//No words?
	if (prevTypedWords.size()==0) 
		return false;

	//At beginning?
	if (cursor == prevTypedWords.begin())
		return false;

	//Ok, delete it, update the cursor.
	cursor--;
	cursorIndex--;
	cursor = prevTypedWords.erase(cursor);

	//Update the trigrams...
	this->updateTrigrams(model);
	return true;
}


template <class ModelType>
std::wstring SentenceList<ModelType>::getPrevTypedWord(ModelType &model, const std::vector<std::wstring>& userDefinedWords) const
{
	std::wstring prevWord = L"";
	std::list<int>::const_iterator it=this->begin();
	for (int currID=0; currID<=this->getCursorIndex(); currID++) {
		if (*it>=0)
			prevWord = model.getWordString(*it);
		else {
			int adjID = -1 - (*it);
			size_t sysSize = waitzar::WZSystemDefinedWords.size();
			if (adjID < (int)sysSize) {
				prevWord = std::wstring(1, waitzar::WZSystemDefinedWords[adjID]);
			} else {
				adjID -= sysSize;
				prevWord = userDefinedWords[adjID];
			}
		}
		it++;
	}
	return prevWord;
}



template <class ModelType>
void SentenceList<ModelType>::updateTrigrams(ModelType &model)
{
	std::vector<unsigned int> trigrams;
	std::list<int>::iterator considered = cursor;
	while (considered!=prevTypedWords.begin() && trigrams.size()<3) {
		considered--;

		if (*considered<0)
			break;

		trigrams.push_back(*considered);
	}
	model.insertTrigram(trigrams);
}




//Iterate
template <class ModelType>
std::list<int>::const_iterator SentenceList<ModelType>::begin() const
{
	return prevTypedWords.begin();
}
template <class ModelType>
std::list<int>::const_iterator SentenceList<ModelType>::end() const
{
	return prevTypedWords.end();
}




} //End waitzar namespace

#endif //_SENTENCELIST



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

