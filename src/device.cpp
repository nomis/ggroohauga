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

#include "ggroohauga/device.h"

#include <Arduino.h>

#include <vector>

#include <uuid/log.h>

namespace ggroohauga {

Device::Device(const __FlashStringHelper *name, HardwareSerial &serial, int rx_pin, int tx_pin)
		: logger_(name, uuid::log::Facility::UUCP), serial_(serial), rx_pin_(rx_pin), tx_pin_(tx_pin) {

}

void Device::start() {
	serial_.begin(BAUD_RATE, UART_CONFIG, rx_pin_, tx_pin_);
}

void Device::loop(Device &other) {
	unsigned long now_ms = millis();
	int data = 0;

	do {
		int available_rx = serial_.available();
		int available_tx = serial_.available();

		if (available_rx <= 0 || available_tx <= 0) {
			break;
		}

		while (available_rx-- > 0 && available_tx-- > 0) {
			data = serial_.read();

			if (data == -1) {
				break;
			}

			if (!other.buffer_.empty()) {
				other.report();
			}

			buffer_.push_back(data);
			other.serial_.write(data);

			if (buffer_.size() == MAX_MESSAGE_LEN) {
				report();
			}

			now_ms = millis();
			last_millis_ = now_ms;
		}
	} while (data != -1);

	if (!buffer_.empty() && now_ms - last_millis_ >= MAX_REPORT_DELAY_MS) {
		report();
	}
}

void Device::report() {
	if (logger_.enabled(uuid::log::Level::TRACE)) {
		static constexpr uint8_t BYTES_PER_LINE = 16;
		static constexpr uint8_t CHARS_PER_BYTE = 3;
		std::array<char, CHARS_PER_BYTE * BYTES_PER_LINE + 1> message{};
		const __FlashStringHelper *prefix = F("<-");
		uint8_t pos = 0;

		for (uint16_t i = 0; i < buffer_.size(); i++) {
			snprintf_P(&message[CHARS_PER_BYTE * pos++], CHARS_PER_BYTE + 1,
				PSTR(" %02X"), buffer_[i]);

			if (pos == BYTES_PER_LINE || i == buffer_.size() - 1) {
				logger_.trace(F("%S%s"), prefix, message.data());
				pos = 0;
				prefix = F("  ");
			}
		}
	}

	buffer_.clear();
}

} // namespace ggroohauga
