from locus import FallbackTemplate

class ContactusTemplate(FallbackTemplate):
	template = r'''
        <h1>Contact Us</h1>
        Thank you for using WaitZar. Please let us know if you have any questions, or any suggestions. You can email <a href="mailto:help@waitzar.com">help@waitzar.com</a>, or just fill in the form below.

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

