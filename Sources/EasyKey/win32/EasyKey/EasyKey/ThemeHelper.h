/*----------------------------------------------------------
OpenKey - The Cross platform Open source Vietnamese Keyboard application.

Copyright (C) 2019 Mai Vu Tuyen
Contact: maivutuyen.91@gmail.com
Github: https://github.com/tuyenvm/OpenKey
Fanpage: https://www.facebook.com/OpenKeyVN

EasyKey - Vietnamese input for Windows 11, based on OpenKey.
Copyright (C) 2026 duongtheson@proton.me

This file is belong to the OpenKey project, Win32 version
which is released under GPL license.
You can fork, modify, improve this program. If you
redistribute your new version, it MUST be open source.
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
	//replace the dialog template font of every control with Segoe UI Variable
	static void applyDialogFont(const HWND& hDlg);
};
