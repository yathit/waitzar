#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import re
import codecs
from burglish_data import *


#Uses the INVALID_COMBINATIONS array to determine if 
#  we generated an invalid word.
def isInvalid(word):
    if not word:
        return True
    for i in xrange(len(word)-1):
        for letterpair in INVALID_COMBINATIONS:
            if letterpair[0].find(word[i])!=-1 and letterpair[1].find(word[i+1])!=-1:
                return True
    return False

    
#Performa  series of substitutions to replace normalized (or badly spelled) Zawgyi with displayable Zawgyi
def zgDeNormalize(roman, word):
    count = 0
    
    debug = None
    if roman == debug:
        print '%s: %s' % (debug, u' '.join(map(lambda x:hex(ord(x)) , word)))

    for regex in ZAWGYI_DE_NORMALIZE:
        #Increment our ID
        count += 1
        
        #Temp
        #print 'Regex:'
        #for temp in regex[0]:
        #    print '    Pattern: ' , u' '.join(map(lambda x:hex(ord(x)) , temp))
        #print '    Replace: ' , u' '.join(map(lambda x:hex(ord(x)) , regex[1]))
    
        #Search and replace
        patterns = regex[0]
        replaceStr = regex[1]
        #if not re.match(u'^[\u1000-\u109F]*$', replaceStr):
        #    raise Exception('Error: Replace string is not unicode. (Pattern %i)' % count)
        foundStr = [-1, -1, -1, True] #start, length, erase_index, pass
        currIndex = 0
        patID = 0
        for patternStr in patterns:
            #Validate and extract
            patID += 1
            #if not re.match(u'^[MENOXF][\u1000-\u109F]+$', patternStr):
            #    raise Exception('Error: Pattern (%i) of line %i is invalid.' % (patID, count))
            flag = patternStr[0]
            pat = patternStr[1:]
        
            #Match: Just match any letter; Erase: Match any letter and save this index
            if flag=='M' or flag=='E':
                if pat==u'*' or pat.find(word[currIndex])==-1:
                    foundStr[3] = False
                    break
                if flag=='E':
                    foundStr[2] = currIndex
                currIndex += 1
                    
            #Not: Match NONE of the letters
            elif flag == 'N':
                if pat.find(word[currIndex])!=-1:
                    foundStr[3] = False
                    break
                currIndex += 1
                    
            #Opt: Match one of the letters or nothing
            elif flag == 'O':
                if pat.find(word[currIndex])!=-1:
                    currIndex += 1
                    
            #Opt-Not: Match anything except one of the letters or nothing
            elif flag == 'X':
                if pat.find(word[currIndex])==-1:
                    currIndex += 1
                    
            #Find: Match the entire string, save its offset and length
            elif flag == 'F':
                if foundStr[1]!=-1:
                    raise Exception('Pattern contains more than one "find" statement.')
                foundID = word.find(pat, currIndex)
                if foundID==-1:
                    foundStr[3] = False
                    break
                currIndex = foundID
                foundStr[0] = currIndex
                foundStr[1] = len(pat)
                currIndex += len(pat)
                
            #Error otherwise:
            else:
                raise Exception('Invalid pattern type.')
                
            #Break?
            if currIndex==len(word):
                break
                
        #Now, modify the string if we found anything
        if not foundStr[3]:
            continue
            
        if roman == debug:
            print "  Found pattern: %i for word." % (count)
            
        #Handle the erase index
        if foundStr[2]!=-1:
            word = word[:foundStr[2]] + word[foundStr[2]+1:]
            if foundStr[2] < foundStr[0]:
                foundStr[0] -= 1
                
        #Handle the replacement
        word = word[:foundStr[0]] + replaceStr + word[foundStr[0]+foundStr[1]:]
        if roman == debug:
            print "  Replacement made: %s" % (u' '.join(map(lambda x:hex(ord(x)) , word)))
            print "  Array: " , foundStr
            sys.exit(1)

    return word
    

