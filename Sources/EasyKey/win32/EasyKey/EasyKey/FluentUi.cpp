/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
//GDI+ needs the full windows.h and objidl.h, include them before stdafx.h trims windows.h down
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include "FluentUi.h"
#include "ThemeHelper.h"

void fillPalette(FluentPalette& pal, const bool& dark, const bool& translucent, const COLORREF& solid) {
	pal.bg = translucent ? 0 : argb(255, solid);
	pal.accent = argb(255, ThemeHelper::getAccentColor(dark, false));
	pal.accentHover = argb(255, ThemeHelper::getAccentColor(dark, true));
	pal.link = pal.accent;
	pal.linkHover = pal.accentHover;
	if (dark) {
		pal.card = 0x0EFFFFFF;
		pal.cardHover = 0x17FFFFFF;
		pal.cardPressed = 0x08FFFFFF;
		pal.stroke = 0x17FFFFFF;
		pal.divider = 0x12FFFFFF;
		pal.track = 0x40000000;
		pal.text = 0xFFFFFFFF;
		pal.sub = 0xFFC8C8C8;
		pal.disabled = 0xFF787878;
		pal.onAccent = 0xFF000000;
		pal.field = 0x14FFFFFF;
		pal.fieldHover = 0x1FFFFFFF;
		pal.good = 0xFF6CCB5F;
		pal.warning = 0xFFFCE100;
	} else {
		pal.card = 0xB8FFFFFF;
		pal.cardHover = 0x80FFFFFF;
		pal.cardPressed = 0x4DFFFFFF;
		pal.stroke = 0x14000000;
		pal.divider = 0x0F000000;
		pal.track = 0x0A000000;
		pal.text = 0xFF1B1B1B;
		pal.sub = 0xFF595959;
		pal.disabled = 0xFFA0A0A0;
		pal.onAccent = 0xFFFFFFFF;
		pal.field = 0xE6FFFFFF;
		pal.fieldHover = 0xB3F9F9F9;
		pal.good = 0xFF0F7B0F;
		pal.warning = 0xFF9D5D00;
	}
}

DWORD argb(const BYTE& alpha, const COLORREF& c) {
	return ((DWORD)alpha << 24) | ((DWORD)GetRValue(c) << 16) | ((DWORD)GetGValue(c) << 8) | GetBValue(c);
}

DWORD fade(const DWORD& color, const int& percent) {
	return (((color >> 24) * percent / 100) << 24) | (color & 0xFFFFFF);
}

void addRoundRect(Gdiplus::GraphicsPath& path, const RECT& rc, float radius, const bool& roundTop, const bool& roundBottom) {
	//inset by half a pixel so 1px strokes stay sharp with PixelOffsetModeHalf
	float x = rc.left + 0.5f, y = rc.top + 0.5f;
	float w = (float)(rc.right - rc.left) - 1, h = (float)(rc.bottom - rc.top) - 1;
	if (radius * 2 > h) radius = h / 2;
	if (radius * 2 > w) radius = w / 2;
	if (radius < 0.5f) radius = 0.5f;
	float d = radius * 2;
	if (roundTop) {
		path.AddArc(x, y, d, d, 180, 90);
		path.AddArc(x + w - d, y, d, d, 270, 90);
	} else {
		path.AddLine(x, y, x + w, y);
	}
	if (roundBottom) {
		path.AddArc(x + w - d, y + h - d, d, d, 0, 90);
		path.AddArc(x, y + h - d, d, d, 90, 90);
	} else {
		path.AddLine(x + w, y + h, x, y + h);
	}
	path.CloseFigure();
}

void fillRound(Gdiplus::Graphics& g, const RECT& rc, const float& radius, const DWORD& fill, const DWORD& stroke,
					  const bool& roundTop, const bool& roundBottom) {
	Gdiplus::GraphicsPath path;
	addRoundRect(path, rc, radius, roundTop, roundBottom);
	if (fill >> 24) {
		Gdiplus::SolidBrush brush((Gdiplus::ARGB)fill);
		g.FillPath(&brush, &path);
	}
	if (stroke >> 24) {
		Gdiplus::Pen pen((Gdiplus::ARGB)stroke, 1);
		g.DrawPath(&pen, &path);
	}
}

void fillRect(Gdiplus::Graphics& g, const DWORD& color, const RECT& rc) {
	Gdiplus::SolidBrush brush((Gdiplus::ARGB)color);
	g.FillRectangle(&brush, (INT)rc.left, (INT)rc.top, (INT)(rc.right - rc.left), (INT)(rc.bottom - rc.top));
}

