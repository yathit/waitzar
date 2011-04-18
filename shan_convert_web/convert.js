var dirConvert = { 
  //Normal letters
  '\u0021' : '\u101E',
  '\u0023' : '\uAA66',
  '\u0024' : '\uAA67',
  '\u0025' : '\uAA68',
  '\u0026' : '\u102D\u1036',
  '\u0027' : '\u107E',
  '\u0040' : '\u101A',
  '\u003F' : '\u104A',
  '\u003E' : '\u1036',
  '\u003C' : '\u108A',
  '\u003B' : '\u1088',
  '\u003A' : '\u1038',
  '\u002F' : '\u104B',
  '\u002C' : '\u1087',
  '\u002E' : '\u1089',
  '\u0042' : '\u103B',
  '\u0043' : '\u1076',  
  '\u0044' : '\u102E',
  '\u0045' : '\u107C',
  '\u0046' : '\u103C',
  '\u0047' : '\u1082',
  '\u0048' : '\u0021',
  '\u0041' : '\u1035',
  '\u004A' : '\u108C\u108B',
  '\u004F' : '\u101E',
  '\u0050' : '\u1081\u1082\u103A',
  '\u0051' : '\u1022',
  '\u0052' : '\u103B\u103D',  
  //'\u0053' : '\u1004\u103A\u1039',
  '\u0053' : '\uAA7F', //NOTE: This is a temp. hack, since kinzi is the only multi-code-point letter here
  '\u0054' : '\u103C',
  '\u0055' : '\u1075',
  '\u0056' : '\uAA6E',
  '\u0057' : '\u107B',
  '\u0058' : '\uAA6A',
  '\u0059' : '\u107F',
  '\u005A' : '\u107D',
  '\u005B' : '\u1082\u103A',
  '\u005E' : '\uAA69',
  '\u0061' : '\u1031',
  '\u0062' : '\u101A',
  '\u0063' : '\u1076',
  '\u0064' : '\u102D',
  '\u0065' : '\u107C',
  '\u0066' : '\u103A',
  '\u0067' : '\u103D',
  '\u0068' : '\u1086',
  '\u0069' : '\u1004',
  '\u006A' : '\u1083',
  '\u006B' : '\u102F',
  '\u006C' : '\u1030',
  '\u006D' : '\u1064',
  '\u006E' : '\u107A',
  '\u00A8' : '\u1080',
  '\u00A9' : '\u109F',
  '\u00AB' : '\u107D\u1030',
  '\u00AC' : '\u1081\u102F',
  '\u007D' : '\u1015',
  '\u007A' : '\u107D',
  '\u0079' : '\u1015',
  '\u004B' : '\u102F',
  '\u004C' : '\u1030',
  '\u004D' : '\u1081\u103D',
  '\u004E' : '\u1081\u1030',  
  '\u006F' : '\u101D',
  '\u0070' : '\u1081',
  '\u0076' : '\u101C',
  '\u0077' : '\u1010',  
  '\u0071' : '\u1078',
  '\u0072' : '\u1019',
  '\u0073' : '\u1084',
  '\u0074' : '\u1022',
  '\u0075' : '\u1075',
  '\u0078' : '\u1011',
  '\u0049' : '\u101B',
  '\u005D' : '\u109B',
  '\u00B5' : '\u1091',
  '\u00B6' : '\u1092',
  '\u2219' : '\u1093',
  '\u00B8' : '\u1094',
  '\u00B9' : '\u1095',
  '\u00BA' : '\u1096',
  '\u00BB' : '\u1097',
  '\u00BC' : '\u1098',
  '\u00BD' : '\u1099',
  '\u2022' : '\u1099',
  '\u201D' : '\u1098',
  '\u201C' : '\u1097',
  '\u2019' : '\u1096',
  '\u2018' : '\u1095',
  '\u00F6' : '\u101B\u102F',
  '\u00F7' : '\u101B\u1030',
  '\u00F8' : '\u101B\u103D',
  '\u00C7' : '\u1049',
  '\u00C6' : '\u1048',
  '\u00C5' : '\u1047',
  '\u00C4' : '\u1046',
  '\u00C3' : '\u1045',
  '\u00C2' : '\u1044',
  '\u00C1' : '\u1043',
  '\u00C0' : '\u1042',
  '\u00BF' : '\u1041',
  '\u00BE' : '\u1040',
  


  //Other letters
  '\u00A0' : '\u2740',
  '\u00A1' : '\u2638',
  '\u00A2' : '\u2729',
  '\u00A3' : '\u263A',
  '\u00A4' : '\u27A9',
  '\u00A5' : '\u270D',
  '\u00A6' : '\u260F',
  '\u00A7' : '\u231A',
  '\u007E' : '\u007E',
  '\u005C' : '\u00F7',
  '\u003D' : '\u003D',
  '\u002D' : '\u002D',
  '\u002B' : '\u002B',
  '\u002A' : '\u273D',
  '\u0029' : '\u0029',
  '\u0028' : '\u0028',
  '\u0022' : '\u0022',
  '\u007B' : '\u00D7',
  '\u007C' : '\u0025',
  '\u005F' : '\u002F',
  '\u0060' : '\u003F',
  
  //Numbers (Arabic)
  '\u0030' : '\u0030',
  '\u0031' : '\u0031',
  '\u0032' : '\u0032',
  '\u0033' : '\u0033',
  '\u0034' : '\u0034',
  '\u0035' : '\u0035',
  '\u0036' : '\u0036',
  '\u0037' : '\u0037',
  '\u0038' : '\u0038',
  '\u0039' : '\u0039',
  
  //Whitespace
  '\t' : '\t',
  '\n' : '\n',
  '\u0020' : '\u0020'
};


