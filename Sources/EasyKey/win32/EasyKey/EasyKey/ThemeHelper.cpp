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
#include <dwmapi.h> //before stdafx.h, its "using namespace std" clashes with rpcndr byte in C++17
#include <map>
#include "ThemeHelper.h"

#pragma comment(lib, "Dwmapi.lib")

//DWM values are spelled out so the project still builds with older SDKs
#define DWMWA_DARK_MODE_BEFORE_20H1 19
#define DWMWA_DARK_MODE 20
#define DWMWA_BACKDROP_TYPE 38
#define DWM_BACKDROP_MICA 2

#define UI_FONT_FACE _T("Segoe UI Variable Text")
#define UI_FONT_FALLBACK _T("Segoe UI")
#define UI_FONT_POINT_SIZE 9

#define PERSONALIZE_KEY _T("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize")

typedef UINT(WINAPI *GetDpiForWindowFunc)(HWND);

static map<UINT, HFONT> uiFonts;

static DWORD getPersonalizeValue(LPCTSTR name, const DWORD& defaultValue) {
	DWORD value = 0;
	DWORD size = sizeof(value);
	if (RegGetValue(HKEY_CURRENT_USER, PERSONALIZE_KEY, name, RRF_RT_REG_DWORD, NULL, &value, &size) != ERROR_SUCCESS)
		return defaultValue;
	return value;
}

static bool isHighContrast() {
	HIGHCONTRAST hc = { sizeof(hc) };
	return SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) && (hc.dwFlags & HCF_HIGHCONTRASTON);
}

bool ThemeHelper::isAppDarkMode() {
	if (isHighContrast())
		return false;
	return getPersonalizeValue(_T("AppsUseLightTheme"), 1) == 0;
}

bool ThemeHelper::isTaskbarDarkMode() {
	//the taskbar was always dark before Windows 10 1903 added this value
	return getPersonalizeValue(_T("SystemUsesLightTheme"), 0) == 0;
}

bool ThemeHelper::isThemeChangedMessage(const UINT& uMsg, const LPARAM& lParam) {
	if (uMsg == WM_THEMECHANGED)
		return true;
	return uMsg == WM_SETTINGCHANGE && lParam != 0 && lstrcmpi((LPCTSTR)lParam, _T("ImmersiveColorSet")) == 0;
}

UINT ThemeHelper::getDpi(const HWND& hWnd) {
	//GetDpiForWindow only exists on Windows 10 1607 and later
	static GetDpiForWindowFunc getDpiForWindow =
		(GetDpiForWindowFunc)GetProcAddress(GetModuleHandle(_T("user32.dll")), "GetDpiForWindow");
	if (getDpiForWindow && hWnd) {
		UINT dpi = getDpiForWindow(hWnd);
		if (dpi > 0)
			return dpi;
	}
	HDC hdc = GetDC(NULL);
	UINT dpi = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(NULL, hdc);
	return dpi;
}

int ThemeHelper::scale(const int& value, const HWND& hWnd) {
	return MulDiv(value, getDpi(hWnd), USER_DEFAULT_SCREEN_DPI);
}

static int CALLBACK onFontFound(const LOGFONT* lf, const TEXTMETRIC* tm, DWORD fontType, LPARAM lParam) {
	*(bool*)lParam = true;
	return 0;
}

LPCTSTR ThemeHelper::getFontFace() {
	//Segoe UI Variable ships with Windows 11 only
	static int hasVariableFont = -1;
	if (hasVariableFont < 0) {
		LOGFONT lf = { 0 };
		lf.lfCharSet = DEFAULT_CHARSET;
		lstrcpyn(lf.lfFaceName, UI_FONT_FACE, LF_FACESIZE);
		bool found = false;
		HDC hdc = GetDC(NULL);
		EnumFontFamiliesEx(hdc, &lf, onFontFound, (LPARAM)&found, 0);
		ReleaseDC(NULL, hdc);
		hasVariableFont = found ? 1 : 0;
	}
	return hasVariableFont ? UI_FONT_FACE : UI_FONT_FALLBACK;
}

static HFONT getUIFont(const UINT& dpi) {
	auto it = uiFonts.find(dpi);
	if (it != uiFonts.end())
		return it->second;
	HFONT font = CreateFont(-MulDiv(UI_FONT_POINT_SIZE, dpi, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, ThemeHelper::getFontFace());
	uiFonts[dpi] = font;
	return font;
}

static bool isUIFont(const HFONT& font) {
	for (auto it = uiFonts.begin(); it != uiFonts.end(); ++it) {
		if (it->second == font)
			return true;
	}
	return false;
}

static BOOL CALLBACK applyFontToChild(HWND hWnd, LPARAM lParam) {
	TCHAR className[16];
	GetClassName(hWnd, className, 16);
	if (lstrcmp(className, _T("#32770")) == 0) //tab pages keep their template font so their children can be compared to it
		return TRUE;

	HFONT font = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
	HFONT templateFont = (HFONT)SendMessage(GetParent(hWnd), WM_GETFONT, 0, 0);
	//leave custom fonts (like the About title) alone
	if (font != NULL && (font == templateFont || isUIFont(font)))
		SendMessage(hWnd, WM_SETFONT, (WPARAM)lParam, TRUE);
	return TRUE;
}

void ThemeHelper::applyDialogFont(const HWND& hDlg) {
	if (lstrcmp(getFontFace(), UI_FONT_FACE) != 0)
		return; //the template font is already Segoe UI
	EnumChildWindows(hDlg, applyFontToChild, (LPARAM)getUIFont(getDpi(hDlg)));
}

void ThemeHelper::applyWindowFrame(const HWND& hWnd) {
	BOOL dark = isAppDarkMode() ? TRUE : FALSE;
	if (FAILED(DwmSetWindowAttribute(hWnd, DWMWA_DARK_MODE, &dark, sizeof(dark))))
		DwmSetWindowAttribute(hWnd, DWMWA_DARK_MODE_BEFORE_20H1, &dark, sizeof(dark));

	//Mica, Windows 11 22H2 and later. The dialog body stays opaque, so it shows in the title bar
	int backdrop = DWM_BACKDROP_MICA;
	DwmSetWindowAttribute(hWnd, DWMWA_BACKDROP_TYPE, &backdrop, sizeof(backdrop));

	if (IsWindowVisible(hWnd)) //repaint the caption now instead of on next activation
		SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}
