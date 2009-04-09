#!/usr/bin/env python

import cgi
import templet
import re


# This works by pulling apart index.html and then subsituting a proper
#  python template (well, a templet...) based on what we are trying to render.
def render_a_page(pagename):
	fMain = open('index.html', 'r')
	showLines = 0
	for lin in fMain:
		if showLines == 0:
			print line
			if re.search('<div +id *= *"main"'):
				showLines = 1
		elif re.search('<div'):
			showLines++
		elif re.search('</div'):
			showLines--
			if showLines==0:
				print line
