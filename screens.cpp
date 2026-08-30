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

#include "screens.h"
#include "commonvars.h"
#include "render.h"
#include "level.h"
#include "sound.h"
#include "savestate.h"
#include "tinyjoypad.h"

uint32_t seedTicker = 1;

// Held rather than re-read from seedTicker so that leaving a randomly
// generated level and coming back rebuilds the very same board.
static uint32_t randomSeedGame = 1;

// Unlike the Arduboy build there is no framebuffer to preserve between
// frames, so every screen simply rebuilds the 128 byte tile map each pass.

static void drawBoard()
{
    // the arrows the sliding modes are played with
    if (gameMode != gmRotate)
    {
        for (uint8_t x = 0; x != boardWidth; x++)
        {
            setTile(boardX + x, boardY - 1, arrowDown);
            setTile(boardX + x, boardY + boardHeight, arrowUp);
        }

        for (uint8_t y = 0; y != boardHeight; y++)
        {
            setTile(boardX - 1, boardY + y, arrowRight);
            setTile(boardX + boardWidth, boardY + y, arrowLeft);
        }
    }

    uint8_t i = 0;
    for (uint8_t y = 0; y != boardHeight; y++)
        for (uint8_t x = 0; x != boardWidth; x++)
            setTile(boardX + x, boardY + y, level[i++]);
}

// Moves the selection, wrapping around the board. The sliding modes let it
// step one cell outside the board onto the arrows, which is what posAdd is.
static void moveSelection(int8_t dx, int8_t dy)
{
    if (dx != 0)
    {
        selectionX += dx;
        if (selectionX >= (int8_t)(boardWidth + posAdd))
            selectionX = -(int8_t)posAdd;
        else if (selectionX < -(int8_t)posAdd)
            selectionX = (int8_t)(boardWidth - 1 + posAdd);
    }

    if (dy != 0)
    {
        selectionY += dy;
        if (selectionY >= (int8_t)(boardHeight + posAdd))
            selectionY = -(int8_t)posAdd;
        else if (selectionY < -(int8_t)posAdd)
            selectionY = (int8_t)(boardHeight - 1 + posAdd);
    }

    playCursorMove();
}

/*------------------------------- title screen -----------------------------*/

static void handleTitle()
{
    if (pressed(btnUp))
    {
        switch (titleStep)
        {
            case tsMainMenu:   if (mainMenu > 0)   { mainMenu--;   playMenuSelect(); } break;
            case tsGameMode:   if (gameMode > 0)   { gameMode--;   playMenuSelect(); } break;
            case tsDifficulty: if (difficulty > 0) { difficulty--; playMenuSelect(); } break;
            default: break;
        }
    }

    if (pressed(btnDown))
    {
        switch (titleStep)
        {
            case tsMainMenu:   if (mainMenu + 1 < mmCount)     { mainMenu++;   playMenuSelect(); } break;
            case tsGameMode:   if (gameMode + 1 < gmCount)     { gameMode++;   playMenuSelect(); } break;
            case tsDifficulty: if (difficulty + 1 < diffCount) { difficulty++; playMenuSelect(); } break;
            default: break;
        }
    }

    if (pressed(btnB) && (titleStep != tsMainMenu))
    {
        titleStep = (titleStep == tsDifficulty) ? tsGameMode : tsMainMenu;
        playMenuBack();
        return;
    }

    if (!pressed(btnA))
        return;

    switch (mainMenu)
    {
        case mmOptions:
            if (titleStep != tsOptions)
            {
                titleStep = tsOptions;
            }
            else
            {
                setSoundOn(!isSoundOn());
                setSoundOnSaveState(isSoundOn());
            }
            playMenuConfirm();
            break;

        case mmCredits:
            titleStep = (titleStep != tsCredits) ? tsCredits : tsMainMenu;
            playMenuConfirm();
            break;

        default:
            // start game: pick the mode, then the difficulty, then play
            if (titleStep < tsDifficulty)
            {
                titleStep++;
                playMenuConfirm();
            }
            else
            {
                selectedLevel = (difficulty == diffRandom)
                              ? 1
                              : lastUnlockedLevel(gameMode, difficulty);

                // the sliding modes give up a ring of cells to their arrows
                posAdd = (gameMode == gmRotate) ? 0 : 1;

                randomSeedGame = seedTicker;
                initLevel(randomSeedGame);
                gameState = gsLevelSelect;
                playMenuConfirm();
            }
            break;
    }
}

