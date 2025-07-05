/*
 * ggroohauga - Alternative console and simulated amplifier interface
 * Copyright 2022,2025  Simon Arlott
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ggroohauga/app.h"

#include <Arduino.h>

namespace ggroohauga {

App::App()
		: con_detect_(F("console"), F("detect"), CON_DETECT, LogicValue::Low,
			5, 5, F("announce"), CON_ANNOUNCE, false, {}),
		con_(F("console"), con_serial_, CON_TX, CON_RX, false, { con_detect_ }),
		amp_detect_(F("amplifier"), F("detect"), AMP_DETECT, LogicValue::Low,
			0, 0, F("announce"), AMP_ANNOUNCE, false, {}),
		power_(F("amplifier"), F("power-in"), AMP_POWER_IN, LogicValue::High,
			5, 5, F("power-out"), CON_POWER_OUT, true, [this] (bool on) {
				if (on) {
					power_on();
				} else {
					power_off();
				}
			}),
		amp_(F("amplifier"), amp_serial_, AMP_TX, AMP_RX, true, { amp_detect_, power_ }) {
}

void App::start() {
	app::App::start();

	con_.start(amp_);
	amp_.start(con_);
	amp_.activate();
	amp_detect_.activate();
	power_.activate();
	led_.begin();
}

void App::loop() {
	app::App::loop();

	con_.loop();
	amp_.loop();

	if (millis() - last_led_ms_ >= 1000) {
		led_.show();
		last_led_ms_ = millis();
	}
}

void App::power_on() {
	con_.activate();
	con_detect_.activate();
}

void App::power_off() {
	con_detect_.deactivate();
	con_.deactivate();
}


} // namespace ggroohauga
