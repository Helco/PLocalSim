#include "globals.h"

//
// This file implements the BatteryStateService
//
// The battery acts like a real one would with this semantics:
//
// .charge_percent | .is_charging | .is_plugged |
//    10 ... 90    | false        | false       |
//    <90          | true         | true        |
//    90           | false        | true        |
//
// Basically if "plugged in" is_charging will be true unless we reach 90%.
// In all other cases it will be false.
//

static BatteryChargeState battery_state = {
	.charge_percent = 90,
	.is_charging = false,
	.is_plugged = false
};

static BatteryStateHandler battery_callback = NULL;
static bool battery_notify = false;

// simulator access
// note that the SDK up to the current beta3 only reports 10-90, never 100 or 0

uint8_t battery_charge_increase(void) {
	if(battery_state.charge_percent < 90) {
		battery_state.charge_percent += 10;
		battery_notify = true;
	}
	else {
		battery_state.is_charging = false;
	}
	return battery_state.charge_percent;
}

uint8_t battery_charge_decrease(void) {
	if(battery_state.charge_percent > 10) {
		battery_state.charge_percent -= 10;
		battery_notify = true;
	}
	if(battery_state.is_plugged) {
		battery_state.is_charging = true;
	}
	return battery_state.charge_percent;
}

void toggle_battery_charger_plugged(void) {
	battery_state.is_plugged = !battery_state.is_plugged;
	
	if(battery_state.is_plugged) {
		battery_state.is_charging = (battery_state.charge_percent < 90);
	}
	else {
		battery_state.is_charging = false;
	}

	battery_notify = true;
}

// pebble API part

BatteryChargeState battery_state_service_peek(void) {
	return battery_state;
}

void battery_state_service_subscribe(BatteryStateHandler handler) {
	battery_callback = handler;
	battery_notify = true;
}

void battery_state_service_unsubscribe() {
	battery_callback = NULL;
	battery_notify = false;
}

void service_battery() {
	if(battery_notify && battery_callback != NULL) {
		battery_callback(battery_state);
		battery_notify = false;
	}
}

