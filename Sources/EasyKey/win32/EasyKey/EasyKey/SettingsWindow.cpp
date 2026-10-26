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
//GDI+ needs the full windows.h and objidl.h, include them before stdafx.h trims windows.h down
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <Shlobj.h>
#include <cmath>
#include "SettingsWindow.h"
#include "AppDelegate.h"
#include "ThemeHelper.h"

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED 0x0320
#endif

#define SETTINGS_CLASS _T("EasyKeySettingsWindow")
#define WINDOW_STYLE (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME)
#define WINDOW_WIDTH 520 //client width in DIPs
#define MIN_WINDOW_HEIGHT 320
#define SIDE 20 //space left and right of the cards

enum ItemKind {
	K_LANG, K_HOTKEY, K_SECTION, K_SEGMENT, K_SEGBUTTON, K_TILE, K_EXPANDER,
	K_GROUP, K_SUBTOGGLE, K_SUBCOMBO, K_SUBBUTTON, K_LINK, K_VERSION
};

enum Option {
	OPT_NONE, OPT_LANGUAGE, OPT_CODE_TABLE,
	OPT_SPELLING, OPT_MODERN, OPT_MACRO, OPT_RUN_WITH_WINDOWS,
	OPT_RESTORE, OPT_ZWJF, OPT_TEMP_OFF_SPELLING,
	OPT_QUICK_TELEX, OPT_QUICK_START, OPT_QUICK_END, OPT_CAPS_FIRST, OPT_MACRO_ENGLISH, OPT_MACRO_AUTO_CAPS,
	OPT_SMART_SWITCH, OPT_REMEMBER_CODE, OPT_FIX_BROWSER, OPT_FIX_CHROMIUM, OPT_TEMP_OFF_EASYKEY,
	OPT_CLIPBOARD, OPT_FLYOUT, OPT_BEEP,
	OPT_RUN_AS_ADMIN, OPT_SHOW_ON_STARTUP, OPT_DESKTOP_SHORTCUT, OPT_CHECK_UPDATE
};

enum Action {
	ACT_EXPAND = 100, ACT_CAPTURE, ACT_MACRO_TABLE, ACT_CONVERT_TOOL, ACT_ABOUT, ACT_CHECK_UPDATE, ACT_DEFAULT_CONFIG
};

enum Icon {
	ICON_SPELLING, ICON_MODERN, ICON_MACRO, ICON_STARTUP
};

struct RowInfo {
	int kind;
	int id;
	LPCTSTR label;
	LPCTSTR hint;
};

struct TileInfo {
	int id;
	int icon;
	LPCTSTR label;
	LPCTSTR hint;
};

static const TileInfo tiles[] = {
	{ OPT_SPELLING, ICON_SPELLING, _T("Kiểm tra chính tả"), _T("Chỉ bỏ dấu khi đúng") },
	{ OPT_MODERN, ICON_MODERN, _T("Dấu kiểu mới"), _T("hoà, thuý") },
	{ OPT_MACRO, ICON_MACRO, _T("Gõ tắt"), _T("ko → không") },
	{ OPT_RUN_WITH_WINDOWS, ICON_STARTUP, _T("Chạy cùng Windows"), _T("Khi đăng nhập") },
};

static const RowInfo advancedRows[] = {
	{ K_GROUP, 0, _T("Bảng mã"), NULL },
	{ K_SUBCOMBO, OPT_CODE_TABLE, _T("Bảng mã cũ"), _T("Chỉ dùng cho văn bản và font cũ") },
	{ K_SUBTOGGLE, OPT_REMEMBER_CODE, _T("Nhớ bảng mã theo từng ứng dụng"), NULL },
	{ K_GROUP, 0, _T("Chính tả"), NULL },
	{ K_SUBTOGGLE, OPT_RESTORE, _T("Tự khôi phục phím khi gõ sai"), NULL },
	{ K_SUBTOGGLE, OPT_ZWJF, _T("Cho phép z, w, j, f làm phụ âm đầu"), NULL },
	{ K_SUBTOGGLE, OPT_TEMP_OFF_SPELLING, _T("Tạm tắt chính tả bằng phím Ctrl"), NULL },
	{ K_GROUP, 0, _T("Gõ nhanh và gõ tắt"), NULL },
	{ K_SUBTOGGLE, OPT_QUICK_TELEX, _T("Gõ nhanh: cc → ch, gg → gi, nn → ng…"), NULL },
	{ K_SUBTOGGLE, OPT_QUICK_START, _T("Phụ âm đầu: f → ph, j → gi, w → qu"), NULL },
	{ K_SUBTOGGLE, OPT_QUICK_END, _T("Phụ âm cuối: g → ng, h → nh, k → ch"), NULL },
	{ K_SUBTOGGLE, OPT_CAPS_FIRST, _T("Viết hoa chữ cái đầu câu"), NULL },
	{ K_SUBTOGGLE, OPT_MACRO_ENGLISH, _T("Gõ tắt cả khi đang gõ tiếng Anh"), NULL },
	{ K_SUBTOGGLE, OPT_MACRO_AUTO_CAPS, _T("Tự viết hoa theo từ gõ tắt"), NULL },
	{ K_GROUP, 0, _T("Ứng dụng"), NULL },
	{ K_SUBTOGGLE, OPT_SMART_SWITCH, _T("Nhớ chế độ Việt/Anh theo từng ứng dụng"), NULL },
	{ K_SUBTOGGLE, OPT_FIX_BROWSER, _T("Sửa lỗi ở ô có gợi ý tự động"), NULL },
	{ K_SUBTOGGLE, OPT_FIX_CHROMIUM, _T("Sửa lỗi trên Chrome, Edge"), NULL },
	{ K_SUBTOGGLE, OPT_TEMP_OFF_EASYKEY, _T("Tạm tắt EasyKey bằng phím Alt"), NULL },
	{ K_SUBTOGGLE, OPT_CLIPBOARD, _T("Gửi chữ qua Clipboard"), NULL },
	{ K_SUBTOGGLE, OPT_FLYOUT, _T("Hiện thông báo khi chuyển Việt/Anh"), NULL },
	{ K_SUBTOGGLE, OPT_BEEP, _T("Kêu bíp khi chuyển Việt/Anh"), NULL },
	{ K_GROUP, 0, _T("Hệ thống"), NULL },
	{ K_SUBTOGGLE, OPT_RUN_AS_ADMIN, _T("Chạy với quyền Admin"), NULL },
	{ K_SUBTOGGLE, OPT_SHOW_ON_STARTUP, _T("Mở cửa sổ này khi EasyKey khởi động"), NULL },
	{ K_SUBTOGGLE, OPT_DESKTOP_SHORTCUT, _T("Tạo lối tắt trên Desktop"), NULL },
	{ K_SUBTOGGLE, OPT_CHECK_UPDATE, _T("Tự kiểm tra bản mới khi khởi động"), NULL },
	{ K_SUBBUTTON, ACT_CHECK_UPDATE, _T("Kiểm tra bản mới ngay"), NULL },
	{ K_SUBBUTTON, ACT_DEFAULT_CONFIG, _T("Đưa mọi cài đặt về mặc định"), NULL },
};

//code tables in the order they are offered, CP 1258 only for people who already use it
static const struct { int code; LPCTSTR name; } codeTables[] = {
	{ 0, _T("Unicode") },
	{ 3, _T("Unicode tổ hợp") },
	{ 1, _T("TCVN3 (ABC)") },
	{ 2, _T("VNI Windows") },
	{ 4, _T("CP 1258") },
};
#define CP1258_CODE 4

#define TEXT_SWITCH _T("Chuyển nhanh")
#define TEXT_CHANGE_KEY _T("Đổi phím")
#define TEXT_CANCEL _T("Huỷ")
#define TEXT_PRESS_KEYS _T("Nhấn tổ hợp phím mới…")
#define TEXT_NO_HOTKEY _T("Chưa đặt phím")
#define TEXT_CHECK _T("Kiểm tra")
#define TEXT_CHECKING _T("Đang kiểm tra…")
#define TEXT_RESET _T("Khôi phục")

