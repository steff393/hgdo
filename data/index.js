// Copyright (c) 2021 steff393, MIT license

var Socket;

const uap_status = {
  OPEN         : 0x0001,
  CLOSED       : 0x0002,
  ERROR        : 0x0010,
  DIRECTION    : 0x0020,
  MOVING       : 0x0040,
  CLOSING      : 0x0060,
  VENTPOS      : 0x0080
};

function init() 
{
	Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
	Socket.onmessage = function(event) { processReceivedCommand(event); };
}
 
 
function processReceivedCommand(evt) {
		const obj = JSON.parse(evt.data);

		if (obj.rawVal & uap_status.OPEN) {
			document.getElementById('state').innerHTML = 'Offen';
		} else if (obj.rawVal & uap_status.CLOSED) {
			document.getElementById('state').innerHTML = 'Geschlossen';
		} else {
			document.getElementById('state').innerHTML = '---';
		}

		if (obj.rawVal & uap_status.MOVING) {
			if (obj.rawVal & uap_status.DIRECTION) {
				document.getElementById('moveDir').innerHTML = 'Schlie&szlig;t';
			} else {
				document.getElementById('moveDir').innerHTML = '&Ouml;ffnet';
			} 
		}	else {
			document.getElementById('moveDir').innerHTML = 'keine';
		}

		if (obj.rawVal & uap_status.VENTPOS) {
			document.getElementById('vent').innerHTML = 'aktiv';
		} else {
			document.getElementById('vent').innerHTML = '---';
		}

		/*if (obj.rawVal & uap_status.ERROR) {
			document.getElementById('error').innerHTML = 'aktiv';
		} else {
			document.getElementById('error').innerHTML = '---';
		}*/

		document.getElementById('timeNow').innerHTML = obj.timeNow;
		document.getElementById('rawValD0').innerHTML = 'd0 = ' + ("00000000" + (obj.rawVal & 0x00FF).toString(2)).slice(-8);
		document.getElementById('rawValD1').innerHTML = 'd1 = ' + ("00000000" + ((obj.rawVal & 0xFF00)>>8).toString(2)).slice(-8);
}
 
 
document.getElementById('btnOpen').addEventListener('click', function() {
	sendText('ACT_OPEN');
});
document.getElementById('btnStop').addEventListener('click', function() {
	sendText('ACT_STOP');
});
document.getElementById('btnClose').addEventListener('click', function() {
	sendText('ACT_CLOSE');
});
document.getElementById('btnExit').addEventListener('click', function() {
	document.getElementById('state').innerHTML = '-';
	document.getElementById('moveDir').innerHTML = '-';
	document.getElementById('vent').innerHTML = '-';
	document.getElementById('timeNow').innerHTML = '-';
	Socket.close();
});


function sendText(data){
	Socket.send(data);
}


window.onload = function(e){ 
	init();
}