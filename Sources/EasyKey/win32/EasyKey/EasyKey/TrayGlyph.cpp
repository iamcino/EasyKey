/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
#include "TrayGlyph.h"
#include <cmath>

#define SAMPLES 8 //per pixel side, for the edges that cannot sit on the grid

struct Point { float x, y; };

static bool insideRoundRect(const float& x, const float& y, const float& left, const float& top,
							const float& right, const float& bottom, const float& radius) {
	if (x < left || x > right || y < top || y > bottom)
		return false;
	float cx = x < left + radius ? left + radius : x > right - radius ? right - radius : x;
	float cy = y < top + radius ? top + radius : y > bottom - radius ? bottom - radius : y;
	return (x - cx) * (x - cx) + (y - cy) * (y - cy) <= radius * radius;
}

static bool insidePolygon(const float& x, const float& y, const Point* points, const int& count) {
	bool in = false;
	for (int i = 0, j = count - 1; i < count; j = i++) {
		if ((points[i].y > y) != (points[j].y > y) &&
			x < (points[j].x - points[i].x) * (y - points[i].y) / (points[j].y - points[i].y) + points[i].x)
			in = !in;
	}
	return in;
}

void drawTrayGlyph(const int& size, const bool& vietnamese,
				   std::vector<unsigned char>& tile, std::vector<unsigned char>& letter) {
	tile.assign(size * size, 0);
	letter.assign(size * size, 0);
	if (size <= 0)
		return;

	//the tile fills the icon at 16 and 20 px, where every pixel counts, and keeps a thin margin above that
	int inset = size < 20 ? 0 : (size + 8) / 24;
	int side = size - 2 * inset;
	float radius = side * 0.22f;

	//letter height and stroke, in whole pixels: E is three bars and two gaps of about one stroke each,
	//and its middle bar has to land on whole pixels too
	int height = (int)floorf(side * 0.62f + 0.5f);
	int stroke = (int)floorf(height / 5.0f + 0.5f);
	if (stroke < 2)
		stroke = 2;
	if ((height - stroke) % 2)
		height++;
	int top = inset + (side - height) / 2;
	int bottom = top + height;

	//E: a stem and three arms, all on the grid
	int eWidth = (int)floorf(height * 0.7f + 0.5f);
	int eLeft = inset + (side - eWidth + 1) / 2; //half a pixel right: the stem makes E heavy on the left
	int eMiddle = top + (height - stroke) / 2;
	int eMiddleRight = eLeft + eWidth - (stroke + 1) / 2;

	//V: two slanted strokes meeting in a flat foot; the top corners and the foot sit on the grid
	float center = inset + side / 2.0f;
	int vWidth = (int)floorf(height * 0.96f + 0.5f);
	if ((vWidth - side) % 2)
		vWidth++;
	int foot = (stroke - side) % 2 ? stroke + 1 : stroke;
	float vLeft = center - vWidth / 2.0f, vRight = center + vWidth / 2.0f;
	float thick = stroke * 1.3f; //measured across, the slant makes it look as heavy as the E bars
	float slope = (center - foot / 2.0f - vLeft) / height;
	float notch = top + (center - vLeft - thick) / slope;
	Point v[] = {
		{ vLeft, (float)top }, { vLeft + thick, (float)top }, { center, notch }, { vRight - thick, (float)top },
		{ vRight, (float)top }, { center + foot / 2.0f, (float)bottom }, { center - foot / 2.0f, (float)bottom }
	};

	for (int py = 0; py < size; py++) {
		for (int px = 0; px < size; px++) {
			int inTile = 0, inLetter = 0;
			for (int sy = 0; sy < SAMPLES; sy++) {
				for (int sx = 0; sx < SAMPLES; sx++) {
					float x = px + (sx + 0.5f) / SAMPLES, y = py + (sy + 0.5f) / SAMPLES;
					if (!insideRoundRect(x, y, (float)inset, (float)inset, (float)(inset + side), (float)(inset + side), radius))
						continue;
					inTile++;
					bool in;
					if (vietnamese) {
						in = y >= top && y <= bottom && insidePolygon(x, y, v, 7);
					} else {
						in = (x >= eLeft && x < eLeft + stroke && y >= top && y < bottom) ||
							(x >= eLeft && x < eLeft + eWidth && ((y >= top && y < top + stroke) || (y >= bottom - stroke && y < bottom))) ||
							(x >= eLeft && x < eMiddleRight && y >= eMiddle && y < eMiddle + stroke);
					}
					inLetter += in ? 1 : 0;
				}
			}
			tile[py * size + px] = (unsigned char)(inTile * 255 / (SAMPLES * SAMPLES));
			letter[py * size + px] = (unsigned char)(inLetter * 255 / (SAMPLES * SAMPLES));
		}
	}
}
