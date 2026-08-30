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

#include "sound.h"
#include "tinyjoypad.h"

// ELECTROLIB's Sound(freq, cycles) drives the buzzer by hand: it holds the pin
// high for (255 - freq) microseconds, then low for the same, once per cycle.
//
// Two consequences worth spelling out, because both are easy to get wrong:
//   - a larger freq is a *higher* note, and the lowest note it can produce is
//     only around 1.5kHz, since the half period cannot exceed 255us;
//   - the second argument is a number of square wave cycles, not a length of
//     time. A beep lasts cycles * 2 * (255 - freq) microseconds, so at a high
//     pitch a small cycle count is over almost instantly.
//
// The delay loop costs a little more than the 1us it asks for, so the real
// notes come out roughly a quarter lower than the nominal arithmetic suggests.
// The figures below are the measured-out approximations.
constexpr uint8_t noteLow = 5;      // ~1.5 kHz
constexpr uint8_t noteMid = 80;     // ~2.2 kHz
constexpr uint8_t noteHigh = 140;   // ~3.3 kHz
constexpr uint8_t noteTop = 180;    // ~5.1 kHz

static uint8_t soundOn = 1;

void setSoundOn(uint8_t value)
{
    soundOn = value ? 1 : 0;
}

uint8_t isSoundOn()
{
    return soundOn;
}

static void beep(uint8_t freq, uint8_t duration)
{
    if (soundOn)
        playTone(freq, duration);
}

// moving between menu entries: one short blip, about 18ms
void playMenuSelect()
{
    beep(noteHigh, 60);
}

// choosing an entry: a rising pair, about 50ms
void playMenuConfirm()
{
    beep(noteMid, 70);
    beep(noteTop, 90);
}

// backing out: the same pair falling, about 57ms
void playMenuBack()
{
    beep(noteTop, 60);
    beep(noteLow, 70);
}

// refusing an action: one low buzz, about 78ms
void playError()
{
    beep(noteLow, 120);
}

// moving the cursor around the board, kept very short as it repeats a lot
void playCursorMove()
{
    beep(noteHigh, 35);
}

// turning or sliding a pipe
void playGameAction()
{
    beep(noteTop, 45);
}

// finishing a level: a rising run, about 110ms
void playLevelClear()
{
    beep(noteLow, 60);
    beep(noteMid, 60);
    beep(noteHigh, 60);
    beep(noteTop, 140);
}
