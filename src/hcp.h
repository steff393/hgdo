// Copyright (c) 2021 steff393, MIT license
// based on: https://github.com/stephan192/hoermann_door/blob/main/pic16/hoermann.h

#ifndef HCP_H
#define HCP_H

typedef enum
{
  HCP_ACTION_STOP         = 0,
  HCP_ACTION_OPEN         = 1,
  HCP_ACTION_CLOSE        = 2,
  HCP_ACTION_VENTING      = 3,
  HCP_ACTION_TOGGLE_LIGHT = 4
} hcp_action_t;

extern void     hcp_setup();
extern void     hcp_loop();
extern uint16_t hcp_getBroadcast(void);
extern void     hcp_triggerAction(hcp_action_t action);

#endif /* HCP_H */
