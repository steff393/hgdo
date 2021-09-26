// Copyright (c) 2021 steff393, MIT license

#include <globalConfig.h>
#include <hcp.h>
#include <SoftwareSerial.h>

SoftwareSerial S;


void hcp_setup() {
  //S.begin(19200, SWSERIAL_8N1, PIN_RO, PIN_DI); 
	S.begin(19200, SWSERIAL_8E1, PIN_DI, PIN_RO); // Heidelberg TEST & PINs VERTAUSCHT
	digitalWrite(PIN_DE_RE, LOW);		// LOW = listen, HIGH = transmit
}


void hcp_loop() {
	if (S.available()) {
		Serial.print(S.read(), HEX);
		Serial.print(" ");
	}
}