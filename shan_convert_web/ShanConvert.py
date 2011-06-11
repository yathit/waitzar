#
# This is a SIL encoding converter for the old Shan formats.
#  Copyright 2011 Seth N. Hetu
#  Released under the terms of the Apache License 2.0
#  See end of file for license terms
#

# Main conversion function
dirConvert = { 
  # Normal letters
  u'\u0021' : u'\u101E',
  u'\u0023' : u'\uAA66',
  u'\u0024' : u'\uAA67',
  u'\u0025' : u'\uAA68',
  u'\u0026' : u'\u102D\u1036',
  u'\u0027' : u'\u107E',
  u'\u0040' : u'\u101A',
  u'\u003F' : u'\u104A',
  u'\u003E' : u'\u1036',
  u'\u003C' : u'\u108A',
  u'\u003B' : u'\u1088',
  u'\u003A' : u'\u1038',
  u'\u002F' : u'\u104B',
  u'\u002C' : u'\u1087',
  u'\u002E' : u'\u1089',
  u'\u0042' : u'\u103B',
  u'\u0043' : u'\u1076',  
  u'\u0044' : u'\u102E',
  u'\u0045' : u'\u107C',
  u'\u0046' : u'\u103C',
  u'\u0047' : u'\u1082',
  u'\u0048' : u'\u0021',
  u'\u0041' : u'\u1035',
  u'\u004A' : u'\u108C\u108B',
  u'\u004F' : u'\u101E',
  u'\u0050' : u'\u1081\u1082\u103A',
  u'\u0051' : u'\u1022',
  u'\u0052' : u'\u103B\u103D',  
  #u'\u0053' : u'\u1004\u103A\u1039',
  u'\u0053' : u'\uAA7F', #NOTE: This is a temp. hack, since kinzi is the only multi-code-point letter here
  u'\u0054' : u'\u103C',
  u'\u0055' : u'\u1075',
  u'\u0056' : u'\uAA6E',
  u'\u0057' : u'\u107B',
  u'\u0058' : u'\uAA6A',
  u'\u0059' : u'\u107F',
  u'\u005A' : u'\u107D',
  u'\u005B' : u'\u1082\u103A',
  u'\u005E' : u'\uAA69',
  u'\u0061' : u'\u1031',
  u'\u0062' : u'\u101A',
  u'\u0063' : u'\u1076',
  u'\u0064' : u'\u102D',
  u'\u0065' : u'\u107C',
  u'\u0066' : u'\u103A',
  u'\u0067' : u'\u103D',
  u'\u0068' : u'\u1086',
  u'\u0069' : u'\u1004',
  u'\u006A' : u'\u1083',
  u'\u006B' : u'\u102F',
  u'\u006C' : u'\u1030',
  u'\u006D' : u'\u1064',
  u'\u006E' : u'\u107A',
  u'\u00A8' : u'\u1080',
  u'\u00A9' : u'\u109F',
  u'\u00AB' : u'\u107D\u1030',
  u'\u00AC' : u'\u1081\u102F',
  u'\u007D' : u'\u2019',
  u'\u007A' : u'\u107D',
  u'\u0079' : u'\u1015',
  u'\u004B' : u'\u102F',
  u'\u004C' : u'\u1030',
  u'\u004D' : u'\u1081\u103D',
  u'\u004E' : u'\u1081\u1030',  
  u'\u006F' : u'\u101D',
  u'\u0070' : u'\u1081',
  u'\u0076' : u'\u101C',
  u'\u0077' : u'\u1010',  
  u'\u0071' : u'\u1078',
  u'\u0072' : u'\u1019',
  u'\u0073' : u'\u1084',
  u'\u0074' : u'\u1022',
  u'\u0075' : u'\u1075',
  u'\u0078' : u'\u1011',
  u'\u0049' : u'\u101B',
  u'\u005D' : u'\u2018',
  u'\u00B5' : u'\u1091',
  u'\u00B6' : u'\u1092',
  u'\u2219' : u'\u1093',
  u'\u00B8' : u'\u1094',
  u'\u00B9' : u'\u1095',
  u'\u00BA' : u'\u1096',
  u'\u00BB' : u'\u1097',
  u'\u00BC' : u'\u1098',
  u'\u00BD' : u'\u1099',
  u'\u2022' : u'\u1099',
  u'\u201D' : u'\u1098',
  u'\u201C' : u'\u1097',
  u'\u2019' : u'\u1096',
  u'\u2018' : u'\u1095',
  u'\u00F6' : u'\u101B\u102F',
  u'\u00F7' : u'\u101B\u1030',
  u'\u00F8' : u'\u101B\u103D',
  u'\u00C7' : u'\u1049',
  u'\u00C6' : u'\u1048',
  u'\u00C5' : u'\u1047',
  u'\u00C4' : u'\u1046',
  u'\u00C3' : u'\u1045',
  u'\u00C2' : u'\u1044',
  u'\u00C1' : u'\u1043',
  u'\u00C0' : u'\u1042',
  u'\u00BF' : u'\u1041',
  u'\u00BE' : u'\u1040',
  
  #Other letters
  u'\u00A0' : u'\u2740',
  u'\u00A1' : u'\u2638',
  u'\u00A2' : u'\u2729',
  u'\u00A3' : u'\u263A',
  u'\u00A4' : u'\u27A9',
  u'\u00A5' : u'\u270D',
  u'\u00A6' : u'\u260F',
  u'\u00A7' : u'\u231A',
  u'\u007E' : u'\u007E',
  u'\u005C' : u'\u00F7',
  u'\u003D' : u'\u003D',
  u'\u002D' : u'\u002D',
  u'\u002B' : u'\u002B',
  u'\u002A' : u'\u273D',
  u'\u0029' : u'\u0029',
  u'\u0028' : u'\u0028',
  u'\u0022' : u'\u0022',
  u'\u007B' : u'\u00D7',
  u'\u007C' : u'\u0025',
  u'\u005F' : u'\u002F',
  u'\u0060' : u'\u003F',
  
  #Numbers (Arabic)
  u'\u0030' : u'\u0030',
  u'\u0031' : u'\u0031',
  u'\u0032' : u'\u0032',
  u'\u0033' : u'\u0033',
  u'\u0034' : u'\u0034',
  u'\u0035' : u'\u0035',
  u'\u0036' : u'\u0036',
  u'\u0037' : u'\u0037',
  u'\u0038' : u'\u0038',
  u'\u0039' : u'\u0039',
  
  #Whitespace
  u'\t' : u'\t',
  u'\n' : u'\n',
  u'\u0020' : u'\u0020'
}


