// Copyright (c) 2021 steff393, MIT license

var Socket;
var errorCnt = 0;
var conTilError = false;


function init() 
{
	Socket = new WebSocket('ws://' + window.location.hostname + ':50000/');
	Socket.onmessage = function(event) { processReceivedCommand(event); };
}
 
 
function processReceivedCommand(evt) {
	//document.getElementById('log').innerHTML = document.getElementById('log').innerHTML.substr(0,4000) + evt.data;
	var line;
	var type;

	line = "? "; // when unknown data
	type = "";
	if (evt.data.search("01 80 ") == 13 && evt.data.search("2 ") == 11) {
		// 58785: 8E 52 01 80 A2 
		line = "< "; type = "&nbsp;&nbsp;&nbsp;&nbsp;SlaveScan";
	}
	if (evt.data.search("00 ") == 7 && evt.data.search("2 ") == 11) {
		// 18249: 00 D2 00 04 DC
		line = "< "; type = "&nbsp;&nbsp;&nbsp;&nbsp;Broadcast" + "<BR>";
		if (conTilError && parseInt(evt.data.substring(13,14)) % 2 == 1) {
			// error detected
			errorCnt++;
		}
	}
	if (evt.data.search("28 ") == 7 && evt.data.search("20 ") == 13 && evt.data.search("1 ") == 11) {
		// 28263: 28 E1 20 75 
		line = "< "; type = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Slave status request";
	}
	if (evt.data.search("80 ") == 7 && evt.data.search("29 ") == 13 && evt.data.search("3 ") == 11) {
		// *14844: 80 B3 29 00 10 93 
		line = "> "; type = "&nbsp;Slave status response";
	}
	if (evt.data.search("80 ") == 7 && evt.data.search("14 28 ") == 13 && evt.data.search("2 ") == 11) {
		// *14844: 80 ?2 14 28 ??
		line = "> "; type = "&nbsp;Slave scan response";
	}
	line += evt.data + type + "<BR>";

	if (errorCnt >= 5) {
		sendText('btnStop');
		errorCnt = 0;
	}
	document.getElementById('log').innerHTML = line + document.getElementById('log').innerHTML.substr(0,5000);

}


document.getElementById('btnCont').addEventListener('click', function() {
	sendText('btnCont');
	conTilError = false;
});
document.getElementById('btnStop').addEventListener('click', function() {
	sendText('btnStop');
});
document.getElementById('btnReset').addEventListener('click', function() {
	document.getElementById('log').innerHTML = "";
});
document.getElementById('btnConErr').addEventListener('click', function() {
	sendText('btnCont');
	conTilError = true;
});


function sendText(data){
	Socket.send(data);
}


window.onload = function(e){ 
	init();
}