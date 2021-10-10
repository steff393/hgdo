// Copyright (c) 2021 steff393, MIT license

#ifndef RFID_H
#define RFID_H

extern void 		rfid_setup();
extern void 		rfid_loop();
extern boolean 	rfid_getEnabled();
extern boolean 	rfid_getReleased();
extern char *   rfid_getLastID();

#endif /* RFID_H */
