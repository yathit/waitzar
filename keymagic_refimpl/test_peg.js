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
   return request.responseText;
}



function parseText(sampleSrc) {
  //Some high-level containers
  options = new Array();

  //Create the parser
  var parseSrc = getParserSource();
  var parser = PEG.buildParser(parseSrc);
  parser.parse(sampleSrc);


  //Quick debug
  var msg = "options[";
  var comma = "";
  for(var key in options) {
    msg += (comma + key + '=' + options[key]);
    comma = ', '
  }
  msg += ']\n';
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


