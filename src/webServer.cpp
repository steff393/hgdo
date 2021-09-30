// Copyright (c) 2021 steff393, MIT license

#include <Arduino.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <globalConfig.h>
#include <hcp.h>
#include <LittleFS.h>
#include <logger.h>
#include <SPIFFSEditor.h>
#include <webServer.h>
#define WIFI_MANAGER_USE_ASYNC_WEB_SERVER
#include <WiFiManager.h>

const uint8_t m = 2;


static const char* otaPage PROGMEM = "%OTARESULT%<br><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

AsyncWebServer server(80);
boolean resetRequested = false;

void onRequest(AsyncWebServerRequest *request){
	//Handle Unknown Request
	request->send_P(404, PSTR("text/plain"), PSTR("Not found"));
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
	//Handle body
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
	//Handle upload
}


String otaProcessor(const String& var){
	if(Update.hasError()){
		return(F("Failed"));
	} else {
		return(F("OK"));
	}
}


String otaProcessorEmpty(const String& var){
	// just replace the template string with nothing, neither ok, nor fail
	return String();
}

uint8_t getSignalQuality(int rssi)
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

void webServer_begin() {
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
		request->send(200, F("text/plain"), F("Resetting the WiFi credentials..."));
		WiFiManager wm;
		wm.resetSettings();
	});

	server.on("/json", HTTP_GET, [](AsyncWebServerRequest *request) {
		DynamicJsonDocument data(2048);
		// modify values
		if (request->hasParam(F("act"))) {
			hcp_triggerAction((hcp_action_t)request->getParam(F("act"))->value().toInt());
		}

		// provide the complete content
		data[F("hgdo")][F("version")] = cfgHgdoVersion;
		data[F("hgdo")][F("bldDate")] = cfgBuildDate;
		data[F("hgdo")][F("timeNow")] = log_time();
		data[F("hgdo")][F("millis")]  = millis();
		data[F("hgdo")][F("status")]  = hcp_getBroadcast();
		data[F("wifi")][F("mac")] = WiFi.macAddress();
		int qrssi = WiFi.RSSI();
		data[F("wifi")][F("rssi")] = qrssi;
		data[F("wifi")][F("signal")] = getSignalQuality(qrssi);
		data[F("wifi")][F("channel")] = WiFi.channel();
		String response;
		serializeJson(data, response);
		log(m, response);
		request->send(200, F("application/json"), response);
	});

	// OTA via http, based on https://gist.github.com/JMishou/60cb762047b735685e8a09cd2eb42a60
	server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
		AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", otaPage, otaProcessorEmpty);
		response->addHeader(F("Connection"), F("close"));
		response->addHeader(F("Access-Control-Allow-Origin"), F("*"));
		request->send(response);
	});

	server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
		// the request handler is triggered after the upload has finished... 
		// create the response, add header, and send response
		AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", otaPage, otaProcessor);
		response->addHeader(F("Connection"), F("close"));
		response->addHeader(F("Access-Control-Allow-Origin"), F("*"));
		resetRequested = true;  // Tell the main loop to restart the ESP
		request->send(response);
	},[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
		//Upload handler chunks in data
		
		if(!index){ // if index == 0 then this is the first frame of data
			Serial.printf("UploadStart: %s\n", filename.c_str());
			Serial.setDebugOutput(true);
			
			// calculate sketch space required for the update
			uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
			if(!Update.begin(maxSketchSpace)){//start with max available size
				Update.printError(Serial);
			}
			Update.runAsync(true); // tell the updaterClass to run in async mode
		}

		//Write chunked data to the free sketch space
		if(Update.write(data, len) != len){
				Update.printError(Serial);
		}
		
		if(final){ // if the final flag is set then this is the last frame of data
			if(Update.end(true)){ //true to set the size to the current progress
					Serial.printf("Update Success: %u B\nRebooting...\n", index+len);
				} else {
					Update.printError(Serial);
				}
				Serial.setDebugOutput(false);
		}
	});
	// OTA via http (end)


	// add the SPIFFSEditor, which can be opened via "/edit"
	server.addHandler(new SPIFFSEditor("" ,"" ,LittleFS));//http_username,http_password));

	server.serveStatic("/", LittleFS, "/");

	// Catch-All Handlers
	// Any request that can not find a Handler that canHandle it
	// ends in the callbacks below.
	server.onNotFound(onRequest);
	server.onFileUpload(onUpload);
	server.onRequestBody(onBody);

	server.begin();
}

void webServer_loop() {
	if (resetRequested){
		ESP.restart();
	}
}
