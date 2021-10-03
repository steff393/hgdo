// Copyright (c) 2021 steff393, MIT license

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <autoClose.h>
#include <button.h>
#include <globalConfig.h>
#include <LittleFS.h>
#include <logger.h>
#include <uap.h>
#include <WiFiManager.h>
#include <webServer.h>
#include <webSocket.h>


bool _handlingOTA = false;


void setup() {
	Serial.begin(115200);
	Serial.println(F("\n\nStarting hgdo ;-)"));
	logger_begin();

	// define a GPIO as output
	pinMode(PIN_DE_RE, OUTPUT);

	if(!LittleFS.begin()){ 
		Serial.println(F("An Error has occurred while mounting LittleFS"));
		return;
	}

	loadConfig();

	WiFiManager wifiManager;
	char ssid[32]; strcpy(ssid, cfgApSsid);
	char pass[63]; strcpy(pass, cfgApPass);
	wifiManager.autoConnect(ssid, pass);

	// setup the Webserver
	webServer_begin();
	webSocket_begin();

	// setup the OTA server
	ArduinoOTA.setHostname("hgdo");
	ArduinoOTA.begin();

	ArduinoOTA.onStart([]() 
	{
		_handlingOTA = true;
	});

	uap_setup();
	btn_setup();

	Serial.print(F("Boot time: ")); Serial.println(millis());
	Serial.print(F("Free heap: ")); Serial.println(ESP.getFreeHeap());
}


void loop() {
	ArduinoOTA.handle();
	if(!_handlingOTA) {
		logger_loop();
		uap_loop();
		btn_loop();
		ac_loop();
		webServer_loop();
		webSocket_loop();
	}
}
