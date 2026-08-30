/*
 * Waternet for TinyJoypad (ATtiny85)
 * Copyright (c) 2026 Willems Davy (joyrider3774)
 *
 * A port of the Arduboy version of Waternet, itself a port of the Game Boy
 * original. The puzzle is to turn or slide the pipes until water reaches
 * every one of them from the source in the middle of the board.
 *
 * For TinyJoypad rev2 (ATtiny85), running at 16MHz from the internal PLL.
 * Build with ATTinyCore, LTO enabled and millis() disabled.
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

#include "commonvars.h"
#include "render.h"
#include "screens.h"
#include "level.h"
#include "savestate.h"
#include "tinyjoypad.h"
#include "FastTinyDriver.h"

void setup()
{
    initHardware();
    TinyOLED_init();

    // reads the stored progress, and restores the sound setting
    initSaveState();

    gameState = gsTitle;
    titleStep = tsMainMenu;
    mainMenu = mmStartGame;
    gameMode = gmRotate;
    difficulty = diffNormal;
    selectedLevel = 1;
    posAdd = 0;
    paused = 0;
    levelDone = 0;
}

void loop()
{
    // keeps running so that the moment a player presses fire is unpredictable
    seedTicker++;

    readInput();
    updateCursorBlink();
    updateScreens();
    renderScreen();
}
