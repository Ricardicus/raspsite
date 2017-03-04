This is a directory containing a program that is intended to be run 
by the client. The program can communicate with the server, set up
a TCP connection and transfer a file using the custom FTP protocol. 

Steps:
	1. Compile the program with the command ”make” in this directory.
	2. Run it accordingly ”./write [server-IP] [file name] [received file name]”

[server-IP]: the IP of the server running the raspserver
[file name]: The name of the file you want to send (argument will be passed to ’fopen’)
[received file name]: What the name shall be on the server. It will be placed under the directory ”downloads” on the server. If a file with the same name exists it will be overwritten. 

If the server is responding than you can transfer files with this one! :) 