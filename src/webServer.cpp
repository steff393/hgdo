// Copyright (c) 2021 steff393, MIT license

#include <Arduino.h>
#include <AsyncElegantOTA.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <globalConfig.h>
#include <LittleFS.h>
#include <logger.h>
#include <SPIFFSEditor.h>
#include <uap.h>
#include <webServer.h>
#define WIFI_MANAGER_USE_ASYNC_WEB_SERVER
#include <WiFiManager.h>

static const uint8_t m = 2;


static AsyncWebServer server(80);
static boolean resetRequested = false;
static boolean resetwifiRequested = false;


static void onRequest(AsyncWebServerRequest *request){
	//Handle Unknown Request
	request->send_P(404, PSTR("text/plain"), PSTR("Not found"));
}


static void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
	//Handle body
}


static void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
	//Handle upload
}


static uint8_t getSignalQuality(int rssi)
{
		int quality = 0;
		if (rssi <= -100) {
				quality = 0;
		} else if (rssi >= -50) {
				quality = 100;
		} else {
				quality = 2 * (rssi + 100);
		}
		return quality;
}


void webServer_setup() {
	server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, F("text/plain"), String(ESP.getFreeHeap()));
	});

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(LittleFS, F("/index.html"), String(), false);
	});

	server.on("/cfg", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(LittleFS, F("/cfg.json"), F("application/json"));
	});

	server.on("/bootlog", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, F("text/plain"), log_getBuffer());
	});

	server.on("/bootlog_reset", HTTP_GET, [](AsyncWebServerRequest *request){
		log_freeBuffer();
		request->send(200, F("text/plain"), F("Cleared"));
	});

	server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, F("text/plain"), F("Resetting the ESP8266..."));
		resetRequested = true;
	});

	server.on("/resetwifi", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, F("text/plain"), F("Resetting the WiFi credentials... Please power off/on"));
		resetwifiRequested = true;
	});

	server.on("/stopcom", HTTP_GET, [](AsyncWebServerRequest *request){
		uap_StopCommunication();
		request->send(200, F("text/html"), F("Communication stopped... <a href=\"/update\">Update</a>"));
	});

	server.on("/json", HTTP_GET, [](AsyncWebServerRequest *request) {
		DynamicJsonDocument data(2048);
		// modify values
		if (request->hasParam(F("act"))) {
			uap_triggerAction((uap_action_t)request->getParam(F("act"))->value().toInt());
		}

		// provide the complete content
		data[F("hgdo")][F("version")] = cfgHgdoVersion;
		data[F("hgdo")][F("bldDate")] = cfgBuildDate;
		data[F("hgdo")][F("timeNow")] = log_time();
		data[F("hgdo")][F("millis")]  = millis();
		uap_status_t bc = uap_getBroadcast();
		data[F("door")][F("open")]     =  (bc & UAP_STATUS_OPEN)?true:false;
		data[F("door")][F("closed")]   =  (bc & UAP_STATUS_CLOSED)?true:false;
		data[F("door")][F("error")]    =  (bc & UAP_STATUS_ERROR)?true:false;
		data[F("door")][F("opening")]  = ((bc & (UAP_STATUS_DIRECTION | UAP_STATUS_MOVING))==UAP_STATUS_MOVING)?true:false;
		data[F("door")][F("closing")]  = ((bc & (UAP_STATUS_DIRECTION | UAP_STATUS_MOVING))==UAP_STATUS_CLOSING)?true:false;
		data[F("door")][F("venting")]  =  (bc & UAP_STATUS_VENTPOS)?true:false;

		data[F("wifi")][F("mac")] = WiFi.macAddress();
		int qrssi = WiFi.RSSI();
		data[F("wifi")][F("rssi")] = qrssi;
		data[F("wifi")][F("signal")] = getSignalQuality(qrssi);
		data[F("wifi")][F("channel")] = WiFi.channel();
		String response;
		serializeJson(data, response);
		LOGEXT(m, "%s", response);
		request->send(200, F("application/json"), response);
	});

	// add the SPIFFSEditor, which can be opened via "/edit"
	server.addHandler(new SPIFFSEditor("" ,"" ,LittleFS));//http_username,http_password));

	server.serveStatic("/", LittleFS, "/");

	// Catch-All Handlers
	// Any request that can not find a Handler that canHandle it
	// ends in the callbacks below.
	server.onNotFound(onRequest);
	server.onFileUpload(onUpload);
	server.onRequestBody(onBody);

	AsyncElegantOTA.begin(&server);    // Start ElegantOTA

	server.begin();
}

void webServer_loop() {
	if (resetRequested){
		ESP.restart();
	}
	if (resetwifiRequested) {
		WiFi.disconnect(true);
		ESP.eraseConfig();
		resetwifiRequested = false;
	}
}
