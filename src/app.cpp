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
		: con_(F("Console"), con_serial_, CON_RX, CON_TX),
		amp_(F("Amplifier"), amp_serial_, AMP_RX, AMP_TX) {

}

void App::start() {
	app::App::start();

	con_.start();
	amp_.start();
}

void App::loop() {
	app::App::loop();

	con_.loop(amp_);
	amp_.loop(con_);
}

} // namespace ggroohauga
