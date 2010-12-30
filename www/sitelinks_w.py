from locus import FallbackTemplate
from json import *

def onlyascii(char):
	if (ord(char) > 128):
		return 'U+' + hex(ord(char))[2:]
	elif (ord(char) < 48 or ord(char) > 127) and char!='_': 
		return ''
	else: 
		return char

class SitelinksTemplate(FallbackTemplate):
	template = u'''
        <h1>Sites That Use WaitZar</h1>

        Quite a lot of people already use WaitZar for their web sites and web logs. Here's a list of the ones we know. Please add your favorite pages to the bottom of the list; our only requirement is that they display the WaitZar sticker on the main page of their site.

        $<hrule_template>

        <h1 style="margin-top: 0px;">Web Sites</h1>

	${{self.generateLinksTable()}}

        $<hrule_template>


	<h1 style="margin-top: 0px;">Suggest a Site to Add</h1>

	If you do not see your favorite site on the list, please add it! We only have two requirements: first, each web site on the list <b>must</b> showcase the WaitZar sticker on their main page, and second, the site must not contain any adult content or spyware.

        <br>&nbsp;<br>To add the WaitZar sticker to your site, use the following html code:
        <br>
        <table border="0"><tr><td>
			<div style="border: 1px solid white; padding:10px; margin-top:10px; width:auto; font-family:monospace; font-size:11pt;">
				&lt;a href="http://www.waitzar.com/"&gt;
				<br>&nbsp;&nbsp;&nbsp;&lt;img src="http://waitzar.googlecode.com/files/wz_usericon.png"/&gt;
				<br>&lt;/a&gt;
			</div>
        </td></tr></table>
        
        <br>After adding the WaitZar sticker, click on this button and tell us about your site, so that we can add you to the list.

		<br><button onclick="document.getElementById('wzNewSiteDiv').style.display='inline';this.style.display='none';">
			<table border="0"><tr>
				<td align="left"><img src="img/btn_add.png"/></td>
				<td valign="middle">&nbsp;&nbsp;&nbsp;Suggest a web site</td>
			</tr></table>
		</button>
		<div id="wzNewSiteDiv" name="wzNewSiteDiv" style="display:none;">
		<form id="wzNewSiteForm" name="wzNewSiteForm">
			<table border="0" rules="none" cellpadding="10" cellspacing="0" style="margin-top:20px; margin-left: 10px;">
				<tr>
					<td width="1" class="gll glt"><img src="img/wz_icon.png" width="48" height="48"/></td>
					<td width="200" class="glt glr">
						<span style="font-size: 12px;">Why are you contacting us?</span>
						<br><select id="requestType" name="requestType" style="width:160;" disabled>
							<option value="comment" selected>Suggesting a Site</option>
						</select>
					</td>
					<td class="glb">&nbsp;</td>
					<td class="glb">&nbsp;</td>
				</tr>
				<tr>
					<td width="260" class="gll" colspan="2">
						<span style="font-size: 12px;">Name of web site:</span>
						<br><input id="webSiteName" class="pdk" onmousedown="unRedAddElement('webSiteName');"  name="webSiteName" type="text" style="width: 230px;" onchange="document.getElementById('previewName').innerHTML=this.value;"/>
					</td>
					<td class="glr glb" height="1" rowspan="4" colspan="2" valign="top">
					Submission Preview:
						<table border="0" cellpadding="0" cellspacing="0" class="helptbl" width="1">
							<tr style="background:#333333;">
								<td width="1" valign="top" align="left"  class="pl10 pt10" style="border-left: 4px solid #AAA;">
									<a class="ninjalink" href="http://www.x.com">
										<div style="width:80px; height:80px; padding-bottom:10px;">
											<img id="previewImage" name="previewImage" src="" border="0"
											style="display: block; margin-left: auto; margin-right: auto;
																	max-width:80px;max-height:80px;
																	width: expression(this.width > 80 ? 80: true);
																	height: expression(this.height > 80 ? 80: true);
																	visibility:hidden;"/>
										</div>
									</a>
								</td>
								<td width="100%" valign="top" align="left"  class="pl10 pt10" style="padding-right:50px; color:#CCC;">
									<a class="ninjalink" href="http://www.x.com">
										<span class="helpbigtxt pdk" id="previewName" name="previewName"></span>
									</a>
									<br><span id="previewURL" name="previewURL"></span>
								</td>
							</tr>
						</table>
						<div>
							<table style="margin-left:auto; margin-right:auto;"><tr><td>
								<br>&nbsp;
								<input style="font-size: 18px;" id="submitAddButton" value="Submit Web Site" type="button" onclick='JavaScript:validateAddRequest();'/>
								<div id="submitAddLoader"></div>
							</td></tr></table>
						</div>
					</td>
				</tr>
				<tr>
					<td width="260" class="gll" colspan="2">
						<span style="font-size: 12px;">URL of main page:</span>
						<br><input id="webSiteURL"  onmousedown="unRedAddElement('webSiteURL');"  name="webSiteURL" type="text" style="width: 280px;" onchange="document.getElementById('previewURL').innerHTML=this.value;"/>
					</td>
				</tr>
				<tr>
					<td width="260" class="gll" colspan="2">
						<span style="font-size: 12px;">URL of site logo: (optional)</span>
						<br><input id="webSiteImage"  name="webSiteImage" type="text" style="width: 280px;" onchange="document.getElementById('previewImage').src=this.value; document.getElementById('previewImage').style.visibility=this.value.length==0?'hidden':'visible';"/>
					</td>
				</tr>
				<tr>
					<td class="gll glb" colspan="2">&nbsp;
					</td>
				</tr>
			</table>
		</form>
		</div>


		$<hrule_template>



		<h1 style="margin-top: 0px;">Report a Site for Removal</h1>

		If we listed a site which does not display the WaitZar sticker, or which is no longer online, please let us know so we can remove it. We'd like to keep this list up-to-date!

		<br>&nbsp;
		<br><button onclick="document.getElementById('wzRemoveSiteDiv').style.display='inline';this.style.display='none';">
			<table border="0"><tr>
				<td align="left"><img src="img/btn_remove.png"/></td>
				<td valign="middle">&nbsp;&nbsp;&nbsp;Remove a web site</td>
			</tr></table>
		</button>
		<div id="wzRemoveSiteDiv" name="wzRemoveSiteDiv" style="display:none;">
		<form id="wzRemoveSiteForm" name="wzRemoveSiteForm">
			<table border="0" rules="none" cellpadding="10" cellspacing="0" style="margin-top:20px; margin-left: 10px;">
				<tr>
					<td width="1" class="gll glt"><img src="img/wz_icon.png" width="48" height="48"/></td>
					<td width="200" class="glt glr">
						<span style="font-size: 12px;">Why are you contacting us?</span>
						<br><select id="requestType" name="requestType" style="width:160;" disabled>
							<option value="comment" selected>Site Removal Request</option>
						</select>
					</td>
					<td class="glb">&nbsp;</td>
					<td class="glb">&nbsp;</td>
				</tr>
				<tr>
					<td width="260" class="gll" colspan="2">
						<span style="font-size: 12px;">Site to Remove:</span>
						<br><select id="removeSite" onmousedown="unRedRemoveElement('removeSite');" name="removeSite" class="pdk" style="width:160;">
							<option selected value="none">Please choose...</option>
							${{self.generateRemovalLists()}}
						</select>
					</td>
					<td class="glr glb" height="1" rowspan="3" colspan="2" valign="top">
						<div>
							<table style="margin-left:auto; margin-right:auto;"><tr><td>
								<br>&nbsp;
								<input style="font-size: 18px;" id="submitRemButton" value="Submit Removal Request" type="button" onclick='JavaScript:validateRemoveRequest();'/>
								<div id="submitRemoveLoader"></div>
							</td></tr></table>
						</div>
					</td>
				</tr>
				<tr>
					<td width="260" class="gll" colspan="2">
						<span style="font-size: 12px;">Reason for Removal:</span>
						<br><select id="removalReason" onmousedown="unRedRemoveElement('removalReason');" name="removalReason" style="width:160;">
							<option selected value="none">Please choose...</option>
							<option value="nosticker">No WaitZar Sticker</option>
							<option value="sitedown">Site Is Down (404)</option>
							<option value="spyware">Site Contains Spyware</option>
							<option value="adultcontent">Site Contains Adult Content</option>
							<option value="other">Other...</option>
						</select>
					</td>
				</tr>
				<tr>
					<td width="260" class="gll glb" colspan="2">
						<span style="font-size: 12px;">Additional Information: (optional)</span>
						<br><textarea id="removalAdditionalInfo"  name="webSiteImage" type="textarea" cols="40" rows="3"></textarea>
					</td>
				</tr>
			</table>
		</form>
		</div>



		$<hrule_template>
		'''	
	
	
	def readLinksFromFile(self):
		#We need to concatenate all lines in the wz_websites file, and then
		#   use our json parser on it
		f = open('wz_websites.txt', 'r')
		allLines = string.join(f.readlines(), ' ')
		return JsonReader().read(allLines)
	
	
	def generateRemovalLists(self):
		#Get our objects
		ourLinks = self.readLinksFromFile()
	
		#Write each option
		for line in ourLinks:
			#Manage hash
			name = 'Error'
			if 'name' in line:
				name = line['name']
			temp = string.replace(string.lower(name), " ", "_")
			namesmall = filter(onlyascii, temp)
			
			self.out.append(self.tabs(5) + '<option value="%s">%s</option>' % (namesmall, name))
		
	
	def tabs(self, amt):
		return '\n' + '\t'*amt

	def generateLinksTable(self):
		#Get our objects
		ourLinks = self.readLinksFromFile()

		#General template start
		self.out.append(self.tabs(2) + '<table border="0" cellpadding="0" cellspacing="0" class="helptbl" width="75%" style="margin-left:70px;">')

		#Print each row
		colorID = 2
		for line in ourLinks:
			#Manage hash - Doesn't work server side...
			#name = line['name'] if 'name' in line else 'Error'
			#link = line['link'] if 'link' in line else 'Error'
			#img = line['img'] if 'img' in line else ''
			
			#Manage hash
			name = 'Error'
			if 'name' in line:
				name = line['name']
			link = 'Error'
			if 'link' in line:
				link = line['link']
			img = ''
			if 'img' in line:
				img = line['img']
			

			#Increment colors -why aren't ternary operators working?
			if colorID==1:
				colorID = 2
			else:
				colorID = 1
			#colorID = 2 if colorID==1 else 2

			#Row
			self.out.append(self.tabs(3) + '<tr class="sitelinkrow%d">' % (colorID,))

			#Cell 1
			self.out.append(self.tabs(4) + '<td width="1" valign="top" align="left"  class="pl10 pt10 sitelinkbord%d">' % (colorID,))
			self.out.append(self.tabs(5) + '<a class="ninjalink" href="%s">' % (link,))
			self.out.append(self.tabs(6) + '<div class="linkimgbkgrd">')
			if len(img) > 0:
				self.out.append(self.tabs(7) + '<img src="%s" border="0" class="linkimgfore" ' % (img,))
				self.out.append(self.tabs(7) + 'style="width: expression(this.width > 80 ? 80: true); height: expression(this.height > 80 ? 80: true);"/>')
			else:
				self.out.append(self.tabs(7) + '&nbsp;')
			self.out.append(self.tabs(6) + '</div>')
			self.out.append(self.tabs(5) + '</a>')
			self.out.append(self.tabs(4) + '</td>')

			#Cell 2
			self.out.append(self.tabs(4) + '<td width="100%" valign="top" align="left"  class="pl10 pt10 sitelinkname">')
			self.out.append(self.tabs(5) + '<a class="ninjalink" href="%s">' % (link,))
			self.out.append(self.tabs(6) + '<span class="helpbigtxt pdk">%s</span>' % (name,))
			self.out.append(self.tabs(5) + '</a>')
			self.out.append(self.tabs(5) + '<br>%s' % (link,))
			self.out.append(self.tabs(4) + '</td>')

			#End row
			self.out.append(self.tabs(3) + '</tr>')

		#General template end
		self.out.append(self.tabs(2) + '</table>')

	
	def reloadText(self, name, invalidEmail, emailServerError):
		#Form post-back
		head = "Request Sent"
		if invalidEmail or emailServerError:
			head = "Error Sending Message"
			
		body = "We will respond to your request soon; thanks again. (We have to check all requests to make sure that we avoid spam.)"
		if emailServerError:
			body = "The feedback server is currently experiencing some difficulties."
		if emailServerError:
			body += "<br>Your email could not be sent.Why not try contacting <a href=\"mailto:help@waitzar.com\">help@waitzar.com</a> directly?"
		ret = "<h1>" + head + "</h1>\n" + body +"<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;"
		
		return ret
