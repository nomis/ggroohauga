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

inline const __FlashStringHelper* to_string(const LogicValue &value) {
	switch (value) {
	case LogicValue::Low:
		return F("LOW");
	case LogicValue::High:
		return F("HIGH");
	case LogicValue::Unknown:
		break;
	}
	return F("UNKNOWN");
}

inline LogicValue operator!(const LogicValue &value) {
	switch (value) {
	case LogicValue::Low:
		return LogicValue::High;
	case LogicValue::High:
		return LogicValue::Low;
	case LogicValue::Unknown:
		break;
	}
	return LogicValue::Unknown;
}

class Device;

class Monitor {
public:
	Monitor(const __FlashStringHelper *name, uint8_t pin, uint8_t mode);
	virtual ~Monitor() = default;

	virtual void start(Device *device = nullptr);
	virtual void loop();

protected:
	virtual void changed(LogicValue value);

	uuid::log::Logger logger_;
	Device *device_ = nullptr;

private:
	const uint8_t pin_;
	const uint8_t mode_;
	LogicValue value_ = LogicValue::Unknown;
};

class Proxy: public Monitor {
public:
	Proxy(const __FlashStringHelper *name,
		const __FlashStringHelper *src_name, uint8_t src_pin,
		LogicValue on_state, unsigned long debounce_on_millis,
		unsigned long hold_off_millis, const __FlashStringHelper *dst_name,
		uint8_t dst_pin);

	void start(Device *device = nullptr) override;
	void loop() override;

protected:
	void changed(LogicValue value) override;

private:
	void update(LogicValue value);
	void log(LogicValue value);

	const __FlashStringHelper *src_name_;
	const __FlashStringHelper *dst_name_;
	const uint8_t src_pin_;
	const uint8_t dst_pin_;
	const LogicValue on_state_;
	const unsigned long debounce_on_millis_;
	const unsigned long hold_off_millis_;
	LogicValue dst_value_ = LogicValue::Unknown;
	bool on_pending_ = false;
	bool hold_ = false;
	unsigned long debounce_start_millis_;
	unsigned long hold_start_millis_;
};

class Device {
public:
	static constexpr int BAUD_RATE = 57600;
	static constexpr int UART_CONFIG = SERIAL_8O1;
	static constexpr size_t MAX_MESSAGE_LEN = 259;

	Device(const __FlashStringHelper *name, HardwareSerial &serial,
		uint8_t rx_pin, uint8_t tx_pin, const std::vector<Proxy> &proxies);

	void start(Device &other);
	void loop();
	void report_both();

private:
	static constexpr unsigned long MAX_REPORT_DELAY_MS = 45;

	void report();

	uuid::log::Logger logger_;
	HardwareSerial &serial_;
	uint8_t rx_pin_;
	uint8_t tx_pin_;
	std::vector<Proxy> proxies_;
	Device *other_;

	std::vector<uint8_t> buffer_;
	unsigned long last_millis_;
};

} // namespace ggroohauga
