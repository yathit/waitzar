#!/usr/bin/env python

import smtplib
from string import *


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
			smtpObj.set_debuglevel(1)
			smtpObj.sendmail(fromEmail, toList, fullMsg)
		except Exception:
			print "EXCEPTION"
			return SENDRESP_SERVER_DOWN
	else:
		return SENDRESP_BAD_EMAIL_ADDRESS
	return SENDRESP_SUCCESS
