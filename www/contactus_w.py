from locus import FallbackTemplate

class ContactusTemplate(FallbackTemplate):
	template = r'''
        <h1>Contact Us</h1>
        Thank you for using WaitZar. Please let us know if you have any questions, or any suggestions. You can email <a href="mailto:help@waitzar.com">help@waitzar.com</a>, or just fill in the form below.

		<script type="text/javascript" language="javascript">
		<!--
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

		//-->
		</script>

		<form name="wzRespForm">
			<table border="0" rules="none" cellpadding="10" cellspacing="0" style="margin-top:20px; margin-left: 10px;">
				<tr>
					<td width="1" style="border-left:1px solid green; border-top:1px solid green;"><img src="img/wz_icon.png" width="48" height="48"/></td>
					<td width="200" style="border-top:1px solid green; border-right:1px solid green;">
						<span style="font-size: 12px;">Why are you contacting us?</span>
						<br><select id="requestType" name="requestType" style="width:160;" onchange='javascript:setLabels();'>
							<option value="comment" selected>Feedback</option>
							<option value="help">Help Request</option>
						</select>
					</td>
					<td style="border-bottom:1px solid green;">&nbsp;</td>
					<td style="border-bottom:1px solid green;">&nbsp;</td>
				</tr>
				<tr>
					<td width="260" style="border-left:1px solid green; border-bottom:1px solid green;" colspan="3" rowspan="3">
						<span id="commentsTag" style="font-size: 12px;">Please type your feedback here:</span>
						<textarea id="comments" onmousedown="unRedElement('comments');" name="comments" cols="40" rows="10"></textarea>
					</td>
					<td style="border-right:1px solid green;" height="1">
						<span id="nameTag" style="font-size: 12px;">Your Name:</span>
						<br><input id="userName"  onmousedown="unRedElement('userName');"  name="userName" type="text" style="width: 200px;"/>
					</td>
				</tr>
				<tr>
					<td style="border-right:1px solid green;" height="1">
						<span id="emailTag" style="font-size: 12px;">Your Email Address: (optional)</span>
						<br><input id="userEmail"  onmousedown="unRedElement('userEmail');"  name="userEmail" type="text" style="width: 200px;"/>
					</td>
				</tr>
				<tr>
					<td style="border-right:1px solid green; border-bottom:1px solid green;" valign="middle" align="center">
						<input style="font-size: 18px;" id="submitButton" value="Submit Feedback" type="button" onclick='JavaScript:validateRequest();'/>
					</td>
				</tr>
			</table>
		</form>
        $<hrule_template>
		'''

