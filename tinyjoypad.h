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

#ifndef TINYJOYPAD_H
#define TINYJOYPAD_H

#include <stdint.h>

// The TinyJoypad has a four way stick and a single fire button, but Waternet
// was written for the Arduboy's two buttons. A tap of fire is A (rotate,
// slide, confirm) and holding it down is B (pause, back).

constexpr uint8_t btnUp = 0x01;
constexpr uint8_t btnDown = 0x02;
constexpr uint8_t btnLeft = 0x04;
constexpr uint8_t btnRight = 0x08;
constexpr uint8_t btnA = 0x10;
constexpr uint8_t btnB = 0x20;

void initHardware();

// Samples the controls once per frame and works out the edges. Everything
// else in the game asks pressed() rather than touching the pins.
void readInput();
uint8_t pressed(uint8_t button);

// Blocks for the duration of the tone, as the TinyJoypad has no sound timer.
void playTone(uint8_t freq, uint8_t duration);

#endif
