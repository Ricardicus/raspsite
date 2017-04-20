# Website
I want to become a better programmer so I set up a web server on my raspberry pi.

The website includes different stuff. 
For now it can be used as a cipher, play games or 
some sort of coffee monitor I haven't figured out what to do with yet..

# Remote Shell
This server will host an encrypted remote shell on port 8081 that one can use 
with the program "session" found under etc/transfer. Consult etc/transfer/README.md 
for further instructions!

# Install

1. SSH into the raspberry pi
2. Clone this repo
3. type: "chmod ugo+x build.sh"
4. Build the program: "./build.sh" 
5. Start a tmux session (so that the server may run even after the ssh session is over): "tmux"
6. Type: "./backend 8080" - now the website will respond on port 8080
7. Leave the tmux session by typing 'Ctrl-B' then 'D'

When you want to stop the server you need to reenter the tmux session, ths is done accordingly:

8. SSH into the raspberry pi
9. Type: "tmux attach" 
10. Type: "q"
11. Refresh the page,

And the server is stopped.

# Regarding step 6 (watchdog)
To keep the server running for as long as possible do the following instead of step 6:

1. Create a file called 'keep-alive.txt' under the directory 'etc': 'echo "For the watchdog." > etc/keep-alive.txt' 
2. Type: 'chmod ugo+x watchdog.sh'
3. Launch the server through the watchdog: "./watchdog.sh 8080"

Continue on with step 7 in the previously described set of steps.
The server is now running for as long as possible, recovering from whatever caused it to crash. 

# Make it your own

The first thing it will do is map "/" to the file "index.html".
With that in mind please feel free to include you own files and write your own pages!
Every file that exists will be outputted with corresponding headers (at least .js and .html, feel free to include more by digging into the code of http.c), except files under /etc/, /src/, /log/, and paths including '~' or '..'. 

You can also write you own cgi's under the directory '/cgi/'. For now it only supports cgi's written in Python and Bash, 
but I might extend it to Perl and other interpreted languages in the future! 
I hope you can have fun with it!
