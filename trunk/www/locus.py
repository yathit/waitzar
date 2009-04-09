#!/usr/bin/env python

import cgi
import templet
import re


# Used for rendering when nothing else is present
class FallbackTemplate(templet.Template):
	template = r'''
		$<header_template>
		$<body_template>
		'''
	
	header_template = r'''
		<h1>Page Not Found</h1>
		'''

	body_template = r'''
		Sorry, the page you followed does not seem to exist.
		<br>Fortunately, the fact that you found this message (instead of a generic error page) means
		that the developers are working on adding this page right now. Please try back in a few hours,
		and you should see something new here.
		<div class="hrsep"><img src="img/hr.jpg"/></div>
	'''


# Load a class by name, from its module
def forname(modname, attname):
    return getattr(__import__(modname), attname)



# This works by pulling apart index.html and then subsituting a proper
#  python template (well, a templet...) based on what we are trying to render.
def render_a_page(pagename):
	# Try to load a properly-named class file with the given URL name; if not, 
	#  fall back to a generic error message.
	bodyTxt = None
	try:
		mtch = re.search('([a-z]+)\.py', pagename)
		if mtch:
			moduleName = mtch.group()
			className = mtch.group(0)[0].capitalize() + mtch.group(0)[1:] + Template
			classInst = forname(moduleName, className)
			bodyTxt = classInst()
	except:
		bodyTxt = FallbackTemplate()

	# Render tha page, inserting our text where the main div would be.
	fMain = open('index.html', 'r')
	showLines = 0
	for line in fMain:
		#Track our counter?
		if showLines > 0:
			if re.search('<div', line):
				showLines += 1
			if re.search('</div', line):
				showLines -= 1
		
		#Output this line?
		if showLines == 0:
			print line
		
		#Have we reached our special main div?
		if re.search('<div +id *= *"main"', line):
			print bodyTxt
			showLines = 1
