// Copyright (c) 2021 steff393, MIT license
// based on https://github.com/joeyoung/arduino_keypads

#include <Arduino.h>
#include <globalConfig.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <LittleFS.h>
#include <logger.h>
#include <uap.h>
#include <Wire.h>

#define I2C_Addr                  0x20  // I2C Address of PCF8574-board: 0x20 - 0x27
#define KEYP_CODE_MAX               20  // different codes
#define KEYP_CODE_LEN                7  // 6 digits, e.g. "123456" + string termination
#define KEYP_NAME_LEN               11  // 10 chars for the person name + string termination
#define MAX_TIME_FOR_CODE         8000  // 8s
#define WRONG_CODE_LIMIT            10  // 10 wrong tries are possible
#define WRONG_CODE_DELAY        900000  // 15min

static const uint8_t m = 6;

static const byte ROWS = 4; //four rows
static const byte COLS = 4; //four columns

//define the symbols on the buttons of the keypads
static char keyPadLayout[ROWS][COLS] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};

static byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
static byte colPins[COLS] = {4, 5, 6, 7}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
static Keypad_I2C i2cKeypad( makeKeymap(keyPadLayout), rowPins, colPins, ROWS, COLS, I2C_Addr); 

static char       code[KEYP_CODE_MAX][KEYP_CODE_LEN];
static char       name[KEYP_CODE_MAX][KEYP_NAME_LEN];

static char       input[KEYP_CODE_LEN];
static uint8_t    pos = 0;
static boolean    enabled           = false;
static uint16_t   wrongCodeCnt = 0;
static uint32_t   wrongCodeTimer = 0;
static uint32_t   startTime = 0;


static boolean readCodes() {
	File file = LittleFS.open(F("/code.txt"), "r");
	if (!file) {
		LOG(m, "Disabled (code.txt not found)", "");
		return(false);
	}

	uint8_t k = 0;
	//LOGN(m, "Codes: ", "");
	while (file.available() && k < KEYP_CODE_MAX) {
		// split the line into tokens limited by ; 
		char     line[KEYP_CODE_LEN + KEYP_NAME_LEN + 5];
		char *   pch;
		uint16_t nrChars;

		nrChars = file.readBytesUntil('\n', line, KEYP_CODE_LEN + KEYP_NAME_LEN + 4);
		
		pch = strtok(line, ";\n"); *code[k] = '\0'; strncat(code[k], pch, KEYP_CODE_LEN - 1);
		pch = strtok(NULL, ";\n"); *name[k] = '\0';	strncat(name[k], pch, KEYP_NAME_LEN - 1);
		
		while (nrChars >= KEYP_CODE_LEN + KEYP_NAME_LEN + 4) {
			// read rest of the line, but simply ignore it
			nrChars = file.readBytesUntil('\n', line, KEYP_CODE_LEN + KEYP_NAME_LEN + 4);
		}

		// if (k > 0 ) { LOGN(0, ", ", ""); }
		// LOGN(0, "%s > %s", code[k], name[k]);
		k++;
	}

	file.close();
	return(true);
}


static void checkCode(char * input) {
	uint8_t k = 0;
	char msg[50];
	sprintf_P(msg, PSTR("Tastenfeld: Code %s"), input);
	log_file(msg);
	if (wrongCodeCnt >= WRONG_CODE_LIMIT) { 
		// all inputs will be ignored until time has elapsed
		return; 
	}
	// Search if this fits to a known code
	while (strncasecmp(input, code[k], KEYP_CODE_LEN - 1) != 0 && k < KEYP_CODE_MAX) {
		k++;
	};
	if (k < KEYP_CODE_MAX) {
		LOG(0, "found: idx=%d, name=%s => opening", k, name[k]);
		// open garage door
		uap_triggerAction(UAP_ACTION_OPEN, SRC_KEYPAD);
		wrongCodeCnt = 0;
	} else {
		wrongCodeCnt++;
		wrongCodeTimer = millis();
		LOG(0, "unknown, cnt=%d", wrongCodeCnt);
	}
}


void key_setup() {
	if (!readCodes()) {
		// there is no code.txt file => function disabled
		enabled = false;
		return;
	}
	enabled = true;

	Wire.begin(PIN_SDA, PIN_SCL);
	Wire.beginTransmission(I2C_Addr);   // Try to establish connection
	if (Wire.endTransmission() != 0) {  // No Connection established
		LOG(m, "No device found on 0x%02x", I2C_Addr);
	} else {
		LOG(m, "Device found on 0x%02x", I2C_Addr);
	}
	i2cKeypad.begin( );
}


void key_loop() {
	if (enabled == false) {
		// avoid unnecessary calls
		return;
	}
	char key = i2cKeypad.getKey();
	
	if (key != NO_KEY) {
		input[pos] = key;
		pos++;
		if (pos == 1) {
			// entering first digit starts a timer
			startTime = millis();
		}
		if (pos == KEYP_CODE_LEN - 1) {
			input[pos] = '\0';              // terminate the code
			LOGN(m, "%s entered, ", input);
			checkCode(input);
			pos = 0;
		}
	}
	if (pos > 0 && millis() - startTime > MAX_TIME_FOR_CODE) {
		LOG(m, "Time elapsed", "");
		pos = 0;
	}
	if (wrongCodeCnt && (millis() - wrongCodeTimer > WRONG_CODE_DELAY)) {
		wrongCodeCnt--;
		wrongCodeTimer = millis();
	}
}


boolean key_getEnabled() {
	return(enabled);
}