void drawLine(Gdiplus::Graphics& g, const DWORD& color, const int& left, const int& right, const int& y) {
	RECT line = { left, y, right, y + 1 };
	fillRect(g, color, line);
}

void drawChevron(Gdiplus::Graphics& g, const DWORD& color, const float& cx, const float& cy, const float& size, const bool& up) {
	float half = size / 2, quarter = size / 4;
	float dir = up ? -1.0f : 1.0f;
	Gdiplus::PointF points[3] = {
		Gdiplus::PointF(cx - half, cy - quarter * dir),
		Gdiplus::PointF(cx, cy + quarter * dir),
		Gdiplus::PointF(cx + half, cy - quarter * dir)
	};
	Gdiplus::Pen pen((Gdiplus::ARGB)color, size / 8);
	pen.SetLineJoin(Gdiplus::LineJoinRound);
	pen.SetStartCap(Gdiplus::LineCapRound);
	pen.SetEndCap(Gdiplus::LineCapRound);
	g.DrawLines(&pen, points, 3);
}

//github mark from Simple Icons (CC0), 24 x 24: per figure the bezier count, the start point, then 3 points per bezier
static const float githubMark[] = {
	25, 12.0f, 0.297f, 5.37f, 0.297f, 0.0f, 5.67f, 0.0f, 12.297f, 0.0f, 17.6f, 3.438f, 22.097f, 8.205f, 23.682f,
	8.805f, 23.795f, 9.025f, 23.424f, 9.025f, 23.105f, 9.025f, 22.82f, 9.015f, 22.065f, 9.01f, 21.065f, 5.672f,
	21.789f, 4.968f, 19.455f, 4.968f, 19.455f, 4.422f, 18.07f, 3.633f, 17.7f, 3.633f, 17.7f, 2.546f, 16.956f,
	3.717f, 16.971f, 3.717f, 16.971f, 4.922f, 17.055f, 5.555f, 18.207f, 5.555f, 18.207f, 6.625f, 20.042f, 8.364f,
	19.512f, 9.05f, 19.205f, 9.158f, 18.429f, 9.467f, 17.9f, 9.81f, 17.6f, 7.145f, 17.3f, 4.344f, 16.268f, 4.344f,
	11.67f, 4.344f, 10.36f, 4.809f, 9.29f, 5.579f, 8.45f, 5.444f, 8.147f, 5.039f, 6.927f, 5.684f, 5.274f, 5.684f,
	5.274f, 6.689f, 4.952f, 8.984f, 6.504f, 9.944f, 6.237f, 10.964f, 6.105f, 11.984f, 6.099f, 13.004f, 6.105f,
	14.024f, 6.237f, 14.984f, 6.504f, 17.264f, 4.952f, 18.269f, 5.274f, 18.269f, 5.274f, 18.914f, 6.927f, 18.509f,
	8.147f, 18.389f, 8.45f, 19.154f, 9.29f, 19.619f, 10.36f, 19.619f, 11.67f, 19.619f, 16.28f, 16.814f, 17.295f,
	14.144f, 17.59f, 14.564f, 17.95f, 14.954f, 18.686f, 14.954f, 19.81f, 14.954f, 21.416f, 14.939f, 22.706f,
	14.939f, 23.096f, 14.939f, 23.411f, 15.149f, 23.786f, 15.764f, 23.666f, 20.565f, 22.092f, 24.0f, 17.592f,
	24.0f, 12.297f, 24.0f, 5.67f, 18.627f, 0.297f, 12.0f, 0.297f
};
//gitlab mark from Simple Icons (CC0), 24 x 24: per figure the bezier count, the start point, then 3 points per bezier
static const float gitlabMark[] = {
	26, 23.6f, 9.593f, 23.589f, 9.564f, 23.578f, 9.535f, 23.567f, 9.507f, 22.478f, 6.665f, 21.389f, 3.823f, 20.3f,
	0.981f, 20.234f, 0.814f, 20.116f, 0.672f, 19.964f, 0.576f, 19.652f, 0.383f, 19.253f, 0.405f, 18.964f, 0.63f,
	18.825f, 0.744f, 18.723f, 0.897f, 18.674f, 1.07f, 17.939f, 3.32f, 17.204f, 5.569f, 16.469f, 7.818f, 13.492f,
	7.818f, 10.515f, 7.818f, 7.537f, 7.818f, 6.802f, 5.569f, 6.067f, 3.32f, 5.332f, 1.07f, 5.284f, 0.896f, 5.182f,
	0.742f, 5.042f, 0.629f, 4.753f, 0.403f, 4.354f, 0.382f, 4.042f, 0.575f, 3.89f, 0.672f, 3.773f, 0.813f, 3.706f,
	0.98f, 2.615f, 3.821f, 1.524f, 6.661f, 0.433f, 9.502f, 0.422f, 9.53f, 0.412f, 9.559f, 0.401f, 9.588f, -0.565f,
	12.112f, 0.255f, 14.971f, 2.413f, 16.598f, 2.416f, 16.601f, 2.42f, 16.604f, 2.424f, 16.607f, 2.434f, 16.614f,
	2.444f, 16.621f, 2.454f, 16.628f, 4.113f, 17.87f, 5.771f, 19.112f, 7.43f, 20.355f, 8.251f, 20.976f, 9.071f,
	21.597f, 9.892f, 22.218f, 10.392f, 22.595f, 10.892f, 22.973f, 11.391f, 23.35f, 11.752f, 23.624f, 12.251f,
	23.624f, 12.611f, 23.35f, 13.111f, 22.973f, 13.611f, 22.595f, 14.111f, 22.218f, 14.931f, 21.597f, 15.752f,
	20.976f, 16.572f, 20.355f, 18.241f, 19.105f, 19.91f, 17.855f, 21.578f, 16.606f, 21.583f, 16.602f, 21.587f,
	16.599f, 21.591f, 16.596f, 23.744f, 14.968f, 24.562f, 12.114f, 23.6f, 9.593f, 23.6f, 9.593f, 23.6f, 9.593f,
	23.6f, 9.593f
};

