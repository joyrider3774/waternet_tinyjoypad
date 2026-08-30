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

#include <string.h>

#include "level.h"
#include "commonvars.h"

// A tile value is a 4 bit wall mask (bit 0 = north, 1 = east, 2 = south,
// 3 = west) plus 16 per tile set: set 0 is dry pipe, set 1 is water filled,
// set 2 is the water source. updateConnected() recomputes the set every move.

// Scratch space for the flood fill and the maze carver. It lives here rather
// than on the stack in each of them because initLevel() still holds the
// generator's frame while it calls updateConnected(), and two 81 byte local
// buffers plus their callers will not fit in the ATtiny85's 512 bytes. The
// two never run at the same time, so one buffer serves both.
static uint8_t cellStack[maxBoardSize + 1];

static uint32_t rngState = 1;

// Arduino's random() drags in far more code than an ATtiny85 can spare, so
// this is a plain xorshift32.
void rndSeed(uint32_t seed)
{
    rngState = seed ? seed : 1;
}

uint8_t rnd(uint8_t limit)
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return (uint8_t)((rngState >> 16) % limit);
}

void moveBlockDown(uint8_t aTile)
{
    uint8_t tmp = level[aTile + boardSize - boardWidth];
    for (uint8_t i = boardSize - boardWidth; i != 0; i -= boardWidth)
        level[aTile + i] = level[aTile + i - boardWidth];
    level[aTile] = tmp;
}

void moveBlockUp(uint8_t aTile)
{
    uint8_t tmp = level[aTile - boardSize + boardWidth];
    for (uint8_t i = boardSize - boardWidth; i != 0; i -= boardWidth)
        level[aTile - i] = level[aTile - i + boardWidth];
    level[aTile] = tmp;
}

void moveBlockRight(uint8_t aTile)
{
    uint8_t tmp = level[aTile + boardWidth - 1];
    memmove(&level[aTile + 1], &level[aTile], boardWidth - 1);
    level[aTile] = tmp;
}

void moveBlockLeft(uint8_t aTile)
{
    uint8_t tmp = level[aTile - boardWidth + 1];
    memmove(&level[aTile - boardWidth + 1], &level[aTile - boardWidth + 2], boardWidth - 1);
    level[aTile] = tmp;
}

// Turning a pipe a quarter turn clockwise moves north to east, east to south
// and so on, which is just a 4 bit rotate of the wall mask. The tile drops
// back to the dry set because updateConnected() recomputes the water anyway.
void rotateBlock(uint8_t aTile)
{
    uint8_t walls = level[aTile] & 0x0f;
    level[aTile] = (uint8_t)(((walls << 1) | (walls >> 3)) & 0x0f);
}

static void shuffleSlide(uint8_t aTile)
{
    switch (rnd(4))
    {
        case 0: moveBlockUp((aTile % boardWidth) + boardSize - boardWidth); break;
        case 1: moveBlockDown(aTile % boardWidth); break;
        case 2: moveBlockLeft(boardWidth - 1 + aTile - (aTile % boardWidth)); break;
        case 3: moveBlockRight(aTile - (aTile % boardWidth)); break;
    }
}

static void shuffleRotate(uint8_t aTile)
{
    uint8_t count = rnd(4);
    for (uint8_t i = 0; i != count; i++)
        rotateBlock(aTile);
}

static void shuffleLevel()
{
    uint8_t j = 0;
    while (j < boardSize)
    {
        switch (gameMode)
        {
            case gmRotate:
                shuffleRotate(j);
                j++;
                break;
            case gmSlide:
                shuffleSlide(j);
                // every slide level is odd in width and height (bar random),
                // so stepping by two is enough of a shuffle and twice as fast
                j += 2;
                break;
            default:
                if (rnd(2) == 0)
                {
                    shuffleSlide(j);
                    j += 2;
                }
                else
                {
                    shuffleRotate(j);
                    j++;
                }
                break;
        }
    }
}

// Marks currentPoint as water filled if it opens onto an already filled
// neighbour, pushing dry neighbours it connects to onto the stack.
static void handleConnectPoint(uint8_t currentPoint, uint8_t *cc)
{
    uint8_t lookUpX = currentPoint % boardWidth;
    uint8_t lookUpY = currentPoint / boardWidth;

    // north
    if ((lookUpY > 0) && (!(level[currentPoint] & 1)))
    {
        uint8_t tmp = currentPoint - boardWidth;
        uint8_t tmp2 = level[tmp];
        if (!((tmp2 < 16) ? (tmp2 & 4) : ((tmp2 - 16) & 4)))
        {
            if (level[currentPoint] < 16)
                level[currentPoint] += 16;
            if (tmp2 < 16)
                cellStack[(*cc)++] = tmp;
        }
    }

    // east
    if ((lookUpX + 1 < boardWidth) && (!(level[currentPoint] & 2)))
    {
        uint8_t tmp = currentPoint + 1;
        uint8_t tmp2 = level[tmp];
        if (!((tmp2 < 16) ? (tmp2 & 8) : ((tmp2 - 16) & 8)))
        {
            if (level[currentPoint] < 16)
                level[currentPoint] += 16;
            if (tmp2 < 16)
                cellStack[(*cc)++] = tmp;
        }
    }

    // south
    if ((lookUpY + 1 < boardHeight) && (!(level[currentPoint] & 4)))
    {
        uint8_t tmp = currentPoint + boardWidth;
        uint8_t tmp2 = level[tmp];
        if (!((tmp2 < 16) ? (tmp2 & 1) : ((tmp2 - 16) & 1)))
        {
            if (level[currentPoint] < 16)
                level[currentPoint] += 16;
            if (tmp2 < 16)
                cellStack[(*cc)++] = tmp;
        }
    }

    // west
    if ((lookUpX > 0) && (!(level[currentPoint] & 8)))
    {
        uint8_t tmp = currentPoint - 1;
        uint8_t tmp2 = level[tmp];
        if (!((tmp2 < 16) ? (tmp2 & 2) : ((tmp2 - 16) & 2)))
        {
            if (level[currentPoint] < 16)
                level[currentPoint] += 16;
            if (tmp2 < 16)
                cellStack[(*cc)++] = tmp;
        }
    }
}

