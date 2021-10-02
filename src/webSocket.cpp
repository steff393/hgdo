// Copyright (c) 2021 steff393, MIT license

#include <Arduino.h>
#include <ArduinoJson.h>
#include <globalConfig.h>
#include <logger.h>
#include <uap.h>
#include <webSocket.h>
#include <WebSocketsServer.h>

const uint8_t m = 3;

#define CYCLE_TIME	  500	
#define JSON_LEN      256

WebSocketsServer webSocket = WebSocketsServer(81);
static uint32_t lastCall   = 0;


void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {
	if(type == WStype_TEXT) {
		LOG(m, "Payload %s", (char *)payload)
		if (strstr_P((char *)payload, PSTR("ACT_OPEN"))) {
			uap_triggerAction(UAP_ACTION_OPEN);
		} else if (strstr_P((char *)payload, PSTR("ACT_STOP"))) {
			uap_triggerAction(UAP_ACTION_STOP);
		} else if (strstr_P((char *)payload, PSTR("ACT_CLOSE"))) {
			uap_triggerAction(UAP_ACTION_CLOSE);
		}
	} 
}


void webSocket_begin() {
	// start the WebSocket connection
	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
}


void webSocket_loop() {
	webSocket.loop();
	if ((millis() - lastCall < CYCLE_TIME)) {
		return;
	}
	lastCall = millis();

	StaticJsonDocument<JSON_LEN> data;
	data[F("rawVal")]    = uap_getBroadcast();
	data[F("timeNow")]   = log_time();
	char response[JSON_LEN];
	serializeJson(data, response, JSON_LEN);
	webSocket.broadcastTXT(response);
}
