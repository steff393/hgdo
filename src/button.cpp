// Copyright (c) 2021 steff393, MIT license

#include <Arduino.h>
#include <autoClose.h>
#include <Bounce2.h>
#include <globalConfig.h>
#include <logger.h>
#include <uap.h>

#define CYCLE_TIME                1		// ms

const uint8_t m = 4;

typedef enum {
  UNKNOWN   = 0,
  UP        = 1,
  DOWN      = 2
} lastMove_t;

static lastMove_t lastMove = UNKNOWN;
static Bounce2::Button btn = Bounce2::Button();


static void triggerButton(bool allowOpen) {
	uap_status_t bc = uap_getBroadcast();

	if ((bc & UAP_STATUS_OPEN) ||                         // when door is completely open   OR
		(!(bc & UAP_STATUS_MOVING) && (lastMove == UP))) {  // when door is standstill and last move was UP   then
		uap_triggerAction(UAP_ACTION_CLOSE, SRC_BUTTON);                // close
		return;
	} 

	if (((bc & UAP_STATUS_CLOSED) && allowOpen) ||         // when door is completely closed AND Opening is allowed   OR
		 (!(bc & UAP_STATUS_MOVING) && (lastMove == DOWN))) {// when door is standstill and last move was DOWN   then
		uap_triggerAction(UAP_ACTION_OPEN, SRC_BUTTON);                  // open
		return;
	}

	// else stop (also when the status is not known, this is the safe state)
	uap_triggerAction(UAP_ACTION_STOP, SRC_BUTTON);
}


void btn_setup() {
	btn.attach(PIN_BTN, INPUT_PULLUP); // USE INTERNAL PULL-UP
	btn.interval(cfgBtnDebounce);
	btn.setPressedState(LOW);
}


void btn_loop() {
	// store the last move
	uap_status_t bc = uap_getBroadcast();
	if (bc & UAP_STATUS_MOVING) {
		if (bc & UAP_STATUS_DIRECTION) {
			lastMove = DOWN;
		} else {
			lastMove = UP;
		}
	}	

	btn.update();
	if (btn.pressed()) {
		LOG(m, "Pressed", "");
		uint8_t hour = log_getHours();
		ac_abort(false);  // abort a possible auto-close, but don't force the stop, this will be done by triggerButton() anyway
		triggerButton(hour >= cfgTimeOn && hour < cfgTimeOff);
	}
}
