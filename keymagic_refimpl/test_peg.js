function count_keys(obj) {
  var i = 0;
  for (var key in obj) { i++; }
  return i;
}

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

function getPreParserSource() {
   //Get request object
   var ua = navigator.userAgent.toLowerCase();
   if (!window.ActiveXObject)
     request = new XMLHttpRequest();
   else if (ua.indexOf('msie 5') == -1)
     request = new ActiveXObject("Msxml2.XMLHTTP");
   else
     request = new ActiveXObject("Microsoft.XMLHTTP");
  
   //Create and send request; return text
   request.open("GET", "keymagic_pre.peg", false);
   request.send(null);
   return request.responseText; 
}


function Prim(type, value, id) {
  this.type = type;
  this.value = value;
  this.id = id;
  
  this.toString = function() {
    return '(' + this.type + ',' + this.value + ',' + this.id + ')';
  };
}


//Some high-level containers
var options = new Array();
var variables = new Array();
var rules = new Array();
var switches = new Array();

//Constructs for the model
var RULES = new Array();
var VARS = new Object();
var SWITCHES = new Object();
var TYPED_STR = '';
var RULE_VK = null;
var VKEY = null;
var STATE = null;

//Internal model constructs
var groups = new Array();
var group_ids = new Array();
var sw_temp = new Array();
var src = '';
var singleASCII = false;
var isLHS = true;
  

VIRT_KEY_CODES = {
  'VK_BACK' : 0x0008,
  'VK_TAB' : 0x0009,
  'VK_ENTER' : 0x000D,
  'VK_SHIFT' : 0x0010,
  'VK_CTRL' :  0x0011,
  'VK_ALT' : 0x0012,
  'VK_PAUSE' : 0x0013,
  'VK_CAPSLOCK' : 0x0014,
  'VK_KANJI' : 0x0019,
  'VK_ESCAPE' : 0x001B,
  'VK_SPACE' : 0x0020,
  'VK_PRIOR' : 0x0021,
  'VK_NEXT' : 0x0022,
  'VK_DELETE' : 0x002E,
  'VK_KEY_0' : 0x0030,
  'VK_KEY_1' : 0x0031,
  'VK_KEY_2' : 0x0032,
  'VK_KEY_3' : 0x0033,
  'VK_KEY_4' : 0x0034,
  'VK_KEY_5' : 0x0035,
  'VK_KEY_6' : 0x0036,
  'VK_KEY_7' : 0x0037,
  'VK_KEY_8' : 0x0038,
  'VK_KEY_9' : 0x0039,
  'VK_KEY_A' : 0x0041,
  'VK_KEY_B' : 0x0042,
  'VK_KEY_C' : 0x0043,
  'VK_KEY_D' : 0x0044,
  'VK_KEY_E' : 0x0045,
  'VK_KEY_F' : 0x0046,
  'VK_KEY_G' : 0x0047,
  'VK_KEY_H' : 0x0048,
  'VK_KEY_I' : 0x0049,
  'VK_KEY_J' : 0x004A,
  'VK_KEY_K' : 0x004B,
  'VK_KEY_L' : 0x004C,
  'VK_KEY_M' : 0x004D,
  'VK_KEY_N' : 0x004E,
  'VK_KEY_O' : 0x004F,
  'VK_KEY_P' : 0x0050,
  'VK_KEY_Q' : 0x0051,
  'VK_KEY_R' : 0x0052,
  'VK_KEY_S' : 0x0053,
  'VK_KEY_T' : 0x0054,
  'VK_KEY_U' : 0x0055,
  'VK_KEY_V' : 0x0056,
  'VK_KEY_W' : 0x0057,
  'VK_KEY_X' : 0x0058,
  'VK_KEY_Y' : 0x0059,
  'VK_KEY_Z' : 0x005A,
  'VK_NUMPAD0' : 0x0060,
  'VK_NUMPAD1' : 0x0061,
  'VK_NUMPAD2' : 0x0062,
  'VK_NUMPAD3' : 0x0063,
  'VK_NUMPAD4' : 0x0064,
  'VK_NUMPAD5' : 0x0065,
  'VK_NUMPAD6' : 0x0066,
  'VK_NUMPAD7' : 0x0067,
  'VK_NUMPAD8' : 0x0068,
  'VK_NUMPAD9' : 0x0069,
  'VK_MULTIPLY' : 0x006A,
  'VK_ADD' : 0x006B,
  'VK_SEPARATOR' : 0x006C,
  'VK_SUBTRACT' : 0x006D,
  'VK_DECIMAL' : 0x006E,
  'VK_DIVIDE' : 0x006F,
  'VK_F1' : 0x0070,
  'VK_F2' : 0x0071,
  'VK_F3' : 0x0072,
  'VK_F4' : 0x0073,
  'VK_F5' : 0x0074,
  'VK_F6' : 0x0075,
  'VK_F7' : 0x0076,
  'VK_F8' : 0x0077,
  'VK_F9' : 0x0078,
  'VK_F10' : 0x0079,
  'VK_F11' : 0x007A,
  'VK_F12' : 0x007B,
  'VK_LSHIFT' : 0x00A0,
  'VK_RSHIFT' : 0x00A1,
  'VK_LCTRL' : 0x00A2,
  'VK_RCTRL' : 0x00A3,
  'VK_LALT' : 0x00A4,
  'VK_RALT' : 0x00A5,
  'VK_COLON' : 0x00BA,
  'VK_OEM_PLUS' : 0x00BB,
  'VK_OEM_COMMA' : 0x00BC,
  'VK_OEM_MINUS' : 0x00BD,
  'VK_OEM_PERIOD' : 0x00BE,
  'VK_QUESTION' : 0x00BF,
  'VK_CFLEX' : 0x00C0,
  'VK_LBRACKET' : 0x00DB,
  'VK_BACKSLASH' : 0x00DC,
  'VK_RBRACKET' : 0x00DD,
  'VK_QUOTE' : 0x00DE,
  'VK_EXCM' : 0x00DF,
  'VK_LESSTHEN' : 0x00E2
}


