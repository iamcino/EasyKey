/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11, based on OpenKey.
Copyright (C) 2026 Cino

This file is belong to the OpenKey project, Win32 version
which is released under GPL license.
You can fork, modify, improve this program. If you
redistribute your new version, it MUST be open source.
-----------------------------------------------------------*/
#pragma once
#include "stdafx.h"

namespace Gdiplus { class Graphics; class GraphicsPath; }

/*
 * Colors and drawing helpers shared by the windows EasyKey paints itself:
 * the settings window and the windows built on FluentWindow.
 */

//Windows 11 Fluent colors, ARGB
struct FluentPalette {
	DWORD bg, card, cardHover, cardPressed, stroke, divider, track;
	DWORD text, sub, disabled, accent, accentHover, onAccent;
	DWORD field, fieldHover, link, linkHover, good, warning;
};

//colors for the light or dark theme; bg is transparent over Mica, solid otherwise
void fillPalette(FluentPalette& pal, const bool& dark, const bool& translucent, const COLORREF& solid);

DWORD argb(const BYTE& alpha, const COLORREF& c);
DWORD fade(const DWORD& color, const int& percent);
RECT makeRect(const int& left, const int& top, const int& width, const int& height);
int centerY(const RECT& rc);

void addRoundRect(Gdiplus::GraphicsPath& path, const RECT& rc, float radius, const bool& roundTop = true, const bool& roundBottom = true);
void fillRound(Gdiplus::Graphics& g, const RECT& rc, const float& radius, const DWORD& fill, const DWORD& stroke = 0,
			   const bool& roundTop = true, const bool& roundBottom = true);
void fillRect(Gdiplus::Graphics& g, const DWORD& color, const RECT& rc);
void drawLine(Gdiplus::Graphics& g, const DWORD& color, const int& left, const int& right, const int& y);
void drawChevron(Gdiplus::Graphics& g, const DWORD& color, const float& cx, const float& cy, const float& size, const bool& up);

//hotkeys in the vSwitchKeyStatus format: 0x100 Ctrl, 0x200 Alt, 0x400 Win, 0x800 Shift
int modifierMask(const WPARAM& vk);
int pressedModifiers();
int countModifiers(const int& mods);
//"Ctrl + Shift + Z" as { "Ctrl", "Shift", "Z" }, one keycap each
vector<wstring> splitHotKey(const wstring& text);
