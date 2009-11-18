/*
 * Copyright 2009 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

//Necessary libraries
#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include "WordBuilder.h"
#include "SentenceList.h"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
using namespace boost::python;


namespace waitzar
{


//Built-in integration with Boost.Python
// For now, call it "libwaitzar" so we don't have to rename the shared library
BOOST_PYTHON_MODULE(libwaitzar)
{
    //Overloaded functions
    std::wstring    (waitzar::WordBuilder::*wk1)(unsigned int)                            = &waitzar::WordBuilder::getWordKeyStrokes;
    std::wstring    (waitzar::WordBuilder::*wk2)(unsigned int, unsigned int)    = &waitzar::WordBuilder::getWordKeyStrokes;

    class_<waitzar::WordBuilder>("WordBuilder", init< const char*, std::vector<std::string> >())
        .def("typeLetter", &waitzar::WordBuilder::typeLetter)
        .def("reset", &waitzar::WordBuilder::reset)
	.def("getParenString", &waitzar::WordBuilder::getParenString)
	.def("getPossibleWords", &waitzar::WordBuilder::getPossibleWords)
	.def("getWordKeyStrokes", wk1)
	.def("getWordKeyStrokes", wk2)
	.def("getCurrSelectedID", &waitzar::WordBuilder::getCurrSelectedID)
    ;

    enum_<waitzar::ENCODING>("encoding")
        .value("unicode", ENCODING_UNICODE)
	.value("zawgyi", ENCODING_ZAWGYI)
	.value("wininnwa", ENCODING_WININNWA)
    ;
	
    //Vector of strings
    class_<std::vector<std::string> >("StringVec")
        .def(vector_indexing_suite<std::vector<std::string> >())
    ;
}


}