void fillMark(Gdiplus::Graphics& g, const int& mark, const RECT& rc, const DWORD& color) {
	const float* data = mark == MARK_GITHUB ? githubMark : mark == MARK_GITLAB ? gitlabMark : NULL;
	int count = mark == MARK_GITHUB ? _countof(githubMark) : mark == MARK_GITLAB ? _countof(gitlabMark) : 0;
	if (!data)
		return;
	float scale = (float)min(rc.right - rc.left, rc.bottom - rc.top) / 24.0f;
	auto point = [&](const int& i) { return Gdiplus::PointF(rc.left + data[i] * scale, rc.top + data[i + 1] * scale); };
	Gdiplus::GraphicsPath path(Gdiplus::FillModeAlternate);
	for (int i = 0; i < count;) {
		int beziers = (int)data[i];
		Gdiplus::PointF last = point(i + 1);
		i += 3;
		path.StartFigure();
		for (int b = 0; b < beziers && i + 6 <= count; b++, i += 6) {
			Gdiplus::PointF next = point(i + 4);
			path.AddBezier(last, point(i), point(i + 2), next);
			last = next;
		}
		path.CloseFigure();
	}
	Gdiplus::SolidBrush brush((Gdiplus::ARGB)color);
	g.FillPath(&brush, &path);
}

RECT makeRect(const int& left, const int& top, const int& width, const int& height) {
	RECT rc = { left, top, left + width, top + height };
	return rc;
}

int centerY(const RECT& rc) {
	return (rc.top + rc.bottom) / 2;
}

int modifierMask(const WPARAM& vk) {
	switch (vk) {
	case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL:
		return 0x100;
	case VK_MENU: case VK_LMENU: case VK_RMENU:
		return 0x200;
	case VK_LWIN: case VK_RWIN:
		return 0x400;
	case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT:
		return 0x800;
	}
	return 0;
}

int pressedModifiers() {
	int mods = 0;
	if (GetKeyState(VK_CONTROL) < 0) mods |= 0x100;
	if (GetKeyState(VK_MENU) < 0) mods |= 0x200;
	if (GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_RWIN) < 0) mods |= 0x400;
	if (GetKeyState(VK_SHIFT) < 0) mods |= 0x800;
	return mods;
}

int countModifiers(const int& mods) {
	int count = 0;
	for (int mask = 0x100; mask <= 0x800; mask <<= 1) {
		if (mods & mask)
			count++;
	}
	return count;
}

//"Ctrl + Shift + Z" as { "Ctrl", "Shift", "Z" }, one keycap each
vector<wstring> splitHotKey(const wstring& text) {
	vector<wstring> keys;
	size_t start = 0;
	while (start < text.size()) {
		size_t end = text.find(_T(" + "), start);
		if (end == wstring::npos)
			end = text.size();
		keys.push_back(text.substr(start, end - start));
		start = end + 3;
	}
	return keys;
}

