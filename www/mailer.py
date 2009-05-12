#!/usr/bin/env python

import smtplib
from email.mime.text import MIMEText
from string import *
import email.Charset
email.Charset.add_charset( 'utf-8', email.Charset.SHORTEST, None, None )



# Ported from Recipe 3.9 in Secure Programming Cookbook for C and C++ by
# John Viega and Matt Messier (O'Reilly 2003)
rfc822_specials = '()<>@,;:\\"[]'
def isAddressValid(addr):
    # First we validate the name portion (name@domain)
    c = 0
    while c < len(addr):
        if addr[c] == '"' and (not c or addr[c - 1] == '.' or addr[c - 1] == '"'):
            c = c + 1
            while c < len(addr):
                if addr[c] == '"': break
                if addr[c] == '\\' and addr[c + 1] == ' ':
                    c = c + 2
                    continue
                if ord(addr[c]) < 32 or ord(addr[c]) >= 127: return 0
                c = c + 1
            else: return 0
            if addr[c] == '@': break
            if addr[c] != '.': return 0
            c = c + 1
            continue
        if addr[c] == '@': break
        if ord(addr[c]) <= 32 or ord(addr[c]) >= 127: return 0
        if addr[c] in rfc822_specials: return 0
        c = c + 1
    if not c or addr[c - 1] == '.': return 0

    # Next we validate the domain portion (name@domain)
    domain = c = c + 1
    if domain >= len(addr): return 0
    count = 0
    while c < len(addr):
        if addr[c] == '.':
            if c == domain or addr[c - 1] == '.': return 0
            count = count + 1
        if ord(addr[c]) <= 32 or ord(addr[c]) >= 127: return 0
        if addr[c] in rfc822_specials: return 0
        c = c + 1

    return count >= 1



SENDRESP_SUCCESS = 0
SENDRESP_BAD_EMAIL_ADDRESS = 1
SENDRESP_SERVER_DOWN = 2
def sendAMail(fromEmail, frName, toEmail, message):
	if (isAddressValid(fromEmail) and isAddressValid(toEmail)):
		toList = [toEmail]

		fullMsg =  "From: " + frName + " <" + fromEmail + ">\n" \
			 + "To: WaitZar Help <" + toEmail + ">\n" \
			 + "Subject: WZ Feedback and Help Request\n\n" \
			 + message

		try:
			smtpObj = smtplib.SMTP('localhost')
			smtpObj.sendmail(fromEmail, toList, fullMsg)
			smtpObj.quit()
		except Exception:
			return SENDRESP_SERVER_DOWN
	else:
		return SENDRESP_BAD_EMAIL_ADDRESS
	return SENDRESP_SUCCESS


def sendAHelpResponse(toEmail, toName, subject, msgFile):
	# Open a plain text file for reading; should be UTF-8
	fp = open(msgFile, 'rb')
	fromEmail = "help@waitzar.com"
	
	# Create a text/plain message
	msg = MIMEText(fp.read())
	fp.close()

	#Prepare to send the message
	msg['Subject'] = subject
	msg['From'] = "WaitZar Help <" + fromEmail + ">"
	msg['To'] = toName + " <" + toEmail + ">"

	# Send the message via our own SMTP server, but don't include the envelope header.
	s = smtplib.SMTP('localhost')
	s.sendmail(fromEmail, [toEmail], msg.as_string())
	s.quit()
