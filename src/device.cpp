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

#include "ggroohauga/device.h"

#include <Arduino.h>

#include <functional>
#include <vector>

#include <uuid/log.h>

namespace ggroohauga {

Device::Device(const __FlashStringHelper *name, HardwareSerial &serial,
		uint8_t rx_pin, uint8_t tx_pin, bool wait,
		const std::vector<std::reference_wrapper<Proxy>> &proxies)
		: logger_(name, uuid::log::Facility::UUCP), serial_(serial),
		rx_pin_(rx_pin), tx_pin_(tx_pin), wait_for_other_(wait),
		proxies_(proxies) {

}

void Device::start(Device &other) {
	other_ = &other;
	waiting_ = wait_for_other_;

	for (auto &proxy : proxies_) {
		proxy.get().start(this);
	}
}

void Device::activate() {
	if (suspend_) {
		suspend_ = false;

		logger_.trace(F("Activate serial"));
		serial_.begin(BAUD_RATE, UART_CONFIG, rx_pin_, tx_pin_);
		waiting_ = wait_for_other_;
	}
}

void Device::deactivate() {
	if (!suspend_) {
		suspend_ = true;

		logger_.trace(F("Deactivate serial"));
		serial_.end();
		buffer_.clear();
	}
}

void Device::loop() {
	unsigned long now_ms = millis();
	int data = 0;

	for (auto &proxy : proxies_) {
		proxy.get().loop();
	}

	if (suspend_) {
		return;
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
			if (!waiting_) {
				other_->serial_.write(data);
				other_->waiting_ = false;
			}

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
				logger_.trace(F("%s%S"), &message.data()[1],
					waiting_ ? F(" [discarded]") : F(""));
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
	pinMode(pin_, INPUT);
}

void Monitor::activate() {
	if (suspend_) {
		suspend_ = false;
		pinMode(pin_, mode_);
	}
}

void Monitor::deactivate() {
	if (!suspend_) {
		suspend_ = true;
		pinMode(pin_, INPUT);
	}
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
		uint8_t dst_pin, bool invert, std::function<void(bool)> change_func)
		: Monitor(name, src_pin,
				on_state == LogicValue::High ? INPUT_PULLDOWN : INPUT_PULLUP),
			src_name_(src_name), dst_name_(dst_name), src_pin_(src_pin),
			dst_pin_(dst_pin), on_state_(on_state),
			debounce_on_millis_(debounce_on_millis),
			hold_off_millis_(hold_off_millis),
			invert_(invert), change_func_(change_func) {

}

void Proxy::start(Device *device) {
	Monitor::start(device);
	pinMode(dst_pin_, INPUT);
}

void Proxy::activate() {
	Monitor::activate();

	if (suspend_) {
		suspend_ = false;

		logger_.trace(F("Activate pin %d (%S) -> %d (%S)"),
				src_pin_, src_name_, dst_pin_, dst_name_);

		if (dst_value_ != LogicValue::Unknown) {
			LogicValue input_value = invert_ ? !dst_value_ : dst_value_;

			digitalWrite(dst_pin_, *dst_value_);
			pinMode(dst_pin_, OUTPUT);

			logger_.trace(F("Pin %d (%S) -> %d (%S): %S (%S -> %S) [unsuspended]"),
				src_pin_, src_name_, dst_pin_, dst_name_,
				input_value == on_state_ ? F("on") : F("off"),
				to_string(input_value), to_string(dst_value_));

			if (input_value == on_state_ && change_func_) {
				change_func_(true);
			}
		}
	}
}

void Proxy::deactivate() {
	if (!suspend_) {
		LogicValue input_value = invert_ ? !dst_value_ : dst_value_;

		suspend_ = true;

		logger_.trace(F("Deactivate pin %d (%S) -> %d (%S)"),
				src_pin_, src_name_, dst_pin_, dst_name_);

		pinMode(dst_pin_, INPUT);

		if (input_value == on_state_ && change_func_) {
			change_func_(false);
		}
	}

	Monitor::deactivate();
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
	LogicValue output_value = invert_ ? !value : value;

	if (dst_value_ != output_value) {
		bool suspended = suspend_;

		dst_value_ = output_value;

		if (!suspend_ && value != on_state_ && change_func_) {
			change_func_(false);
		}

		if (suspend_) {
			logger_.trace(F("Pin %d (%S) -> %d (%S): %S (%S -> %S) [suspended]"),
				src_pin_, src_name_, dst_pin_, dst_name_,
				value == on_state_ ? F("on") : F("off"),
				to_string(value), to_string(output_value));
		} else {
			if (!suspended) {
				digitalWrite(dst_pin_, *output_value);
				pinMode(dst_pin_, OUTPUT);

				logger_.trace(F("Pin %d (%S) -> %d (%S): %S (%S -> %S)"),
					src_pin_, src_name_, dst_pin_, dst_name_,
					value == on_state_ ? F("on") : F("off"),
					to_string(value), to_string(output_value));
			}

			if (value == on_state_ && change_func_) {
				change_func_(true);
			}
		}
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
