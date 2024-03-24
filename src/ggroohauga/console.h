/*
 * ggroohauga - Alternative console and simulated amplifier interface
 * Copyright 2022,2024  Simon Arlott
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

#include "app/console.h"

#include <memory>
#include <string>
#include <vector>

namespace ggroohauga {

class GgroohaugaShell: public app::AppShell {
public:
	~GgroohaugaShell() override = default;

protected:
	GgroohaugaShell(app::App &app, Stream &stream, unsigned int context, unsigned int flags);

	void display_banner() override;
};

} // namespace ggroohauga
