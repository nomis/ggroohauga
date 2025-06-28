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

#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include <vector>

#include "app/app.h"
#include "device.h"

namespace ggroohauga {

class App: public app::App {
private:
#if defined(ARDUINO_LOLIN_S3)
	static constexpr int LED_PIN = 38;

	static constexpr int CON_RX = 4; /* MCU TX (Console RX) */
	static constexpr int CON_TX = 6; /* MCU RX (Console TX) */
	static constexpr int CON_DETECT = 17;
	static constexpr int CON_ANNOUNCE = 48; /* no glitches on power cycle */
	static constexpr int CON_POWER_OUT = 40; /* no glitches on power cycle */
	static constexpr auto &con_serial_ = Serial1;

	static constexpr int AMP_RX = 10; /* MCU TX (Amplifier RX) */
	static constexpr int AMP_TX = 9; /* MCU RX (Amplifier TX) */
	static constexpr int AMP_DETECT = 14;
	static constexpr int AMP_ANNOUNCE = 13;
	static constexpr int AMP_POWER_IN = 8;
	static constexpr auto &amp_serial_ = Serial2;
#elif defined(ARDUINO_ESP_S3_DEVKITC)
	static constexpr int LED_PIN = 38;

	static constexpr int CON_RX = 4; /* MCU TX (Console RX) */
	static constexpr int CON_TX = 6; /* MCU RX (Console TX) */
	static constexpr int CON_DETECT = 17;
	static constexpr int CON_ANNOUNCE = 35; /* no glitches on power cycle */
	static constexpr int CON_POWER_OUT = 40; /* no glitches on power cycle */
	static constexpr auto &con_serial_ = Serial1;

	static constexpr int AMP_RX = 10; /* MCU TX (Amplifier RX) */
	static constexpr int AMP_TX = 9; /* MCU RX (Amplifier TX) */
	static constexpr int AMP_DETECT = 48;
	static constexpr int AMP_ANNOUNCE = 47;
	static constexpr int AMP_POWER_IN = 8;
	static constexpr auto &amp_serial_ = Serial2;
#elif defined(ARDUINO_ESP_S3_DEVKITM)
	static constexpr int LED_PIN = 48;

	static constexpr int CON_RX = 2; /* MCU TX (Console RX) */
	static constexpr int CON_TX = 4; /* MCU RX (Console TX) */
	static constexpr int CON_DETECT = 8;
	static constexpr int CON_ANNOUNCE = 36; /* no glitches on power cycle */
	static constexpr int CON_POWER_OUT = 41; /* no glitches on power cycle */
	static constexpr auto &con_serial_ = Serial1;

	static constexpr int AMP_RX = 14; /* MCU TX (Amplifier RX) */
	static constexpr int AMP_TX = 13; /* MCU RX (Amplifier TX) */
	static constexpr int AMP_DETECT = 33;
	static constexpr int AMP_ANNOUNCE = 26;
	static constexpr int AMP_POWER_IN = 10;
	static constexpr auto &amp_serial_ = Serial2;
#else
# error "Unknown board"
#endif

public:
	App();

	void start() override;
	void loop() override;

private:
	void power_on();
	void power_off();

	Proxy con_detect_;
	Device con_;
	Proxy amp_detect_;
	Proxy power_;
	Device amp_;

	Adafruit_NeoPixel led_{1, LED_PIN, NEO_GRB | NEO_KHZ800};
	unsigned long last_led_ms_{0};
};

} // namespace ggroohauga