typedef BOOL(WINAPI *AdjustWindowRectExForDpiFunc)(LPRECT, DWORD, BOOL, DWORD, UINT);

/*-------------------------------- drawing helpers --------------------------------*/

static DWORD argb(const BYTE& alpha, const COLORREF& c) {
	return ((DWORD)alpha << 24) | ((DWORD)GetRValue(c) << 16) | ((DWORD)GetGValue(c) << 8) | GetBValue(c);
}

static DWORD fade(const DWORD& color, const int& percent) {
	return (((color >> 24) * percent / 100) << 24) | (color & 0xFFFFFF);
}

static void addRoundRect(Gdiplus::GraphicsPath& path, const RECT& rc, float radius, const bool& roundTop = true, const bool& roundBottom = true) {
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

static void fillRound(Gdiplus::Graphics& g, const RECT& rc, const float& radius, const DWORD& fill, const DWORD& stroke = 0,
					  const bool& roundTop = true, const bool& roundBottom = true) {
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

static void fillRect(Gdiplus::Graphics& g, const DWORD& color, const RECT& rc) {
	Gdiplus::SolidBrush brush((Gdiplus::ARGB)color);
	g.FillRectangle(&brush, (INT)rc.left, (INT)rc.top, (INT)(rc.right - rc.left), (INT)(rc.bottom - rc.top));
}

static void drawLine(Gdiplus::Graphics& g, const DWORD& color, const int& left, const int& right, const int& y) {
	RECT line = { left, y, right, y + 1 };
	fillRect(g, color, line);
}

static void drawChevron(Gdiplus::Graphics& g, const DWORD& color, const float& cx, const float& cy, const float& size, const bool& up) {
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

//20x20 line icons of the quick tiles, the same paths as the design mockup
static void drawIcon(Gdiplus::Graphics& g, const int& icon, const float& x, const float& y, const float& size, const DWORD& color) {
	float f = size / 20;
	Gdiplus::Pen pen((Gdiplus::ARGB)color, 1.5f * f);
	pen.SetLineJoin(Gdiplus::LineJoinRound);
	pen.SetStartCap(Gdiplus::LineCapRound);
	pen.SetEndCap(Gdiplus::LineCapRound);
	auto P = [&](const float& px, const float& py) { return Gdiplus::PointF(x + px * f, y + py * f); };
	switch (icon) {
	case ICON_SPELLING: {
		Gdiplus::PointF a[] = { P(3, 15), P(6, 5), P(9, 15) };
		g.DrawLines(&pen, a, 3);
		g.DrawLine(&pen, P(4.2f, 11.5f), P(7.8f, 11.5f));
		Gdiplus::PointF check[] = { P(11, 12), P(13.5f, 14.5f), P(18, 8) };
		g.DrawLines(&pen, check, 3);
		break;
	}
	case ICON_MODERN: {
		Gdiplus::PointF a[] = { P(5, 16), P(10, 5), P(15, 16) };
		g.DrawLines(&pen, a, 3);
		g.DrawLine(&pen, P(7.2f, 11.5f), P(12.8f, 11.5f));
		g.DrawLine(&pen, P(9, 2.5f), P(11, 1));
		break;
	}
	case ICON_MACRO: {
		Gdiplus::PointF bolt[] = { P(11, 2), P(4, 11), P(9, 11), P(8, 18), P(15, 9), P(10, 9) };
		g.DrawPolygon(&pen, bolt, 6);
		break;
	}
	case ICON_STARTUP:
		g.DrawLine(&pen, P(10, 2), P(10, 9));
		//power ring, open at the top
		g.DrawArc(&pen, x + 3.5f * f, y + 3.12f * f, 13 * f, 13 * f, 307.98f, 284.04f);
		break;
	}
}

static RECT makeRect(const int& left, const int& top, const int& width, const int& height) {
	RECT rc = { left, top, left + width, top + height };
	return rc;
}

static int centerY(const RECT& rc) {
	return (rc.top + rc.bottom) / 2;
}

/*-------------------------------- keys --------------------------------*/

static int modifierMask(const WPARAM& vk) {
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

static int pressedModifiers() {
	int mods = 0;
	if (GetKeyState(VK_CONTROL) < 0) mods |= 0x100;
	if (GetKeyState(VK_MENU) < 0) mods |= 0x200;
	if (GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_RWIN) < 0) mods |= 0x400;
	if (GetKeyState(VK_SHIFT) < 0) mods |= 0x800;
	return mods;
}

static int countModifiers(const int& mods) {
	int count = 0;
	for (int mask = 0x100; mask <= 0x800; mask <<= 1) {
		if (mods & mask)
			count++;
	}
	return count;
}

//"Ctrl + Shift + Z" as { "Ctrl", "Shift", "Z" }, one keycap each
static vector<wstring> splitHotKey(const wstring& text) {
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

static LPCTSTR codeTableName(const int& code) {
	for (auto& table : codeTables) {
		if (table.code == code)
			return table.name;
	}
	return codeTables[0].name;
}

/*-------------------------------- window --------------------------------*/

SettingsWindow::SettingsWindow(const HINSTANCE& hInstance) {
	this->hInstance = hInstance;
	SetRectEmpty(&heroArea);
	SetRectEmpty(&expandArea);
	ThemeHelper::startGdiplus();
}

SettingsWindow::~SettingsWindow() {
	destroy();
	freeFonts();
}

void SettingsWindow::show() {
	static ATOM windowClass = 0;
	if (!windowClass) {
		WNDCLASSEX wc = { sizeof(wc) };
		wc.lpfnWndProc = windowProc;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
		wc.hIconSm = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = SETTINGS_CLASS;
		windowClass = RegisterClassEx(&wc);
	}

	//open on the monitor with the mouse, usually the one whose tray was clicked
	POINT cursor;
	GetCursorPos(&cursor);
	MONITORINFO mi = { sizeof(mi) };
	GetMonitorInfo(MonitorFromPoint(cursor, MONITOR_DEFAULTTOPRIMARY), &mi);
	CreateWindowEx(0, SETTINGS_CLASS, _T("EasyKey"), WINDOW_STYLE,
		mi.rcWork.left, mi.rcWork.top, 200, 200, NULL, NULL, hInstance, this);
	if (!hWnd)
		return;

	dpi = ThemeHelper::getDpi(hWnd);
	applyTheme();
	createFonts();
	fitWindow(true);
	ShowWindow(hWnd, SW_SHOWNORMAL);
	SetForegroundWindow(hWnd);
}

void SettingsWindow::bringOnTop() {
	if (!hWnd)
		return;
	if (IsIconic(hWnd))
		ShowWindow(hWnd, SW_RESTORE);
	SetForegroundWindow(hWnd);
}

void SettingsWindow::fillData() {
	if (!hWnd)
		return;
	layout();
	InvalidateRect(hWnd, NULL, FALSE);
}

HWND SettingsWindow::getHwnd() {
	return hWnd;
}

void SettingsWindow::destroy() {
	if (capturing) {
		capturing = false;
		vSwitchKeyStatus = savedSwitchKey;
	}
	if (hWnd) {
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		DestroyWindow(hWnd);
		hWnd = NULL;
	}
}

int SettingsWindow::s(const int& value) const {
	return MulDiv(value, dpi, USER_DEFAULT_SCREEN_DPI);
}

void SettingsWindow::applyTheme() {
	//Windows 11 Fluent colors over Mica, see the 2026 design mockup
	bool dark = ThemeHelper::isAppDarkMode();
	COLORREF solid = dark ? RGB(0x20, 0x20, 0x20) : RGB(0xF3, 0xF3, 0xF3);
	translucent = ThemeHelper::applyBackdrop(hWnd, solid);
	pal.bg = translucent ? 0 : argb(255, solid);
	pal.accent = argb(255, ThemeHelper::getAccentColor(dark, false));
	pal.accentHover = argb(255, ThemeHelper::getAccentColor(dark, true));
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
		pal.link = 0xFF99EBFF;
		pal.linkHover = 0xFFC7F4FF;
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
		pal.link = 0xFF005FB8;
		pal.linkHover = 0xFF003E92;
	}
}

void SettingsWindow::createFonts() {
	freeFonts();
	fontBody = ThemeHelper::createFont(false, (float)s(14), Gdiplus::FontStyleRegular);
	fontCaption = ThemeHelper::createFont(false, (float)s(12), Gdiplus::FontStyleRegular);
	fontSmall = ThemeHelper::createFont(false, (float)s(13), Gdiplus::FontStyleRegular);
	fontStrong = ThemeHelper::createFont(true, (float)s(14), Gdiplus::FontStyleRegular);
	fontTitle = ThemeHelper::createFont(true, (float)s(17), Gdiplus::FontStyleRegular);
	fontBadge = ThemeHelper::createFont(false, (float)s(18), Gdiplus::FontStyleBold);
	textFormat = Gdiplus::StringFormat::GenericTypographic()->Clone();
	textFormat->SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap | Gdiplus::StringFormatFlagsLineLimit |
		Gdiplus::StringFormatFlagsMeasureTrailingSpaces);
	textFormat->SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
}

void SettingsWindow::freeFonts() {
	Gdiplus::Font** fonts[] = { &fontBody, &fontCaption, &fontSmall, &fontStrong, &fontTitle, &fontBadge };
	for (Gdiplus::Font** font : fonts) {
		delete *font;
		*font = NULL;
	}
	delete textFormat;
	textFormat = NULL;
}

int SettingsWindow::textWidth(Gdiplus::Font* font, LPCTSTR text) const {
	HDC hdc = GetDC(NULL);
	Gdiplus::RectF box;
	{
		Gdiplus::Graphics g(hdc);
		g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
		g.MeasureString(text, -1, font, Gdiplus::PointF(0, 0), textFormat, &box);
	}
	ReleaseDC(NULL, hdc);
	return (int)ceil(box.Width);
}

/*-------------------------------- layout --------------------------------*/

void SettingsWindow::layout() {
	int width = s(WINDOW_WIDTH);
	int left = s(SIDE), right = width - s(SIDE);
	int y = s(4);
	items.clear();
	SetRectEmpty(&expandArea);

	auto push = [&](const int& kind, const int& id, LPCTSTR label, LPCTSTR hint, const RECT& rc) {
		Item item = {};
		item.kind = kind;
		item.id = id;
		item.label = label;
		item.hint = hint;
		item.rc = rc;
		item.textRight = rc.right - s(16);
		items.push_back(item);
		return (int)items.size() - 1;
	};
	auto add = [&](const int& kind, const int& id, LPCTSTR label, LPCTSTR hint, const int& height) {
		int i = push(kind, id, label, hint, makeRect(left, y, right - left, height));
		y += height;
		return i;
	};
	auto placeSwitch = [&](Item& item) {
		item.ctl = makeRect(item.rc.right - s(16) - s(40), centerY(item.rc) - s(10), s(40), s(20));
		item.textRight = item.ctl.left - s(16);
	};
	auto placeField = [&](Item& item, const int& fieldWidth) {
		item.ctl = makeRect(item.rc.right - s(16) - fieldWidth, centerY(item.rc) - s(16), fieldWidth, s(32));
		item.textRight = item.ctl.left - s(12);
	};

	//Vietnamese or English, two big buttons and the hotkey below them
	int pad = s(6), gap = s(6);
	int inner = right - left - pad * 2;
	int half = (inner - gap) / 2;
	heroArea = makeRect(left, y, right - left, pad + s(72) + gap + s(36) + pad);
	push(K_LANG, 1, _T("Tiếng Việt"), NULL, makeRect(left + pad, y + pad, half, s(72)));
	push(K_LANG, 0, _T("Tiếng Anh"), NULL, makeRect(left + pad + half + gap, y + pad, inner - half - gap, s(72)));
	int i = push(K_HOTKEY, 0, TEXT_SWITCH, NULL, makeRect(left + pad, y + pad + s(72) + gap, inner, s(36)));
	items[i].linkAction = ACT_CAPTURE;
	int linkWidth = textWidth(fontSmall, capturing ? TEXT_CANCEL : TEXT_CHANGE_KEY) + s(16);
	items[i].link = makeRect(items[i].rc.right - s(2) - linkWidth, centerY(items[i].rc) - s(14), linkWidth, s(28));
	y = heroArea.bottom + s(20);

	//input type as a segmented control
	add(K_SECTION, 0, _T("Kiểu gõ"), NULL, s(20));
	y += s(8);
	vector<LPCTSTR>& inputTypes = EasyKeyManager::getInputType();
	int segment = add(K_SEGMENT, 0, NULL, NULL, s(44));
	int count = (int)inputTypes.size();
	int segmentWidth = (right - left - s(8) - s(4) * (count - 1)) / max(1, count);
	for (int t = 0; t < count; t++) {
		int x = left + s(4) + t * (segmentWidth + s(4));
		int w = t == count - 1 ? right - s(4) - x : segmentWidth;
		push(K_SEGBUTTON, t, inputTypes[t], NULL, makeRect(x, items[segment].rc.top + s(4), w, s(36)));
	}
	y += s(20);

	//the options most people change, as tiles
	add(K_SECTION, 0, _T("Thường dùng"), NULL, s(20));
	y += s(8);
	int tileWidth = (right - left - s(8)) / 2;
	for (int t = 0; t < 4; t++) {
		int x = t % 2 == 0 ? left : right - tileWidth;
		int top = y + (t / 2) * (s(68) + s(8));
		push(K_TILE, tiles[t].id, tiles[t].label, tiles[t].hint, makeRect(x, top, tileWidth, s(68)));
	}
	y += s(68) * 2 + s(8) + s(16);

	//everything else is folded into one expander
	int expander = add(K_EXPANDER, ACT_EXPAND, _T("Nâng cao"), _T("Bảng mã, chính tả, gõ nhanh, ứng dụng, hệ thống"), s(56));
	items[expander].ctl = makeRect(items[expander].rc.right - s(16) - s(16), centerY(items[expander].rc) - s(8), s(16), s(16));
	items[expander].textRight = items[expander].ctl.left - s(12);
	if (expanded) {
		bool afterLabel = false;
		int buttonWidth = max(max(textWidth(fontBody, TEXT_CHECK), textWidth(fontBody, TEXT_CHECKING)),
			textWidth(fontBody, TEXT_RESET)) + s(32);
		int comboWidth = s(160);
		for (auto& table : codeTables)
			comboWidth = max(comboWidth, textWidth(fontBody, table.name) + s(11 + 36));
		for (const RowInfo& row : advancedRows) {
			if (row.kind == K_GROUP) {
				add(row.kind, row.id, row.label, NULL, s(36));
				afterLabel = true;
				continue;
			}
			i = add(row.kind, row.id, row.label, row.hint, s(row.hint ? 56 : 44));
			items[i].afterLabel = afterLabel;
			afterLabel = false;
			if (row.kind == K_SUBTOGGLE)
				placeSwitch(items[i]);
			else
				placeField(items[i], row.kind == K_SUBCOMBO ? comboWidth : buttonWidth);
		}
		y += s(6);
		expandArea = makeRect(left, items[expander].rc.top, right - left, y - items[expander].rc.top);
	}
	y += s(12);

	//footer
	int x = left + s(4);
	LPCTSTR links[] = { _T("Bảng gõ tắt"), _T("Chuyển mã"), _T("Giới thiệu") };
	int actions[] = { ACT_MACRO_TABLE, ACT_CONVERT_TOOL, ACT_ABOUT };
	for (int l = 0; l < 3; l++) {
		i = push(K_LINK, actions[l], links[l], NULL, makeRect(x, y, textWidth(fontSmall, links[l]), s(32)));
		x = items[i].rc.right + s(20);
	}
	add(K_VERSION, 0, NULL, NULL, s(32));
	y += s(12);
	contentHeight = y;

	if (focusItem >= (int)items.size() || (focusItem >= 0 && !isInteractive(focusItem, focusPart)))
		focusItem = -1;
	if (hotItem >= (int)items.size())
		hotItem = -1;
	if (pressedItem >= (int)items.size())
		pressedItem = -1;
	scrollTo(scrollY);
}

static void adjustWindowRect(RECT& rc, const UINT& dpi) {
	static AdjustWindowRectExForDpiFunc adjustForDpi =
		(AdjustWindowRectExForDpiFunc)GetProcAddress(GetModuleHandle(_T("user32.dll")), "AdjustWindowRectExForDpi");
	if (adjustForDpi)
		adjustForDpi(&rc, WINDOW_STYLE, FALSE, 0, dpi);
	else
		AdjustWindowRectEx(&rc, WINDOW_STYLE, FALSE, 0);
}

void SettingsWindow::fitWindow(const bool& center) {
	layout();
	RECT frame = { 0, 0, s(WINDOW_WIDTH), contentHeight };
	adjustWindowRect(frame, dpi);
	int width = frame.right - frame.left, height = frame.bottom - frame.top;

	MONITORINFO mi = { sizeof(mi) };
	GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &mi);
	int workWidth = mi.rcWork.right - mi.rcWork.left, workHeight = mi.rcWork.bottom - mi.rcWork.top;
	height = min(height, workHeight - s(48));

	RECT current;
	GetWindowRect(hWnd, &current);
	int x = current.left, y = current.top;
	if (center) {
		x = mi.rcWork.left + (workWidth - width) / 2;
		y = mi.rcWork.top + (workHeight - height) / 2;
	}
	SetWindowPos(hWnd, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
	layout();
}

int SettingsWindow::viewHeight() const {
	RECT client;
	GetClientRect(hWnd, &client);
	return client.bottom;
}

int SettingsWindow::maxScroll() const {
	return max(0, contentHeight - viewHeight());
}

void SettingsWindow::scrollTo(int y) {
	y = max(0, min(y, maxScroll()));
	if (y == scrollY)
		return;
	scrollY = y;
	if (!draggingThumb && pressedItem < 0) {
		//the content moved under the mouse
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hWnd, &pt);
		int item = -1, part = 0;
		hitTest(pt, item, part);
		hotItem = item;
		hotPart = part;
	}
	InvalidateRect(hWnd, NULL, FALSE);
}

void SettingsWindow::ensureVisible(const int& item) {
	if (item < 0 || item >= (int)items.size())
		return;
	const RECT& rc = items[item].rc;
	if (rc.top - s(8) < scrollY)
		scrollTo(rc.top - s(8));
	else if (rc.bottom + s(8) > scrollY + viewHeight())
		scrollTo(rc.bottom + s(8) - viewHeight());
}

bool SettingsWindow::getThumb(RECT& track, RECT& thumb) const {
	int view = viewHeight();
	if (contentHeight <= view || view <= 0)
		return false;
	RECT client;
	GetClientRect(hWnd, &client);
	track = makeRect(client.right - s(14), s(4), s(12), view - s(8));
	int trackHeight = track.bottom - track.top;
	int thumbHeight = max(s(32), trackHeight * view / contentHeight);
	int thumbTop = track.top + (trackHeight - thumbHeight) * scrollY / max(1, maxScroll());
	int thumbWidth = (hotScrollBar || draggingThumb) ? s(6) : s(3);
	thumb = makeRect(track.right - s(3) - thumbWidth, thumbTop, thumbWidth, thumbHeight);
	return true;
}

/*-------------------------------- painting --------------------------------*/

RECT SettingsWindow::toView(const RECT& rc) const {
	RECT r = rc;
	OffsetRect(&r, 0, -scrollY);
	return r;
}

void SettingsWindow::drawText(Gdiplus::Graphics& g, LPCTSTR text, const RECT& rc, Gdiplus::Font* font, const DWORD& color,
							  const int& hAlign, const int& vAlign) {
	if (!text || !*text || rc.right <= rc.left)
		return;
	static const Gdiplus::StringAlignment align[] = {
		Gdiplus::StringAlignmentNear, Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentFar
	};
	textFormat->SetAlignment(align[hAlign]);
	textFormat->SetLineAlignment(align[vAlign]);
	Gdiplus::RectF box((float)rc.left, (float)rc.top, (float)(rc.right - rc.left), (float)(rc.bottom - rc.top));
	Gdiplus::SolidBrush brush((Gdiplus::ARGB)color);
	g.DrawString(text, -1, font, box, textFormat, &brush);
}

void SettingsWindow::paintSwitch(Gdiplus::Graphics& g, const RECT& rc, const bool& on, const bool& hot, const bool& enabled) {
	float height = (float)(rc.bottom - rc.top);
	float knob = (float)s(hot && enabled ? 14 : 12);
	float cy = rc.top + height / 2;
	if (on) {
		fillRound(g, rc, height / 2, !enabled ? pal.disabled : hot ? pal.accentHover : pal.accent);
		float cx = rc.right - height / 2;
		Gdiplus::SolidBrush knobBrush((Gdiplus::ARGB)(enabled ? pal.onAccent : pal.field));
		g.FillEllipse(&knobBrush, cx - knob / 2, cy - knob / 2, knob, knob);
	} else {
		DWORD color = enabled ? pal.sub : pal.disabled;
		fillRound(g, rc, height / 2, hot && enabled ? pal.fieldHover : 0, color);
		float cx = rc.left + height / 2;
		Gdiplus::SolidBrush knobBrush((Gdiplus::ARGB)color);
		g.FillEllipse(&knobBrush, cx - knob / 2, cy - knob / 2, knob, knob);
	}
}

void SettingsWindow::paintItem(Gdiplus::Graphics& g, const Item& item, const int& index) {
	RECT rc = toView(item.rc);
	RECT ctl = toView(item.ctl);
	int cy = centerY(rc);
	bool hot = index == hotItem && hotPart == 0;
	bool pressed = hot && index == pressedItem && pressedPart == 0;
	bool enabled = item.kind != K_SUBTOGGLE || isOptionEnabled(item.id);
	DWORD textColor = enabled ? pal.text : pal.disabled;

	//label with an optional hint below it
	auto drawLabel = [&](Gdiplus::Font* font, const int& left, const DWORD& color, const DWORD& hintColor) {
		RECT label = { left, rc.top, item.textRight, rc.bottom };
		if (item.hint) {
			RECT top = label, bottom = label;
			top.bottom = cy + s(1);
			bottom.top = cy + s(1);
			drawText(g, item.label, top, font, color, 0, 2);
			drawText(g, item.hint, bottom, fontCaption, hintColor, 0, 0);
		} else {
			drawText(g, item.label, label, font, color, 0, 1);
		}
	};
	auto rowHover = [&]() {
		if (hot) {
			RECT hover = { rc.left + 1, rc.top + 1, rc.right - 1, rc.bottom };
			fillRect(g, pressed ? pal.cardPressed : pal.cardHover, hover);
		}
	};
	//keycaps with a thicker bottom edge, like the keys they stand for
	auto drawKeys = [&](const wstring& hotKey, int x) {
		vector<wstring> keys = splitHotKey(hotKey);
		for (size_t k = 0; k < keys.size(); k++) {
			if (k > 0) {
				int plus = textWidth(fontCaption, _T("+"));
				drawText(g, _T("+"), makeRect(x, rc.top, plus, rc.bottom - rc.top), fontCaption, pal.sub, 0, 1);
				x += plus + s(6);
			}
			int w = textWidth(fontCaption, keys[k].c_str()) + s(16);
			RECT cap = makeRect(x, cy - s(11), w, s(22));
			RECT edge = cap;
			OffsetRect(&edge, 0, 1);
			fillRound(g, edge, (float)s(6), pal.stroke);
			fillRound(g, cap, (float)s(6), pal.field, pal.stroke);
			drawText(g, keys[k].c_str(), cap, fontCaption, pal.text, 1, 1);
			x = cap.right + s(6);
		}
		return x;
	};

	switch (item.kind) {
	case K_LANG: {
		bool on = (vLanguage != 0) == (item.id != 0);
		if (on)
			fillRound(g, rc, (float)s(12), pal.accent);
		else if (hot)
			fillRound(g, rc, (float)s(12), pressed ? pal.cardPressed : pal.cardHover);
		RECT badge = makeRect(rc.left + s(14), cy - s(20), s(40), s(40));
		if (on)
			fillRound(g, badge, (float)s(10), fade(0xFFFFFFFF, 22));
		else
			fillRound(g, badge, (float)s(10), pal.field, pal.stroke);
		DWORD color = on ? pal.onAccent : pal.text;
		drawText(g, item.id ? _T("V") : _T("E"), badge, fontBadge, color, 1, 1);
		RECT title = { badge.right + s(12), rc.top, rc.right - s(8), cy + s(2) };
		drawText(g, item.label, title, fontTitle, color, 0, 2);
		RECT state = { title.left, cy + s(2), title.right, rc.bottom };
		drawText(g, on ? _T("Đang dùng") : _T("Bấm để chuyển"), state, fontCaption, fade(color, 80), 0, 0);
		break;
	}
	case K_HOTKEY: {
		int x = rc.left + s(8);
		int labelWidth = textWidth(fontCaption, item.label);
		drawText(g, item.label, makeRect(x, rc.top, labelWidth, rc.bottom - rc.top), fontCaption, pal.sub, 0, 1);
		x += labelWidth + s(8);
		RECT link = toView(item.link);
		if (capturing) {
			if (captureModifiers) {
				x = drawKeys(EasyKeyHelper::getHotKeyString(captureModifiers | 0xFE), x);
				drawText(g, _T("+ …"), makeRect(x, rc.top, link.left - x, rc.bottom - rc.top), fontCaption, pal.sub, 0, 1);
			} else {
				drawText(g, TEXT_PRESS_KEYS, makeRect(x, rc.top, link.left - x, rc.bottom - rc.top), fontCaption, pal.text, 0, 1);
			}
		} else {
			wstring hotKey = EasyKeyHelper::getHotKeyString(vSwitchKeyStatus);
			if (hotKey.empty())
				drawText(g, TEXT_NO_HOTKEY, makeRect(x, rc.top, link.left - x, rc.bottom - rc.top), fontCaption, pal.text, 0, 1);
			else
				drawKeys(hotKey, x);
		}
		bool linkHot = index == hotItem && hotPart == 1;
		if (linkHot)
			fillRound(g, link, (float)s(6), pal.cardHover);
		drawText(g, capturing ? TEXT_CANCEL : TEXT_CHANGE_KEY, link, fontSmall, linkHot ? pal.linkHover : pal.link, 1, 1);
		break;
	}
	case K_SECTION:
		drawText(g, item.label, makeRect(rc.left + s(4), rc.top, rc.right - rc.left, rc.bottom - rc.top), fontStrong, pal.text, 0, 2);
		break;
	case K_SEGMENT:
		fillRound(g, rc, (float)s(12), pal.track, pal.stroke);
		break;
	case K_SEGBUTTON: {
		bool on = item.id == vInputType;
		if (on)
			fillRound(g, rc, (float)s(8), pal.field, pal.stroke);
		else if (hot)
			fillRound(g, rc, (float)s(8), pressed ? pal.cardPressed : pal.cardHover);
		drawText(g, item.label, rc, on ? fontStrong : fontBody, on || hot ? pal.text : pal.sub, 1, 1);
		break;
	}
	case K_TILE: {
		bool on = getOption(item.id);
		DWORD fill = on ? (hot ? pal.accentHover : pal.accent) : (pressed ? pal.cardPressed : hot ? pal.cardHover : pal.card);
		fillRound(g, rc, (float)s(12), fill, on ? 0 : pal.stroke);
		DWORD color = on ? pal.onAccent : pal.text;
		int icon = 0;
		for (auto& tile : tiles) {
			if (tile.id == item.id)
				icon = tile.icon;
		}
		drawIcon(g, icon, (float)(rc.left + s(14)), (float)(cy - s(10)), (float)s(20), color);
		RECT label = { rc.left + s(14) + s(20) + s(12), rc.top, rc.right - s(10), cy + s(1) };
		drawText(g, item.label, label, fontStrong, color, 0, 2);
		RECT hint = { label.left, cy + s(2), label.right, rc.bottom };
		drawText(g, item.hint, hint, fontCaption, fade(color, 85), 0, 0);
		break;
	}
	case K_EXPANDER:
		if (expanded) {
			//the header and its rows share one card, see paint()
			if (hot)
				fillRect(g, pressed ? pal.cardPressed : pal.cardHover, rc);
			drawLine(g, pal.divider, rc.left + 1, rc.right - 1, rc.bottom - 1);
		} else {
			fillRound(g, rc, (float)s(12), pressed ? pal.cardPressed : hot ? pal.cardHover : pal.card, pal.stroke);
		}
		drawLabel(fontBody, rc.left + s(16), pal.text, pal.sub);
		drawChevron(g, pal.text, (ctl.left + ctl.right) / 2.0f, (float)centerY(ctl), (float)s(10), expanded);
		break;
	case K_GROUP:
		drawText(g, item.label, makeRect(rc.left + s(16), rc.top, rc.right - rc.left - s(32), rc.bottom - rc.top - s(6)),
			fontStrong, pal.sub, 0, 2);
		break;
	case K_SUBTOGGLE:
		if (!item.afterLabel)
			drawLine(g, pal.divider, rc.left + 1, rc.right - 1, rc.top);
		if (enabled)
			rowHover();
		drawLabel(fontBody, rc.left + s(16), textColor, enabled ? pal.sub : pal.disabled);
		paintSwitch(g, ctl, getOption(item.id), hot, enabled);
		break;
	case K_SUBCOMBO: {
		if (!item.afterLabel)
			drawLine(g, pal.divider, rc.left + 1, rc.right - 1, rc.top);
		rowHover();
		drawLabel(fontBody, rc.left + s(16), pal.text, pal.sub);
		fillRound(g, ctl, (float)s(6), hot ? pal.fieldHover : pal.field, pal.stroke);
		RECT field = { ctl.left + s(11), ctl.top, ctl.right - s(32), ctl.bottom };
		drawText(g, codeTableName(vCodeTable), field, fontBody, pal.text, 0, 1);
		drawChevron(g, pal.sub, (float)(ctl.right - s(18)), (float)centerY(ctl), (float)s(10), false);
		break;
	}
	case K_SUBBUTTON: {
		if (!item.afterLabel)
			drawLine(g, pal.divider, rc.left + 1, rc.right - 1, rc.top);
		bool active = isInteractive(index, 0);
		drawLabel(fontBody, rc.left + s(16), pal.text, pal.sub);
		fillRound(g, ctl, (float)s(6), (hot && active) ? (pressed ? pal.field : pal.fieldHover) : pal.field, pal.stroke);
		LPCTSTR text = item.id == ACT_CHECK_UPDATE ? (checkingUpdate ? TEXT_CHECKING : TEXT_CHECK) : TEXT_RESET;
		drawText(g, text, ctl, fontBody, active ? pal.text : pal.disabled, 1, 1);
		break;
	}
	case K_LINK:
		drawText(g, item.label, rc, fontSmall, hot ? pal.linkHover : pal.link, 0, 1);
		break;
	case K_VERSION: {
		static wstring version = _T("EasyKey ") + EasyKeyHelper::getVersionString();
		RECT text = { rc.left, rc.top, rc.right - s(4), rc.bottom };
		drawText(g, version.c_str(), text, fontCaption, pal.sub, 2, 1);
		break;
	}
	}

	//keyboard focus ring
	if (focusVisible && index == focusItem && GetFocus() == hWnd) {
		RECT ring = focusPart == 1 ? toView(item.link) : (item.kind == K_SUBBUTTON ? ctl : rc);
		InflateRect(&ring, s(2), s(2));
		Gdiplus::GraphicsPath path;
		RECT inner = ring;
		InflateRect(&inner, -1, -1); //centre the 2px pen on the ring
		bool large = item.kind == K_LANG || item.kind == K_TILE || item.kind == K_EXPANDER;
		addRoundRect(path, inner, (float)s(large ? 14 : 8));
		Gdiplus::Pen pen((Gdiplus::ARGB)pal.text, 2);
		g.DrawPath(&pen, &path);
	}
}

void SettingsWindow::paint(HDC hdc) {
	RECT client;
	GetClientRect(hWnd, &client);
	if (client.right <= 0 || client.bottom <= 0)
		return;

	//double buffered in a premultiplied 32 bit bitmap, its transparent pixels show the Mica backdrop
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = client.right;
	bmi.bmiHeader.biHeight = -client.bottom;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	void* bits = NULL;
	HBITMAP bitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (!bitmap)
		return;
	HDC mem = CreateCompatibleDC(hdc);
	HGDIOBJ oldBitmap = SelectObject(mem, bitmap);

	RECT view = { 0, scrollY, client.right, scrollY + client.bottom };
	RECT visible;
	{
		Gdiplus::Bitmap canvas(client.right, client.bottom, client.right * 4, PixelFormat32bppPARGB, (BYTE*)bits);
		Gdiplus::Graphics g(&canvas);
		g.Clear(Gdiplus::Color((Gdiplus::ARGB)pal.bg));
		g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
		//ClearType needs an opaque background, grayscale works on Mica too
		g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

		if (IntersectRect(&visible, &heroArea, &view))
			fillRound(g, toView(heroArea), (float)s(16), pal.card, pal.stroke);
		Gdiplus::GraphicsPath expandPath;
		if (expanded) {
			addRoundRect(expandPath, toView(expandArea), (float)s(12));
			if (IntersectRect(&visible, &expandArea, &view))
				fillRound(g, toView(expandArea), (float)s(12), pal.card, pal.stroke);
		}
		for (int i = 0; i < (int)items.size(); i++) {
			if (!IntersectRect(&visible, &items[i].rc, &view))
				continue;
			//rows of the open expander stay inside its rounded card
			bool clip = expanded && (items[i].kind == K_EXPANDER || items[i].kind == K_GROUP || items[i].kind == K_SUBTOGGLE ||
				items[i].kind == K_SUBCOMBO || items[i].kind == K_SUBBUTTON);
			if (clip)
				g.SetClip(&expandPath);
			paintItem(g, items[i], i);
			if (clip)
				g.ResetClip();
		}
		RECT track, thumb;
		if (getThumb(track, thumb))
			fillRound(g, thumb, (thumb.right - thumb.left) / 2.0f, fade(pal.sub, hotScrollBar || draggingThumb ? 80 : 50));
	}

	BitBlt(hdc, 0, 0, client.right, client.bottom, mem, 0, 0, SRCCOPY);
	SelectObject(mem, oldBitmap);
	DeleteObject(bitmap);
	DeleteDC(mem);
}

/*-------------------------------- input --------------------------------*/

bool SettingsWindow::isInteractive(const int& item, const int& part) const {
	if (item < 0 || item >= (int)items.size())
		return false;
	const Item& it = items[item];
	switch (it.kind) {
	case K_LANG:
		return part == 0 && (vLanguage != 0) != (it.id != 0);
	case K_HOTKEY:
		return part == 1;
	case K_SEGBUTTON:
		return part == 0 && it.id != vInputType;
	case K_TILE:
	case K_EXPANDER:
	case K_SUBCOMBO:
	case K_LINK:
		return part == 0;
	case K_SUBTOGGLE:
		return part == 0 && isOptionEnabled(it.id);
	case K_SUBBUTTON:
		return part == 0 && !(it.id == ACT_CHECK_UPDATE && checkingUpdate);
	}
	return false;
}

bool SettingsWindow::hitTest(const POINT& pt, int& item, int& part) const {
	item = -1;
	part = 0;
	RECT client;
	GetClientRect(hWnd, &client);
	if (!PtInRect(&client, pt))
		return false;
	POINT content = { pt.x, pt.y + scrollY };
	for (int i = 0; i < (int)items.size(); i++) {
		const Item& it = items[i];
		if (it.linkAction && PtInRect(&it.link, content)) {
			item = i;
			part = 1;
			return true;
		}
		const RECT& area = it.kind == K_SUBBUTTON ? it.ctl : it.rc;
		if (PtInRect(&area, content) && isInteractive(i, 0)) {
			item = i;
			return true;
		}
	}
	return false;
}

void SettingsWindow::setHot(const int& item, const int& part) {
	if (item == hotItem && part == hotPart)
		return;
	hotItem = item;
	hotPart = part;
	InvalidateRect(hWnd, NULL, FALSE);
}

void SettingsWindow::moveFocus(const int& direction) {
	vector<pair<int, int>> stops;
	for (int i = 0; i < (int)items.size(); i++) {
		for (int part = 0; part < 2; part++) {
			if (isInteractive(i, part))
				stops.push_back(make_pair(i, part));
		}
	}
	if (stops.empty())
		return;
	int current = -1;
	for (int i = 0; i < (int)stops.size(); i++) {
		if (stops[i].first == focusItem && stops[i].second == focusPart)
			current = i;
	}
	int count = (int)stops.size();
	int next = current < 0 ? (direction > 0 ? 0 : count - 1) : (current + direction + count) % count;
	focusItem = stops[next].first;
	focusPart = stops[next].second;
	focusVisible = true;
	ensureVisible(focusItem);
	InvalidateRect(hWnd, NULL, FALSE);
}

void SettingsWindow::activate(const int& index, const int& part) {
	if (!isInteractive(index, part))
		return;
	//copy, the actions below can rebuild the item list
	Item item = items[index];
	switch (item.kind) {
	case K_LANG:
		setOption(OPT_LANGUAGE, item.id != 0);
		break;
	case K_HOTKEY:
		doAction(item.linkAction);
		break;
	case K_SEGBUTTON:
		APP_SET_DATA(vInputType, item.id);
		SystemTrayHelper::updateData();
		layout();
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case K_TILE:
	case K_SUBTOGGLE:
		setOption(item.id, !getOption(item.id));
		break;
	case K_SUBCOMBO:
		showDropDown(item);
		break;
	case K_EXPANDER:
	case K_SUBBUTTON:
	case K_LINK:
		doAction(item.id);
		break;
	}
}

void SettingsWindow::doAction(const int& action) {
	switch (action) {
	case ACT_EXPAND:
		expanded = !expanded;
		layout();
		if (expanded) {
			//bring the new rows into view
			for (const Item& item : items) {
				if (item.kind == K_EXPANDER)
					scrollTo(item.rc.top - s(8));
			}
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case ACT_CAPTURE:
		if (capturing)
			endCapture(false, 0);
		else
			startCapture();
		break;
	case ACT_MACRO_TABLE:
		AppDelegate::getInstance()->onMacroTable();
		break;
	case ACT_CONVERT_TOOL:
		AppDelegate::getInstance()->onConvertTool();
		break;
	case ACT_ABOUT:
		AppDelegate::getInstance()->onEasyKeyAbout();
		break;
	case ACT_CHECK_UPDATE:
		checkUpdate();
		break;
	case ACT_DEFAULT_CONFIG:
		if (MessageBox(hWnd, _T("Bạn có chắc chắn muốn đưa mọi cài đặt về mặc định?"), _T("EasyKey"),
			MB_ICONEXCLAMATION | MB_YESNO) == IDYES) {
			AppDelegate::getInstance()->onDefaultConfig();
		}
		break;
	}
}

/*-------------------------------- options --------------------------------*/

bool SettingsWindow::getOption(const int& id) const {
	int switchKey = capturing ? savedSwitchKey : vSwitchKeyStatus;
	switch (id) {
	case OPT_LANGUAGE: return vLanguage != 0;
	case OPT_SPELLING: return vCheckSpelling != 0;
	case OPT_MODERN: return vUseModernOrthography != 0;
	case OPT_MACRO: return vUseMacro != 0;
	case OPT_RUN_WITH_WINDOWS: return vRunWithWindows != 0;
	case OPT_RESTORE: return vRestoreIfWrongSpelling != 0;
	case OPT_ZWJF: return vAllowConsonantZFWJ != 0;
	case OPT_TEMP_OFF_SPELLING: return vTempOffSpelling != 0;
	case OPT_QUICK_TELEX: return vQuickTelex != 0;
	case OPT_QUICK_START: return vQuickStartConsonant != 0;
	case OPT_QUICK_END: return vQuickEndConsonant != 0;
	case OPT_CAPS_FIRST: return vUpperCaseFirstChar != 0;
	case OPT_MACRO_ENGLISH: return vUseMacroInEnglishMode != 0;
	case OPT_MACRO_AUTO_CAPS: return vAutoCapsMacro != 0;
	case OPT_SMART_SWITCH: return vUseSmartSwitchKey != 0;
	case OPT_REMEMBER_CODE: return vRememberCode != 0;
	case OPT_FIX_BROWSER: return vFixRecommendBrowser != 0;
	case OPT_FIX_CHROMIUM: return vFixChromiumBrowser != 0;
	case OPT_TEMP_OFF_EASYKEY: return vTempOffOpenKey != 0;
	case OPT_CLIPBOARD: return vSendKeyStepByStep == 0;
	case OPT_FLYOUT: return vShowSwitchFlyout != 0;
	case OPT_BEEP: return HAS_BEEP(switchKey) != 0;
	case OPT_RUN_AS_ADMIN: return vRunAsAdmin != 0;
	case OPT_SHOW_ON_STARTUP: return vShowOnStartUp != 0;
	case OPT_DESKTOP_SHORTCUT: return vCreateDesktopShortcut != 0;
	case OPT_CHECK_UPDATE: return vCheckNewVersion != 0;
	}
	return false;
}

bool SettingsWindow::isOptionEnabled(const int& id) const {
	switch (id) {
	case OPT_RESTORE:
	case OPT_ZWJF:
	case OPT_TEMP_OFF_SPELLING:
		return vCheckSpelling != 0;
	case OPT_MACRO_ENGLISH:
	case OPT_MACRO_AUTO_CAPS:
		return vUseMacro != 0;
	case OPT_FIX_CHROMIUM:
		return vFixRecommendBrowser != 0;
	}
	return true;
}

void SettingsWindow::setOption(const int& id, const bool& on) {
	int val = on ? 1 : 0;
	switch (id) {
	case OPT_LANGUAGE:
		APP_SET_DATA(vLanguage, val);
		if (vUseSmartSwitchKey) {
			setAppInputMethodStatus(EasyKeyHelper::getFrontMostAppExecuteName(), vLanguage | (vCodeTable << 1));
			saveSmartSwitchKeyData();
		}
		break;
	case OPT_SPELLING:
		APP_SET_DATA(vCheckSpelling, val);
		vSetCheckSpelling();
		break;
	case OPT_MODERN: APP_SET_DATA(vUseModernOrthography, val); break;
	case OPT_MACRO: APP_SET_DATA(vUseMacro, val); break;
	case OPT_RUN_WITH_WINDOWS:
		APP_SET_DATA(vRunWithWindows, val);
		EasyKeyHelper::registerRunOnStartup(vRunWithWindows);
		break;
	case OPT_RESTORE: APP_SET_DATA(vRestoreIfWrongSpelling, val); break;
	case OPT_ZWJF: APP_SET_DATA(vAllowConsonantZFWJ, val); break;
	case OPT_TEMP_OFF_SPELLING: APP_SET_DATA(vTempOffSpelling, val); break;
	case OPT_QUICK_TELEX: APP_SET_DATA(vQuickTelex, val); break;
	case OPT_QUICK_START: APP_SET_DATA(vQuickStartConsonant, val); break;
	case OPT_QUICK_END: APP_SET_DATA(vQuickEndConsonant, val); break;
	case OPT_CAPS_FIRST: APP_SET_DATA(vUpperCaseFirstChar, val); break;
	case OPT_MACRO_ENGLISH: APP_SET_DATA(vUseMacroInEnglishMode, val); break;
	case OPT_MACRO_AUTO_CAPS: APP_SET_DATA(vAutoCapsMacro, val); break;
	case OPT_SMART_SWITCH: APP_SET_DATA(vUseSmartSwitchKey, val); break;
	case OPT_REMEMBER_CODE: APP_SET_DATA(vRememberCode, val); break;
	case OPT_FIX_BROWSER: APP_SET_DATA(vFixRecommendBrowser, val); break;
	case OPT_FIX_CHROMIUM: APP_SET_DATA(vFixChromiumBrowser, val); break;
	case OPT_TEMP_OFF_EASYKEY: APP_SET_DATA(vTempOffOpenKey, val); break;
	case OPT_CLIPBOARD: APP_SET_DATA(vSendKeyStepByStep, on ? 0 : 1); break;
	case OPT_FLYOUT: APP_SET_DATA(vShowSwitchFlyout, val); break;
	case OPT_BEEP:
		vSwitchKeyStatus = on ? (vSwitchKeyStatus | 0x8000) : (vSwitchKeyStatus & ~0x8000);
		APP_SET_DATA(vSwitchKeyStatus, vSwitchKeyStatus);
		break;
	case OPT_RUN_AS_ADMIN:
		APP_SET_DATA(vRunAsAdmin, val);
		requestRestartAsAdmin();
		break;
	case OPT_SHOW_ON_STARTUP: APP_SET_DATA(vShowOnStartUp, val); break;
	case OPT_DESKTOP_SHORTCUT:
		APP_SET_DATA(vCreateDesktopShortcut, val);
		if (on)
			EasyKeyManager::createDesktopShortcut();
		break;
	case OPT_CHECK_UPDATE: APP_SET_DATA(vCheckNewVersion, val); break;
	}
	SystemTrayHelper::updateData();
	if (hWnd) {
		layout();
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

void SettingsWindow::showDropDown(const Item& item) {
	HMENU menu = CreatePopupMenu();
	for (auto& table : codeTables) {
		if (table.code == CP1258_CODE && vCodeTable != CP1258_CODE)
			continue;
		AppendMenu(menu, MF_STRING | (table.code == vCodeTable ? MF_CHECKED : MF_UNCHECKED), table.code + 1, table.name);
	}

	RECT field = toView(item.ctl);
	MapWindowPoints(hWnd, NULL, (LPPOINT)&field, 2);
	TPMPARAMS params = { sizeof(params) };
	params.rcExclude = field;
	int command = (int)TrackPopupMenuEx(menu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_VERTICAL,
		field.left, field.bottom, hWnd, &params);
	DestroyMenu(menu);
	if (command <= 0 || command - 1 == vCodeTable)
		return;

	APP_SET_DATA(vCodeTable, command - 1);
	if (vRememberCode) {
		setAppInputMethodStatus(EasyKeyHelper::getFrontMostAppExecuteName(), vLanguage | (vCodeTable << 1));
		saveSmartSwitchKeyData();
	}
	SystemTrayHelper::updateData();
	InvalidateRect(hWnd, NULL, FALSE);
}

void SettingsWindow::requestRestartAsAdmin() {
	EasyKeyHelper::registerRunOnStartup(false);
	if (vRunAsAdmin && !IsUserAnAdmin()) {
		int msgboxID = MessageBox(
			hWnd,
			_T("Bạn cần khởi động lại EasyKey để chạy với quyền Admin.\nKhởi động lại ngay?"),
			_T("EasyKey"),
			MB_ICONEXCLAMATION | MB_YESNO
		);
		if (msgboxID == IDYES) {
			PostQuitMessage(0);
			ShellExecute(0, L"runas", EasyKeyHelper::getFullPath().c_str(), 0, 0, SW_SHOWNORMAL);
		}
	} else {
		EasyKeyHelper::registerRunOnStartup(vRunWithWindows);
	}
}

void SettingsWindow::checkUpdate() {
	checkingUpdate = true;
	InvalidateRect(hWnd, NULL, FALSE);
	UpdateWindow(hWnd);

	string newVersion;
	bool hasUpdate = EasyKeyManager::checkUpdate(newVersion);
	checkingUpdate = false;
	InvalidateRect(hWnd, NULL, FALSE);
	if (hasUpdate) {
		WCHAR msg[256];
		wsprintf(msg, TEXT("Đã có EasyKey %s, bạn có muốn cập nhật không?"), utf8ToWideString(newVersion).c_str());
		if (MessageBox(hWnd, msg, _T("EasyKey"), MB_ICONEXCLAMATION | MB_YESNO) == IDYES) {
			WCHAR path[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, path);
			wsprintf(path, TEXT("%s\\EasyKeyUpdate.exe"), path);
			ShellExecute(0, L"", path, 0, 0, SW_SHOWNORMAL);
			AppDelegate::getInstance()->onEasyKeyExit();
		}
	} else {
		MessageBox(hWnd, _T("Bạn đang dùng phiên bản mới nhất."), _T("EasyKey"), MB_OK);
	}
}

/*-------------------------------- switch key capture --------------------------------*/

void SettingsWindow::startCapture() {
	if (capturing)
		return;
	capturing = true;
	savedSwitchKey = vSwitchKeyStatus;
	captureModifiers = 0;
	//keep the keyboard hook from acting on the old hotkey while the new one is typed
	vSwitchKeyStatus = (savedSwitchKey & 0x8000) | 0xFE0000FE;
	SetFocus(hWnd);
	layout();
	InvalidateRect(hWnd, NULL, FALSE);
}

void SettingsWindow::endCapture(const bool& save, const int& hotKey) {
	if (!capturing)
		return;
	capturing = false;
	vSwitchKeyStatus = savedSwitchKey;
	if (save) {
		vSwitchKeyStatus = (savedSwitchKey & 0x8000) | hotKey;
		APP_SET_DATA(vSwitchKeyStatus, vSwitchKeyStatus);
		SystemTrayHelper::updateData();
	}
	if (hWnd) {
		layout();
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

bool SettingsWindow::onCaptureKey(const UINT& uMsg, const WPARAM& vk) {
	bool down = uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN;
	int mask = modifierMask(vk);
	if (down) {
		if (mask) {
			captureModifiers |= mask;
			InvalidateRect(hWnd, NULL, FALSE);
			return true;
		}
		int mods = pressedModifiers();
		if (vk == VK_ESCAPE && mods == 0) {
			endCapture(false, 0);
		} else if (mods != 0 && (vk == VK_SPACE || keyCodeToCharacter((Uint32)vk) != 0)) {
			endCapture(true, mods | (int)vk | ((int)vk << 24));
		} else {
			MessageBeep(MB_OK); //a hotkey needs at least one modifier and a typing key
		}
		return true;
	}
	//modifiers only, like Ctrl + Shift: saved when the first of them is released
	if (mask && (captureModifiers & mask)) {
		if (countModifiers(captureModifiers) >= 2) {
			endCapture(true, captureModifiers | 0xFE | (0xFE << 24));
		} else {
			captureModifiers = 0;
			InvalidateRect(hWnd, NULL, FALSE);
		}
	}
	return true;
}

/*-------------------------------- messages --------------------------------*/

LRESULT CALLBACK SettingsWindow::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_NCCREATE) {
		SettingsWindow* window = (SettingsWindow*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		window->hWnd = hWnd;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
	}
	SettingsWindow* window = (SettingsWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (window)
		return window->handleMessage(uMsg, wParam, lParam);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT SettingsWindow::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
	switch (uMsg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		paint(hdc);
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND:
		return 1;
	case WM_SIZE:
		scrollTo(scrollY);
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_GETMINMAXINFO: {
		//fixed width, the height can be changed
		RECT frame = { 0, 0, s(WINDOW_WIDTH), s(MIN_WINDOW_HEIGHT) };
		adjustWindowRect(frame, dpi);
		MINMAXINFO* info = (MINMAXINFO*)lParam;
		info->ptMinTrackSize.x = info->ptMaxTrackSize.x = frame.right - frame.left;
		info->ptMinTrackSize.y = frame.bottom - frame.top;
		return 0;
	}
	case WM_DPICHANGED: {
		dpi = HIWORD(wParam);
		createFonts();
		layout();
		RECT* suggested = (RECT*)lParam;
		SetWindowPos(hWnd, NULL, suggested->left, suggested->top,
			suggested->right - suggested->left, suggested->bottom - suggested->top, SWP_NOZORDER | SWP_NOACTIVATE);
		layout();
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	}
	case WM_SETTINGCHANGE:
	case WM_THEMECHANGED:
		if (ThemeHelper::isThemeChangedMessage(uMsg, lParam)) {
			applyTheme();
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_DWMCOLORIZATIONCOLORCHANGED:
		applyTheme(); //the accent color changed
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_MOUSEMOVE: {
		if (!trackingMouse) {
			TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hWnd, 0 };
			TrackMouseEvent(&tme);
			trackingMouse = true;
		}
		RECT track, thumb;
		if (draggingThumb) {
			if (getThumb(track, thumb)) {
				int range = (track.bottom - track.top) - (thumb.bottom - thumb.top);
				if (range > 0)
					scrollTo(dragStartScroll + (pt.y - dragStartY) * maxScroll() / range);
			}
			return 0;
		}
		bool onBar = getThumb(track, thumb) && PtInRect(&track, pt);
		if (onBar != hotScrollBar) {
			hotScrollBar = onBar;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		int item = -1, part = 0;
		if (!onBar)
			hitTest(pt, item, part);
		setHot(item, part);
		return 0;
	}
	case WM_MOUSELEAVE:
		trackingMouse = false;
		hotScrollBar = false;
		setHot(-1, 0);
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT) {
			bool onLink = hotItem >= 0 && hotItem < (int)items.size() && (hotPart == 1 || items[hotItem].kind == K_LINK);
			SetCursor(LoadCursor(NULL, onLink ? IDC_HAND : IDC_ARROW));
			return TRUE;
		}
		break;
	case WM_MOUSEWHEEL:
		scrollTo(scrollY - GET_WHEEL_DELTA_WPARAM(wParam) * s(48) / WHEEL_DELTA);
		return 0;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK: {
		SetFocus(hWnd);
		focusVisible = false;
		RECT track, thumb;
		if (getThumb(track, thumb) && PtInRect(&track, pt)) {
			if (PtInRect(&thumb, pt)) {
				draggingThumb = true;
				dragStartY = pt.y;
				dragStartScroll = scrollY;
				SetCapture(hWnd);
			} else {
				scrollTo(scrollY + (pt.y < thumb.top ? -1 : 1) * viewHeight() * 9 / 10);
			}
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		int item = -1, part = 0;
		hitTest(pt, item, part);
		if (capturing && (item < 0 || items[item].kind != K_HOTKEY))
			endCapture(false, 0);
		if (item >= 0) {
			pressedItem = item;
			pressedPart = part;
			SetCapture(hWnd);
		}
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	}
	case WM_LBUTTONUP: {
		if (draggingThumb) {
			draggingThumb = false;
			ReleaseCapture();
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if (pressedItem < 0)
			return 0;
		int pressed = pressedItem, pressedOn = pressedPart;
		pressedItem = -1;
		ReleaseCapture();
		int item = -1, part = 0;
		if (hitTest(pt, item, part) && item == pressed && part == pressedOn) {
			focusItem = item;
			focusPart = part;
			activate(item, part);
		}
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	}
	case WM_CAPTURECHANGED:
		draggingThumb = false;
		pressedItem = -1;
		return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (capturing && onCaptureKey(uMsg, wParam))
			return 0;
		if (uMsg != WM_KEYDOWN)
			break;
		switch (wParam) {
		case VK_TAB:
			moveFocus(GetKeyState(VK_SHIFT) < 0 ? -1 : 1);
			return 0;
		case VK_DOWN:
		case VK_RIGHT:
			moveFocus(1);
			return 0;
		case VK_UP:
		case VK_LEFT:
			moveFocus(-1);
			return 0;
		case VK_SPACE:
		case VK_RETURN:
			if (!(lParam & 0x40000000) && focusItem >= 0) { //ignore auto-repeat
				focusVisible = true;
				activate(focusItem, focusPart);
			}
			return 0;
		case VK_ESCAPE:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return 0;
		case VK_PRIOR:
			scrollTo(scrollY - viewHeight() * 9 / 10);
			return 0;
		case VK_NEXT:
			scrollTo(scrollY + viewHeight() * 9 / 10);
			return 0;
		case VK_HOME:
			scrollTo(0);
			return 0;
		case VK_END:
			scrollTo(maxScroll());
			return 0;
		}
		break;
	case WM_CHAR:
	case WM_SYSCHAR:
		if (capturing)
			return 0;
		break;
	case WM_SETFOCUS:
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_KILLFOCUS:
		endCapture(false, 0);
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_CLOSE:
		//deletes this window, do not touch members afterwards
		AppDelegate::getInstance()->closeSettingsWindow();
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
