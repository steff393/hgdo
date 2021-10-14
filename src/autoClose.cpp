// Copyright (c) 2021 steff393, MIT license

#include <Arduino.h>
#include <globalConfig.h>
#include <logger.h>
#include <uap.h>

#define CYCLE_TIME                100		// ms
#define TWO_MINUTES            120000   // ms

static const uint8_t m = 5;

typedef enum {
	AC_INIT             = 0,
	AC_START            = 1,
	AC_PREWARN          = 2,
	AC_WAIT             = 3,
	AC_CLOSE            = 4,
	AC_ABORT_WAIT       = 5
} acState_t;

static acState_t acState        = AC_INIT;
static uint32_t  acTimer        = 0;
static uint32_t  lastCall       = 0;


void ac_trigger() {
	// trigger an automatic closing with pre-warning
	if (acState == AC_INIT) {
		LOG(m, "Started", "");
		acState = AC_START;
	}
}

void ac_abort(bool forceStop) {
	// stop and abort everything
	LOG(m, "Aborted", "");
	acState = AC_ABORT_WAIT;
	acTimer = millis();
	if (forceStop) {
		LOG(m, "Stopped", "");
		uap_triggerAction(UAP_ACTION_STOP);
	}
}


void ac_loop() {
	if (millis() - lastCall < CYCLE_TIME) {
		// avoid unnecessary frequent calls 
		return;
	}
	lastCall = millis();

	uint8_t      hour = log_getHours();
  uint8_t      mins = log_getMinutes();

	if ((hour == cfgAcTime) && (mins == 0)) {
		ac_trigger();
	}

	switch (acState) {
		case AC_INIT: {
			break;
		}
		case AC_START: {
			// start a short closing to warn
			uap_triggerAction(UAP_ACTION_CLOSE);
			acState = AC_PREWARN;
			acTimer = millis();
			break;
		}
		case AC_PREWARN: {
			if (millis() - acTimer > cfgAcDur1 * 1000) {
				// wait to give people time to react
				uap_triggerAction(UAP_ACTION_STOP);
				acState = AC_WAIT;
				acTimer = millis();
			}
			break;
		}
		case AC_WAIT: {
			if (millis() - acTimer > cfgAcDur2 * 1000) {
				// continue the movement
				uap_triggerAction(UAP_ACTION_CLOSE);
				acState = AC_CLOSE;
				acTimer = millis();
			}
			// if during AC_WAIT a movement is detected, then abort the auto-closeing (but give 1000ms to stop actually)
			uap_status_t bc = uap_getBroadcast();
			if ((millis() - acTimer > 1000) && (bc & UAP_STATUS_MOVING)) {
				ac_abort(false);
			}
			break;
		}
		case AC_CLOSE:             // fall-through
		case AC_ABORT_WAIT: {
			// wait more than a minute to avoid triggering again
			if (millis() - acTimer > TWO_MINUTES) {
				acState = AC_INIT;
				acTimer = 0;
			}
			break;
		}
		default: ; // do nothing
	}
}