#Generate the standard combinations of onset+rhyme
#This is the simplest step to understand, but the most complex to code.
def GenerateStandardCombinations():
    #Loop through each possible onset/rhyme combination.
    results = []
    for onsPair in COMMON_ONSETS:
        for rhymPair in COMMON_RHYMES:
            #Pull out the associated array
            romanOns = onsPair[0]
            onsets = onsPair[1]
            romanRhym = rhymPair[0]
            rhymes = rhymPair[1]
            
            #Each onset or rhyme array may contain multiple entries. Loop through these
            for onset in onsets:
                for rhyme in rhymes:
                    #Some substitution; messy, I know
                    if (romanOns == u'-'):
                        romanOns = u''
                    if (romanRhym == u'-'):
                        romanRhym = u''
                
                    #Form the combined word
                    if rhyme.find(u'-')==-1:
                        raise Exception('Error: Can\'t have a rhyme with no dash: %s->%s' % (romanRhym, map(lambda x:hex(ord(x)) , rhyme)))
                    newWord = rhyme.replace(u'-', onset, 1)
                    
                    #Is this invalid?
                    if isInvalid(newWord):
                        continue

                    #Apply normalization to this word, in case we generated some invalid Zawgyi-One letters.
                    newRoman = romanOns+romanRhym
                    newWord = zgDeNormalize(newRoman, newWord)
                    
                    #Add this word to the result set
                    results.append([newRoman, newWord])

    #Done
    return results

    
#This is basically done for us
def GenerateSpecialWords():
    results = []
    for item in SPECIAL_WORDS:
        results.append([item[0], item[1]])
    return results

    
#For now, this doesn't do much
#TODO: Eventually generate numbers as the user types them
def GenerateNumbers():
    results = []
    for i in xrange(10):
        results.append([unichr(ord(u'0')+i), NUMBER_CONSTRUCTOR[2][i]])
    return results
    
    
#Expand our results
def GenerateExpandedWords(wordlist):
    results = []
    for entry in wordlist:
        word = entry[1]
        for pattern in EXPAND_PATTERNS:
            foundIndex = word.find(pattern[0])
            if foundIndex==-1:
                continue
            if pattern[1] and foundIndex+len(pattern[0])<len(word) and pattern[1].find(word[foundIndex+len(pattern[0])])!=-1:
                continue
                
            #Is this word invalid?
            newWord = word[:foundIndex] + pattern[2] + word[foundIndex+len(pattern[0]):]
            if isInvalid(newWord):
                continue
                
            #It passed; add a new entry to our results list
            results.append([entry[0], newWord])
    return results

#Create our full word list. Generates individual lists for each step, then 
#  returns a full set containing no duplicates.
def BuildZawgyiWordlist():
    #Get single, partial word lists
    print 'Step 1'
    words_special     = GenerateSpecialWords()
    print 'Step 2'
    words_numbers  = GenerateNumbers()
    print 'Step 3'
    words_standard  = GenerateStandardCombinations()
    print 'Step 4'
    words_expanded = GenerateExpandedWords(words_special + words_numbers + words_standard)
    print 'Step 5'
    sublists = [words_special, words_numbers, words_standard, words_expanded]
    
    #Combine them, return
    res = []
    flags = {}
    for slist in sublists:
        for item in slist:
            try:
                #if item[0]==u'lyaun' or item[0]==u'shaun':
                #    print item[0], flags.has_key(item[1])
                item_id = item[0] + ':' + item[1]
                if not flags.has_key(item_id):
                    res.append(item)
                    flags[item_id] = True
            except IndexError as ex:
                print 'Error: ' , len(item) , ' , ', map(lambda x:hex(ord(x)) , item[0])
                raise ex

    #Output
    print '     Standard words: ', len(words_standard) 
    print ' +   Special words: ', len(words_special)
    print ' +   Number words: ', len(words_numbers)
    print ' +   Expanded words: ', len(words_expanded)
    print '------------------------------------'
    print ' =   Total words: ', len(res)
    
    return res
    
if __name__ == "__main__":
    #Get the list of words
    wordlist = BuildZawgyiWordlist()
    
    #Gather data
    counts = {}
    
    #Print it to a file
    out = codecs.open("burglish_wordlist.txt", "w", "utf-8")
    for entry in wordlist:
        out.write(u'%s = %s\n' % (entry[1], entry[0]))
        
        #Data
        if not counts.has_key(entry[0]):
            counts[entry[0]] = 0
        counts[entry[0]] += 1
    out.close()
    
    #Get the max count
    temp = dict(map(lambda x: (x[1],x[0]),counts.items()))
    print 'Largest number of entries: ',  temp[max(temp.keys())] , ":", counts[temp[max(temp.keys())]]
