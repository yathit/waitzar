#!/usr/bin/env python

import cgi


#Render our page
execfile("index.py")
safe_render(__file__)


#Doing it this way kind of gets around our security trick, unfortunately...
#  However, it only omits syntax errors, which we should be checking on the command
#  line anyway. It still catches runtime errors properly, to some degree.
import locus
class DownloadTemplate(locus.FallbackTemplate):
	template = r'''
		<h1>Download WaitZar</h1>
		In progress...
		'''


