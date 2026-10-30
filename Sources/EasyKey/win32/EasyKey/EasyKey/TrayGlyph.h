/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
#pragma once
#include <vector>

/*
 * The tray icon, drawn for the exact pixel size the taskbar asks for instead of
 * scaled from a fixed set of sizes: the rounded key cap of the app icon with V or E on it.
 * The bars and stem of E sit on whole pixels at every size, so the letter stays sharp.
 * Plain C++ with no Windows calls, so it can be checked anywhere.
 */

#define TRAY_GLYPH_E	0 //English
#define TRAY_GLYPH_V	1 //Vietnamese
#define TRAY_GLYPH_EV	2 //English and Vietnamese mixed

//coverage of size x size pixels, row by row, 0..255: the tile, and the letters (TRAY_GLYPH_*) inside it;
//the rows the tile spans, for its gradient
void drawTrayGlyph(const int& size, const int& letters,
				   std::vector<unsigned char>& tile, std::vector<unsigned char>& letter,
				   int& tileTop, int& tileBottom);