static void drawTitle()
{
    hideCursor();
    clearScreen();
    printMessage(4, 1, TXT("WATERNET"));

    switch (titleStep)
    {
        case tsMainMenu:
            printMessage(5, 4, TXT("START"));
            printMessage(5, 5, TXT("OPTIONS"));
            printMessage(5, 6, TXT("CREDITS"));
            setTile(4, 4 + mainMenu, leftMenu);
            break;

        case tsGameMode:
            printMessage(5, 4, TXT("ROTATE"));
            printMessage(5, 5, TXT("SLIDE"));
            printMessage(5, 6, TXT("ROSLID"));
            setTile(4, 4 + gameMode, leftMenu);
            break;

        case tsDifficulty:
            printMessage(3, 2, TXT("VERY EASY"));
            printMessage(3, 3, TXT("EASY"));
            printMessage(3, 4, TXT("NORMAL"));
            printMessage(3, 5, TXT("HARD"));
            printMessage(3, 6, TXT("VERY HARD"));
            printMessage(3, 7, TXT("RANDOM"));
            setTile(2, 2 + difficulty, leftMenu);
            break;

        case tsOptions:
            printMessage(4, 4, isSoundOn() ? TXT("SOUND ON") : TXT("SOUND OFF"));
            setTile(3, 4, leftMenu);
            break;

        default:
            printMessage(3, 4, TXT("CREATED BY"));
            printMessage(2, 5, TXT("WILLEMS DAVY"));
            printMessage(2, 6, TXT("JOYRIDER3774"));
            break;
    }
}

/*------------------------------ level select ------------------------------*/

static void handleLevelSelect()
{
    if (pressed(btnB))
    {
        playMenuBack();
        titleStep = tsMainMenu;
        gameState = gsTitle;
        return;
    }

    if (pressed(btnA))
    {
        if (levelUnlocked(gameMode, difficulty, selectedLevel - 1))
        {
            playMenuConfirm();
            paused = 0;
            gameState = gsGame;
            return;
        }

        playError();
    }

    // With the random difficulty there is no level list to walk, so left and
    // right simply deal a fresh board.
    if (pressed(btnLeft))
    {
        if (difficulty == diffRandom)
        {
            playMenuSelect();
            randomSeedGame = seedTicker;
            initLevel(randomSeedGame);
        }
        else if (selectedLevel > 1)
        {
            playMenuSelect();
            selectedLevel--;
            initLevel(randomSeedGame);
        }
    }

    if (pressed(btnRight))
    {
        if (difficulty == diffRandom)
        {
            playMenuSelect();
            randomSeedGame = seedTicker;
            initLevel(randomSeedGame);
        }
        else if (selectedLevel < maxLevel)
        {
            playMenuSelect();
            selectedLevel++;
            initLevel(randomSeedGame);
        }
    }
}

static void drawLevelSelect()
{
    hideCursor();
    clearScreen();

    printMessage(maxBoardBgWidth, 0, TXT("LEVEL:"));
    printNumber(maxBoardBgWidth + 4, 1, selectedLevel, 2);

    printMessage(maxBoardBgWidth, 2,
                 levelUnlocked(gameMode, difficulty, selectedLevel - 1) ? TXT("OPEN") : TXT("LOCKED"));

    printMessage(maxBoardBgWidth, 4, TXT("a:"));
    printMessage(maxBoardBgWidth, 5, TXT("PLAY"));
    printMessage(maxBoardBgWidth, 6, TXT("HOLD"));
    printMessage(maxBoardBgWidth, 7, TXT("BACK"));

    drawBoard();
}

/*---------------------------------- game ----------------------------------*/

// Applies the action fire triggers on the current cell: a rotate inside the
// board, a row or column slide when sitting on one of the arrows.
static void doAction()
{
    uint8_t insideX = (selectionX >= 0) && (selectionX < (int8_t)boardWidth);
    uint8_t insideY = (selectionY >= 0) && (selectionY < (int8_t)boardHeight);

    if (insideX && insideY)
    {
        if (gameMode == gmSlide)
        {
            playError();
            return;
        }
        rotateBlock((uint8_t)selectionX + ((uint8_t)selectionY * boardWidth));
    }
    else if (insideX)
    {
        if (selectionY < 0)
            moveBlockDown((uint8_t)selectionX);
        else
            moveBlockUp((uint8_t)selectionX + ((uint8_t)(boardHeight - 1) * boardWidth));
    }
    else if (insideY)
    {
        if (selectionX < 0)
            moveBlockRight((uint8_t)selectionY * boardWidth);
        else
            moveBlockLeft((uint8_t)(boardWidth - 1) + ((uint8_t)selectionY * boardWidth));
    }
    else
    {
        // a corner, which belongs to no row or column
        playError();
        return;
    }

    moves++;
    playGameAction();
    updateConnected();
    levelDone = isLevelDone();

    if (levelDone)
        playLevelClear();
}

