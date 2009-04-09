#!/usr/bin/env python

import cgi


# We need this file to be as simple as possible, so that all 
#  normal errors are caught and printed. We do this by 
#  giving it only one method, only one import, and one print
#  statement before the "try" branch
def safe_render(pagename):
	print "Content-Type: text/html\n"
	try:
		import locus
		render_a_page(pagename)
	except:
		print "<html><head><title>Server-Side Error</title></head>\n<body>"
		print "<!-- --><h1>Sorry, An Error Occurred</h1>\n"
		print "Please try back in a few minutes; we are working hard to fix this problem.\n"
		print "<br>&nbsp;\n"
		print "<br>&nbsp;\n"
		print "<br>&nbsp;\n"
		print "<h1 style=\"color:#999999;\">Error Specifics</h1>\n<span style=\"color:#996666;\">"
		
		#Print our error in a safe way
		cgi.print_exception()
		
		print "</span></body></html>"
