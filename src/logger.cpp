// Copyright (c) 2021 steff393, MIT license

#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <globalConfig.h>
#include <LittleFS.h>
#include <logger.h>
#include <NTPClient.h>

#define TIME_LEN            10   // "23:12:01: "
#define MOD_LEN              6   // "WEBS: "
#define CLEAN_PERIOD  86400000ul // 1 day in ms

static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP, cfgNtpServer, 3600, 60000); // GMT+1 and update every minute

static const char *mod[9] = {"", "CFG ", "WEBS", "SOCK", "BTN ", "AUTO", "KEYP", "RFID", "UAP "};
static char *   bootLog;
static uint16_t bootLogSize;
static uint32_t lastClean = 0;
static char     log_date[11];    // dd.mm.yyyy
static uint8_t  log_month;


static boolean getDstGermany(uint32_t unixtime) {
	
	const uint32_t SEKUNDEN_PRO_TAG   =  86400ul; /*  24* 60 * 60 */
	const uint32_t TAGE_IM_GEMEINJAHR =    365ul; /* kein Schaltjahr */
	const uint32_t TAGE_IN_4_JAHREN   =   1461ul; /*   4*365 +   1 */
	const uint32_t TAGE_IN_100_JAHREN =  36524ul; /* 100*365 +  25 - 1 */
	const uint32_t TAGE_IN_400_JAHREN = 146097ul; /* 400*365 + 100 - 4 + 1 */
	const uint32_t TAGN_AD_1970_01_01 = 719468ul; /* Tagnummer bezogen auf den 1. Maerz des Jahres "Null" */
	uint8_t month;
	uint8_t wday;
	uint8_t hour;
	
	uint16_t day;
	uint32_t dayN;
	uint32_t temp;
	
	 // *unixtime += 3600; // +1 für UTC->D
	
	// alle berechnungen nachfolgend
	dayN = (TAGN_AD_1970_01_01 + unixtime / SEKUNDEN_PRO_TAG);

	wday = (uint8_t)(((unixtime / 3600 / 24) + 4) % 7); // weekday
	// Schaltjahrregel des Gregorianischen Kalenders: Jedes durch 100 teilbare Jahr ist kein Schaltjahr, es sei denn, es ist durch 400 teilbar. 
	temp = 4 * (dayN + TAGE_IN_100_JAHREN + 1) / TAGE_IN_400_JAHREN - 1;
	dayN -= TAGE_IN_100_JAHREN * temp + temp / 4;
	
	// Schaltjahrregel des Julianischen Kalenders:
	//   Jedes durch 4 teilbare Jahr ist ein Schaltjahr. 
	temp = 4 * (dayN + TAGE_IM_GEMEINJAHR + 1) / TAGE_IN_4_JAHREN - 1;
	dayN -= TAGE_IM_GEMEINJAHR * temp + temp / 4;
	
	// dayN enthaelt jetzt nur noch die Tage des errechneten Jahres bezogen auf den 1. Maerz. 
	month = (uint8_t)((5 * (uint16_t)dayN + 2) / 153);
	day = (uint16_t)((uint16_t)dayN - (uint16_t)(month * 153 + 2) / 5 + 1);

	hour = (uint8_t)((unixtime % SEKUNDEN_PRO_TAG) / 3600);

	// vom Jahr, das am 1. Maerz beginnt auf unser normales Jahr umrechnen:
	month = (uint8_t)((uint8_t)(month + 3) % 13); 
	
	// ---------------------- Jahr berechnen (ungefähr ;-) ) --------------------------
	uint16_t daysSinceNewYear2020 = unixtime / SEKUNDEN_PRO_TAG - 18263ul;     // 18263 Tage zwischen 1.1.1970 und 1.1.2020
	uint16_t year = 2020   +   4 * (daysSinceNewYear2020 / TAGE_IN_4_JAHREN)   +   (daysSinceNewYear2020 % TAGE_IN_4_JAHREN) / TAGE_IM_GEMEINJAHR;
	sprintf_P(log_date, PSTR("%02d.%02d.%4d"), day, month, year);
	log_month = month;
	// ----------------------------------------------------------------------

	if( month < 3 || month > 10 )     // month 1, 2, 11, 12
		return 0;         // -> Winter

	if(day - wday >= 25 && (wday || hour >= 2)) { // after last Sunday 2:00
		
		if(month == 10)       // October -> Winter
			return 0;
		
		} else {          // before last Sunday 2:00
		
		if(month == 3)        // March -> Winter
			return 0;
			
		}
		return 1;
		//*unixtime += 3600; // nochmal+1 für Sommerzeit

}


void log(uint8_t module, const char *msg, boolean newLine /* =true */) {
	char output[TIME_LEN + MOD_LEN + 1];

	if (module) {
		strcpy(output, timeClient.getFormattedTime().c_str()); // 2 for ": "
		strcat(output, ": ");
		strncat(output, mod[module], MOD_LEN-2); // 2 for ": "
		strcat(output, ": ");
	} else {
		strcpy(output, "");
	}
	// print to Serial
	Serial.print(output);
	Serial.print(msg);
	if (newLine) {
		Serial.print("\n");
	}
	// print to bootLog, if there is still enough space
	if ((strlen(bootLog)+strlen(output)+strlen(msg) + 5) < bootLogSize) {
		strcat(bootLog, output);
		strcat(bootLog, msg);
		if (newLine) {
			strcat(bootLog, "\n");
		}
	}
}


String log_time() {
	return(timeClient.getFormattedTime());
}


uint8_t log_getHours() {
  return ((timeClient.getEpochTime()  % 86400L) / 3600);
}


uint8_t log_getMinutes() {
  return ((timeClient.getEpochTime() % 3600) / 60);
}


uint32_t log_unixTime() {
	// The pfox chart needs the 'real' unixtime, not corrected for timezone or DST
	uint32_t time = timeClient.getEpochTime() - 3600;

	if (getDstGermany(time)) { 
		time = time - 3600;
	}
	return(time);
}


char* log_getBuffer() {
	return(bootLog);
}


void log_freeBuffer() {
	// set string-end character to first position to indicate an empty string
	bootLog[0] = '\0';
}


void log_file(const char *msg) {
	if (cfgLogMonths != 0) {
		char filename[10];
		sprintf(filename, PSTR("/%02d.txt"), log_month);      // e.g. October: "/10.txt"
		File logFile = LittleFS.open(filename, "a");
		logFile.printf_P(PSTR("%s, %s: %s\n"), log_date, timeClient.getFormattedTime(), msg);
		logFile.close();
	}
}


void log_cleanFiles() {
	int8_t monthToDelete = 0;
	if (log_month > 0) {
		monthToDelete = log_month - cfgLogMonths;
		if (monthToDelete < 0) {
			monthToDelete += 12;
		}
		char filename[10];
		sprintf(filename, PSTR("/%02d.txt"), monthToDelete);      // e.g. October: "/10.txt"
		LittleFS.remove(filename);
	}
	lastClean = millis();
}


void logger_setup() {
	bootLogSize = 5000;
	bootLog = (char *) malloc(bootLogSize);
	bootLog[0] = '\0';

	// connect to NTP time server
	timeClient.begin();
}


void logger_loop() {
	timeClient.update();
	if (getDstGermany(timeClient.getEpochTime())) timeClient.setTimeOffset(7200);
	
	// daily clean-up of files
	if (millis() - lastClean > CLEAN_PERIOD) {
		log_cleanFiles();
	}
}
