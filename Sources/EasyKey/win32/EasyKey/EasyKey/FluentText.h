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
#include "stdafx.h"

/*
 * Text for the windows EasyKey paints itself, measured and drawn with DirectWrite.
 * GDI+ snaps glyphs to the pixel grid, which breaks letters up when Windows scales
 * the screen; DirectWrite keeps their shapes at any scale, like the rest of Windows 11.
 */

struct FluentFont;
struct IDWriteFontCollection;

//Be Vietnam Pro, which EasyKey carries inside the exe; Segoe UI Variable or Segoe UI where it cannot load
IDWriteFontCollection* fluentFontCollection(); //NULL means the system fonts
LPCWSTR fluentFontFamily();
LPCWSTR fluentGdiFontFace(); //for GDI controls, NULL when GDI could not take it

//strong is semibold, bold asks for more but gets semibold, the heaviest weight EasyKey carries
FluentFont* createFluentFont(const bool& strong, const float& pixels, const bool& bold = false);
void freeFluentFont(FluentFont*& font);

int fluentTextWidth(FluentFont* font, const wstring& text);
int fluentTextHeight(FluentFont* font, const wstring& text, const int& width);

//text is collected while GDI+ paints the shapes, then drawn over them in one go
class FluentTextBatch {
public:
	//align: 0 near, 1 center, 2 far; vAlign 3 centers the ink of the letters both ways (badges); clip is in canvas pixels
	void add(const wstring& text, const RECT& rc, FluentFont* font, const DWORD& color,
			 const int& hAlign, const int& vAlign, const bool& wrap, const RECT& clip);
	//draws on the 32 bit premultiplied bitmap selected into dc, then forgets the text
	void draw(const HDC& dc, const int& width, const int& height);
private:
	struct Item {
		wstring text;
		RECT rc, clip;
		FluentFont* font;
		DWORD color;
		int hAlign, vAlign;
		bool wrap;
	};
	vector<Item> items;
};
