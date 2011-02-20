import os
import codecs
import json
import re

#This script parses the old "Myanmar.model" file and converts it to a new, shiny JSON formatted file. 
#  It outputs both a "terse" and a "readable" format.


#####
#Saving functionality
#####
BOM = u'\uFEFF'  #ugh....
def saveUtf8File(path, stream):
	f = codecs.open(path, encoding='utf-8', mode='w')
	f.write(BOM)
	f.write(stream)
	f.close()

def saveModels(jsonStruct):
	#Terse
	saveUtf8File('new_model_terse.txt', json.dumps(jsonStruct, ensure_ascii=False, separators=(',',':')))
  
	#Pretty-print
	saveUtf8File('new_model_spaced.txt', json.dumps(jsonStruct, ensure_ascii=False, sort_keys=True, indent=2))
  
  
  
#####
#Reading the old file
#####
wordlist = []
shortcuts = {}
old_nexuslist = []
old_prefixlookup = []
old_prefixresults = []
def readOldFileDictionaryLine(line):
	words = line[line.find('[')+1:line.rfind(']')].split(',')
	for wordD in words:
		wordS = u''
		letters = wordD.split('-')    	
		for letter in letters:
			wordS += unichr(0x1000 + int(letter, 16))    		
		wordlist.append(wordS)

	
def readOldFileNexusLine(line):
	res = {}
	entries = line[line.find('{')+1:line.rfind('}')].split(',')
	for entry in entries:
		halves = entry.split(':')
		res[halves[0]] = int(halves[1])
	old_nexuslist.append(res)
	
def readOldFilePrefixLine(line):
	entries = line[line.find('{')+1:line.rfind('}')].split(',')
	results = line[line.find('[')+1:line.rfind(']')].split(',')
	res = {}
	for entry in entries:
		if len(entry)==0:
			continue
		halves = entry.split(':')
		res[int(halves[0])] = int(halves[1])
	old_prefixlookup.append(res)
	res = []
	for entry in results:
		res.append(int(entry))
	old_prefixresults.append(res)
	
def readOldFilePatSintLine(line):
	#x + y = z
	x = line[:line.find('+')].strip()
	y = line[line.find('+')+1:line.find('=')].strip()
	z = line[line.find('=')+1:].strip()
	if not shortcuts.has_key(x):
		shortcuts[x] = {}
	shortcuts[x][y] = z
	
	
	
	
#####
#Writing the new file
#####
lookup = {}
ngrams = {}
lastchance = ['a?g=aung']
def buildAndSaveModel():
	res = {}
	res['words'] = wordlist
	res['lookup'] = lookup
	if ngrams:
		res['ngrams'] = ngrams
	if lastchance:
		res['lastchance'] = lastchance
	if shortcuts:
		res['shortcuts'] = shortcuts
	saveModels(res)
	
	
	
	
#####
#Recursively building the new model
#####
def buildPrefixes(myanmar, lookupList, currNgramNode):
	for wordID,prefixJump in lookupList.iteritems():
		nextMM = (myanmar+'/') if myanmar else u''
		nextMM += wordlist[wordID]
		currNgramNode[nextMM] = old_prefixresults[prefixJump]

		nextLookup = old_prefixlookup[prefixJump]
		if nextLookup:
			buildPrefixes(nextMM, nextLookup, currNgramNode)
		

def buildLookup(roman, nexusID, currLookupNode):
	for letter,number in old_nexuslist[nexusID].iteritems():
		if letter=='~':
			currLookupNode[letter] = old_prefixresults[number]
			
			prefEntries = old_prefixlookup[number]
			if prefEntries:
				ngrams[roman] = {}
				buildPrefixes(u'', prefEntries, ngrams[roman])
				#for wordID,prefixJump in prefEntries.iteritems():
				#	buildPrefixes(wordlist[wordID], prefixJump, ngrams[roman])
		else:
			currLookupNode[letter] = {}
			buildLookup(roman+letter, number, currLookupNode[letter])

def constructNewModel():
	buildLookup('', 0, lookup)
	



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
f = codecs.open('old_patsint.txt', encoding='utf-8', mode='r')
for line in f:
	line = line.strip()
	if line and (not line.startswith('#')):
		readOldFilePatSintLine(line)
f.close()
constructNewModel()
buildAndSaveModel()
print 'Done'