# Helper
CONS = u'\u1000\u1001\u1002\u1003\u1004\u1005\u1006\u1007\u1008\u1009\u100A\u100B\u100C\u100D\u100E\u100F' + \
u'\u1010\u1011\u1012\u1013\u1014\u1015\u1016\u1017\u1018\u1019\u101A\u101B\u101C\u101D\u101E\u101F' + \
u'\u1020\u1021\u1022\u1023\u1024\u1025\u1026\u1027\u1028\u1029\u102A' + \
u'\u103F' + \
u'\u1041\u1042\u1043\u1044\u1045\u1046\u1047\u1048\u1049' + \
u'\u104E' + \
u'\u105A\u105B\u105C\u105D' + \
u'\u1061\u1065\u1066' + \
u'\u106E\u106F\u1070' + \
u'\u1075\u1076\u1077\u1078\u1079\u107A\u107B\u107C\u107D\u107E\u107F\u1080\u1081' + \
u'\u108E' + \
u'\uAA60\uAA61\uAA62\uAA63\uAA64\uAA65\uAA66\uAA67\uAA68\uAA69\uAA6A\uAA6B\uAA6C\uAA6D\uAA6E\uAA6F' + \
u'\uAA71\uAA72\uAA73\uAA74\uAA75\uAA76'


#Input normalization order. Cannot go backwards
input_norm = [
  #E vowel
  u'\u1031\u1084',

  #Medial R
  u'\u103C',

  #Consonant
  CONS,

  #Everything else
  u'\uAA7F' +
  u'\u103B\u105E\u105F' +
  u'\u103D\u1082' +
  u'\u103E\u1060' +
  u'\u102D\u102E\u1032\u1033\u1034\u1035\u1036\u1071\u1072\u1073\u1074\u1085\u109D' +
  u'\u102F\u1030' +
  u'\u1086' +
  u'\u102B\u102C\u1062\u1063\u1067\u1068\u1083' +
  u'\u1036\u1032' +
  u'\u1037' +
  u'\u103A' +
  u'\u1038\u1087\u1088\u1089\u108A\u108B\u108C\u108D\u108F\u109A\u109B\u109C'
]

