import os
import codecs
import json
import re

#This script parses the old "Myanmar.model" file and converts it to a new, shiny JSON formatted file. 
#  It outputs both a "terse" and a "readable" format.


#####
#Saving functionality
#####
def saveUtf8File(path, stream):
  f = codecs.open(path, encoding='utf-8', mode='w')
  f.write(stream)
  f.close()

def saveModels(jsonStruct):
  #Terse
  saveUtf8File('new_model_terse.txt', json.dumps(test, ensure_ascii=False, separators=(',',':')))
  
  #Pretty-print
  saveUtf8File('new_model_terse.txt', json.dumps(test, ensure_ascii=False, sort_keys=True, indent=2))
  
  
  
#####
#Reading the old file
#####
wordlist = []
old_nexuslist = []
old_prefixlist = []
def readOldFileDictionaryLine(line):
    words = line[line.find('[')+1:line.rfind(']')].split(',')
    for wordD in words:
    	wordS = u''
    	letters = wordD.split('-')
    	for letter in letters:
    		wordS += unichr(0x1000 + int(letter, 16))
		wordlist.append(wordS)
	
def readOldFileNexusLine(line):
	pass
	
def readOldFilePrefixLine(line):
	pass




#####
#Main
#####
f = open('old_model.txt', 'r')
mode = -1
modes = [readOldFileDictionaryLine, readOldFileNexusLine, readOldFilePrefixLine]
for line in f:
	if line.startswith('#'):
		mode += 1
	else:
		modes[mode](line)
f.close()
print 'Done'



