/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "WZFactory.h"

using std::map;
using std::wstring;


WZFactory::WZFactory(void)
{
}


WZFactory::~WZFactory(void)
{
}


InputMethod* WZFactory::makeInputMethod(wstring id, DummyInputMethod* candidate)
{
	InputMethod* res = NULL;

	//Check some required settings (display name and encoding are set elsewhere... maybe we should put this here?)
	if (candidate->options.count(L"type")==0)
		throw std::exception("Cannot construct input manager: no \"type\"");

	//First, the type is important
	//TODO: Add "sanitize_id" to values
	if (candidate->options[L"type"].get() == L"builtin") {
		//Built-in types are known entirely by our core code
		if (id==L"waitzar")
			res = new WaitZar();
		else if (id==L"mywin")
			res = new WaitZar(); //TODO: Change!
		else
			throw std::exception("Invalid \"builtin\" Input Manager.");
	} else {
		throw std::exception("Invalid \"type\" for Input Manager.");
	}

	//Return our resultant IM
	return res;
}


DisplayMethod* WZFactory::makeDisplayMethod(std::wstring id, TYPES type, std::map<std::wstring, Option<std::wstring> > settings)
{
	//For now
	return new PngFont();
}


Transformation* WZFactory::makeTransformation(std::wstring id, TYPES type, std::map<std::wstring, Option<std::wstring> > settings)
{
	//For now
	return new Zg2Uni();
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
