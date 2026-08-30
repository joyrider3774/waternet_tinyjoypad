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

#include <avr/eeprom.h>

#include "savestate.h"
#include "commonvars.h"
#include "sound.h"

// The Arduboy build squeezed five bits per entry because it shares its EEPROM
// with every other game on the device. The ATtiny85 has its 512 bytes all to
// itself, so a plain byte per game mode and difficulty is cheaper in flash
// and easier to follow. Nothing is mirrored in RAM; reads go to EEPROM.
//
//   0 .. 17   highest unlocked level per (mode, difficulty)
//   18        options, bit 0 = sound on

constexpr uint8_t entryCount = gmCount * diffCount;
static uint8_t *const addrLevels = (uint8_t *)0;
static uint8_t *const addrOptions = (uint8_t *)entryCount;

static uint8_t entryFor(uint8_t mode, uint8_t diff)
{
    return (mode * diffCount) + diff;
}

static uint8_t validateSaveState()
{
    for (uint8_t i = 0; i != entryCount; i++)
    {
        uint8_t unlocked = eeprom_read_byte(addrLevels + i);
        if ((unlocked == 0) || (unlocked > levelCount))
            return 0;
    }

    return eeprom_read_byte(addrOptions) <= 1;
}

void initSaveState()
{
    if (!validateSaveState())
    {
        // first run, or the block belonged to another game
        for (uint8_t i = 0; i != entryCount; i++)
            eeprom_update_byte(addrLevels + i, 1);   // level one is open
        eeprom_update_byte(addrOptions, 1);          // sound on
    }

    setSoundOn(isSoundOnSaveState());
}

uint8_t lastUnlockedLevel(uint8_t mode, uint8_t diff)
{
    return eeprom_read_byte(addrLevels + entryFor(mode, diff));
}

uint8_t levelUnlocked(uint8_t mode, uint8_t diff, uint8_t level)
{
    return lastUnlockedLevel(mode, diff) > level;
}

void unlockLevel(uint8_t mode, uint8_t diff, uint8_t level)
{
    if ((level + 1 > lastUnlockedLevel(mode, diff)) && (level < levelCount))
        eeprom_update_byte(addrLevels + entryFor(mode, diff), level + 1);
}

void setSoundOnSaveState(uint8_t value)
{
    eeprom_update_byte(addrOptions, value ? 1 : 0);
}

uint8_t isSoundOnSaveState()
{
    return eeprom_read_byte(addrOptions) & 1;
}
