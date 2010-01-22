/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _WZFACTORY
#define _WZFACTORY

#include <map>
#include <string>

#include "Interfaces.h"
#include "Input/WaitZar.h"
#include "Display/PngFont.h"
#include "Transform/Zg2Uni.h"

/**
 * Implementation of our factory interface: make input/display managers and transformers on demand
 */
class WZFactory
{
public:
	WZFactory(void);
	~WZFactory(void);

	//Builders
	static InputMethod* makeInputMethod(std::wstring id, DummyInputMethod* candidate);
	static DisplayMethod* makeDisplayMethod(std::wstring id, TYPES type, std::map<std::wstring, Option<std::wstring> > settings);
	static Transformation* makeTransformation(std::wstring id, TYPES type, std::map<std::wstring, Option<std::wstring> > settings);
};


#endif //_WZFACTORY

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

