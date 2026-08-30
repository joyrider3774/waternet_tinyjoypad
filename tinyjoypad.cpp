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

#include <Arduino.h>
#include <util/delay.h>

#include "tinyjoypad.h"
// ELECTROLIB defines its helpers in the header, so it may only be included
// from this one translation unit. It also expects _delay_us() to already be
// in scope, hence the util/delay.h above.
#include "ELECTROLIB.h"

// How long fire must be held before it counts as B rather than A, counted in
// frames rather than off a timer.
//
// A frame is one pass of the main loop, and its length barely varies: it is
// dominated by shifting a full 1KB of pixels out to the panel. The bit banged
// i2c_write() costs about 125 cycles a byte, and a frame sends roughly 1064 of
// them, so a frame is around 8-10ms at 16MHz - call it 110 frames a second.
// 45 frames is therefore a bit under half a second.
//
// This deliberately uses no timer peripheral. The ATtiny85's Timer1 is a
// chip-specific 8 bit unit that looks nothing like the ATmega part of the same
// name, and simulators do not always implement it; anything reading TCNT1 works
// on hardware but silently never triggers where that register stays at zero.
// The frame count has no such dependency and is accurate enough for telling a
// tap from a deliberate hold.
constexpr uint8_t longPressFrames = 45;

static uint8_t edges;        // buttons that triggered this frame
static uint8_t heldDirs;     // directions held on the previous frame
static uint8_t heldFrames;   // frames fire has been down for
static uint8_t fireHandled;  // set once a hold has already fired B

void initHardware()
{
    TINYJOYPAD_INIT();
}

void playTone(uint8_t freq, uint8_t duration)
{
    Sound(freq, duration);
}

void readInput()
{
    edges = 0;

    // Directions fire on the press. The Arduboy build acted on the release,
    // which feels sluggish with an analogue stick.
    uint8_t dirs = 0;
    if (TINYJOYPAD_UP)    dirs |= btnUp;
    if (TINYJOYPAD_DOWN)  dirs |= btnDown;
    if (TINYJOYPAD_LEFT)  dirs |= btnLeft;
    if (TINYJOYPAD_RIGHT) dirs |= btnRight;

    edges |= (uint8_t)(dirs & ~heldDirs);
    heldDirs = dirs;

    if (BUTTON_DOWN)
    {
        if (heldFrames < longPressFrames)
            heldFrames++;

        // B triggers as soon as the hold is long enough, without waiting for
        // the release, so the pause box appears while the button is down.
        if ((heldFrames >= longPressFrames) && !fireHandled)
        {
            edges |= btnB;
            fireHandled = 1;
        }
    }
    else
    {
        // a press let go before it became a hold is an A press
        if (heldFrames && !fireHandled)
            edges |= btnA;

        heldFrames = 0;
        fireHandled = 0;
    }
}

uint8_t pressed(uint8_t button)
{
    return (edges & button) ? 1 : 0;
}
