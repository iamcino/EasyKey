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


//posted to a dialog after Windows has rescaled it for a new DPI
#define WM_EASYKEY_DPI_CHANGED (WM_APP + 101)

/*
 * Windows 11 look: dark/light title bar, Mica backdrop, Segoe UI Variable
 * font and monochrome tray icon matching the taskbar color.
 */
class ThemeHelper {
public:
	static bool isAppDarkMode();
	static bool isTaskbarDarkMode();
	static bool isThemeChangedMessage(const UINT& uMsg, const LPARAM& lParam);

	static UINT getDpi(const HWND& hWnd);
	static int scale(const int& value, const HWND& hWnd);
	static LPCTSTR getFontFace();

	//apply title bar color and backdrop, call again when the theme changes
	static void applyWindowFrame(const HWND& hWnd);
	//Mica behind the whole window, client area included, for windows that paint their own
	//content with alpha. Returns false and uses a solid caption in fallbackColor when
	//Mica is unavailable (transparency effects off, high contrast, before Windows 11 22H2)
	static bool applyBackdrop(const HWND& hWnd, const COLORREF& fallbackColor);
	//accent for filled controls in the light or dark theme: the Windows accent as Fluent uses it,
	//one of the presets or the custom color, see vAccentColor
	static COLORREF getAccentColor(const bool& dark, const bool& hover);
	//preset accents, index 1 to ACCENT_PRESETS; 0 is the Windows accent
	static const int ACCENT_PRESETS = 7;
	static COLORREF getPresetAccent(const int& index, const bool& dark);
	static LPCTSTR getPresetName(const int& index);
	//the same hue, lighter and a little softer, for the dark theme
	static COLORREF deriveDarkAccent(const COLORREF& light);
	//WCAG contrast ratio, 1 to 21
	static double getContrast(const COLORREF& a, const COLORREF& b);
	//start GDI+ once for the whole app
	static void startGdiplus();
	//light or dark popup menus for the whole app, call again when the theme changes
	static void applyMenuTheme();
};
