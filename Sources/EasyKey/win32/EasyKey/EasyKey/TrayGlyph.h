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
 * scaled from a fixed set of sizes: a solid rounded tile with V or E on it.
 * Bars and stems sit on whole pixels at every size, so the letter stays sharp.
 * Plain C++ with no Windows calls, so it can be checked anywhere.
 */

//coverage of size x size pixels, row by row, 0..255: the tile, and the letter inside it
void drawTrayGlyph(const int& size, const bool& vietnamese,
				   std::vector<unsigned char>& tile, std::vector<unsigned char>& letter);
