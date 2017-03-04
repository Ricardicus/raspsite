# This is an example cgi written in python. 
#
# Motivation:
# * If you want to make your own web-site out of this
#   you might want to write cgi's in a different language than C. 
#
# Notes:
# * Every HTTP header will be stored under the os.environ map
# 	You can use URI encoded data by parsing the os.environ["path"] variable.
# 	When you end the header section remeber that print outputs \n in the end. 
# 	If you enter arguments to the URI, do it like this: 
# 	if your file is called 'example.py', then request the following: "example.py?[uri-data]"
# 	The question mark is needed for the server to distinguish between the data and the file path.
# 	If you don't include it you will get 404 not founds. 
# 	
# Have fun! :) 

import os

print "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r"
print "Hello! You entered the path: " + os.environ["path"]