//Helper
var CONS =   
  '\u1000\u1001\u1002\u1003\u1004\u1005\u1006\u1007\u1008\u1009\u100A\u100B\u100C\u100D\u100E\u100F'
 +'\u1010\u1011\u1012\u1013\u1014\u1015\u1016\u1017\u1018\u1019\u101A\u101B\u101C\u101D\u101E\u101F'
 +'\u1020\u1021\u1022\u1023\u1024\u1025\u1026\u1027\u1028\u1029\u102A'
 +'\u103F'
 +'\u1041\u1042\u1043\u1044\u1045\u1046\u1047\u1048\u1049'
 +'\u104E'
 +'\u105A\u105B\u105C\u105D'
 +'\u1061\u1065\u1066'
 +'\u106E\u106F\u1070'
 +'\u1075\u1076\u1077\u1078\u1079\u107A\u107B\u107C\u107D\u107E\u107F\u1080\u1081'
 +'\u108E'
 +'\uAA60\uAA61\uAA62\uAA63\uAA64\uAA65\uAA66\uAA67\uAA68\uAA69\uAA6A\uAA6B\uAA6C\uAA6D\uAA6E\uAA6F'
 +'\uAA71\uAA72\uAA73\uAA74\uAA75\uAA76';


//Input normalization order. Cannot go backwards
var input_norm = [
  //E vowel
  '\u1031\u1084',

  //Medial R
  '\u103C',

  //Consonant
  CONS,

  //Everything else
  '\uAA7F'
 +'\u103B\u105E\u105F'
 +'\u103D\u1082'
 +'\u103E\u1060'
 +'\u102D\u102E\u1032\u1033\u1034\u1035\u1036\u1071\u1072\u1073\u1074\u1085\u109D'
 +'\u102F\u1030'
 +'\u1086'
 +'\u102B\u102C\u1062\u1063\u1067\u1068\u1083'
 +'\u1036\u1032'
 +'\u1037'
 +'\u103A'
 +'\u1038\u1087\u1088\u1089\u108A\u108B\u108C\u108D\u108F\u109A\u109B\u109C'
];

