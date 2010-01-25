/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _TRANSFORM_ZG2UNI
#define _TRANSFORM_ZG2UNI

#include "Settings/Interfaces.h"

/**
 * Placeholder class: to be used for Zg2Uni Conversion method
 */
class Zg2Uni : public Transformation
{
public:
	Zg2Uni();

	//Convert
	const std::wstring& convert(const std::wstring& src);
};


#endif //_TRANSFORM_ZG2UNI

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

