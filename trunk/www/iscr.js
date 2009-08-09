      //Pre-load the images, so that we don't see a broken link on the first mouse-over.
      // Borrowed from http://www.softouch.on.ca/scratch/js-roll2.htm
      if (document.images) {
        //(width, height)
        var img1=new Image(300,76);
        var img2=new Image(300,76);
        var img3=new Image(300,76);
        var img4=new Image(48,47);

        //Location
        img1.src="img/spotlight.png";
        img2.src="img/spotlight_over.png";
        img3.src="img/spotlight_down.png";
        img4.src="img/loading.gif";
      }


      //An array of our buttons
      var buttons=new Array (
        "img/spotlight.png",
        "img/spotlight_over.png",
        "img/spotlight_down.png"
      );


      //Swap image "name" with button id "item"
      function swapImage(name,item)
      {
        if (document.images) {
          document.images[name].src=buttons[item];
        }
      }

      //Shortcut handlers
      function highlightLink(name)
      {
        swapImage(name, 1);
        document.getElementById(name+"Txt").style.color = "yellow";
        document.getElementById(name+"Txt").style.textShadow = "#AA0000 -1.5px -1.5px 1.5px";

        return true;
      }
      function pressLink(name)
      {
        swapImage(name, 2);
        document.getElementById(name+"Txt").style.left = "2px";
        document.getElementById(name+"Txt").style.top = "2px";
        return true;
      }
      function releaseLink(name)
      {
        swapImage(name, 1);
        document.getElementById(name+"Txt").style.left = "0px";
        document.getElementById(name+"Txt").style.top = "0px";
        return true;
      }
      function leaveLink(name)
      {
        swapImage(name, 0);
        document.getElementById(name+"Txt").style.color = "#44FFAA";
        document.getElementById(name+"Txt").style.textShadow = "#000088 -1.5px -1.5px 1.5px";
        return true;
      }


      //Simple AJAX functionality
      function xmlhttpPost(strURL, formName) {
        if (formName == "wzRespForm") {
      		document.getElementById("submitButton").disabled = true;
	      	document.getElementById("submitButton").value = "Please Wait"
      		document.getElementById("submitLoader").innerHTML = "<img src=\"img/loading.gif\"/>";
      	} else if (formName == "wzNewSiteForm") {
      		document.getElementById("submitAddButton").disabled = true;
	      	document.getElementById("submitAddButton").value = "Please Wait"
      		document.getElementById("submitAddLoader").innerHTML = "<img src=\"img/loading.gif\"/>";
      	}

        var xmlHttpReq = false;
        var self = this;
        var params = getquerystring(formName);
        if (window.XMLHttpRequest) {
          //Mozilla/Safari
          self.xmlHttpReq = new XMLHttpRequest();
        } else if (window.ActiveXObject) {
          //IE
          self.xmlHttpReq = new ActiveXObject("Microsoft.XMLHTTP");
        }
        self.xmlHttpReq.open('POST', strURL, true);
        self.xmlHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
		//self.xmlHttpReq.setRequestHeader("Content-length", params.length);
		//self.xmlHttpReq.setRequestHeader("Connection", "close");

        self.xmlHttpReq.onreadystatechange = function() {
          if (self.xmlHttpReq.readyState == 4) {
            updatepage(self.xmlHttpReq.responseText);
          }
        }
        self.xmlHttpReq.send(params);
      }

      function xmlhttpGet(strURL) {
        document.getElementById("main").innerHTML = "<img src=\"img/loading.gif\"/>";

        var xmlHttpReq = false;
        var self = this;
        if (window.XMLHttpRequest) {
          //Mozilla/Safari
          self.xmlHttpReq = new XMLHttpRequest();
        } else if (window.ActiveXObject) {
          //IE
          self.xmlHttpReq = new ActiveXObject("Microsoft.XMLHTTP");
        }
        self.xmlHttpReq.open('GET', strURL+"?partial=true", true);
        self.xmlHttpReq.onreadystatechange = function() {
          if (self.xmlHttpReq.readyState == 4) {
            updatepage(self.xmlHttpReq.responseText);
          }
        }
        self.xmlHttpReq.send("");
      }

      function getquerystring(formName) {
        var form     = document.forms[formName];
        
        //Here we go...
        var name = "";
	var email = "";
        var comments = "";
	if (formName == "wzRespForm") {
		name = form.userName.value;
		email = form.userEmail.value;
		comments = form.comments.value;
	} else if (formName == "wzNewSiteForm") {
		//Make a nice JSON string
		var jsonObj = {
		   "name" : form.webSiteName.value,
		   "link" : form.webSiteURL.value
		};
		if (form.webSiteImage.value.length>0) {
			jsonObj['img'] = form.webSiteImage.value;
		}

	
		name = "WZ Site Add Request"
		email = "";
		comments = "Site Name: " + form.webSiteName.value + "\n" 
			 + "Site URL: " + form.webSiteURL.value + "\n" 
			 + "Site Image: " + form.webSiteImage.value + "\n\n"
			 + "JSON Line: " + jsonObj.toJSONString() + "\n";
      	}
        
        if (email.length==0) {
          email = "help@waitzar.com"
        }

        // NOTE: no '?' before querystring
        qstr = 'name=' + escape(name) + '&email=' + escape(email) + '&comments=' + escape(comments) + '&partial=yes';
        return qstr;
      }

      function updatepage(str){
        document.getElementById("main").innerHTML = str;
      }


			//This script is used by contactus.py, since AJAX-ing it doesn't seem to work.
		  //Set the text of our labels based on the context
		  function setLabels(){
		    if (document.getElementById("requestType").selectedIndex == 0) {
				document.getElementById("commentsTag").innerHTML = "Please type your feedback here:";
				document.getElementById("emailTag").innerHTML = "Your Email Address: (optional)";
				document.getElementById("submitButton").value = "Submit Feedback";
		    } else {
				document.getElementById("commentsTag").innerHTML = "Please describe what you need help with:";
				document.getElementById("emailTag").innerHTML = "Your Email Address:";
				document.getElementById("submitButton").value = "Ask For Help";
		    }
		  }

		  var errorCount = 0;
		  var errorAddCount = 0;
		  var errorRemoveCount = 0;
		  
		  function validateAddRequest() {
		    //Reset all borders
		  	errorAddCount = 0;

		  	if (document.getElementById("webSiteName").value.length==0) {
  		  		document.getElementById("webSiteName").style.border = "3px solid red";
  		  		document.getElementById("webSiteName").style.backgroundColor = "#FFCCCC";
  		  		document.getElementById("webSiteName").value = "Please enter a name for this web site.";
  		  		errorAddCount = errorAddCount + 1;
		  	}
		  	
		  	if (document.getElementById("webSiteURL").value.length==0) {
  		  		document.getElementById("webSiteURL").style.border = "3px solid red";
  		  		document.getElementById("webSiteURL").style.backgroundColor = "#FFCCCC";
  		  		document.getElementById("webSiteURL").value = "Please enter this web site's URL.";
  		  		errorAddCount = errorAddCount + 1;
		  	}
		  	
		    	if (errorAddCount==0) {
				xmlhttpPost("sitelinks.py", "wzNewSiteForm");
			} else {
				document.getElementById("submitAddButton").disabled = true;
			}
		  }
		  
		  function unRedAddElement(elem) {
		    if (document.getElementById(elem).style.border.match("red")) {
				document.getElementById(elem).style.border = "";
				document.getElementById(elem).style.backgroundColor = "";
				document.getElementById(elem).value = "";
				errorAddCount = errorAddCount - 1;
			}

		    if (errorAddCount==0) {
				document.getElementById("submitAddButton").disabled = false;
			}
		  }
		  
		  
		  function validateRemoveRequest() {
		  
		  }


		  function validateRequest() {
		    //Reset all borders
		  	errorCount = 0;

		  	if (document.getElementById("comments").value.length==0) {
  		  		document.getElementById("comments").style.border = "3px solid red";
  		  		document.getElementById("comments").style.backgroundColor = "#FFCCCC";
  		  		document.getElementById("comments").value = "Please enter a brief description.";
  		  		errorCount = errorCount + 1;
		  	}

		  	if (document.getElementById("userName").value.length==0) {
  		  		document.getElementById("userName").style.border = "3px solid red";
  		  		document.getElementById("userName").style.backgroundColor = "#FFCCCC";
  		  		document.getElementById("userName").value = "Please type your name.";
  		  		errorCount = errorCount + 1;
		  	}

		  	if (document.getElementById("userEmail").value.length==0 && document.getElementById("requestType").selectedIndex == 1) {
  		  		document.getElementById("userEmail").style.border = "3px solid red";
  		  		document.getElementById("userEmail").style.backgroundColor = "#FFCCCC";
  		  		document.getElementById("userEmail").value = "Please type your email.";
  		  		errorCount = errorCount + 1;
		  	}



		    if (errorCount==0) {
				xmlhttpPost("contactus.py", "wzRespForm");
			} else {
				document.getElementById("submitButton").disabled = true;
			}
		  }


		  function unRedElement(elem) {
		    if (document.getElementById(elem).style.border.match("red")) {
				document.getElementById(elem).style.border = "";
				document.getElementById(elem).style.backgroundColor = "";
				document.getElementById(elem).value = "";
				errorCount = errorCount - 1;
			}

		    if (errorCount==0) {
				document.getElementById("submitButton").disabled = false;
			}
		  }
