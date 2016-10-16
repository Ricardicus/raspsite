
// This is a program used for sending secrets!
// The page will run on a browser supporting HTML5. 

// Here are some global variables used in the cipher system
var selectedChars;
var encodev;
var place;

// Timer Id. I want to store the animation timer somewhere.
var animateTimerId;

// Initilises global variables prepares the page for action!
function init(){
	selectedChars = ['a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','å','ä','ö','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','X','Y','W','Z','Å','Ä','Ö','.','"',"'",'0','1','2','3','4','5','6','7','8','9','-','?','#','!','@','/','&',')',':','(',';',' ',',', '[',']'];
	encodev = [1,4,5,12,-13,14,14,10,9,11,18,97];
	place = {};

	for(var i = 0;i<selectedChars.length;i++){
		place[selectedChars[i]] = i;
	}

	// Storing the timer identification of the animation
	animateTimerId = window.setInterval("drawfunction1();",50);
}

// My crypto function! Encrypts the string provided in 'x' and displays the result on a paragraph on the page
function crypt(x){
	var message = "";

	for(h in x){
		if(place[x[h]] === undefined){
			if(x[h] == "&nbsp;"){
			} else if(x[h] == ' '){
			} else if(x[h] == '&nbsp'){
			}
		}
		message+=selectedChars[(selectedChars.length*200 + place[x[h]] + encodev[h % encodev.length])%selectedChars.length].replace( /&nbsp/g, ' ' );
	}
	document.getElementById("displaymsg").innerHTML = message
}

// My decrypto function! Decrypts the string provided in 'x' and displays the result on a paragraph on the page
function decrypt(x){
	var message = "";

	for(h in x){
		if(place[x[h]] === undefined){
			if(x[h] == "&nbsp;"){
			} else if(x[h] == ' '){
			} else if(x[h] == '&nbsp'){
			}
		}
		message+=selectedChars[(selectedChars.length*200 + place[x[h]] - encodev[h % encodev.length])%selectedChars.length].replace( /&nbsp/g, ' ' );
	}
	document.getElementById("displaymsg").innerHTML = message
}

// Sets the 'secret' key provided by the client.
// The key from the user input is provided in the string 'x'   
function setKey(x){
	encodev = [0];
	for(h in x){
		encodev.push(x.charCodeAt(h));
	}
}

// Some variables involved in the animation in the corner
var pos = 0;
var dpos = 0.05;
var pinch = 5;
var t = 0;
var dt = 0.02;

// used in the animation..
var starts = []
for(var x = 0;x<10;x++){
	starts.push(0.0);
}

// animation function!
drawfunction1=function(){
	var canvas = document.getElementById("canvs");
	var ctx = canvas.getContext("2d");
	ctx.fillStyle = "white";
	ctx.fillRect(0,0,canvas.width,canvas.height);
	ctx.strokeStyle = "black";
	for(var x = 0; x<5;x++){
		var r = 10*x + 10;
		starts[x] = ((pos*(x+1) / 100) + pinch)% 1000;

		var e = (starts[x]+1)

		ctx.beginPath();
		ctx.lineWidth = 8;
		ctx.arc(canvas.width/2, canvas.height/2, r, starts[x] * Math.PI , e * Math.PI);
 		ctx.stroke();
		//console.log("hej!")
 		pos = (pos+dpos) % 200;
	}
	dpos += 0.1*Math.cos(2*Math.PI*t);
	t = (t+dt) % 100; 
};