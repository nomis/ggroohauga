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

#include "ggroohauga/console.h"

#include <memory>
#include <string>
#include <vector>

#include <uuid/console.h>
#include <uuid/log.h>

#include "ggroohauga/app.h"
#include "app/config.h"
#include "app/console.h"

using ::uuid::flash_string_vector;
using ::uuid::console::Commands;
using ::uuid::console::Shell;
using LogLevel = ::uuid::log::Level;
using LogFacility = ::uuid::log::Facility;

using ::app::AppShell;
using ::app::CommandFlags;
using ::app::Config;
using ::app::ShellContext;

#define MAKE_PSTR(string_name, string_literal) static const char __pstr__##string_name[] __attribute__((__aligned__(sizeof(int)))) PROGMEM = string_literal;
#define MAKE_PSTR_WORD(string_name) MAKE_PSTR(string_name, #string_name)
#define F_(string_name) FPSTR(__pstr__##string_name)

namespace ggroohauga {

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wunused-const-variable"
#pragma GCC diagnostic pop

static inline App &to_app(Shell &shell) {
	return static_cast<App&>(dynamic_cast<app::AppShell&>(shell).app_);
}

static inline GgroohaugaShell &to_shell(Shell &shell) {
	return dynamic_cast<GgroohaugaShell&>(shell);
}

#define NO_ARGUMENTS std::vector<std::string>{}

static inline void setup_commands(std::shared_ptr<Commands> &commands) {

}

GgroohaugaShell::GgroohaugaShell(app::App &app) : Shell(), AppShell(app) {

}

void GgroohaugaShell::display_banner() {
	AppShell::display_banner();
	println(F("┌─────────────┐"));
	println(F("│   🧙 DEAN   │"));
	println(F("│  BORN TO 🤘 │"));
	println(F("│🎸 LIVE FATS │"));
	println(F("│DIE YO GNU 🎶│"));
	println(F("└─────────────┘"));
	println();
}

} // namespace ggroohauga

namespace app {

void setup_commands(std::shared_ptr<Commands> &commands) {
	ggroohauga::setup_commands(commands);
}

} // namespace app
