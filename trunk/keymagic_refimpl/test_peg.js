function getParserSource() {
   //Get request object
   var ua = navigator.userAgent.toLowerCase();
   if (!window.ActiveXObject)
     request = new XMLHttpRequest();
   else if (ua.indexOf('msie 5') == -1)
     request = new ActiveXObject("Msxml2.XMLHTTP");
   else
     request = new ActiveXObject("Microsoft.XMLHTTP");
  
   //Create and send request; return text
   request.open("GET", "keymagic.peg", false);
   request.send(null);
   return request.responseText; //+0x04;
}


function Prim(type, value, id) {
  this.type = type;
  this.value = value;
  this.id = id;
  this.toString = function() {
    return '(' + this.type + ',' + this.value + ',' + this.id + ')';
  };
}


function parseText(sampleSrc) {
  //Some high-level containers
  options = new Array();
  variables = new Array();
  rules = new Array();
  switches = new Array();

  //Create the parser
  var parseSrc = getParserSource();
  var parser = PEG.buildParser(parseSrc);
  parser.parse(sampleSrc);


  //Quick debug: options
  var msg = "options[";
  var comma = "";
  for(var key in options) {
    msg += (comma + key + '=' + options[key]);
    comma = ', '
  }
  msg += ']\n';
  
  //Quick debug: variables
  msg += "variables[\n";
  for(var key in variables) {
    msg += ('  $' + key + ' = ')
    comma = ''
    var prod_list = variables[key];
    for(var i=0; i<prod_list.length; i++) {
      msg += (comma + prod_list[i].toString());
      comma = ', '
    }
    msg += '\n';
  }
  msg += ']\n';
  
  //Alert debug
  alert(msg);
}


function parse() {
  document.getElementById('result').innerHTML = '(<i>Processing...</i>)';
  try {
    parseText(document.getElementById('scrSource').value);
  } catch (err) {
    document.getElementById('result').innerHTML = '<b>ERROR</b>: ' + err;
    return;
  }
  document.getElementById('result').innerHTML = '<b>Done</b>';
}


