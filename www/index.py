#!/usr/bin/env python

import cgi

def main():
	print "Content-Type: text/html\n"
	try:
		import locus
	except:
		print "<html><head><title>Server-Side Error</title></head>\n<body>"
		print "<!-- --><h1>Sorry, An Error Occurred</h1>\n"
		print "Please try back in a few minutes; we are working hard to fix this problem.\n"
		print "<br>&nbsp;\n"
		print "<br>&nbsp;\n"
		print "<br>&nbsp;\n"
		print "<h1 style=\"color:#999999;\">Error Specifics</h1>\n<span style=\"color:#999999; font=weight:bold;\">"
		
		#Print our error in a safe way
		cgi.print_exception()
		
		print "</span></body></html>"


if __name__ == '__main__':
	main()
