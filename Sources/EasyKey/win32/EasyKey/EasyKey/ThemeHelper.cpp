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
#include <objidl.h>
#include <gdiplus.h>
#include <map>
#include "ThemeHelper.h"

#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "gdiplus.lib")

//DWM values are spelled out so the project still builds with older SDKs
#define DWMWA_DARK_MODE_BEFORE_20H1 19
#define DWMWA_DARK_MODE 20
#define DWMWA_CAPTION_COLOR 35
#define DWMWA_COLOR_DEFAULT 0xFFFFFFFF
#define MICA_MIN_BUILD 22621 //DWMWA_SYSTEMBACKDROP_TYPE, Windows 11 22H2
#define DWMWA_BACKDROP_TYPE 38
#define DWM_BACKDROP_NONE 1
#define DWM_BACKDROP_MICA 2

//undocumented uxtheme exports (by ordinal) used by Explorer for dark menus, Windows 10 1903 and later
#define UXTHEME_SET_PREFERRED_APP_MODE 135
#define UXTHEME_FLUSH_MENU_THEMES 136
enum PreferredAppMode { AppModeDefault, AppModeAllowDark, AppModeForceDark, AppModeForceLight };
typedef PreferredAppMode(WINAPI *SetPreferredAppModeFunc)(PreferredAppMode);
typedef void(WINAPI *FlushMenuThemesFunc)();

#define UI_FONT_FACE _T("Segoe UI Variable Text")
#define UI_FONT_FALLBACK _T("Segoe UI")
#define UI_FONT_POINT_SIZE 9

#define PERSONALIZE_KEY _T("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize")
#define ACCENT_KEY _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Accent")

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

static void applyDarkTitleBar(const HWND& hWnd) {
	BOOL dark = ThemeHelper::isAppDarkMode() ? TRUE : FALSE;
	if (FAILED(DwmSetWindowAttribute(hWnd, DWMWA_DARK_MODE, &dark, sizeof(dark))))
		DwmSetWindowAttribute(hWnd, DWMWA_DARK_MODE_BEFORE_20H1, &dark, sizeof(dark));
}

static void repaintFrame(const HWND& hWnd) {
	if (IsWindowVisible(hWnd)) //repaint the caption now instead of on next activation
		SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void ThemeHelper::applyWindowFrame(const HWND& hWnd) {
	applyDarkTitleBar(hWnd);

	//Mica, Windows 11 22H2 and later. The dialog body stays opaque, so it shows in the title bar
	int backdrop = DWM_BACKDROP_MICA;
	DwmSetWindowAttribute(hWnd, DWMWA_BACKDROP_TYPE, &backdrop, sizeof(backdrop));
	repaintFrame(hWnd);
}

static DWORD getWindowsBuild() {
	typedef LONG(WINAPI *RtlGetVersionFunc)(PRTL_OSVERSIONINFOW);
	static DWORD build = 0;
	if (build == 0) {
		RtlGetVersionFunc rtlGetVersion = (RtlGetVersionFunc)GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "RtlGetVersion");
		RTL_OSVERSIONINFOW info = { sizeof(info) };
		if (rtlGetVersion && rtlGetVersion(&info) == 0)
			build = info.dwBuildNumber;
	}
	return build;
}

