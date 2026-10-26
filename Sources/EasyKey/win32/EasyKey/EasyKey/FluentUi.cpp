/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11, based on OpenKey.
Copyright (C) 2026 Cino

This file is belong to the OpenKey project, Win32 version
which is released under GPL license.
You can fork, modify, improve this program. If you
redistribute your new version, it MUST be open source.
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

