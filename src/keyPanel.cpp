// Copyright (c) 2021 steff393, MIT license
// based on https://github.com/joeyoung/arduino_keypads

#include <Arduino.h>
#include <globalConfig.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <logger.h>
#include <Wire.h>

#define CYCLE_TIME                100		// ms
#define I2C_Addr 0x20                   // I2C Address of PCF8574-board: 0x20 - 0x27

const uint8_t m = 6;

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

//define the symbols on the buttons of the keypads
char keyPadLayout[ROWS][COLS] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};

byte rowPins[ROWS] = {3, 2, 1, 0}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 6, 5, 4}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad_I2C i2cKeypad( makeKeymap(keyPadLayout), rowPins, colPins, ROWS, COLS, I2C_Addr); 


void key_setup() {
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
		LOG(m, "%c pressed", key);
	}
}
