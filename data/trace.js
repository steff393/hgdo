// Copyright (c) 2021 steff393, MIT license

var Socket;


function init() 
{
	Socket = new WebSocket('ws://' + window.location.hostname + ':82/');
	Socket.onmessage = function(event) { processReceivedCommand(event); };
}
 
 
function processReceivedCommand(evt) {
	//document.getElementById('log').innerHTML = document.getElementById('log').innerHTML.substr(0,4000) + evt.data;
	document.getElementById('log').innerHTML = evt.data + document.getElementById('log').innerHTML.substr(0,5000);
}


document.getElementById('btnCont').addEventListener('click', function() {
	sendText('btnCont');
});
document.getElementById('btnStop').addEventListener('click', function() {
	sendText('btnStop');
});
document.getElementById('btnReset').addEventListener('click', function() {
	document.getElementById('log').innerHTML = "";
});


function sendText(data){
	Socket.send(data);
}


window.onload = function(e){ 
	init();
}