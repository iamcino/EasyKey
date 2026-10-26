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
#include <cmath>
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

#define PERSONALIZE_KEY _T("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize")
#define ACCENT_KEY _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Accent")

typedef UINT(WINAPI *GetDpiForWindowFunc)(HWND);


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
	if (vThemeMode != 0)
		return vThemeMode == 2;
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

static COLORREF getWindowsAccent(const bool& dark) {
	//AccentPalette holds 8 RGBA shades from Light3 to Dark3, the accent itself is the 4th.
	//Fluent fills controls with Dark1 in the light theme and Light2 in the dark theme
	BYTE palette[32];
	DWORD size = sizeof(palette);
	if (RegGetValue(HKEY_CURRENT_USER, ACCENT_KEY, _T("AccentPalette"), RRF_RT_REG_BINARY, NULL, palette, &size) != ERROR_SUCCESS || size < 32)
		return dark ? RGB(0x60, 0xCD, 0xFF) : RGB(0x00, 0x5F, 0xB8);
	BYTE* c = palette + (dark ? 1 : 4) * 4;
	return RGB(c[0], c[1], c[2]);
}

//deep shades carry white text in the light theme, pastel and neon ones carry black text in the dark theme
static const struct { LPCTSTR vi, en; COLORREF light, dark; } accentPresets[ThemeHelper::ACCENT_PRESETS] = {
	{ _T("Chàm"), _T("Indigo"), RGB(0x4F, 0x46, 0xE5), RGB(0xA5, 0xB4, 0xFC) },
	{ _T("Oải hương"), _T("Lavender"), RGB(0x7C, 0x3A, 0xED), RGB(0xC4, 0xB5, 0xFD) },
	{ _T("Ngọc"), _T("Teal"), RGB(0x0F, 0x76, 0x6E), RGB(0x5E, 0xEA, 0xD4) },
	{ _T("Chanh"), _T("Lime"), RGB(0x4D, 0x7C, 0x0F), RGB(0xBE, 0xF2, 0x64) },
	{ _T("San hô"), _T("Coral"), RGB(0xC2, 0x41, 0x0C), RGB(0xFD, 0xBA, 0x8C) },
	{ _T("Hồng"), _T("Rose"), RGB(0xBE, 0x18, 0x5D), RGB(0xF9, 0xA8, 0xD4) },
	{ _T("Mocha"), _T("Mocha"), RGB(0x8B, 0x5E, 0x4A), RGB(0xD9, 0xB8, 0xA6) },
};

COLORREF ThemeHelper::getPresetAccent(const int& index, const bool& dark) {
	if (index < 1 || index > ACCENT_PRESETS)
		return getWindowsAccent(dark);
	return dark ? accentPresets[index - 1].dark : accentPresets[index - 1].light;
}

LPCTSTR ThemeHelper::getPresetName(const int& index) {
	if (index < 1 || index > ACCENT_PRESETS)
		return _T("Windows");
	return EasyKeyHelper::isEnglishUi() ? accentPresets[index - 1].en : accentPresets[index - 1].vi;
}

static COLORREF mix(const COLORREF& a, const COLORREF& b, const int& percentB) {
	return RGB((GetRValue(a) * (100 - percentB) + GetRValue(b) * percentB) / 100,
		(GetGValue(a) * (100 - percentB) + GetGValue(b) * percentB) / 100,
		(GetBValue(a) * (100 - percentB) + GetBValue(b) * percentB) / 100);
}

COLORREF ThemeHelper::getAccentColor(const bool& dark, const bool& hover) {
	COLORREF color;
	if (vAccentColor >= 1 && vAccentColor <= ACCENT_PRESETS)
		color = getPresetAccent(vAccentColor, dark);
	else if (vAccentColor == ACCENT_PRESETS + 1)
		color = dark ? (vAccentAutoDark ? deriveDarkAccent((COLORREF)vAccentLight) : (COLORREF)vAccentDark) : (COLORREF)vAccentLight;
	else
		color = getWindowsAccent(dark);
	//hover moves a tenth of the way to the window color, like Fluent's 90% accent fill
	return hover ? mix(color, dark ? RGB(0x20, 0x20, 0x20) : RGB(0xFF, 0xFF, 0xFF), 10) : color;
}

COLORREF ThemeHelper::deriveDarkAccent(const COLORREF& light) {
	double r = GetRValue(light) / 255.0, g = GetGValue(light) / 255.0, b = GetBValue(light) / 255.0;
	double high = max(r, max(g, b)), low = min(r, min(g, b));
	double h = 0, s = 0, l = (high + low) / 2;
	if (high != low) {
		double d = high - low;
		s = l > 0.5 ? d / (2 - high - low) : d / (high + low);
		h = high == r ? (g - b) / d + (g < b ? 6 : 0) : high == g ? (b - r) / d + 2 : (r - g) / d + 4;
		h /= 6;
	}
	l = 0.78;
	s = min(s, 0.9);
	double q = l < 0.5 ? l * (1 + s) : l + s - l * s, p = 2 * l - q;
	auto channel = [&](double t) {
		if (t < 0) t += 1;
		if (t > 1) t -= 1;
		double v = t < 1.0 / 6 ? p + (q - p) * 6 * t : t < 0.5 ? q : t < 2.0 / 3 ? p + (q - p) * (2.0 / 3 - t) * 6 : p;
		return (BYTE)(v * 255 + 0.5);
	};
	return RGB(channel(h + 1.0 / 3), channel(h), channel(h - 1.0 / 3));
}

static double luminance(const COLORREF& c) {
	auto linear = [](const BYTE& v) {
		double x = v / 255.0;
		return x <= 0.03928 ? x / 12.92 : pow((x + 0.055) / 1.055, 2.4);
	};
	return 0.2126 * linear(GetRValue(c)) + 0.7152 * linear(GetGValue(c)) + 0.0722 * linear(GetBValue(c));
}

double ThemeHelper::getContrast(const COLORREF& a, const COLORREF& b) {
	double la = luminance(a), lb = luminance(b);
	return (max(la, lb) + 0.05) / (min(la, lb) + 0.05);
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