//Output normalization order
var output_norm = [
  //Kinzi
  '\uAA7F',

  //Consonant
  CONS,

  //(No stacked1, stacked2, asat)

  //Medial Y, R, W, H
  '\u103B\u105E\u105F',
  '\u103C',
  '\u103D\u1082',
  '\u103E\u1060',

  //(No mon asat)

  //E vowel
  '\u1031\u1084',

  //(No shan E vowel)

  //Upper vowel, lower vowel
  '\u102D\u102E\u1032\u1033\u1034\u1035\u1036\u1071\u1072\u1073\u1074\u1085\u109D',
  '\u102F\u1030',

  //(No Karen vowel)

  //Shan vowel
  '\u1086',

  //A vowel
  '\u102B\u102C\u1062\u1063\u1067\u1068\u1083',

  //Anusvara
  '\u1036\u1032',

  //(No Pwo tone)

  //Dot below
  '\u1037',

  //(No mon H)

  //Visible virama
  '\u103A',

  //Visarga
  '\u1038\u1087\u1088\u1089\u108A\u108B\u108C\u108D\u108F\u109A\u109B\u109C'
  
  //(No reduplication)
];


function find_letter(str, letter) {
  for (var i=0; i<str.length; i++) {
    if (str[i]==letter) { return i; }
  }
  return -1;
}


//Match one of these arrays, return an id:
function match_arr(arr, letter) {
  for (id in arr) {
    var entry = arr[id];
    if (find_letter(entry, letter) != -1) {
      return id;
    }
  }
  return -1;
}


//Flush an array of strings
function flush(arr) {
  var res = '';
  for (id in arr) {
    var elem = arr[id];
    if (elem=='\uAA7F') {
      elem =  '\u1004\u103A\u1039';
    }
    res += elem;
    arr[id] = '';
  }
  return res;
}



var unknown = '';
function convert(source) {
  //First, direct convert
  var res = '';
  unknown = '';
  for (c in source) {
    var letter = source[c];
    if (letter in dirConvert) {
      res += dirConvert[letter];
    } else {
      unknown += (unknown.length!=0?' ':' ') + letter;
    }
  }

  //Now, normalize it
  var final_result = '';
  var norm_in_id = 0;
  var norm_out = [];
  norm_out.length = output_norm.length;
  for (var i=0; i<norm_out.length; i++) {
    norm_out[i] = '';
  }
  for (c in res) {
    var letter = res[c];
    
    //Get its corresponding entry IDs in input_norm and output_norm
    var in_id = match_arr(input_norm, letter);
    var out_id = match_arr(output_norm, letter);
    if (in_id==-1 && out_id==-1) {
      //No match; flush the input
      final_result += flush(norm_out);
      norm_in_id = 0;

      //Append it
      final_result += letter;

      //Report unknown unicode letters
      var ord = letter.charCodeAt[0];
      if ((ord>=0x1000 && ord<=0x109F) || (ord>=0xAA60 && ord<=0xAA7F)) {
        unknown += (unknown.length!=0?' ':' ') + '(' + ord + ')';
      }
    } else if (in_id==-1 || out_id==-1) {
      //Error!
      unknown = '*** ' + letter + '  ' + in_id + ' ' + out_id
      return '';
    } else {
      //A match exists; add it to output_norm. But first, check if this will make us 'go backwards'
      if (in_id < norm_in_id) {
        final_result += flush(norm_out);
        norm_in_id = 0;
      }
      
      //Slot it
      norm_out[out_id] += letter;
      norm_in_id = in_id;

      //Special case! Consonant will always advance the ID by 1:
      if (norm_in_id==2) {
        norm_in_id += 1;
      }
    }
  }

  //Append any remaining letters
  final_result += flush(norm_out);

  return final_result;
}


function convertToUni() {
  var source = document.getElementById('scrSource').value;
  var done = convert(source);
  document.getElementById('scrFinish').value = done;
  document.getElementById('unknown').value = unknown;
}


