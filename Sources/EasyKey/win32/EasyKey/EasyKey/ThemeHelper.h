/*----------------------------------------------------------
OpenKey - The Cross platform Open source Vietnamese Keyboard application.

Copyright (C) 2019 Mai Vu Tuyen
Contact: maivutuyen.91@gmail.com
Github: https://github.com/tuyenvm/OpenKey
Fanpage: https://www.facebook.com/OpenKeyVN

EasyKey - Vietnamese input for Windows 11, based on OpenKey.
Copyright (C) 2026 Cino

This file is belong to the OpenKey project, Win32 version
which is released under GPL license.
You can fork, modify, improve this program. If you
redistribute your new version, it MUST be open source.
-----------------------------------------------------------*/
#pragma once
#include "stdafx.h"

namespace Gdiplus { class Font; }

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
	//Windows accent color as Fluent uses it for filled controls in the light or dark theme
	static COLORREF getAccentColor(const bool& dark, const bool& hover);
	//start GDI+ once for the whole app
	static void startGdiplus();
	//Segoe UI Variable for GDI+ text, strong is semibold. The caller deletes the font
	static Gdiplus::Font* createFont(const bool& strong, const float& pixels, const int& style);
	//light or dark popup menus for the whole app, call again when the theme changes
	static void applyMenuTheme();
	//replace the dialog template font of every control with Segoe UI Variable
	static void applyDialogFont(const HWND& hDlg);
};