static void nextLevel()
{
    if (difficulty == diffRandom)
    {
        randomSeedGame = seedTicker;
        initLevel(randomSeedGame);
        return;
    }

    unlockLevel(gameMode, difficulty, selectedLevel);

    if (selectedLevel < maxLevel)
    {
        selectedLevel++;
        initLevel(randomSeedGame);
    }
    else
    {
        // every level of this mode and difficulty is finished
        gameState = gsLevelSelect;
    }
}

static void handleGame()
{
    if (levelDone)
    {
        if (pressed(btnA))
        {
            playMenuConfirm();
            nextLevel();
        }
        return;
    }

    if (paused)
    {
        if (pressed(btnA))
        {
            playMenuConfirm();
            paused = 0;
        }
        else if (pressed(btnB))
        {
            playMenuBack();
            paused = 0;
            // hand the selector back the level in its starting state
            initLevel(randomSeedGame);
            gameState = gsLevelSelect;
        }
        return;
    }

    if (pressed(btnUp))    moveSelection(0, -1);
    if (pressed(btnDown))  moveSelection(0, 1);
    if (pressed(btnLeft))  moveSelection(-1, 0);
    if (pressed(btnRight)) moveSelection(1, 0);

    if (pressed(btnA))
        doAction();
    else if (pressed(btnB))
    {
        playMenuBack();
        paused = 1;
    }
}

static void drawGame()
{
    if (paused || levelDone)
        hideCursor();
    else
        setCursorTile(boardX + selectionX, boardY + selectionY);

    clearScreen();

    printMessage(maxBoardBgWidth, 0, TXT("LEVEL:"));
    printNumber(maxBoardBgWidth + 4, 1, selectedLevel, 2);

    printMessage(maxBoardBgWidth, 2, TXT("MOVES:"));
    printNumber(maxBoardBgWidth + 1, 3, moves, 5);

    printMessage(maxBoardBgWidth, 4, TXT("a:"));
    switch (gameMode)
    {
        case gmRotate: printMessage(maxBoardBgWidth, 5, TXT("ROTATE")); break;
        case gmSlide:  printMessage(maxBoardBgWidth, 5, TXT("SLIDE"));  break;
        default:       printMessage(maxBoardBgWidth, 5, TXT("ROSLID")); break;
    }

    printMessage(maxBoardBgWidth, 6, TXT("HOLD"));
    printMessage(maxBoardBgWidth, 7, TXT("BACK"));

    drawBoard();

    if (paused)
    {
        printMessage(0, 1, TXT("[**************]"));
        printMessage(0, 2, TXT("|PLEASE CONFIRM+"));
        printMessage(0, 3, TXT("|              +"));
        printMessage(0, 4, TXT("|   a PLAY     +"));
        printMessage(0, 5, TXT("|  HOLD TO QUIT+"));
        printMessage(0, 6, TXT("<##############>"));
    }
    else if (levelDone)
    {
        printMessage(1, 2, TXT("[************]"));
        printMessage(1, 3, TXT("| LEVEL DONE +"));
        printMessage(1, 4, TXT("|            +"));
        printMessage(1, 5, TXT("| a CONTINUE +"));
        printMessage(1, 6, TXT("<############>"));
    }
}

/*--------------------------------------------------------------------------*/

void updateScreens()
{
    // Input is acted on before drawing so a move shows up in the same frame,
    // and the draw is dispatched again afterwards in case the state changed.
    switch (gameState)
    {
        case gsTitle:       handleTitle();       break;
        case gsLevelSelect: handleLevelSelect(); break;
        default:            handleGame();        break;
    }

    switch (gameState)
    {
        case gsTitle:       drawTitle();       break;
        case gsLevelSelect: drawLevelSelect(); break;
        default:            drawGame();        break;
    }
}
