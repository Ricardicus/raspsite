# Website
I want to become a better programmer so I set up a web server on my raspberry pi.

The website includes different stuff. 
For now it can be used as a cipher, play games or 
some sort of coffee monitor I haven't figured out what to do with yet..

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

# Make it you own

The first thing it will do is output "index.html"
Feel free to write your own pages!
Every file that exists will be outputted with corresponding headers (at least .js and .html, feel free to include more by digging into the code of http.c), exept files under /etc/, /src/, /log/, and paths including '~' or '..'. 

You can also write you owns cgi's under the directory '/cgi/' for now it only supports cgi's written i python, 
but I might extend it to perl, bash and other interpreted languages in the future! 
I hope you have fun with it. 
