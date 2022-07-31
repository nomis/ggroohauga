/*
 * ggroohauga - Alternative console and simulated amplifier interface
 * Copyright 2022  Simon Arlott
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
		: con_(F("console"), con_serial_, CON_RX, CON_TX,
			{
				Proxy(F("console"), F("detect"), CON_DETECT, LogicValue::Low, 0, 1000, F("announce"), CON_ANNOUNCE),
			}),
		amp_(F("amplifier"), amp_serial_, AMP_RX, AMP_TX,
			{
				Proxy(F("amplifier"), F("detect"), AMP_DETECT, LogicValue::High, 0, 0, F("announce"), AMP_ANNOUNCE),
				Proxy(F("amplifier"), F("power-in"), AMP_POWER_IN, LogicValue::High, 50, 1000, F("power-out"), CON_POWER_OUT),
			}) {

}

void App::start() {
	app::App::start();

	// Turn LED off
	digitalWrite(LED_PIN, LOW);
	pinMode(LED_PIN, OUTPUT);

	con_.start(amp_);
	amp_.start(con_);
}

void App::loop() {
	app::App::loop();

	con_.loop();
	amp_.loop();
}

} // namespace ggroohauga
