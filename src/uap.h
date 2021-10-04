// Copyright (c) 2021 steff393, MIT license
// based on: https://github.com/stephan192/hoermann_door/blob/main/pic16/hoermann.h

#ifndef UAP_H
#define UAP_H

typedef enum {
  UAP_ACTION_STOP         = 0,
  UAP_ACTION_OPEN         = 1,
  UAP_ACTION_CLOSE        = 2,
  UAP_ACTION_VENTING      = 3,
  UAP_ACTION_TOGGLE_LIGHT = 4
} uap_action_t;

typedef enum {
  UAP_STATUS_OPEN         = 0x0001,
  UAP_STATUS_CLOSED       = 0x0002,
  UAP_STATUS_ERROR        = 0x0010,
  UAP_STATUS_DIRECTION    = 0x0020,
  UAP_STATUS_MOVING       = 0x0040,
  UAP_STATUS_CLOSING      = 0x0060,
  UAP_STATUS_VENTPOS      = 0x0080
} uap_status_t;

typedef enum {
  SRC_OTHER               = 0,
  SRC_WEBSOCKET           = 1,
  SRC_WEBSERVER           = 2,
  SRC_BUTTON              = 3,
  SRC_AUTOCLOSE           = 4,
  SRC_KEYPAD              = 5
} uap_source_t;

extern void         uap_setup();
extern void         uap_loop();
extern uap_status_t uap_getBroadcast(void);
extern void         uap_triggerAction(uap_action_t action, uap_source_t source = SRC_OTHER);

#endif /* UAP_H */
