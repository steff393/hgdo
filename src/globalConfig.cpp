// Copyright (c) 2021 steff393, MIT license
// based on https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/ConfigFile/ConfigFile.ino

#include <ArduinoJson.h>
#include <globalConfig.h>
#include <LittleFS.h>
#include <logger.h>

const uint8_t m = 1;

char cfgHgdoVersion[]     = "v0.1.0";           // hgdo version
char cfgBuildDate[]       = "2021-10-30";	      // hgdo build date

char     cfgApSsid[32];	              // SSID of the initial Access Point
char     cfgApPass[63];               // Password of the initial Access Point
char     cfgNtpServer[30];            // NTP server
uint8_t  cfgTxEnable;                 // 0: disable TX, 1: enable TX
uint8_t  cfgTimeOn;                   // Hour to  enable button
uint8_t  cfgTimeOff;                  // Hour to disable button
uint16_t cfgBtnDebounce;              // Debounce time for button [ms]
uint8_t  cfgAcTime;                   // Hour to start the auto-close [h]
uint8_t  cfgAcDur1;                   // Duration of the auto-close PREWARN phase [s]
uint8_t  cfgAcDur2;                   // Duration of the auto-close WAIT    phase [s]
uint8_t  cfgPdWaitTime;               // Duration of the package drop  WAIT phase [s]
uint8_t  cfgPdTimeout;                // Timeout of the package drop function, when venting position is missed [s]
uint8_t  cfgHwVersion;                // Selection of the used HW
uint8_t  cfgLogMonths;                // Months to be logged
uint8_t  cfgTrace;                    // 0: disable Trace Feature, 1: enable
uint8_t  cfgAutoErrorCorr;            // 0: disable AutoErrorCorrection, 1: enable


static bool createConfig() {
	StaticJsonDocument<1024> doc;

	// default configuration parameters
	doc["cfgApSsid"]              = F("hgdo");
	doc["cfgApPass"]              = F("hgdo1234");
	doc["cfgTxEnable"]           	= 1;
	
	File configFile = LittleFS.open(F("/cfg.json"), "w");
	if (!configFile) {
		return(false);
	}

	serializeJson(doc, configFile);
	configFile.close();
	return (true);
}


static boolean checkConfig(JsonDocument& doc) {
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
	cfgTimeOn                 = doc["cfgTimeOn"]            | 24;
	cfgTimeOff                = doc["cfgTimeOff"]           | 0;
	cfgBtnDebounce            = doc["cfgBtnDebounce"]       | 300;
	cfgAcTime                 = doc["cfgAcTime"]            | 24;
	cfgAcDur1                 = doc["cfgAcDur1"]            | 2;
	cfgAcDur2                 = doc["cfgAcDur2"]            | 30;
	cfgPdWaitTime             = doc["cfgPdWaitTime"]        | 15;
	cfgPdTimeout              = doc["cfgPdTimeout"]         | 10;
	cfgHwVersion              = doc["cfgHwVersion"]         | 20;
	cfgLogMonths              = doc["cfgLogMonths"]         | 0;
	cfgTrace                  = doc["cfgTrace"]             | 0;
	cfgAutoErrorCorr          = doc["cfgAutoErrorCorr"]     | 0;

	LOG(m, "cfgHgdoVersion: %s", cfgHgdoVersion);
	LOG(m, "cfgBuildDate: %s"  , cfgBuildDate);
}
