// Copyright (c) 2021 steff393, MIT license

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

// standard log
#define LOG(MODULE, TEXT, ...)    {char s[100]; snprintf_P(s, sizeof(s), PSTR(TEXT), __VA_ARGS__); log(MODULE, s);}
// standard log without newline
#define LOGN(MODULE, TEXT, ...)   {char s[100]; snprintf_P(s, sizeof(s), PSTR(TEXT), __VA_ARGS__); log(MODULE, s, false);}
// large log
#define LOGEXT(MODULE, TEXT, ...) {char s[600]; snprintf_P(s, sizeof(s), PSTR(TEXT), __VA_ARGS__); log(MODULE, s);};

extern void logger_begin();
extern void logger_loop();

extern void     log(uint8_t module, const char *msg, boolean newLine=true);
extern String   log_time();
extern uint8_t  log_getHours();
extern uint8_t  log_getMinutes();
extern uint32_t log_unixTime();

extern char* log_getBuffer(); 
extern void log_freeBuffer(); 

extern void log_file(const char *msg);

#endif /* LOGGER_H */
