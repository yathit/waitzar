/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "WZFactory.h"

using std::map;
using std::wstring;
using std::vector;
using std::pair;


WZFactory::WZFactory(void)
{
}


WZFactory::~WZFactory(void)
{
}

InputMethod* WZFactory::makeInputMethod(const std::wstring& id, const std::map<std::wstring, std::wstring>& options)
{
	InputMethod* res = NULL;

	//Check some required settings 
	if (options.count(L"type")==0)
		throw std::exception("Cannot construct input manager: no \"type\"");
	if (options.count(L"encoding")==0)
		throw std::exception("Cannot construct input manager: no \"encoding\"");
	if (options.count(sanitize_id(L"display-name"))==0)
		throw std::exception("Cannot construct input manager: no \"display-name\"");

	//First, generate an actual object, based on the type.
	if (sanitize_id(options.find(L"type")->second) == L"builtin") {
		//Built-in types are known entirely by our core code
		vector< pair<int, unsigned short> > temp; //TODO: Remove
		if (id==L"waitzar")
			res = new WaitZar(temp);
		else if (id==L"mywin")
			res = new WaitZar(temp); //TODO: Change!
		else
			throw std::exception(ConfigManager::glue(L"Invalid \"builtin\" Input Manager: ", id).c_str());
		res->type = BUILTIN;
	} else {
		throw std::exception(ConfigManager::glue(L"Invalid type (",options.find(L"type")->second, L") for Input Manager: ", id).c_str());
	}

	//Now, add general settings
	res->id = id;
	res->displayName = options.find(sanitize_id(L"display-name"))->second;
	res->encoding.id = sanitize_id(options.find(L"encoding")->second);

	//Return our resultant IM
	return res;
}


DisplayMethod* WZFactory::makeDisplayMethod(const std::wstring& id, const std::map<std::wstring, std::wstring>& options)
{
	DisplayMethod* res = NULL;

	//Check some required settings 
	if (options.count(L"type")==0)
		throw std::exception("Cannot construct display method: no \"type\"");
	if (options.count(L"encoding")==0)
		throw std::exception("Cannot construct display method: no \"encoding\"");

	//First, generate an actual object, based on the type.
	if (sanitize_id(options.find(L"type")->second) == L"builtin") {
		//Built-in types are known entirely by our core code
		if (id==L"zawgyibmp")
			res = new PngFont();
		else
			throw std::exception(ConfigManager::glue(L"Invalid \"builtin\" Display Method: ", id).c_str());
		res->type = BUILTIN;
	} else {
		throw std::exception(ConfigManager::glue(L"Invalid type (",options.find(L"type")->second, L") for Display Method: ", id).c_str());
	}

	//Now, add general settings
	res->id = id;
	res->encoding.id = sanitize_id(options.find(L"encoding")->second);

	//Return our resultant DM
	return res;
}


Transformation* WZFactory::makeTransformation(const std::wstring& id, const std::map<std::wstring, std::wstring>& options)
{
	Transformation* res = NULL;

	//Check some required settings 
	if (options.count(L"type")==0)
		throw std::exception("Cannot construct transformation: no \"type\"");
	if (options.count(sanitize_id(L"from-encoding"))==0)
		throw std::exception("Cannot construct transformation: no \"from-encoding\"");
	if (options.count(sanitize_id(L"to-encoding"))==0)
		throw std::exception("Cannot construct transformation: no \"to-encoding\"");

	//First, generate an actual object, based on the type.
	if (sanitize_id(options.find(L"type")->second) == L"builtin") {
		//Built-in types are known entirely by our core code
		if (id==L"uni2zg")
			res = new Uni2Uni();   //TODO: Implement
		else if (id==L"uni2wi")
			res = new Uni2Uni();   //TODO: Implement
		else if (id==L"zg2uni")
			res = new Zg2Uni();
		else
			throw std::exception(ConfigManager::glue(L"Invalid \"builtin\" Transformation: ", id).c_str());
		res->type = BUILTIN;
	} else {
		throw std::exception(ConfigManager::glue(L"Invalid type (",options.find(L"type")->second, L") for Transformation: ", id).c_str());
	}

	//Now, add general settings
	res->id = id;
	res->fromEncoding.id = sanitize_id(options.find(sanitize_id(L"from-encoding"))->second);
	res->toEncoding.id = sanitize_id(options.find(sanitize_id(L"to-encoding"))->second);

	//Optional settings
	res->hasPriority = false;
	if (options.count(sanitize_id(L"has-priority"))>0)
		res->hasPriority = read_bool(options.find(sanitize_id(L"has-priority"))->second);

	//Return our resultant Transformation
	return res;
}

Encoding WZFactory::makeEncoding(const std::wstring& id, const std::map<std::wstring, std::wstring>& options)
{
	Encoding res;

	//Check some required settings 
	if (options.count(sanitize_id(L"display-name"))==0)
		throw std::exception("Cannot construct encoding: no \"display-name\"");

	//General Settings
	res.id = id;
	res.displayName = options.find(sanitize_id(L"display-name"))->second;

	//Optional settings
	res.canUseAsOutput = false;
	if (options.count(L"image")>0)
		res.imagePath = options.find(L"image")->second;
	if (options.count(sanitize_id(L"use-as-output"))>0)
		res.canUseAsOutput = read_bool(options.find(sanitize_id(L"use-as-output"))->second);
	if (options.count(L"initial")>0)
		res.initial = options.find(L"initial")->second;

	//Return our resultant DM
	return res;
}

//Move these later
std::wstring WZFactory::sanitize_id(const std::wstring& str)
{
	return ConfigManager::sanitize_id(str);
}
bool WZFactory::read_bool(const std::wstring& str)
{
	return ConfigManager::read_bool(str);
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
