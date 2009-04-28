from locus import FallbackTemplate

class ContactusTemplate(FallbackTemplate):
	template = r'''
        <h1>Contact Us</h1>
        Thank you for using WaitZar. Please let us know if you have any questions, or any suggestions. You can email <a href="mailto:help@waitzar.com">help@waitzar.com</a>, or just fill in the form below.

		<form name="wzRespForm">
			<table border="0" rules="none" cellpadding="10" cellspacing="0" style="margin-top:20px; margin-left: 10px;">
				<tr>
					<td width="1" class="gll glt"><img src="img/wz_icon.png" width="48" height="48"/></td>
					<td width="200" class="glt glr">
						<span style="font-size: 12px;">Why are you contacting us?</span>
						<br><select id="requestType" name="requestType" style="width:160;" onchange='javascript:setLabels();'>
							<option value="comment" selected>Feedback</option>
							<option value="help">Help Request</option>
						</select>
					</td>
					<td class="glb">&nbsp;</td>
					<td class="glb">&nbsp;</td>
				</tr>
				<tr>
					<td width="260" class="gll glb" colspan="3" rowspan="3">
						<span id="commentsTag" style="font-size: 12px;">Please type your feedback here:</span>
						<textarea id="comments" onmousedown="unRedElement('comments');" name="comments" cols="40" rows="10"></textarea>
					</td>
					<td class="glr" style="height="1">
						<span id="nameTag" style="font-size: 12px;">Your Name:</span>
						<br><input id="userName"  onmousedown="unRedElement('userName');"  name="userName" type="text" style="width: 200px;"/>
					</td>
				</tr>
				<tr>
					<td class="glr" style="height="1">
						<span id="emailTag" style="font-size: 12px;">Your Email Address: (optional)</span>
						<br><input id="userEmail"  onmousedown="unRedElement('userEmail');"  name="userEmail" type="text" style="width: 200px;"/>
					</td>
				</tr>
				<tr>
					<td class="glr glb" valign="middle" align="center">
						<input style="font-size: 18px;" id="submitButton" value="Submit Feedback" type="button" onclick='JavaScript:validateRequest();'/>
						<div id="submitLoader"></div>
					</td>
				</tr>
			</table>
		</form>
        $<hrule_template>
		'''
		
	def reloadText(self, name, invalidEmail, emailServerError):
		#Form post-back
		head = "Message Sent"
		if invalidEmail or emailServerError:
			head = "Error Sending Message"
			
		body = "Thanks for contacting us, " + name + ". We will read your message as soon as we can."
		if invalidEmail:
			body = "The email address that you specified was invalid."
		elif emailServerError:
			body = "The feedback server is currently experiencing some difficulties."
		if invalidEmail or emailServerError:
			body += "<br>Your email could not be sent.Why not try contacting <a href=\"mailto:help@waitzar.com\">help@waitzar.com</a> directly?"
		ret = "<h1>" + head + "</h1>\n" + body +"<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;"
		
		return ret
