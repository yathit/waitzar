#!/usr/bin/env python

import cgi
import os
import templet
import re


# Used for rendering when nothing else is present
class FallbackTemplate(templet.Template):
	template = r'''
		$<header_template>
		$<body_template>
		$<hrule_template>
		'''
	
	header_template = r'''
		<h1>Page Not Found</h1>
		'''

	body_template = r'''
		Sorry, the page you followed does not seem to exist.
		<br>Fortunately, the fact that you found this message (instead of a generic error page) means
		that the developers are working on adding this page right now. Please try back in a few hours,
		and you should see something new here.
	'''
	
	hrule_template = r'''
		<div class="hrsep"><img src="img/hr.jpg"/></div>
	'''


# This works by pulling apart index.html and then subsituting a proper
#  python template (well, a templet...) based on what we are trying to render.
def render_a_page(pagename):
	#Read field-storage ONCE
	fields = cgi.FieldStorage()

	# Try to load a properly-named class file with the given URL name; if not, 
	#  fall back to a generic error message.
	bodyTxt = None
	try:
		mtch = re.search('([a-z]+)\.py', pagename)
		if mtch:
			moduleName = mtch.group(1)
			className = moduleName[0].capitalize() + moduleName[1:] + "Template"

			classInst = getattr(__import__(moduleName + "_w"), className)
			if fields.has_key("name") and fields.has_key("comments") and fields.has_key("email"):
				bodyTxt = classInst.reloadText(classInst(), fields.getfirst('name'), fields.getfirst('email'), fields.getfirst('comments'))
			else:
				bodyTxt = classInst()

		else:
			raise Exception
	except (AttributeError, ImportError):
		bodyTxt = FallbackTemplate()
		
	if fields.has_key("partial"):
		# Return only the necessary part with AJAX
		print bodyTxt
	else:
		#DEBUG
		if False:
			print "REQUEST_METHOD:", os.environ["REQUEST_METHOD"] , "<br>"
			print "Values: <br>"
			f = fields
			for k in f.keys():
				print "%s: %s<br>" % (k, f.getfirst(k))
			print "Done<br>"
			return
	
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