bool ThemeHelper::applyBackdrop(const HWND& hWnd, const COLORREF& fallbackColor) {
	applyDarkTitleBar(hWnd);
	bool mica = getWindowsBuild() >= MICA_MIN_BUILD && !isHighContrast() &&
		getPersonalizeValue(_T("EnableTransparency"), 1) != 0;
	if (mica) {
		int backdrop = DWM_BACKDROP_MICA;
		mica = SUCCEEDED(DwmSetWindowAttribute(hWnd, DWMWA_BACKDROP_TYPE, &backdrop, sizeof(backdrop)));
	}
	//with the frame extended, DWM shows the backdrop wherever the client area is transparent
	MARGINS margins = { 0 };
	if (mica)
		margins.cxLeftWidth = margins.cxRightWidth = margins.cyTopHeight = margins.cyBottomHeight = -1;
	DwmExtendFrameIntoClientArea(hWnd, &margins);
	if (!mica) {
		int backdrop = DWM_BACKDROP_NONE;
		DwmSetWindowAttribute(hWnd, DWMWA_BACKDROP_TYPE, &backdrop, sizeof(backdrop));
	}
	COLORREF caption = mica ? DWMWA_COLOR_DEFAULT : fallbackColor;
	DwmSetWindowAttribute(hWnd, DWMWA_CAPTION_COLOR, &caption, sizeof(caption));
	repaintFrame(hWnd);
	return mica;
}

COLORREF ThemeHelper::getAccentColor(const bool& dark, const bool& hover) {
	//AccentPalette holds 8 RGBA shades from Light3 to Dark3, the accent itself is the 4th.
	//Fluent fills controls with Dark1 in the light theme and Light2 in the dark theme
	BYTE palette[32];
	DWORD size = sizeof(palette);
	if (RegGetValue(HKEY_CURRENT_USER, ACCENT_KEY, _T("AccentPalette"), RRF_RT_REG_BINARY, NULL, palette, &size) != ERROR_SUCCESS || size < 32) {
		if (dark)
			return hover ? RGB(0x5A, 0xBD, 0xEA) : RGB(0x60, 0xCD, 0xFF);
		return hover ? RGB(0x19, 0x6E, 0xBF) : RGB(0x00, 0x5F, 0xB8);
	}
	int index = dark ? (hover ? 2 : 1) : (hover ? 3 : 4);
	BYTE* c = palette + index * 4;
	return RGB(c[0], c[1], c[2]);
}

void ThemeHelper::startGdiplus() {
	static ULONG_PTR token = 0;
	if (!token) {
		Gdiplus::GdiplusStartupInput input;
		Gdiplus::GdiplusStartup(&token, &input, NULL);
	}
}

Gdiplus::Font* ThemeHelper::createFont(const bool& strong, const float& pixels, const int& style) {
	//GDI+ only knows regular and bold, semibold comes from the named semibold families
	static LPCWSTR regular[] = { L"Segoe UI Variable Text", L"Segoe UI" };
	static LPCWSTR semibold[] = { L"Segoe UI Variable Text Semibold", L"Segoe UI Semibold" };
	LPCWSTR* faces = strong ? semibold : regular;
	for (int i = 0; i < 2; i++) {
		Gdiplus::FontFamily family(faces[i]);
		if (family.IsAvailable())
			return new Gdiplus::Font(&family, pixels, style, Gdiplus::UnitPixel);
	}
	return new Gdiplus::Font(Gdiplus::FontFamily::GenericSansSerif(), pixels,
		strong ? Gdiplus::FontStyleBold : style, Gdiplus::UnitPixel);
}

void ThemeHelper::applyMenuTheme() {
	HMODULE uxtheme = GetModuleHandle(_T("uxtheme.dll"));
	if (!uxtheme)
		uxtheme = LoadLibraryEx(_T("uxtheme.dll"), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (!uxtheme)
		return;
	SetPreferredAppModeFunc setPreferredAppMode =
		(SetPreferredAppModeFunc)GetProcAddress(uxtheme, MAKEINTRESOURCEA(UXTHEME_SET_PREFERRED_APP_MODE));
	FlushMenuThemesFunc flushMenuThemes =
		(FlushMenuThemesFunc)GetProcAddress(uxtheme, MAKEINTRESOURCEA(UXTHEME_FLUSH_MENU_THEMES));
	if (!setPreferredAppMode || !flushMenuThemes)
		return;
	setPreferredAppMode(isAppDarkMode() ? AppModeForceDark : AppModeForceLight);
	flushMenuThemes();
}
