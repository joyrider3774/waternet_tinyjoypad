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

#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

// The TinyJoypad drives its buzzer from a blocking busy loop, so there is no
// background music here - only short effects, as on the Arduboy build's
// sound (not music) channel.

void setSoundOn(uint8_t value);
uint8_t isSoundOn();

void playMenuSelect();
void playMenuConfirm();
void playMenuBack();
void playError();
void playCursorMove();
void playGameAction();
void playLevelClear();

#endif
