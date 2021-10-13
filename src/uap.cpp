// Copyright (c) 2021 steff393, MIT license
// based on: https://github.com/stephan192/hoermann_door/blob/main/pic16/hoermann.c

#include <Arduino.h>
#include <globalConfig.h>
#include <logger.h>
#include <SoftwareSerial.h>
#include <uap.h>


#define CYCLE_TIME                1   // ms
#define CYCLE_TIME_SLOW         100   // ms

#define BROADCAST_ADDR            0x00
#define MASTER_ADDR               0x80
#define UAP1_ADDR                 0x28

#define UAP1_TYPE                 0x14

#define CMD_SLAVE_SCAN            0x01
#define CMD_SLAVE_STATUS_REQUEST  0x20
#define CMD_SLAVE_STATUS_RESPONSE 0x29

#define RESPONSE_DEFAULT          0x1000
#define RESPONSE_STOP             0x0000
#define RESPONSE_OPEN             0x1001
#define RESPONSE_CLOSE            0x1002
#define RESPONSE_VENTING          0x1010
#define RESPONSE_TOGGLE_LIGHT     0x1008

#define CRC8_INITIAL_VALUE        0xF3

static const uint8_t m  = 8;

/* CRC table for polynomial 0x07 */
static const uint8_t crctable[256] = {
	0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
	0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
	0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
	0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
	0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
	0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
	0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
	0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
	0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
	0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
	0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
	0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
	0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
	0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
	0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
	0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

static SoftwareSerial S;
static uint32_t lastCall       = 0;
static uint32_t lastCallSlow   = 0;

static uint8_t rx_buffer[15+3] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static bool rx_message_ready = false;

static uint8_t tx_buffer[15+3] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static bool tx_message_ready = false;
static uint8_t tx_length = 0;

static uint16_t slave_respone_data = RESPONSE_DEFAULT;
static uint16_t broadcast_status = 0;
static uint16_t broadcast_status_old = 0;
static lastMove_t lastMove = UNKNOWN;
static boolean ignoreNextEvent = true;     // will also ignore wrong edge detection after reset

static const char *src[7] = {"Unbekannt: ", "Websocket: ", "Webserver: ", "Taster: ", "Auto-Close: ", "Tastenfeld: ", "RFID: "};

static void printByte(byte b) {
	if (b < 16) {
		Serial.print("0");
	}
	Serial.print(b, HEX);
	Serial.print(" ");
}

static uint8_t calc_crc8(uint8_t *p_data, uint8_t length) {
	uint8_t i;
	uint8_t data;
	uint8_t crc = CRC8_INITIAL_VALUE;
	
	for(i = 0; i < length; i++) {
		/* XOR-in next input byte */
		data = *p_data ^ crc;
		p_data++;
		/* get current CRC value = remainder */
		crc = crctable[data];
	}
	
	return crc;
}


static void parse_message(void) {
	uint8_t length;
	uint8_t counter;
	
	length = rx_buffer[1] & 0x0F;
	counter = (rx_buffer[1] & 0xF0) + 0x10;
	
	if(rx_buffer[0] == BROADCAST_ADDR) {
		if(length == 0x02) {
			broadcast_status = rx_buffer[2];
			broadcast_status |= (uint16_t)rx_buffer[3] << 8;
			Serial.print(" Broadcast");
		}
	}
	if(rx_buffer[0] == UAP1_ADDR)	{
		// Bus scan command?
		if((length == 0x02) && (rx_buffer[2] == CMD_SLAVE_SCAN)) {
			tx_buffer[0] = MASTER_ADDR;
			tx_buffer[1] = 0x02 | counter;
			tx_buffer[2] = UAP1_TYPE;
			tx_buffer[3] = UAP1_ADDR;
			tx_buffer[4] = calc_crc8(tx_buffer, 4);
			tx_length = 5;
			tx_message_ready = true;
			Serial.print(" BusScan UAP1");
		}
		// Slave status request command?
		if((length == 0x01) && (rx_buffer[2] == CMD_SLAVE_STATUS_REQUEST)) {
			tx_buffer[0] = MASTER_ADDR;
			tx_buffer[1] = 0x03 | counter;
			tx_buffer[2] = CMD_SLAVE_STATUS_RESPONSE;
			tx_buffer[3] = (uint8_t)slave_respone_data;
			tx_buffer[4] = (uint8_t)(slave_respone_data>>8);
			slave_respone_data = RESPONSE_DEFAULT;
			tx_buffer[5] = calc_crc8(tx_buffer, 5);
			tx_length = 6;
			tx_message_ready = true;
			Serial.print(" SlaveStatusReq");
		}
	}
	Serial.println("");
}


uap_status_t uap_getBroadcast(void) {
  return (uap_status_t) broadcast_status;
}


void uap_triggerAction(uap_action_t action, uap_source_t source /*= SRC_OTHER*/) {
  char msg[50];
	strcpy(msg, src[source]);
	switch(action) {
    case UAP_ACTION_STOP: {
			strcat_P(msg, PSTR("Stop"));
      slave_respone_data = RESPONSE_STOP;
      break;
    }
    case UAP_ACTION_OPEN: {
			strcat_P(msg, PSTR("Öffnen"));
      slave_respone_data = RESPONSE_OPEN;
      break;
    }
    case UAP_ACTION_CLOSE: {
			strcat_P(msg, PSTR("Schließen"));
      slave_respone_data = RESPONSE_CLOSE;
      break;
    }
    case UAP_ACTION_VENTING: {
			strcat_P(msg, PSTR("Lüftung"));
      slave_respone_data = RESPONSE_VENTING;
      break;
    }
    case UAP_ACTION_TOGGLE_LIGHT: {
			strcat_P(msg, PSTR("Licht"));
      slave_respone_data = RESPONSE_TOGGLE_LIGHT;
      break;
    }
		default: ; // do nothing
  }
	ignoreNextEvent = true;
	log_file(msg);
}


static void receive() {
	static int8_t    counter      = 0;
	static uint8_t   length       = 0;
	static bool      syncNeeded   = true;
	static byte      prevData1    = 0;
	static byte      prevData2    = 0;
	byte data;
	
	while (S.available()) {
		data = S.read();	// read buffer, but ignore as long as last message was not parsed
		printByte(data);	// print to serial as hex value

		if (syncNeeded) {
			// wait for the sequence 01 80 CRC and then start again
			if ((prevData1 == CMD_SLAVE_SCAN) && (prevData2 == MASTER_ADDR)) {
				prevData1 = 0;
				prevData2 = 0;
				syncNeeded = false;
				Serial.println("Sync found");
			} else {
				// do nothing, just copy the data
				prevData1 = prevData2;
				prevData2 = data;
			}
			return;
		}

		if (!rx_message_ready) {
			rx_buffer[counter] = data;
			counter++;
			if (counter == 2) {
				length = (data & 0x0F) + 3; // 3 = ADR + LEN + CRC 
			} else if (counter == length) {
				if(calc_crc8(rx_buffer, length) == 0x00) {
					rx_message_ready = true;
					Serial.print(" CRC ok ");
				} else {
					Serial.print(" CRC nok ");
					syncNeeded = true;
				}
				Serial.print(millis());
				counter = 0;
			}
		}
	}
}

static void transmit() {
	for (uint8_t i=0; i<tx_length; i++) {
		printByte(tx_buffer[i]);	// print to serial as hex value
	}

	// Generate Sync break
	Serial.print(" Transmit "); Serial.print(millis());
	digitalWrite(PIN_DE_RE, HIGH);		// LOW = listen, HIGH = transmit
	S.begin(9600, SWSERIAL_7N1);
	S.write(0x00);
	S.flush();

	// Transmit
	S.begin(19200, SWSERIAL_8N1);
	S.write(tx_buffer, tx_length);
	S.flush();
	tx_message_ready = false;
	digitalWrite(PIN_DE_RE, LOW);		// LOW = listen, HIGH = transmit
	Serial.print(" finished: "); Serial.println(millis());
}


void uap_setup() {
	LOG(m, "HwVersion: %d", cfgHwVersion);
	if (cfgHwVersion == 10) {
		S.begin(19200, SWSERIAL_8N1, PIN_DI, PIN_RO); // inverted
	} else {
		S.begin(19200, SWSERIAL_8N1, PIN_RO, PIN_DI);
	}
	digitalWrite(PIN_DE_RE, LOW);		// LOW = listen, HIGH = transmit
}


void uap_loop() {
	static uint8_t delay_counter = 0;
	receive();

	if (millis() - lastCall < CYCLE_TIME) {
		// avoid unnecessary frequent calls 
		return;
	}
	lastCall = millis();

	if (rx_message_ready) {
		parse_message();
		rx_message_ready = false;
		delay_counter = 3;
    // Wait 3ms before answering. If not the Supramatic doesn't accept our answer.
	}

	if(cfgTxEnable && tx_message_ready && (delay_counter == 0)) {
		transmit();
	}

	if (delay_counter > 0) {
    delay_counter--;
  }
}


static boolean posEdge(const uint16_t mask, const uint16_t value) {
	if (((broadcast_status & mask) == value) && ((broadcast_status_old & mask) != value)) {
		return(true);
	} else {
		return(false);
	}
}


void uap_loop_slow() {
	if (millis() - lastCallSlow < CYCLE_TIME_SLOW) {
		// avoid unnecessary frequent calls 
		return;
	}
	lastCallSlow = millis();

	// check for status changes
	if (broadcast_status != broadcast_status_old) {
		if (ignoreNextEvent) {
			ignoreNextEvent = false;
		} else {
			char msg[50];
			strcpy(msg, src[SRC_OTHER]);
			if (posEdge(UAP_STATUS_OPEN, UAP_STATUS_OPEN)) {
				strcat_P(msg, PSTR("Offen"));
				log_file(msg);
			} else if (posEdge(UAP_STATUS_CLOSED, UAP_STATUS_CLOSED)) {
				strcat_P(msg, PSTR("Geschlossen"));
				log_file(msg);
			} else if (posEdge(UAP_STATUS_MOVING, 0)) {
				strcat_P(msg, PSTR("Stop"));
				log_file(msg);
			}
			if (posEdge(UAP_STATUS_CLOSING, UAP_STATUS_CLOSING)) {
				strcat_P(msg, PSTR("Schließen"));
				log_file(msg);
			}
			if (posEdge(UAP_STATUS_CLOSING, UAP_STATUS_MOVING)) {
				strcat_P(msg, PSTR("Öffnen"));
				log_file(msg);
			}
		}
	}
	broadcast_status_old = broadcast_status;
	
	// store the last move
	if (broadcast_status & UAP_STATUS_MOVING) {
		if (broadcast_status & UAP_STATUS_DIRECTION) {
			lastMove = DOWN;
		} else {
			lastMove = UP;
		}
	}	
}


lastMove_t uap_getLastMove() {
	return(lastMove);
}