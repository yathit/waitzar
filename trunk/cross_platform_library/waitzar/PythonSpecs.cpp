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

#include <boost/python.hpp>
using namespace boost::python;


//Built-in integration with Boost.Python
// For now, call it "libwaitzar" so we don't have to rename the shared library
BOOST_PYTHON_MODULE(libwaitzar)
{
    class_<waitzar::WordBuilder>("WordBuilder", init< const char*, std::vector<std::string> >())
        .def("typeLetter", &waitzar::WordBuilder::typeLetter)
        .def("reset", &waitzar::WordBuilder::reset)
    ;
}

