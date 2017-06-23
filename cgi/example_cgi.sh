#!/bin/sh
# This is an example cgi written in bash. 
#
# Motivation:
# * If you want to make your own web-site out of this
#   you might want to write cgi's in a different language than C. 
#
# Notes:
# * Every HTTP header will be stored in capital letter variables without the dash,'-', letter. 
# 	You can use URI encoded data by parsing the $PATH variable.
# 	When you end the header section remeber that echo prints outputs \n in the end. 
# 	If you enter arguments to the URI, do it like this: 
# 	if your file is called 'example.sh', then request the following: "example.sh?[uri-data]"
# 	The question mark is needed for the server to distinguish between the data and the file path.
# 	If you don't include it you will get 404 not founds. 
#
#	All arguments passed after the 'path?arg1=value1&arg2=value2 will be
#	accessible with the environment variables ARG1 and ARG2 respectively (note capital letters)
#	All HTTP forms will also be accessible with in the same manner, with capital letters, e.g. COOKIE
# 	
# Have fun! :) 

echo "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r"
echo "Hello! You entered the path: $PATH"
echo "You have the cookie: $COOKIE" 