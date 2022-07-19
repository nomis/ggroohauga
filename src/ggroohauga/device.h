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

#include <uuid/log.h>

#include "../app/app.h"

namespace ggroohauga {

class Device {
public:
	static constexpr int BAUD_RATE = 57600;
	static constexpr int UART_CONFIG = SERIAL_8O1;
	static constexpr size_t MAX_MESSAGE_LEN = 259;

	Device(const __FlashStringHelper *name, HardwareSerial &serial, int rx_pin, int tx_pin);

	void start();
	void loop(Device &other);

private:
	static constexpr unsigned long MAX_REPORT_DELAY_MS = 45;

	void report();

	uuid::log::Logger logger_;
	HardwareSerial &serial_;
	int rx_pin_;
	int tx_pin_;
	std::vector<uint8_t> buffer_;
	unsigned long last_millis_;
};

} // namespace ggroohauga