function VKey(type, mods, key) {
  this.find = function(arr, item) {
    for (var i=0; i<arr.length; i++) {
      if (arr[i] == item) { return true; }
    }
    return false;
  }

  this.type = type;
  this.modShift = this.find(mods, 'VK_SHIFT') 
  this.modAlt = this.find(mods, 'VK_ALT') 
  this.modCtrl = this.find(mods, 'VK_CTRL') 
  this.vkCode = VIRT_KEY_CODES[key];
  
  //For compatibility with "Prim"
  this.value = this;
  this.id = -1;
  
  this.toString = function() {
    return '(' + this.type + ',' + this.modShift + ',' + this.modAlt + ',' + this.modCtrl + ',' + this.vkCode + ')';
  };
}


//Simple pre-parser
//  No semantic knowledge whatsoever (except for options)
function preParseText(sampleSrc) {
  //Clear options array
  options.length = 0;
  
  //Run our pre-parser
  var parseSrc = getPreParserSource();
  var parser = PEG.buildParser(parseSrc);
  return parser.parse(sampleSrc);
}



function parseText(sampleSrc) {
  //Clear the other arrays
  variables.length = 0;
  rules.length = 0;
  switches.length = 0;

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
  
  //Quick debug: switches
  msg += "switches[";
  var comma = "";
  for(var key in switches) {
    msg += (comma + key);
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
  
  //Quick debug: rules
  msg += "rules[\n";
  for(var i=0; i<rules.length; i++) {
    var prod_list = rules[i].lhs;
    comma = ''
    for(var x=0; x<prod_list.length; x++) {
      msg += (comma + prod_list[x].toString());
      comma = ', '
    }
    msg += ' => ';
    prod_list = rules[i].rhs;
    comma = ''
    for(var x=0; x<prod_list.length; x++) {
      msg += (comma + prod_list[x].toString());
      comma = ', '
    }
    msg += '\n';
  }
  msg += ']\n';
  
  //Alert debug
  document.getElementById('parseTree').value = msg;
}


function ensure(test, text) {
  if (!test) { throw "Ensure failed: " + text; }
}

var always = function() { return true; }
var nothing = function() { }

function find_final_state(s) {
  //Any transition should eventually lead to the final state regardless of the option
  if (s.transitions.length==0) { return s; }
  return find_final_state(s.transitions[0].state);
}

function Trans(t, s) {
  this.test  = t;
  this.state = s;
}

function State(toPerform) {
  this.action = toPerform;
  this.transitions = new Array();

  this.add_transition = function(test, state) {
    this.transitions.push(new Trans(test, state));
    return state;
  }

  //Attempt to move to the next state
  this.next_transition = function() {
    for (i=0; i<this.transitions.len; i++) {
      if (this.transitions[i].t()) {
        return this.transitions[i].state;
      }
    }
    return null;
  }

  this.is_end_state = function() {
    return this.transitions.length == 0;
  }
}




function make_state(prim, side) {
  ensure(side=='lhs' || side=='rhs', "side=='lhs' || side=='rhs'");
  var id = prim.type + ':' + side;
  var ret = null;

  //String, LHS
  if (id=='STRING:lhs') {
    ret = new State(nothing);
    ret.add_transition(function(){
      return str.starts_with(prim.val.reverse());
    }, new State(function() {
      groups.push(prim.value.reverse());
      group_ids.push(-1);
      str = str.substr(prim.value.len, str.len);
    }));
  }

  //String, RHS
  if (id=='STRING:rhs') {
    ret = new State(nothing);
    ret.add_transition(always, new State(function() {
      str.append(prim.value);
    }));
  }

  //Virtual Key, LHS
  if (id=='VIRT_KEY:lhs') {
    ret = new State(nothing);
    ret.add_transition(function(){
      if (str.len==0 || VKEY == null) { return false; }
      if (!VKEY.matches(prim.val)) { return false; }
      return true;
    }, new State(function() {
      groups.push(str[0]);
      group_ids.push(-1);
      str = str.substr(1, str.len);
    }));
  }
  
  //Variable, RHS
  if (id=='VAR_NAME:rhs') {
    ret = new State(nothing);
    ret.add_transition(always, new State(function() {
      var s = VARS[prim.value].simple;
      str.append(s);
    }));
  }
  
  //Backref, RHS
  if (id=='BACKREF:rhs') {
    ret = new State(nothing);
    ret.add_transition(always, new State(function() {
      var s = groups[prim.id];
      str.append(s);
    }));
  }

  //Wildcard, LHS
  if (id=='WILDCARD:lhs') {
    ret = new State(nothing);
    ret.add_transition(function() {
      if (str.len==0) { return false };
      if (str[0]>=0x21 && str[0]<=0x7D)   { return true; }
      if (str[0]>=0xFF && str[0]<=0xFFFD) { return true; }
      return false;
    }, new State(function() {
      groups.push(str[0]);
      group_ids.push(-1);
      str = str.substr(1, str.len);
    }));
  }

  //Wildcard Variable All, LHS
  if (id=='WILD_VAR_ALL:lhs') {
    ret = new State(nothing);
    branch = new State(nothing);
    ret.add_transition(function() {
      return str.length > 0;
    }, branch);
    finalSt = new State(nothing);
    for (i=0; i<prim.value.len; i++) {
      next = branch.add_transition(function() {
        str[0] == prim.value[i];
      }, new State(function() {
        groups.push(str[0]);
        group_ids.push(i);
        str = str.substr(1, str.len);
      }));
      next.add_transition(always, finalSt);
    }
  }

  //Wildcard Variable None, LHS
  if (id=='WILD_VAR_NONE:lhs') {
    ret = new State(nothing);
    next = new State(nothing);
    ret.add_transition(function() {
      return str.length > 0;
    }, next);
    for (i=0; i<prim.value.len; i++) {
      next = next.add_transition(function() {
        str[0] != prim.value[i];
      }, new State(nothing));
    }
    next.action = function() {
      groups.push(str[0]);
      group_ids.push(-1);
      str = str.substr(1, str.len);
    }
  }

  //WILD_BACKREF, RHS
  if (id=='WILD_BACKREF:rhs') {
    ret = new State(nothing);
    ret.add_transition(always, new State(function() {
      var s = VARS[prim.value].simple[group_ids[prim.id]];
      str.append(s);
    }));
  }

  //Switch, LHS
  if (id=='SWITCH:lhs') {
    ret = new State(nothing);
    ret.add_transition(function(){
      return SWITCHES[prim.value];
    }, new State(function() {
      sw_temp.push(prim.value);
    }));
  }

  //Switch, RHS
  if (id=='SWITCH:rhs') {
    ret = new State(nothing);
    ret.add_transition(always, new State(function() {
      SWITCHES[prim.value] = true;
    }));
  }
  
  //Variable, LHS
  if (id=='VAR_NAME:lhs') {
    ret = new State(nothing);
    ret.add_transition(always, new State(function() {
      //Change the next transition
      old_transition = next_transition;
      plus_one_state = next_transition();
      next_transition = function() {
        next_state = VARS[prim.value].enter_context(plus_one_state);
        next_transition = old_transition;
        return next_state;   //Jump into the variable's state machine
      }
    }));
  }

  //Return the item we created, or null if nothing matched
  return ret;
}


function build_state_tree(token_list, side, reverse) {
  ensure(side=='rhs' || side=='lhs', "build_state_tree(side is rhs or lhs)");
  ensure(token_list.length > 0, "build_state_tree(token list not empty)");

  var elems = new Array();
  elems.length = token_list.length;
  for (var i=0;i<token_list.length; i++) {
    var t_id = i
	if (reverse) { t_id = token_list.length - i - 1; }
	
    //Translate this token into a primitive and then a state
    var elem = make_state(token_list[t_id], side);		
    if (elem == null) { return null; }  //Silently fail
    elems[i] = elem;
	
    //Hook the previous state into this one
    if (i>0) {
      var prev = find_final_state(elems[i-1]);
      prev.transitions = elem.transitions;
    }
  }
  
  return elems[0];
};


function make_simple(token_list) {
  ensure(token_list.length > 0, "make_simple(token_list not empty)");
  
  var res = '';
  for (var i=0; i<token_list.length; i++) {
    var p = token_list[i];

    //Invalid types cause the entire function to silently fail
    if (p.type=='SWITCH' || p.type=='WILD' || p.type=='BACKREF' || 
        p.type=='WILD_VAR_ALL' || p.type=='WILD_VAR_NONE' || p.type=='WILD_BACKREF') 
      return null;

    //Valid types
    if (p.type=='STRING') { res += p.value; }
    if (p.type=='VIRT_KEY') {
      var val = p.value.getANum();
      if (val == '\0') { return null; }
      res += val;
    }
    if (p.type=='VAR_ELEMENT') {
      var val = VARS[p.value].simple;
      if (val == null) { return null; }
      res += val[p.id];
    }
    if (p.type=='VAR_NAME') {
      var val = VARS[p.value].simple;
      if (val == null) { return null; }
      res += val;
    }
	
    //Just double-check our token type
    ensure(p.type=='STRING' || p.type=='VIRT_KEY' || p.type=='VAR_ELEMENT' || p.type=='VAR_NAME', "p.type=='STRING' || p.type=='VIRT_KEY' || p.type=='VAR_ELEMENT' || p.type=='VAR_NAME'"); 
  }
}


function Var(token) {
  this.reset = function() {
    this.ret_stack.length = 0;   //In case a previous match aborted halfway through
  };
  
  //Allow context switching
  this.enter_context = function(next_state) {
    this.ret_stack.push(next_state);
    return this.rev_states;
  }

  this.name = token.name;
  this.rev_states = build_state_tree(token.items, 'lhs', true);
  this.simple = make_simple(token.items);
  this.ret_stack = new Array();

  //Need to add a special "end" state for variables
  this.last = find_final_state(this.rev_states);
  this.last.add_transition(always, new State(function() {
      STATE = ret_stack.pop();
      STATE.action(); //Perform this state's action for it.
    }));
};


function Rule(token) {
  this.combine_halves = function(lhs, rhs) {
    var ret = new State(nothing);
    ret = ret.add_transition(always, new State(function() {
      groups.length = 0;
      group_ids.length = 0;
      sw_temp.length = 0;
      src = TYPED_STR.reverse();
      isLHS = true;
    }));
	
    ret.add_transition(always, lhs);
    ret = find_final_state(ret);

    ret = ret.add_transition(function(){
      return !singleASCII && src.len>=0;
    }, new State(function() {
      src = src.reverse();
      groups = groups.reverse();
      group_ids = group_ids.reverse();
      for (key in sw_temp) {
        SWITCHES[key] = false;
      }
      isLHS = false;
      if (RULE_VK!=null) { VKEY = null; }
    }));
	
    ret.add_transition(always, rhs);
    ret = find_final_state(ret);
    
    ret = ret.add_transition(always, new State(function() {
      diff = str_diff(TYPED_STR, str);
      if (diff.len==0 || (diff.len==1 && diff[0].ord>=0x20 && diff[0].ord<=0x7F)) {
        singleASCII = true;
      }
      TYPED_STR = src;
    }));
  };
  
  this.get_primary_vkey = function(prims, res, allowed) {
    //Check each primitive in the list
    for (var key in prims) {
	  if (!prims.hasOwnProperty(key)) { continue; }
      var prim = prims[key];
      if (prim.type=='VIRT_KEY') {
        ensure(res==null && allowed, "res==null && allowed")  //Only 1 result, and VirtKeys are still allowed
        res = prim.value
      }

      if (prim.type=='VAR_NAME') {
        //Needs to be handled recursively
        res = this.get_primary_vkey(VARS[prim.value].rev_states, res, allowed) 
      }

      if (prim.type!='VIRT_KEY' && prim.type!='SWITCH' && prim.type!='VAR_NAME') {
        //Can't match a VirtKey unless it's at the end of the string
        allowed = false
      }
    }
    return res
  };
  
  this.start = this.combine_halves(
    build_state_tree(token.lhs, 'lhs', true),
    build_state_tree(token.rhs, 'rhs', false)
  );
  this.VkPress = this.get_primary_vkey(token.lhs, null, true);
}


function buildRulesArray() {
  RULES.length = 0;
  for (var i=0; i<rules.length; i++) {
    RULES.push(new Rule(rules[i]));
  }
}

function buildVarsArray() {
  VARS = new Object(); //Remove all keys
  for (var key in variables) {
    var prim_list = variables[key];
    VARS[key] = new Var({'name':key,'items':prim_list});
  }
}


function parse() {
  document.getElementById('result').innerHTML = '(<i>Processing...</i>)';
  document.getElementById('preParse').value = '';
  document.getElementById('parseTree').value = '';
  try {
    var preParsed = preParseText(document.getElementById('scrSource').value);
    document.getElementById('preParse').value = preParsed;
    parseText(preParsed);
    
    //Now, convert it to a state machine
	buildVarsArray();
    buildRulesArray();
  } catch (err) {
    document.getElementById('result').innerHTML = '<b>ERROR[Line: ' + err.line + ',Col: ' + err.column + ']</b>: ' + err;
	document.getElementById('result').innerHTML += '  Rules[' + RULES.length + '/' + rules.length + ']  Vars[' + count_keys(VARS) + '/' + count_keys(variables) +']';
    return;
  }
  document.getElementById('result').innerHTML = '<b>Done</b>';
}


