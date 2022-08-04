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

Device::Device(const __FlashStringHelper *name, HardwareSerial &serial,
		uint8_t rx_pin, uint8_t tx_pin, const std::vector<Proxy> &proxies)
		: logger_(name, uuid::log::Facility::UUCP), serial_(serial),
		rx_pin_(rx_pin), tx_pin_(tx_pin), proxies_(proxies) {

}

void Device::start(Device &other) {
	other_ = &other;

	serial_.begin(BAUD_RATE, UART_CONFIG, rx_pin_, tx_pin_);

	for (auto &proxy : proxies_) {
		proxy.start(this);
	}
}

void Device::loop() {
	unsigned long now_ms = millis();
	int data = 0;

	for (auto &proxy : proxies_) {
		proxy.loop();
	}

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

			if (!other_->buffer_.empty()) {
				other_->report();
			}

			if (data == 0xAA
					&& !buffer_.empty()
					&& buffer_[0] != 0xAA) {
				report();
			}

			buffer_.push_back(data);
			other_->serial_.write(data);

			if (buffer_.size() == MAX_MESSAGE_LEN) {
				report();
			} else if (buffer_.size() >= 4
					&& buffer_[0] == 0xAA
					&& buffer_.size() >= buffer_[2] + 4) {
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

void Device::report_both() {
	other_->report();
	report();
}

void Device::report() {
	if (logger_.enabled(uuid::log::Level::TRACE)) {
		static constexpr uint8_t BYTES_PER_LINE = 24;
		static constexpr uint8_t CHARS_PER_BYTE = 3;
		std::array<char, CHARS_PER_BYTE * BYTES_PER_LINE + 1> message{};
		uint8_t pos = 0;

		for (uint16_t i = 0; i < buffer_.size(); i++) {
			snprintf_P(&message[CHARS_PER_BYTE * pos++], CHARS_PER_BYTE + 1,
				PSTR(" %02X"), buffer_[i]);

			if (pos == BYTES_PER_LINE || i == buffer_.size() - 1) {
				logger_.trace(F("%s"), &message.data()[1]);
				pos = 0;
			}
		}
	}

	buffer_.clear();
}

Monitor::Monitor(const __FlashStringHelper *name, uint8_t pin, uint8_t mode)
		: logger_(name, uuid::log::Facility::UUCP), pin_(pin), mode_(mode) {

}

void Monitor::start(Device *device) {
	device_ = device;
	pinMode(pin_, mode_);
}

void Monitor::loop() {
	LogicValue value;

	value << digitalRead(pin_);

	if (value != value_) {
		changed(value);
		value_ = value;
	}
}

void Monitor::changed(LogicValue value) {
	if (device_)
		device_->report_both();

	logger_.trace(F("Pin %d: %S"), pin_,
		value == LogicValue::High ? F("HIGH") : F("LOW"));
}

Proxy::Proxy(const __FlashStringHelper *name,
		const __FlashStringHelper *src_name, uint8_t src_pin,
		LogicValue on_state, unsigned long debounce_on_millis,
		unsigned long hold_off_millis, const __FlashStringHelper *dst_name,
		uint8_t dst_pin)
		: Monitor(name, src_pin,
				on_state == LogicValue::High ? INPUT_PULLDOWN : INPUT_PULLUP),
			src_name_(src_name), dst_name_(dst_name), src_pin_(src_pin),
			dst_pin_(dst_pin), on_state_(on_state),
			debounce_on_millis_(debounce_on_millis),
			hold_off_millis_(hold_off_millis) {

}

void Proxy::start(Device *device) {
	Monitor::start(device);
	digitalWrite(dst_pin_, !*on_state_);
	pinMode(dst_pin_, OUTPUT);
}

void Proxy::loop() {
	Monitor::loop();

	if (hold_ && ::millis() - hold_start_millis_ >= hold_off_millis_) {
		hold_ = false;
	}

	if (on_pending_ && !hold_
			&& ::millis() - debounce_start_millis_ >= debounce_on_millis_) {
		device_->report_both();

		update(on_state_);
		on_pending_ = false;
	}
}

void Proxy::changed(LogicValue value) {
	device_->report_both();

	if (value == on_state_) {
		if (debounce_on_millis_ > 0) {
			debounce_start_millis_ = ::millis();
			on_pending_ = true;
		} else if (hold_) {
			on_pending_ = true;
		}

		if (on_pending_) {
			log(value);
		} else {
			update(value);
		}
	} else {
		if (hold_off_millis_ > 0 && dst_value_ != LogicValue::Unknown) {
			hold_ = true;
			hold_start_millis_ = ::millis();
		}

		on_pending_ = false;
		update(value);
	}
}

void Proxy::update(LogicValue value) {
	if (dst_value_ != value) {
		digitalWrite(dst_pin_, *value);
		if (dst_value_ == LogicValue::Unknown) {
			pinMode(dst_pin_, OUTPUT);
		}
		dst_value_ = value;

		logger_.trace(F("Pin %d (%S) -> %d (%S): %S (%S)"),
			src_pin_, src_name_, dst_pin_, dst_name_,
			value == on_state_ ? F("on") : F("off"), to_string(value));
	} else {
		log(value);
	}
}

void Proxy::log(LogicValue value) {
	logger_.trace(F("Pin %d (%S): %S (%S)%S%S"),
		src_pin_, src_name_,
		value == on_state_ ? F("on") : F("off"), to_string(value),
		(on_pending_ && debounce_on_millis_ > 0) ? F(" [debounce]") : F(""),
		hold_ ? F(" [hold]") : F(""));
}

} // namespace ggroohauga
