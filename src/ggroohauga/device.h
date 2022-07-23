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

enum class LogicValue : int8_t {
	Low = -1,
	Unknown = 0,
	High = 1,
};

inline int operator*(const LogicValue &value) {
	return value == LogicValue::Low ? LOW : HIGH;
}

inline LogicValue& operator<<(LogicValue &lhs, const int &rhs) {
	return lhs = (rhs == LOW ? LogicValue::Low : LogicValue::High);
}

inline int& operator<<(int &lhs, const LogicValue &rhs) {
	return lhs = *rhs;
}

class Device {
public:
	static constexpr int BAUD_RATE = 57600;
	static constexpr int UART_CONFIG = SERIAL_8O1;
	static constexpr size_t MAX_MESSAGE_LEN = 259;

	Device(const __FlashStringHelper *name, HardwareSerial &serial,
		uint8_t rx_pin, uint8_t tx_pin, uint8_t detect_pin,
		uint8_t detect_mode, uint8_t announce_pin);

	void start();
	void loop(Device &other);

private:
	static constexpr unsigned long MAX_REPORT_DELAY_MS = 45;

	void report();

	uuid::log::Logger logger_;
	HardwareSerial &serial_;
	uint8_t rx_pin_;
	uint8_t tx_pin_;
	uint8_t detect_pin_;
	uint8_t detect_mode_;
	uint8_t announce_pin_;

	std::vector<uint8_t> buffer_;
	unsigned long last_millis_;
	LogicValue detect_ = LogicValue::Unknown;
};

} // namespace ggroohauga
