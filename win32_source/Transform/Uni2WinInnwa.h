/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#ifndef _TRANSFORM_UNI2WININNWA
#define _TRANSFORM_UNI2WININNWA

#include "Transform/Transformation.h"
#include "NGram/wz_utilities.h"
#include "Burglish/fontmap.h"
#include "Burglish/fontconv.h"

/**
 * Placeholder class: right now, it just combines two existing transformations
 */
class Uni2WinInnwa : public Transformation
{
public:
	Uni2WinInnwa();

	//Convert
	void convertInPlace(std::wstring& src) const;
};


#endif //_TRANSFORM_UNI2WININNWA

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

