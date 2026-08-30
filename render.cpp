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

#include "render.h"
#include "commonvars.h"
#include "FastTinyDriver.h"
#include "blocktiles.h"
#include "selectortiles.h"

// the tile map standing in for Arduboy's pixel framebuffer
static uint8_t screen[screenWidth * screenHeight];

// cursor position in tile coordinates, and its blink state
static uint8_t cursorX, cursorY, cursorOn, blinkCount, blinkPhase;

constexpr uint8_t cursorHidden = 0xff;
constexpr uint8_t blinkFrames = 6;

// The cursor is drawn as 8 small bracket tiles on the cells surrounding the
// selected one. This maps a (dx+1, dy+1) neighbour offset to its selector
// tile, matching the sprite layout the Arduboy version used. 0xff = no tile.
static const uint8_t cursorTileFor[9] PROGMEM =
{
    6, 0, 4,
    3, 0xff, 1,
    7, 2, 5,
};

// punctuation and the two button glyphs, in the order of punctChars
static const char punctChars[] PROGMEM = ":ab[]<>+*|#";
static const uint8_t punctTiles[] PROGMEM = { 116, 119, 117, 70, 64, 73, 67, 63, 62, 69, 65 };

void clearScreen()
{
    for (uint8_t i = 0; i != sizeof(screen); i++)
        screen[i] = tileBlank;
}

void setTile(uint8_t x, uint8_t y, uint8_t tile)
{
    if ((x < screenWidth) && (y < screenHeight))
        screen[(y << 4) + x] = tile;
}

void fillTiles(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t tile)
{
    for (uint8_t j = 0; j != h; j++)
        for (uint8_t i = 0; i != w; i++)
            setTile(x + i, y + j, tile);
}

// Translates a character to its tile in the block tile set, which carries the
// game's font. Unknown characters become a blank tile.
static uint8_t tileForChar(char c)
{
    if ((c >= 'A') && (c <= 'Z'))
        return (uint8_t)c + 25;

    if ((c >= '0') && (c <= '9'))
        return (uint8_t)c + 32;

    for (uint8_t i = 0; i != sizeof(punctTiles); i++)
        if (c == (char)pgm_read_byte(&punctChars[i]))
            return pgm_read_byte(&punctTiles[i]);

    return 61;
}

void printMessage(uint8_t x, uint8_t y, const char *msg)
{
    while (1)
    {
        char c = (char)pgm_read_byte(msg++);
        if (c == '\0')
            return;
        setTile(x++, y, tileForChar(c));
    }
}

// Prints value right aligned in a field of the given width.
void printNumber(uint8_t x, uint8_t y, uint16_t value, uint8_t digits)
{
    uint8_t i = digits;
    while (i != 0)
    {
        i--;
        setTile(x + i, y, tileForChar('0' + (value % 10)));
        value /= 10;
        if (value == 0)
            break;
    }
    // blank out any unused leading positions
    while (i != 0)
    {
        i--;
        setTile(x + i, y, tileBlank);
    }
}

void setCursorTile(uint8_t x, uint8_t y)
{
    cursorX = x;
    cursorY = y;
    cursorOn = 1;
}

void hideCursor()
{
    cursorOn = 0;
    cursorX = cursorHidden;
    cursorY = cursorHidden;
}

void updateCursorBlink()
{
    if (++blinkCount >= blinkFrames)
    {
        blinkCount = 0;
        blinkPhase ^= 1;
    }
}

// Returns the selector tile data for this cell, or nullptr when the cursor
// does not touch it.
static const uint8_t *cursorOverlay(uint8_t col, uint8_t page)
{
    if (!cursorOn || blinkPhase)
        return nullptr;

    uint8_t dx = (uint8_t)(col - cursorX + 1);
    uint8_t dy = (uint8_t)(page - cursorY + 1);
    if ((dx > 2) || (dy > 2))
        return nullptr;

    uint8_t tile = pgm_read_byte(&cursorTileFor[dy * 3 + dx]);
    if (tile == 0xff)
        return nullptr;

    return &selectorTiles[tile << 3];
}

// Expands the tile map onto the display, one 8 pixel page at a time.
void renderScreen()
{
    const uint8_t *row = screen;

    for (uint8_t page = 0; page != screenHeight; page++)
    {
        ssd1306_selectPage(page);

        for (uint8_t col = 0; col != screenWidth; col++)
        {
            uint8_t tile = row[col];
            const uint8_t *src = (tile == tileBlank) ? nullptr : &blockTiles[(uint16_t)tile << 3];
            const uint8_t *ov = cursorOverlay(col, page);

            for (uint8_t b = 0; b != 8; b++)
            {
                uint8_t pixels = (src == nullptr) ? 0 : pgm_read_byte(src + b);
                if (ov != nullptr)
                    pixels |= pgm_read_byte(ov + b);
                i2c_write(pixels);
            }
        }

        i2c_stop();
        row += screenWidth;
    }
}
