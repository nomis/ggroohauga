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

#pragma once

#include <Arduino.h>

#include <vector>

#include "../app/app.h"
#include "device.h"

namespace ggroohauga {

class App: public app::App {
private:
#if defined(ARDUINO_LOLIN_S2_MINI)
	static constexpr int CON_RX = 35;
	static constexpr int CON_TX = 34;
	static constexpr int CON_DETECT = 33;
	static constexpr int CON_ANNOUNCE = 36;
	static constexpr int CON_POWER = 15;
	static constexpr auto &con_serial_ = Serial0;

	static constexpr int AMP_RX = 18;
	static constexpr int AMP_TX = 17;
	static constexpr int AMP_DETECT = 16;
	static constexpr int AMP_ANNOUNCE = 21;
	static constexpr auto &amp_serial_ = Serial1;
#else
# error "Unknown board"
#endif

public:
	App();

	void start() override;
	void loop() override;

private:
	Device con_;
	Device amp_;
};

} // namespace ggroohauga
