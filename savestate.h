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

#ifndef SAVESTATE_H
#define SAVESTATE_H

#include <stdint.h>

void initSaveState();

uint8_t levelUnlocked(uint8_t mode, uint8_t diff, uint8_t level);
uint8_t lastUnlockedLevel(uint8_t mode, uint8_t diff);
void unlockLevel(uint8_t mode, uint8_t diff, uint8_t level);

void setSoundOnSaveState(uint8_t value);
uint8_t isSoundOnSaveState();

#endif
