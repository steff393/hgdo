// Copyright (c) 2021 steff393, MIT license

#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#define PIN_DI              2   // GPIO2, NodeMCU pin D4 --> connect to DI (Transmit to Modbus)
#define PIN_RO              5   // GPIO5, NodeMCU pin D1 --> connect to RO (Receive from Modbus)
#define PIN_DE_RE           4   // GPIO4, NodeMCU pin D2 --> connect to DE & RE
#define PIN_BTN             0   // GPIO0, NodeMCU pin D3, FLASH button
#define PIN_SDA            14   // GPIO14,NodeMCU pin D5
#define PIN_SCL            12   // GPIO12,NodeMCU pin D6

extern char     cfgHgdoVersion[];            // hgdo version
extern char     cfgBuildDate[];              // hgdo build date

extern char     cfgApSsid[32];               // SSID of the initial Access Point
extern char     cfgApPass[63];               // Password of the initial Access Point
extern char     cfgNtpServer[30];            // NTP server
extern uint8_t  cfgTxEnable;                 // 0: disable TX, 1: enable TX
extern uint8_t  cfgTimeOn;                   // Hour to  enable button [h]
extern uint8_t  cfgTimeOff;                  // Hour to disable button [h]
extern uint16_t cfgBtnDebounce;              // Debounce time for button [ms]
extern uint8_t  cfgAcTime;                   // Hour to start the auto-close [h]
extern uint8_t  cfgAcDur1;                   // Duration of the auto-close PREWARN phase [s]
extern uint8_t  cfgAcDur2;                   // Duration of the auto-close WAIT    phase [s]


extern void loadConfig();

#endif	/* GLOBALCONFIG_H */
