
// player info
var player_name = "";

// highscore info
var scores = [];

// global variables telling the snake how to move
var dr = 0;
var dc = 0;

// the game matrix
var blocks = [];
var timerId = 0;

// the n in the nxn-matrix 'blocks' 
var nbrofblocks = 32;

// where to find the snake
var snakeR = 0;
var snakeC = 0;

// points achieved!
var points = 0;

// globlad variable determining whether or not the game is lost
var gmover = false;

// game initialisation!
function init(){

  points = 0;

  blocks = [];
  for(var x = 0;x<nbrofblocks;x++){
    blocks.push([]);
    for(var i = 0;i<nbrofblocks;i++){
      blocks[x].push(0)
    }
  }  
  
  snakeR = Math.floor(Math.random() * (nbrofblocks+1))%nbrofblocks;
  snakeC = Math.floor(Math.random() * (nbrofblocks+1))%nbrofblocks;

  blocks[snakeR][snakeC] = 1;
  placeFood();


  var phi = Math.floor(Math.random() * (4+1)) * 0.5 * Math.PI;
  dr = Math.sin(phi);
  dc = Math.cos(phi);

  gmover = false;
  go();
}

function post_result(){

  var url = "game.cgi?action=post_score";

  var data = {name: player_name, score: points, action: "post"};

  $.ajax({type: "POST", url: url, data: data, dataType: "text",
    success: function(response){
      setTimeout(get_highscore, 1000);
    }, 
    error: function(){
      console.log("highscore was not posted");
    }
  });

}

/* Lazy copy paste from https://www.w3schools.com/js/js_cookies.asp */
function get_cookie(cname) {
    var name = cname + "=";
    var decodedCookie = decodeURIComponent(document.cookie);
    var ca = decodedCookie.split(';');
    for(var i = 0; i <ca.length; i++) {
        var c = ca[i];
        while (c.charAt(0) == ' ') {
            c = c.substring(1);
        }
        if (c.indexOf(name) == 0) {
            return c.substring(name.length, c.length);
        }
    }
    return "";
}

function load_cookie(successCallback){
  $.ajax({type: "GET", url: "game.cgi?action=load-game-cookie", dataType: "text",
    success: function(response){
      //alert("loaded cookie:" + response);
      var d = new Date();
      // Expires after 2 hours
      d.setTime(d.getTime() + 1000*60*60*2);
      document.cookie = response + "expires=" + d.toUTCString() + ";";

      // Checking if the cookie was loaded or not:
      var name = response.split("=")[0];
      if ( get_cookie(name).length < 2 ) {
        alert("Your browser does not support cookies. Your results will therefore not be registered to the data base (sorry)!");
      }

      if ( !(typeof successCallback === "undefined") ){
        successCallback();
      }
    },
    error: function(){
      console.log("Failed to load key cookie........... :(");
    }
  });
}

function get_highscore(){

  $("#ajax-loader").css("display", "inherit");

  $.ajax({type: "GET", url: "game.cgi?action=get_highscore", dataType: "text",
    success: function(response){

      $("#highscore-row").html("<tbody>");
      var data = response.split('\n');
      for ( var i = 0; i < data.length -1; ){
        $("#highscore-row").append("<tr><td>" + (parseInt(i/2) + 1) + ".</td><td> " + data[i] + "</td><td>" + data[i+1] + "</td></tr>");
        i += 2;
      }

      $("#ajax-loader").css("display", "none");
    }, 
    error: function(){
      console.log("highscore was not loaded");
      $("#ajax-loader").css("display", "none");
    }
  });

}

function show_key_help_buttons(){
  $("#aid-btns").css("display", "inherit");
  $("#need-help-btn").css("display", "none")
}

// the game loop!
function go(){

  timerId = window.setInterval(function(){
    drawit();
    move(dr,dc);
  },100);

}

// moving the snake
function move(dr, dc){

  var newR = (nbrofblocks + snakeR + dr)%nbrofblocks;
  var newC = (nbrofblocks + snakeC + dc)%nbrofblocks;

  if(blocks[newR][newC] == 0){
    blocks[newR][newC] = 1 + blocks[snakeR][snakeC];
    removeTail();
  }else if(blocks[newR][newC] > 0){
    console.log("game over!");
    gameover();
  }else{
    blocks[newR][newC] = 1 + blocks[snakeR][snakeC];
    placeFood();
    points++;
  }

  snakeR = newR;
  snakeC = newC;

}

