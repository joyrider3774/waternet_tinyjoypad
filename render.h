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

#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <avr/pgmspace.h>

// The ATtiny85 has no room for a 1KB pixel framebuffer, but Waternet is
// entirely built from 8x8 tiles on a 16x8 grid - which lines up exactly with
// the SSD1306's eight 8-pixel pages. So the "framebuffer" is a 128 byte tile
// map, and renderScreen() expands it to pixels one page at a time straight
// onto the I2C bus.

// convenience wrapper so call sites read like normal string literals
#define TXT(s) ((const char *)PSTR(s))

void clearScreen();
void setTile(uint8_t x, uint8_t y, uint8_t tile);
void fillTiles(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t tile);
void printMessage(uint8_t x, uint8_t y, const char *msg);
void printNumber(uint8_t x, uint8_t y, uint16_t value, uint8_t digits);

void setCursorTile(uint8_t x, uint8_t y);
void hideCursor();
void updateCursorBlink();

void renderScreen();

#endif
