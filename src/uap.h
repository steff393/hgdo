// Copyright (c) 2021 steff393, MIT license
// based on: https://github.com/stephan192/hoermann_door/blob/main/pic16/hoermann.h

#ifndef UAP_H
#define UAP_H

typedef enum
{
  UAP_ACTION_STOP         = 0,
  UAP_ACTION_OPEN         = 1,
  UAP_ACTION_CLOSE        = 2,
  UAP_ACTION_VENTING      = 3,
  UAP_ACTION_TOGGLE_LIGHT = 4
} uap_action_t;

extern void     uap_setup();
extern void     uap_loop();
extern uint16_t uap_getBroadcast(void);
extern void     uap_triggerAction(uap_action_t action);

#endif /* UAP_H */
