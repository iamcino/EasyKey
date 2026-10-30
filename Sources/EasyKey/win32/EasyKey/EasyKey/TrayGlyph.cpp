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

//distance from (x, y) to the segment a-b, squared
static float distanceToSegment2(const float& x, const float& y, const Point& a, const Point& b) {
	float dx = b.x - a.x, dy = b.y - a.y;
	float t = ((x - a.x) * dx + (y - a.y) * dy) / (dx * dx + dy * dy);
	t = t < 0 ? 0 : t > 1 ? 1 : t;
	float px = a.x + t * dx - x, py = a.y + t * dy - y;
	return px * px + py * py;
}

//E: a stem and three arms on whole pixels, from left at the rows top to top + height
static bool insideE(const float& x, const float& y, const int& left, const int& top, const int& height,
					const int& width, const int& stroke) {
	int bottom = top + height;
	int middle = top + (height - stroke) / 2;
	int middleRight = left + width - (stroke + 1) / 2;
	return (x >= left && x < left + stroke && y >= top && y < bottom) ||
		(x >= left && x < left + width && ((y >= top && y < top + stroke) || (y >= bottom - stroke && y < bottom))) ||
		(x >= left && x < middleRight && y >= middle && y < middle + stroke);
}

void drawTrayGlyph(const int& size, const int& letters,
				   std::vector<unsigned char>& tile, std::vector<unsigned char>& letter,
				   int& tileTop, int& tileBottom) {
	tile.assign(size * size, 0);
	letter.assign(size * size, 0);
	tileTop = tileBottom = 0;
	if (size <= 0)
		return;

	//the key cap of the app icon: wider than tall, so the tray and the taskbar show the same thing.
	//It fills the width at 16 and 20 px, where every pixel counts, and keeps a thin margin above that
	int inset = size < 20 ? 0 : (size + 8) / 24;
	int side = size - 2 * inset;
	int capHeight = (int)floorf(side * 0.78f + 0.5f);
	if ((size - capHeight) % 2)
		capHeight++;
	tileTop = (size - capHeight) / 2;
	tileBottom = tileTop + capHeight;
	float radius = capHeight * 0.3f;
	bool two = letters == TRAY_GLYPH_EV;

	//E: three bars and two gaps of about one stroke each, and its middle bar has to land on whole pixels too.
	//Next to V it is narrower, so both fit the cap with room around them
	int height = (int)floorf(capHeight * 0.6f + 0.5f);
	int stroke = (int)floorf(height / 5.0f + 0.5f);
	if (stroke < 2)
		stroke = 2;
	if ((height - stroke) % 2)
		height++;
	int top = tileTop + (capHeight - height) / 2;
	int eWidth = (int)floorf(height * (two ? 0.62f : 0.7f) + 0.5f);

	//V: two strokes with round ends meeting at the foot, like the app icon
	float vHeight = two ? (float)height : capHeight * 0.64f;
	float vStroke = two ? height * 0.24f : capHeight * 0.17f;
	if (vStroke < 1.6f)
		vStroke = 1.6f;
	float vWidth = two ? vHeight * 0.8f : vHeight; //alone, the V is as wide as it is tall
	float vTop = two ? (float)top : tileTop + (capHeight - vHeight) / 2.0f;

	//alone each letter sits in the middle of the cap; EV is centered as a pair, a stroke apart
	int eLeft;
	float vCenter;
	if (two) {
		float gap = stroke * 0.6f > 1 ? stroke * 0.6f : 1;
		float pair = eWidth + gap + vWidth;
		eLeft = (int)floorf(inset + (side - pair) / 2.0f + 0.5f);
		vCenter = eLeft + eWidth + gap + vWidth / 2;
	} else {
		eLeft = inset + (side - eWidth + 1) / 2; //half a pixel right: the stem makes E heavy on the left
		vCenter = inset + side / 2.0f;
	}
	Point vLeft = { vCenter - vWidth / 2 + vStroke / 2, vTop + vStroke / 2 };
	Point vRight = { vCenter + vWidth / 2 - vStroke / 2, vTop + vStroke / 2 };
	Point vFoot = { vCenter, vTop + vHeight - vStroke / 2 };
	float reach = vStroke * vStroke / 4;
	bool drawE = letters != TRAY_GLYPH_V, drawV = letters != TRAY_GLYPH_E;

	for (int py = 0; py < size; py++) {
		for (int px = 0; px < size; px++) {
			int inTile = 0, inLetter = 0;
			for (int sy = 0; sy < SAMPLES; sy++) {
				for (int sx = 0; sx < SAMPLES; sx++) {
					float x = px + (sx + 0.5f) / SAMPLES, y = py + (sy + 0.5f) / SAMPLES;
					if (!insideRoundRect(x, y, (float)inset, (float)tileTop, (float)(inset + side), (float)tileBottom, radius))
						continue;
					inTile++;
					bool in = (drawV && (distanceToSegment2(x, y, vLeft, vFoot) <= reach || distanceToSegment2(x, y, vRight, vFoot) <= reach)) ||
						(drawE && insideE(x, y, eLeft, top, height, eWidth, stroke));
					inLetter += in ? 1 : 0;
				}
			}
			tile[py * size + px] = (unsigned char)(inTile * 255 / (SAMPLES * SAMPLES));
			letter[py * size + px] = (unsigned char)(inLetter * 255 / (SAMPLES * SAMPLES));
		}
	}
}
