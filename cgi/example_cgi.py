# This is an example cgi written in python. 
#
# Motivation:
# * If you want to make your own web-site out of this
#   you might want to write cgi's in a different language than C. 
#
# Notes:
# * Every HTTP header will be stored in capital letters under the os.environ map
# 	You can use URI encoded data by parsing the os.environ["path"] variable.
# 	When you end the header section remeber that print outputs \n in the end. 
# 	If you enter arguments to the URI, do it like this: 
# 	if your file is called 'example.py', then request the following: "example.py?[uri-data]"
# 	The question mark is needed for the server to distinguish between the data and the file path.
# 	If you don't include it you will get 404 not founds. 
# 	
#	All arguments passed after the 'path?arg1=value1&arg2=value2 will be
#	accessible with the values ARG1 and ARG2 respectively (note capital letters) into the os.environ maps.
#	All HTTP forms will also be accessible with in the same manner, with capital letters, e.g. COOKIE
#
# Have fun! :)

import os

print "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r"
print "Hello! You entered the path: " + os.environ["URL"]
print "I can also see that your browser has provided the following cookie: " + os.environ["COOKIE"]
