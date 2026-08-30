# Waternet (TinyJoypad)

The TinyJoypad (ATtiny85) version of Waternet, a puzzle game where you turn or
slide pipes until water reaches every one of them from the source in the middle
of the board. Ported from the [Arduboy version](https://github.com/joyrider3774/waternet_arduboy),
which in turn came from the Game Boy original.

The game concept is based on the Net and Netslide puzzles from
[Simon Tatham's Portable Puzzle Collection](https://www.chiark.greenend.org.uk/~sgtatham/puzzles/).

## Controls

The TinyJoypad has one action button where the Arduboy has two, so it does
double duty:

| Input | Action |
|-------|--------|
| Stick | Move the cursor, or change the selection in a menu |
| **Tap** fire | Confirm, rotate a pipe, or slide a row or column (the Arduboy's A) |
| **Hold** fire | Back, or pause while playing (the Arduboy's B) |


## Game modes

The aim in every mode is to connect all the pipes so water flows through them
from the source. How you do that is what changes:

- **Rotate** - press fire on a pipe to turn it a quarter turn.
- **Slide** - press fire on one of the arrows around the board to shift that
  whole row or column one step in the direction of the arrow.
- **Roslid** - both of the above.

Difficulties run from Very Easy (5x5) to Very Hard (10x8), plus Random, which
deals an endless supply of boards of random size instead of a numbered list of
25 levels. In the sliding modes the board gives up its outer ring of cells to
the arrows, so the playable area is two cells smaller in each direction.

Progress is saved to EEPROM: finishing a level unlocks the next one, per game
mode and difficulty. The sound on/off setting is stored alongside it.

## Building

Board: **ATtiny85**, using [ATTinyCore](https://github.com/SpenceKonde/ATTinyCore)
(tested with 1.5.2).

| Setting | Value |
|---------|-------|
| Clock | 16 MHz (PLL) |
| LTO | Enabled |
| millis()/micros() | **Disabled** |

LTO and disabling `millis()` are not optional - the game does not fit in 8KB
without them. The current build is **7320 of 8192 bytes flash (89%)** and
**332 bytes of RAM**, leaving 180 bytes for the stack (the deepest call chain
needs well under 100).

Program the chip with an Arduino in "Arduino as ISP" mode. A prebuilt
[waternet_tinyjoypad.ino.hex](waternet_tinyjoypad.ino.hex) is included, and runs
in the [TinyJoypad emulator](https://github.com/joyrider3774/Tinyjoypad_Emulator).

## How the port works

The Arduboy build draws into a 1KB pixel framebuffer, which is twice the
ATtiny85's entire RAM. But Waternet is already built purely from 8x8 tiles on a
16x8 grid, and that lines up exactly with the SSD1306's eight 8-pixel pages. So
the framebuffer became a **128 byte tile map**: `setTile()` writes a tile number,
and `renderScreen()` expands one page at a time straight onto the I2C bus, with
the cursor OR'd over the cells it touches. Most of the drawing code carried over
unchanged.

The tile data needed no pixel conversion at all: `Arduboy2::drawBitmap()`
copies each byte straight into a column of its screen buffer, so the Arduboy
arrays are already in the same page format the SSD1306 wants. (The row-major
routine is `drawSlowXYBitmap()`, which this game never used.)


Other changes worth knowing about:

- **`random()` became a xorshift32** ([level.cpp](level.cpp)). AVR's `random()`
  costs far more flash than an 8KB part can spare. Levels are still reproducible
  from their level number, so a given level is the same board every time - but
  the boards differ from the Arduboy version's, since the generator differs.
- **`rotateBlock()` is now arithmetic.** The original's 42-case switch is just a
  4-bit rotate of the wall mask (north becomes east, east becomes south, ...).
- **One shared cell stack.** `generateLevel()` and `updateConnected()` each had
  an 81 byte local buffer, and `initLevel()` holds the first while calling the
  second - about 345 bytes of stack, which does not fit. They never run at the
  same time, so they share one static buffer.
- **The save format is a plain byte per slot** rather than the Arduboy's 5-bit
  packing, which existed only because Arduboy games share one EEPROM.
- **An out of bounds read was fixed.** The Arduboy `generateLevel()` tests
  `(level[tmp] == 0xf) && (bounds)`, indexing before checking, so it reads
  `level[-1]` and friends. The operands are swapped here so `&&` short circuits.

## What is not in this version

Cut to fit in 8KB, all of it present in the Arduboy build:

- the RLE title screen bitmap (there is a plain text title instead)
- the intro and the congratulations screen
- the 13 help screens
- all music (the TinyJoypad's buzzer is driven by a blocking busy loop, so
  there is no way to play music under the game); sound effects are kept

## Credits

Waternet is created by Willems Davy (joyrider3774).

- `FastTinyDriver` and `ELECTROLIB` are by Électro L.I.B (Daniel C.),
  [tinyjoypad.com](https://www.tinyjoypad.com), used under the GPLv3.

## License

GPLv3 - see [LICENSE](LICENSE).
