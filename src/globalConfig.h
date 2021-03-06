// Copyright (c) 2021 steff393, MIT license

#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#define PIN_DI              5   // GPIO5, NodeMCU pin D1
#define PIN_RO              2   // GPIO2, NodeMCU pin D4
#define PIN_DE_RE           4   // GPIO4, NodeMCU pin D2
#define PIN_BTN             0   // GPIO0, NodeMCU pin D3, FLASH button
#define PIN_SDA            14   // GPIO14,NodeMCU pin D5
#define PIN_SCL            12   // GPIO12,NodeMCU pin D6
#define PIN_RST             0   // GPIO0, NodeMCU pin D3 
#define PIN_SS             15   // GPIO15,NodeMCU pin D8

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
extern uint8_t  cfgPdTimeOn;                 // Hour to  enable package drop function
extern uint8_t  cfgPdTimeOff;                // Hour to disable package drop function
extern uint8_t  cfgPdWaitTime;               // Duration of the package drop  WAIT phase [s]
extern uint8_t  cfgPdTimeout;                // Timeout of the package drop function, when venting position is missed [s]
extern uint16_t cfgPdWaitError;              // Wait time for error correction before start [ms]
extern uint8_t  cfgHwVersion;                // Selection of the used HW
extern uint16_t cfgHeapReset;                // Minimal heap limit before a reset is executed [bytes]
extern uint8_t  cfgLogMonths;                // Months to be logged
extern uint8_t  cfgTrace;                    // 0: disable Trace Feature, 1: enable
extern uint8_t  cfgAutoErrorCorr;            // 0: disable AutoErrorCorrection, 1: enable
extern uint8_t  cfgMasterAddr;               // Master address: 128 (0x80) per default, 144 (0x90) for HAP1-HCP-Adapter

extern void loadConfig();

#endif	/* GLOBALCONFIG_H */
