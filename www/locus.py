#!/usr/bin/env python

import cgi
import templet
import re


# This works by pulling apart index.html and then subsituting a proper
#  python template (well, a templet...) based on what we are trying to render.
def render_a_page(pagename):
	fMain = open('index.html', 'r')
	showLines = 0
	for line in fMain:
		#Track our counter?
		if showLines > 0:
			if re.search('<div', line):
				showLines += 1
			elif re.search('</div', line):
				showLines -= 1
		
		#Output this line?
		if showLines == 0:
			print line
		
		#Have we reached our special main div?
		if re.search('<div +id *= *"main"', line):
			showLines = 1
