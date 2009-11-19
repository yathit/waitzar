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
    //Overloaded functions: getWordKeyStrokes
    std::wstring    (waitzar::WordBuilder::*wk1)(unsigned int)                            = &waitzar::WordBuilder::getWordKeyStrokes;
    std::wstring    (waitzar::WordBuilder::*wk2)(unsigned int, unsigned int)    = &waitzar::WordBuilder::getWordKeyStrokes;

    //Overloaded functions: moveCursorRight
    bool    (waitzar::SentenceList::*cr1)(int, bool, WordBuilder&)   = &waitzar::SentenceList::moveCursorRight;
    bool    (waitzar::SentenceList::*cr2)(int, WordBuilder&)             = &waitzar::SentenceList::moveCursorRight;

    class_<waitzar::WordBuilder>("WordBuilder", init< const char*, std::vector<std::string> >())
        .def("typeLetter", &waitzar::WordBuilder::typeLetter)
        .def("reset", &waitzar::WordBuilder::reset)
	.def("getParenString", &waitzar::WordBuilder::getParenString)
	.def("getPossibleWords", &waitzar::WordBuilder::getPossibleWords)
	.def("getWordKeyStrokes", wk1)
	.def("getWordKeyStrokes", wk2)
	.def("getCurrSelectedID", &waitzar::WordBuilder::getCurrSelectedID)
	.def("typeSpace", &waitzar::WordBuilder::typeSpace)
    ;
	
    class_<waitzar::SentenceList>("SentenceList")
        .def("clear", &waitzar::SentenceList::clear)
        .def("insert", &waitzar::SentenceList::insert)
	.def("getCursorIndex", &waitzar::SentenceList::getCursorIndex)
	.def("moveCursorRight",cr1)
	.def("moveCursorRight", cr2)
	.def("size", &waitzar::SentenceList::size)
	.def("deleteNext", &waitzar::SentenceList::deleteNext)
	.def("deletePrev", &waitzar::SentenceList::deletePrev)

        //Iterators play differently in C++ and Python
        .property("words", range(&waitzar::SentenceList::begin, &waitzar::SentenceList::end))

        //Sometimes needed for consistency
	.def("updateTrigrams", &waitzar::SentenceList::updateTrigrams)
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

    //Vector of unsigned integers
    class_<std::vector<unsigned int> >("UIntVec")
        .def(vector_indexing_suite<std::vector<unsigned int> >())
    ;

    //Pair of bool, unsigned int
    typedef std::pair<bool, unsigned int> BoolUintPair;
    to_python_converter< BoolUintPair, PairToTupleConverter<bool, unsigned int> >();
    //class_<std::pair<bool, unsigned int> >("BooUIntPt")
    //    .def(vector_indexing_suite<std::pair<bool, unsigned int> >())
    //;
}


}
