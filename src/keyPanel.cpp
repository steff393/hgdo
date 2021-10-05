// Copyright (c) 2021 steff393, MIT license
// based on https://github.com/joeyoung/arduino_keypads

#include <Arduino.h>
#include <globalConfig.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <LittleFS.h>
#include <logger.h>
#include <Wire.h>

#define CYCLE_TIME                 100	// ms
#define I2C_Addr                  0x20  // I2C Address of PCF8574-board: 0x20 - 0x27
#define KEYP_CODE_MAX               20  // different codes
#define KEYP_CODE_LEN                7  // 6 digits, e.g. "123456" + string termination
#define KEYP_NAME_LEN               11  // 10 chars for the person name + string termination

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

static char code[KEYP_CODE_MAX][KEYP_CODE_LEN];
static char name[KEYP_CODE_MAX][KEYP_NAME_LEN];

static boolean readCodes() {
  File file = LittleFS.open(F("/code.txt"), "r");
  if (!file) {
    LOG(m, "Disabled (code.txt not found)", "");
    return(false);
  }

  uint8_t k = 0;
  LOGN(m, "Codes: ", "");
  while (file.available() && k < KEYP_CODE_MAX) {
    // split the line into tokens limited by ; 
		char line[KEYP_CODE_LEN + KEYP_CODE_LEN + 5];




		file.readBytesUntil('\n', line, KEYP_CODE_LEN + KEYP_CODE_LEN + 1);
  	
		
		
		
		char * pch;
		pch = strtok(line, ";");
		//pch = strtok((char*)file.readStringUntil('\n').c_str(), ";");
    strncpy(code[k], pch, KEYP_CODE_LEN - 1);
		pch = strtok(NULL, ";");
		strncpy(name[k], pch, KEYP_NAME_LEN - 1);

    if (k > 0 ) { LOGN(0, ", ", ""); }
    LOGN(0, "%s-%s", code[k], name[k]);
    k++;
  }

  file.close();
  return(true);
}


void key_setup() {
	readCodes();

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
	char key = i2cKeypad.getKey();
	
	if (key != NO_KEY) {
		LOGN(0, "%c,", key);
	}
	if (key == 'A') {
		LOG(0, "-->%c", key);
	}
}