// placing food items on the screen (green dots)
function placeFood(){
  var tryR = Math.floor(Math.random()*(nbrofblocks))%nbrofblocks;
  var tryC = Math.floor(Math.random()*(nbrofblocks))%nbrofblocks;

  var i = 0;

  while(blocks[tryR][tryC]>0 && i<10){
    tryR = Math.floor(Math.random()*(nbrofblocks));
    tryC = Math.floor(Math.random()*(nbrofblocks));
    i++;
  }

  if(i==10){
    for(var x = 0; x<nbrofblocks;x++){
      for(var n=0;n<nbrofblocks;n++){
        if(blocks[x][n] == 0){
          tryR = x;
          tryC = n;
          break;
        }
      }
    }
  }

  blocks[tryR][tryC] = -1;
}

// removing the end of the snake (crucial algorithm for making this game work)
function removeTail(){

  for(var x = 0; x<nbrofblocks;x++){
    for(var i = 0;i<nbrofblocks;i++){
      if(blocks[x][i] > 0){
        blocks[x][i] = blocks[x][i] - 1;
      }
    }
  }
  
}

// function called upon game lost.
function gameover(){
//  alert("game over!!!!");
  clearInterval(timerId);
  $("#restart-btn").css("visibility", "visible");
  $("#aid-btns").css("display", "none");

  post_result();
  get_highscore();

  gmover = true;
}

// handling the focus of the keylistener (solution found in thread on stackoverflow.com)
var lastDownTarget, canvas;

// the timer identifier used for handling the restart calls.  
var timerId = 0;

// drawing the game on the canvas according to the content of the nxn-matrix 'blocks'
function drawit(){

  var ctx = canvas.getContext("2d");
  ctx.clearRect(0,0,canvas.width,canvas.height);
  ctx.globalAlpha = 0.1;
  ctx.fillStyle = "black";
  ctx.fillRect(0,0,canvas.width,canvas.height);
  ctx.globalAlpha=1.0;

  document.getElementById("points").innerHTML = "Points: " + String(points);

  var blockwidth = canvas.width / nbrofblocks;
  var blockheight = canvas.height / nbrofblocks;

  for(var x = 0; x<nbrofblocks;x++){
    for(var i=0;i<nbrofblocks;i++){
      if(blocks[x][i] == 0){
        ctx.fillStyle = "white";
      } else if(blocks[x][i] == -1) {
        ctx.fillStyle = "green";
      } else{
        ctx.fillStyle = "black";
      }
      ctx.fillRect(i*blockwidth,x*blockheight,blockwidth-1,blockheight-1);
    }
  }

}

// key pressed handling
function keypressed(event){

  var y = event.keyCode;

    if(y == 65 || y == 37){
      left();
    }else if(y == 68 || y == 39){
      right();
    }else if(y == 83 || y == 40){
      down();
    } else if(y == 87){
      up();
    } else if(y == 38){
      up();
    }

}

// Functions used to modify the snakes direction!
function left(){
  if(!(dr==0&&dc==1)){
    dr = 0;
    dc = -1;
  }
}

function right(){
  if(!(dr==0&&dc==-1)){
    dr = 0;
    dc = 1;
  }
}

function up(){
  if(!(dr==1&&dc==0)){
    dr = -1;
    dc = 0;
  }
}

function down(){
  if(!(dr==-1&&dc==0)){
    dr = 1;
    dc = 0;
  }
}

// function called upon hitting the restart button after game lost
function restart(){
  if(gmover){
    init();
    $("#restart-btn").css("visibility", "hidden")
    gmover=false;
  }
}

function add_handlers(){
  $(".overout")
  .mouseover(function() {
    $(this).fadeTo(100,1.0);
  })
  .mouseout(function() {
    $(this).fadeTo(100,0.5);
  });

  $("#name-give-btn").unbind("click").click(function(){
    player_name = $("#name-give-input").val();
    $("#name-give-div").css("display", "none");
    $("#name-give-btn").css("display", "none");
    $("#stuff-div").css("display", "inherit");
    $("#canvas-div").css("display", "inherit");    
    $("#need-help-btn").css("display", "inherit");

    $("#player-name-p").html("Player: " + player_name);

    get_highscore();
    init();
  });
}

function to_hide(){
  $("#need-help-btn").css("display", "none");
}

$(document).ready(function(){

  add_handlers();
  load_cookie(get_highscore);
  to_hide();

  canvas = document.getElementById("tcanvas");

  document.addEventListener('mousedown',function(event){
    lastDownTarget = event.target;
  },false);

  document.addEventListener('keydown',function(event){
    if(lastDownTarget == canvas){
      keypressed(event);
    }
  },false);

});