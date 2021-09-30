// Copyright (c) 2021 steff393, MIT license
// based on https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/ConfigFile/ConfigFile.ino

#include <ArduinoJson.h>
#include <globalConfig.h>
#include <LittleFS.h>
#include <logger.h>

const uint8_t m = 1;

char cfgHgdoVersion[]     = "v0.0.1";           // hgdo version
char cfgBuildDate[]       = "2021-09-30";	      // hgdo build date

char     cfgApSsid[32];	              // SSID of the initial Access Point
char     cfgApPass[63];               // Password of the initial Access Point
char     cfgNtpServer[30];            // NTP server
uint8_t  cfgTxEnable;		              // 0: disable TX, 1: enable TX

bool createConfig() {
	StaticJsonDocument<1024> doc;

	// default configuration parameters
	// -------------------------------------
	doc["cfgApSsid"]              = F("hgdo");
	doc["cfgApPass"]              = F("hgdo1234");
	doc["cfgTxEnable"]           	= 1;
	// -------------------------------------
	
	File configFile = LittleFS.open(F("/cfg.json"), "w");
	if (!configFile) {
		return(false);
	}

	serializeJson(doc, configFile);
	configFile.close();
	return (true);
}


boolean checkConfig(JsonDocument& doc) {
	File configFile = LittleFS.open(F("/cfg.json"), "r");
	if (!configFile) {
		LOG(m, "Failed to open config file... Creating default config...","")
		if (createConfig()) {
			LOG(0, "Successful!", "");
			configFile = LittleFS.open(F("/cfg.json"), "r");
		} else {
			LOG(m, "Failed to create default config... Please try to erase flash","");
			return(false);
		}
	}

	size_t size = configFile.size();
	if (size > 1024) {
		LOG(m, "Config file size is too large","");
		return(false);
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	configFile.readBytes(buf.get(), size);
	
	auto error = deserializeJson(doc, buf.get());
	if (error) {
		LOG(m, "Failed to parse config file: %s", error.c_str());
		return(false);
	}
	configFile.close();

	//configFile = LittleFS.open("/cfg.json", "r");
	//log(m, configFile.readString());
	//configFile.close();
	return(true);
}

void loadConfig() {
	StaticJsonDocument<1024> doc;
	if (!checkConfig(doc)) {
		LOG(m, "Using default config", "");
		deserializeJson(doc, F("{}"));
	}

	strncpy(cfgApSsid,          doc["cfgApSsid"]            | "hgdo",             sizeof(cfgApSsid));
	strncpy(cfgApPass,          doc["cfgApPass"]            | "hgdo1234",         sizeof(cfgApPass));
	strncpy(cfgNtpServer,       doc["cfgNtpServer"]         | "europe.pool.ntp.org", sizeof(cfgNtpServer));
	cfgTxEnable               = doc["cfgTxEnable"]          | 1;
	
	LOG(m, "cfgHgdoVersion: %s", cfgHgdoVersion);
	LOG(m, "cfgBuildDate: %s"  , cfgBuildDate);

}
