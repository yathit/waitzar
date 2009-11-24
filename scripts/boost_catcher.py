#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import re

# This script takes as input a spirit-json project folder and a boost header folder. 
# It outputs a list of all boost files that are included, using a regex like "*boost/*.hpp" (even if defines would exclude it)
# It then copies these into a "boost" directory in the current folder, and outputs the size of such files
# It is to be used for integrating new versions of spirit-boost with new versions of boost.
# Note that, to save space, you might want to manually edit json_spirit_reader.cpp manually remove 
#    the test for #if BOOST_VERSION >= 103800  ---we are always going to use Boost 1.40 or greater

#Our matcher 
boostlib_regex = '#include.*[<>"](boost/.*\.hpp)[<>"]'

#Prompt the user
json_spirit_path = "D:\\Open Source Projects\\json_spirit_v4.02\\json_spirit"
print 'Please enter the location of json-spirit:'
print '  [Enter for default: ', json_spirit_path, ']'
var = raw_input()
if var:
    json_spirit_path = var

#Prompt the user again
boost_path = "D:\\Programs\\boost\\boost_1_40"
print 'Please enter the location of boost:'
print '  [Enter for default: ', boost_path, ']'
var = raw_input()
if var:
    boost_path = var

#Ok, begin. First, read all *.cpp and *.h files from json into an array.
not_yet_searched = set([])
for parent,subdirs,files in os.walk(json_spirit_path): 
    for file in files:
        if file.endswith('.cpp') or file.endswith('.h'):
            not_yet_searched.add(os.path.join(parent, file))

#Step 2: Until not_yet_searched is empty, read all files and add any boost includes
#           to the list of files to scan. Avoid duplicates, of course.
already_searched = set([])
boost_libs = set([])
while not_yet_searched:
    #Tag all entries, reset
    already_searched |= not_yet_searched
    to_search = set(not_yet_searched)
    not_yet_searched = set([])

    #Now, loop through each file that's left, adding dependencies as you go
    for path in to_search:
        #Open, scan each line for unmet dependencies
        file = open(path, 'r')
        for line in file:
            m = re.search(boostlib_regex, line)
            if m:
                newfile = os.path.join(boost_path, m.group(1))
                if not newfile in already_searched:
                    not_yet_searched.add(newfile)
                    boost_libs.add(newfile)
        file.close()


#Step 3: list these files and their sizes
print 'The following needed files are part of boost:'
total_size = 0
to_print_boost = [(path.replace(boost_path, '', 1), os.path.getsize(path))  for path in boost_libs]
path_buffer = len(reduce(lambda x,y:x if len(x[0])>len(y[0]) else y, to_print_boost) [0]) + 5
for item in to_print_boost:
    total_size += item[1]
    print item[0].ljust(path_buffer, '.'), str(item[1]/1024).rjust(4), 'kb'
print 'Total files: ' , len(boost_libs), ' adding up to ', total_size/1024, 'kb'

print 'Done'




