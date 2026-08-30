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

#ifndef COMMONVARS_H
#define COMMONVARS_H

#include <stdint.h>

// The screen is 16x8 tiles of 8x8 pixels. The board lives in the left
// 10 columns, the status sidebar in the right 6.
constexpr uint8_t screenWidth = 16;
constexpr uint8_t screenHeight = 8;

constexpr uint8_t maxBoardWidth = 10;
constexpr uint8_t maxBoardHeight = 8;
constexpr uint8_t maxBoardSize = maxBoardWidth * maxBoardHeight;

constexpr uint8_t maxBoardBgWidth = 10;
constexpr uint8_t maxBoardBgHeight = 8;

// game states
constexpr uint8_t gsTitle = 0;
constexpr uint8_t gsLevelSelect = 1;
constexpr uint8_t gsGame = 2;

// difficulties
constexpr uint8_t diffVeryEasy = 0;
constexpr uint8_t diffEasy = 1;
constexpr uint8_t diffNormal = 2;
constexpr uint8_t diffHard = 3;
constexpr uint8_t diffVeryHard = 4;
constexpr uint8_t diffRandom = 5;
constexpr uint8_t diffCount = 6;

// game modes
constexpr uint8_t gmRotate = 0;
constexpr uint8_t gmSlide = 1;
constexpr uint8_t gmRotateSlide = 2;
constexpr uint8_t gmCount = 3;

// main menu entries
constexpr uint8_t mmStartGame = 0;
constexpr uint8_t mmOptions = 1;
constexpr uint8_t mmCredits = 2;
constexpr uint8_t mmCount = 3;

// title screen steps
constexpr uint8_t tsMainMenu = 0;
constexpr uint8_t tsGameMode = 1;
constexpr uint8_t tsDifficulty = 2;
constexpr uint8_t tsOptions = 3;
constexpr uint8_t tsCredits = 4;

constexpr uint8_t levelCount = 25;

// tile numbers of the glyphs used outside of printMessage()
constexpr uint8_t tileBlank = 0xff;   // renderer sentinel: emit 8 empty columns
constexpr uint8_t arrowUp = 120;
constexpr uint8_t arrowRight = 121;
constexpr uint8_t arrowDown = 122;
constexpr uint8_t arrowLeft = 123;
constexpr uint8_t leftMenu = 118;

extern uint8_t startPos, maxLevel, selectedLevel, boardX, boardY, difficulty,
               gameState, boardWidth, boardHeight, boardSize, levelDone,
               titleStep, gameMode, posAdd, mainMenu, paused;

extern int8_t selectionX, selectionY;
extern uint16_t moves;
extern uint8_t level[maxBoardSize];

#endif
