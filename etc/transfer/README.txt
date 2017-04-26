This is a directory containing programs that are intended to be run 
by a client. The are two programs here that can communicate with the server.
The write program can set up a TCP connection and transfer a file using my own 
FTP protocol (RFTP, as in Ricardicus Fantastic Transfer Protocol). 
The session program can set up a TCP connection and use my own SSH
protocol (RSSH, as in RSSH not Secure Shell) implemented on the server side.

Set up:
	Compile the program with the command ”make” in this directory.
Write program:
	Run the program accordingly ”./write [server-IP] [file name] [received file name]”
Session program:
	Run the program accordingly ”./session [server-IP]”

[server-IP]: the IP of the server running the raspserver
[file name]: The name of the file you want to send (argument will be passed to ’fopen’)
[received file name]: What the name shall be on the server. It will be placed under the directory ”downloads” on the server. If a file with the same name exists it will be overwritten. 

If the server is responding than you can transfer files with this one! :) 