#Output normalization order
output_norm = [
  #Kinzi
  u'\uAA7F',

  #Consonant
  CONS,

  #(No stacked1, stacked2, asat)

  #Medial Y, R, W, H
  u'\u103B\u105E\u105F',
  u'\u103C',
  u'\u103D\u1082',
  u'\u103E\u1060',

  #(No mon asat)

  #E vowel
  u'\u1031\u1084',

  #(No shan E vowel)

  #Upper vowel, lower vowel
  u'\u102D\u102E\u1032\u1033\u1034\u1035\u1036\u1071\u1072\u1073\u1074\u1085\u109D',
  u'\u102F\u1030',

  #(No Karen vowel)

  #Shan vowel
  u'\u1086',

  #A vowel
  u'\u102B\u102C\u1062\u1063\u1067\u1068\u1083',

  #Anusvara
  u'\u1036\u1032',

  #(No Pwo tone)

  #Dot below
  u'\u1037',

  #(No mon H)

  #Visible virama
  u'\u103A',

  #Visarga
  u'\u1038\u1087\u1088\u1089\u108A\u108B\u108C\u108D\u108F\u109A\u109B\u109C'
  
  #(No reduplication)
]



def find_letter(str, letter):
  for i in xrange(len(str)):
    if str[i]==letter:
      return i

  return -1


# Match one of these arrays, return an id:
def match_arr(arr, letter):
  for id in xrange(len(arr)):
    entry = arr[id]
    if find_letter(entry, letter) != -1:
      return id

  return -1


# Flush an array of strings
def flush(arr):
  res = u''
  for id in xrange(len(arr)):
    elem = arr[id]
    if elem==u'\uAA7F':
      elem =  u'\u1004\u103A\u1039'
    res += elem
    arr[id] = u''

  return res

def esc(str):
  res = u''
  for chr in src:
    res += '\\u' + hex(ord(chr))[2:].upper()
  return res

def fixQuotes(letter, prevStr, single, double):
  if letter!=single or len(prevStr)==0 or prevStr[-1]!=single:
    return u''

  return double


unknown = u'';
def convert(source):
  #First, direct convert
  res = u''
  unknown = u''
  for letter in source:
    if dirConvert.has_key(letter):
      curr = dirConvert[letter]
      
      #Special case, quotes
      fix = fixQuotes(curr, res, u'\u2018', u'\u201C')
      fix += fixQuotes(curr, res, u'\u2019', u'\u201D')
      if fix:
        res = res[:len(res)-1]
        curr = fix
      
      res += curr
      
    else:
      unknown += u':'*(len(unknown)>0) + letter

  #Now, normalize it
  final_result = u''
  norm_in_id = 0
  norm_out = []
  #norm_out.length = output_norm.length;
  for i in output_norm:
    norm_out.append(u'')
  for letter in res:
    #Get its corresponding entry IDs in input_norm and output_norm
    in_id = match_arr(input_norm, letter)
    out_id = match_arr(output_norm, letter)
    if in_id==-1 and out_id==-1:
      #No match; flush the input
      final_result += flush(norm_out)
      norm_in_id = 0

      #Append it
      final_result += letter

      #Report unknown unicode letters
      ordVal = ord(letter[0])
      if (ordVal>=0x1000 and ordVal<=0x109F) or (ordVal>=0xAA60 and ordVal<=0xAA7F):
        unknown += u':'*(len(unknown)>0) + u'(' + str(ordVal) + u')'
    elif in_id==-1 or out_id==-1:
      #Error!
      unknown = u'*** ' + letter + u'  ' + str(in_id) + u' ' + str(out_id)
      raise UnicodeError(u'Bad string: %s' % unknown)
    else:
      #A match exists; add it to output_norm. But first, check if this will make us 'go backwards'
      if in_id < norm_in_id:
        final_result += flush(norm_out)
        norm_in_id = 0
      
      #Slot it
      norm_out[out_id] += letter
      norm_in_id = in_id

      #Special case! Consonant will always advance the ID by 1
      if norm_in_id==2:
        norm_in_id += 1

  #Append any remaining letters
  final_result += flush(norm_out)

  return final_result


# Main conversion function
#   Specify this in the "Function Name" box of the SIL converter.
def ShanConvertString(str):
  if not isinstance(str, unicode):
    raise UnicodeError(u'Invalid (non-unicode) input string: %s' % str)

  return convert(str)

if __name__ == "__main__":
  src = u']]Twj:qgrf:vlnf;pof:]cj;} b[,erfaOurf:}}'
  expected = '“တြႃးၸွမ်းလူၺ်ႈႁဝ်း‘ၶႃႈ’ ယႂ်ႇၼမ်သေၵမ်း”'
  res = ShanConvertString(src)
  
  print src
  print esc(res)
  print esc(expected)





# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
