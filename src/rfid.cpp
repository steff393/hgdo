// Copyright (c) 2021 steff393, MIT license

#include <Arduino.h>
#include <globalConfig.h>
#include <LittleFS.h>
#include <logger.h>
#include <MFRC522.h>
#include <SPI.h>
#include <uap.h>

#define CYCLE_TIME		             500	// 500ms
#define INHIBIT_AFTER_DETECTION   3000  // 3s wait time after a card was detected
#define RELEASE_TIME             60000  // lock again, if car was not connected within 60s
#define RFID_CHIP_MAX               10  // different RFID cards
#define RFID_CHIP_LEN                9  // 4 Hex values, e.g. "0ab5c780" + string termination
#define RFID_NAME_LEN               11  // 10 chars for the person name + string termination
#define WRONG_CODE_LIMIT            10  // 10 wrong tries are possible
#define WRONG_CODE_DELAY        900000  // 15min

static const uint8_t m  = 7;

static MFRC522 mfrc522(PIN_SS, PIN_RST);

static char      chipID[RFID_CHIP_LEN];
static char      chip[RFID_CHIP_MAX][RFID_CHIP_LEN];
static char      name[RFID_CHIP_MAX][RFID_NAME_LEN];

static uint32_t  lastCall          = 0;
static uint32_t  lastDetect        = 0;
static uint32_t  lastReleased      = 0;
static boolean   enabled           = false;
static boolean   released          = false;
static uint16_t  wrongCodeCnt      = 0;
static uint32_t  wrongCodeTimer    = 0;


static boolean readCards() {
	File file = LittleFS.open(F("/rfid.txt"), "r");
	if (!file) {
		LOG(m, "Disabled (rfid.txt not found)", "");
		return(false);
	}

	uint8_t k = 0;
	//LOGN(m, "Cards: ", "");
	while (file.available() && k < RFID_CHIP_MAX) {
		// split the line into tokens limited by ; 
		char     line[RFID_CHIP_LEN + RFID_NAME_LEN + 5];
		char *   pch;
		uint16_t nrChars;

		nrChars = file.readBytesUntil('\n', line, RFID_CHIP_LEN + RFID_NAME_LEN + 4);
		
		pch = strtok(line, ";\n"); *chip[k] = '\0'; strncat(chip[k], pch, RFID_CHIP_LEN - 1);
		pch = strtok(NULL, ";\n"); *name[k] = '\0';	strncat(name[k], pch, RFID_NAME_LEN - 1);
		
		while (nrChars >= RFID_CHIP_LEN + RFID_NAME_LEN + 4) {
			// read rest of the line, but simply ignore it
			nrChars = file.readBytesUntil('\n', line, RFID_CHIP_LEN + RFID_NAME_LEN + 4);
		}

		// if (k > 0 ) { LOGN(0, ", ", ""); }
		// LOGN(0, "%s > %s", chip[k], name[k]);
		k++;
	}

	file.close();
	return(true);
}


static void checkCode(char * input) {
	uint8_t k = 0;
	char msg[50];
	sprintf_P(msg, PSTR("RFID: Code %s"), input);
	log_file(msg);
	if (wrongCodeCnt >= WRONG_CODE_LIMIT) { 
		// all inputs will be ignored until time has elapsed
		return; 
	}
	// Search if this fits to a known code
	while (strncasecmp(input, chip[k], RFID_CHIP_LEN - 1) != 0 && k < RFID_CHIP_MAX) {
		k++;
	};
	if (k < RFID_CHIP_MAX) {
		LOG(0, "found: idx=%d, name=%s => opening", k, name[k]);
		released = true;
		lastReleased = millis();

		// open garage door
		uap_triggerAction(UAP_ACTION_OPEN, SRC_RFID);
		wrongCodeCnt = 0;
	} else {
		wrongCodeCnt++;
		wrongCodeTimer = millis();
		LOG(0, "unknown, cnt=%d", wrongCodeCnt);
	}
}


void rfid_setup() {
	if (!readCards()) {
		// there is no rfid.txt file => function disabled
		enabled = false;
		return;
	}
	enabled = true;

	SPI.begin();

	// Initialize MFRC522
	mfrc522.PCD_Init();
	 
	delay(10);  
	Serial.println("");
	mfrc522.PCD_DumpVersionToSerial();	// dump some details
}


void rfid_loop() {
	if ((millis() - lastCall < CYCLE_TIME) || (enabled == false) || (millis() - lastDetect < INHIBIT_AFTER_DETECTION)) {
		// avoid unnecessary frequent calls
		return;
	}
	lastCall = millis();

	if ((lastReleased != 0) && (millis() - lastReleased > RELEASE_TIME)) {
		// time elapsed --> RFID chip no longer allowed
		released = false;
		lastReleased = 0;
	}


	// Check for new card
	if (mfrc522.PICC_IsNewCardPresent()) {
		// wait a little longer this time to avoid multiple reads
		lastDetect = millis();

		mfrc522.PICC_ReadCardSerial();

		// First 4 byte should be sufficient
		sprintf(chipID, "%02x%02x%02x%02x", mfrc522.uid.uidByte[0], mfrc522.uid.uidByte[1], mfrc522.uid.uidByte[2], mfrc522.uid.uidByte[3]);
		LOGN(m, "Detected: %s ... ", chipID);
		checkCode(chipID);
	}
	if (wrongCodeCnt && (millis() - wrongCodeTimer > WRONG_CODE_DELAY)) {
		wrongCodeCnt--;
		wrongCodeTimer = millis();
	}
}


boolean rfid_getEnabled() {
	return(enabled);
}


boolean rfid_getReleased() {
	return(released);
}


char * rfid_getLastID() {
	return(chipID);
}

