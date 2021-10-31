// Copyright (c) 2021 steff393, MIT license

#include <Arduino.h>
#include <autoClose.h>
#include <globalConfig.h>
#include <logger.h>
#include <uap.h>

#define CYCLE_TIME                100		// ms
#define AC_TRIGGER_INTERVAL      5000   // ms    

static const uint8_t m = 5;

typedef enum {
	AC_INIT             = 0,
	AC_START            = 1,
	AC_PREWARN          = 2,
	AC_WAIT             = 3,
	AC_CLOSE            = 4,
	AC_ABORT_WAIT       = 5,
	PD_CORRECT_ERROR    = 10,
	PD_START            = 11,
	PD_GOTO_VENTING     = 12,
	PD_WAIT             = 13
} acState_t;

static acState_t acState        = AC_INIT;
static uint32_t  acTimer        = 0;
static uint32_t  lastCall       = 0;


void ac_trigger() {
	if (acState == AC_INIT) {
		LOG(m, "Start", "");
		acState = AC_START;
	}
}


void pd_trigger() {
	if (acState == AC_INIT && (uap_getBroadcast() & UAP_STATUS_CLOSED)) {
		LOG(m, "Start Paketdienst", "");
		if (uap_getBroadcast() & UAP_STATUS_ERROR) {
			// if an error is present, then it's not possible to go directly to venting. Send first a "close", wait and then start as usual
			uap_triggerAction(UAP_ACTION_CLOSE, SRC_AUTOCLOSE);
			acState = PD_CORRECT_ERROR;
			acTimer = millis();
		} else {
			acState = PD_START;
		}
	}
}


void ac_abort(bool forceStop) {
	// stop and abort everything
	LOG(m, "Aborted", "");
	acState = AC_ABORT_WAIT;
	acTimer = millis();
	if (forceStop) {
		LOG(m, "Stopped", "");
		uap_triggerAction(UAP_ACTION_STOP, SRC_AUTOCLOSE);
	}
}


void ac_loop() {
	if (millis() - lastCall < CYCLE_TIME) {
		// avoid unnecessary frequent calls 
		return;
	}
	lastCall = millis();

  uint32_t      seconds = log_getSecSinceMidnight();
	if ((seconds >= (uint32_t)cfgAcTime * 3600) && (seconds <= (uint32_t)cfgAcTime * 3600 + AC_TRIGGER_INTERVAL / 1000)) {
		// check for a 5s time-window to safely detect, but not trigger multiple times
		ac_trigger();
	}

	// --- STATE MACHINE ---
	switch (acState) {
		case AC_INIT: {
			break;
		}
		case AC_START: {
			// start a short closing to warn
			uap_triggerAction(UAP_ACTION_CLOSE, SRC_AUTOCLOSE);
			acState = AC_PREWARN;
			acTimer = millis();
			break;
		}
		case AC_PREWARN: {
			if (millis() - acTimer > cfgAcDur1 * 1000) {
				// wait to give people time to react
				uap_triggerAction(UAP_ACTION_STOP, SRC_AUTOCLOSE);
				acState = AC_WAIT;
				acTimer = millis();
			}
			break;
		}
		case AC_WAIT: {
			if (millis() - acTimer > cfgAcDur2 * 1000) {
				// continue the movement
				uap_triggerAction(UAP_ACTION_CLOSE, SRC_AUTOCLOSE);
				acState = AC_CLOSE;
				acTimer = millis();
			}
			// if during AC_WAIT a movement is detected, then abort the auto-closeing (but give 1000ms to stop actually)
			if ((uap_getBroadcast() & UAP_STATUS_MOVING) && (millis() - acTimer > 1000)) {
				ac_abort(false);
			}
			break;
		}
		case AC_CLOSE:             // fall-through
		case AC_ABORT_WAIT: {
			// wait to avoid triggering again
			if (millis() - acTimer > AC_TRIGGER_INTERVAL + 1000) {
				acState = AC_INIT;
				acTimer = 0;
			}
			break;
		}


		case PD_CORRECT_ERROR: {
			if (millis() - acTimer > cfgPdWaitError) {
				acState = PD_START;
			}
			break;
		}
		case PD_START: {
			// start the first movement, e.g. a short closing to warn
			uap_triggerAction(UAP_ACTION_VENTING, SRC_AUTOCLOSE);
			acState = PD_GOTO_VENTING;
			acTimer = millis();
			break;
		}
		case PD_GOTO_VENTING: {
			if ((uap_getBroadcast() & UAP_STATUS_VENTPOS) || (millis() - acTimer > cfgPdTimeout * 1000)) {
				// venting position reached or timeout elapsed
				acState = PD_WAIT;
				acTimer = millis();
			}
			break;
		}
		case PD_WAIT: {
			if (millis() - acTimer > cfgPdWaitTime * 1000) {
				// close again
				uap_triggerAction(UAP_ACTION_CLOSE, SRC_AUTOCLOSE);
				acState = AC_CLOSE;
			}
			break;
		}
		default: ; // do nothing
	}
}
