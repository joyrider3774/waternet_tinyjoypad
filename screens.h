/*
 * Waternet for TinyJoypad (ATtiny85)
 * Copyright (c) 2026 Willems Davy (joyrider3774)
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SCREENS_H
#define SCREENS_H

#include <stdint.h>

// Runs one frame of whichever screen is current: acts on the input that was
// just read, then rebuilds the tile map from the resulting state.
void updateScreens();

// Free running counter used as an entropy source. There is no clock to read
// on an ATtiny85, so the seed comes from how long the player took to press.
extern uint32_t seedTicker;

#endif