void updateConnected()
{
    // strip every tile back to the dry set
    for (uint8_t i = 0; i != boardSize; i++)
    {
        if (level[i] > 31)
            level[i] -= 32;
        else if (level[i] > 15)
            level[i] -= 16;
    }

    // flood out from the water source
    uint8_t cc = 1;
    handleConnectPoint(startPos, &cc);
    while (--cc > 0)
    {
        if (level[cellStack[cc]] < 16)
            handleConnectPoint(cellStack[cc], &cc);
    }

    // and give the source its own tile set
    if (level[startPos] > 15)
        level[startPos] += 16;
    else
        level[startPos] += 32;
}

// Carves a perfect maze with a randomised depth first search, so every level
// is solvable and every cell is reachable from the source.
static void generateLevel()
{
    uint8_t neighbours[4];
    uint8_t cc = 0;
    uint8_t currentPoint = 0;
    uint8_t visitedRooms = 1;

    memset(level, 0x0f, boardSize);

    while (visitedRooms != boardSize)
    {
        uint8_t neighboursFound = 0;
        uint8_t lookUpX = currentPoint % boardWidth;
        uint8_t lookUpY = currentPoint / boardWidth;

        // bounds are tested before indexing, unlike the Arduboy original
        if ((lookUpX + 1 < boardWidth) && (level[currentPoint + 1] == 0x0f))
            neighbours[neighboursFound++] = currentPoint + 1;

        if ((lookUpX > 0) && (level[currentPoint - 1] == 0x0f))
            neighbours[neighboursFound++] = currentPoint - 1;

        if ((lookUpY > 0) && (level[currentPoint - boardWidth] == 0x0f))
            neighbours[neighboursFound++] = currentPoint - boardWidth;

        if ((lookUpY + 1 < boardHeight) && (level[currentPoint + boardWidth] == 0x0f))
            neighbours[neighboursFound++] = currentPoint + boardWidth;

        if (neighboursFound == 0)
        {
            currentPoint = cellStack[--cc];
            continue;
        }

        uint8_t selectedNeighbour = neighbours[rnd(neighboursFound)];
        uint8_t tmp = selectedNeighbour % boardWidth;

        if (tmp > lookUpX)          // neighbour lies east
        {
            level[selectedNeighbour] &= ~8;
            level[currentPoint] &= ~2;
        }
        else if (tmp < lookUpX)     // neighbour lies west
        {
            level[selectedNeighbour] &= ~2;
            level[currentPoint] &= ~8;
        }
        else if ((selectedNeighbour / boardWidth) < lookUpY)   // neighbour lies north
        {
            level[selectedNeighbour] &= ~4;
            level[currentPoint] &= ~1;
        }
        else                                                   // neighbour lies south
        {
            level[selectedNeighbour] &= ~1;
            level[currentPoint] &= ~4;
        }

        if (neighboursFound > 1)
            cellStack[cc++] = currentPoint;

        currentPoint = selectedNeighbour;
        visitedRooms++;
    }
}

// The level is solved once no tile is left in the dry set.
uint8_t isLevelDone()
{
    for (uint8_t i = 0; i != boardSize; i++)
        if (level[i] < 16)
            return 0;

    return 1;
}

void initLevel(uint32_t aRandomSeed)
{
    levelDone = 0;
    moves = 0;

    // Seeding from the level number keeps a given level identical every time
    // it is played; only the random difficulty uses a live seed.
    if (difficulty != diffRandom)
        rndSeed((uint32_t)selectedLevel + ((uint32_t)difficulty * 500) + ((uint32_t)gameMode * 50));
    else
        rndSeed(aRandomSeed);

    maxLevel = levelCount;
    switch (difficulty)
    {
        case diffVeryEasy: boardWidth = 5;  boardHeight = 5; break;
        case diffEasy:     boardWidth = 6;  boardHeight = 6; break;
        case diffNormal:   boardWidth = 7;  boardHeight = 7; break;
        case diffHard:     boardWidth = 8;  boardHeight = 8; break;
        case diffVeryHard: boardWidth = 10; boardHeight = 8; break;
        default:
            boardWidth = 5 + rnd(maxBoardWidth - 5 + 1);
            boardHeight = 5 + rnd(maxBoardHeight - 5 + 1);
            maxLevel = 0;   // the random difficulty has no level count
            break;
    }

    // the sliding modes give up a ring of cells to the arrows
    boardWidth -= posAdd + posAdd;
    boardHeight -= posAdd + posAdd;
    boardSize = boardWidth * boardHeight;

    generateLevel();

    boardX = (maxBoardBgWidth - boardWidth) >> 1;
    boardY = (maxBoardBgHeight - boardHeight) >> 1;
    startPos = (boardWidth >> 1) + ((boardHeight >> 1) * boardWidth);

    selectionX = boardWidth >> 1;
    selectionY = boardHeight >> 1;

    // the generated level is the solution, so scramble it
    shuffleLevel();
    updateConnected();
}
