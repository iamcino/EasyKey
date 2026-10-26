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
#include "SettingsWindow.h"
#include "AppDelegate.h"
#include "ThemeHelper.h"

#pragma comment(lib, "gdiplus.lib")

#define SETTINGS_CLASS _T("EasyKeySettingsWindow")
#define WINDOW_STYLE (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME)
#define WINDOW_WIDTH 560 //client width in DIPs
#define MIN_WINDOW_HEIGHT 320

enum ItemKind {
	K_HERO, K_SECTION, K_COMBO, K_HOTKEY, K_TOGGLE, K_EXPANDER,
	K_GROUP, K_SUBTOGGLE, K_SUBBUTTON, K_LINK, K_VERSION
};

enum Option {
	OPT_NONE, OPT_LANGUAGE, OPT_INPUT_TYPE, OPT_CODE_TABLE, OPT_SWITCH_KEY,
	OPT_SPELLING, OPT_MODERN, OPT_MACRO, OPT_RUN_WITH_WINDOWS,
	OPT_RESTORE, OPT_ZWJF, OPT_TEMP_OFF_SPELLING,
	OPT_QUICK_TELEX, OPT_QUICK_START, OPT_QUICK_END, OPT_CAPS_FIRST, OPT_MACRO_ENGLISH, OPT_MACRO_AUTO_CAPS,
	OPT_SMART_SWITCH, OPT_REMEMBER_CODE, OPT_FIX_BROWSER, OPT_FIX_CHROMIUM, OPT_TEMP_OFF_EASYKEY,
	OPT_CLIPBOARD, OPT_BEEP,
	OPT_RUN_AS_ADMIN, OPT_SHOW_ON_STARTUP, OPT_DESKTOP_SHORTCUT, OPT_CHECK_UPDATE
};

enum Action {
	ACT_EXPAND = 100, ACT_MACRO_TABLE, ACT_CONVERT_TOOL, ACT_ABOUT, ACT_CHECK_UPDATE, ACT_DEFAULT_CONFIG
};

struct RowInfo {
	int kind;
	int id;
	LPCTSTR label;
	LPCTSTR hint;
};

static const RowInfo mainToggles[] = {
	{ K_TOGGLE, OPT_SPELLING, _T("Kiểm tra chính tả"), _T("Chỉ bỏ dấu khi từ đúng chính tả tiếng Việt") },
	{ K_TOGGLE, OPT_MODERN, _T("Đặt dấu kiểu mới"), _T("hoà, thuý thay vì hòa, thúy") },
	{ K_TOGGLE, OPT_MACRO, _T("Gõ tắt"), _T("Ví dụ: ko → không") },
	{ K_TOGGLE, OPT_RUN_WITH_WINDOWS, _T("Khởi động cùng Windows"), NULL },
};

static const RowInfo advancedRows[] = {
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
	{ K_SUBTOGGLE, OPT_REMEMBER_CODE, _T("Nhớ bảng mã theo từng ứng dụng"), NULL },
	{ K_SUBTOGGLE, OPT_FIX_BROWSER, _T("Sửa lỗi ở ô có gợi ý tự động"), NULL },
	{ K_SUBTOGGLE, OPT_FIX_CHROMIUM, _T("Sửa lỗi trên Chrome, Edge"), NULL },
	{ K_SUBTOGGLE, OPT_TEMP_OFF_EASYKEY, _T("Tạm tắt EasyKey bằng phím Alt"), NULL },
	{ K_SUBTOGGLE, OPT_CLIPBOARD, _T("Gửi chữ qua Clipboard"), NULL },
	{ K_SUBTOGGLE, OPT_BEEP, _T("Kêu bíp khi chuyển Việt/Anh"), NULL },
	{ K_GROUP, 0, _T("Hệ thống"), NULL },
	{ K_SUBTOGGLE, OPT_RUN_AS_ADMIN, _T("Chạy với quyền Admin"), NULL },
	{ K_SUBTOGGLE, OPT_SHOW_ON_STARTUP, _T("Mở cửa sổ này khi EasyKey khởi động"), NULL },
	{ K_SUBTOGGLE, OPT_DESKTOP_SHORTCUT, _T("Tạo lối tắt trên Desktop"), NULL },
	{ K_SUBTOGGLE, OPT_CHECK_UPDATE, _T("Tự kiểm tra bản mới khi khởi động"), NULL },
	{ K_SUBBUTTON, ACT_CHECK_UPDATE, _T("Kiểm tra bản mới ngay"), NULL },
	{ K_SUBBUTTON, ACT_DEFAULT_CONFIG, _T("Đưa mọi cài đặt về mặc định"), NULL },
};

#define TEXT_ON _T("Bật")
#define TEXT_OFF _T("Tắt")
#define TEXT_PRESS_KEYS _T("Nhấn tổ hợp phím…")
#define TEXT_NO_HOTKEY _T("Chưa đặt")
#define TEXT_MACRO_TABLE _T("Bảng gõ tắt")
#define TEXT_CHECK _T("Kiểm tra")
#define TEXT_CHECKING _T("Đang kiểm tra…")
#define TEXT_RESET _T("Khôi phục")

typedef BOOL(WINAPI *AdjustWindowRectExForDpiFunc)(LPRECT, DWORD, BOOL, DWORD, UINT);

static ULONG_PTR gdiplusToken = 0;
static int gdiplusUsers = 0;

/*-------------------------------- drawing helpers --------------------------------*/

static Gdiplus::Color toColor(const COLORREF& c) {
	return Gdiplus::Color(255, GetRValue(c), GetGValue(c), GetBValue(c));
}

static void addRoundRect(Gdiplus::GraphicsPath& path, const RECT& rc, float radius, const bool& roundTop = true, const bool& roundBottom = true) {
	//inset by half a pixel so 1px strokes stay sharp with PixelOffsetModeHalf
	float x = rc.left + 0.5f, y = rc.top + 0.5f;
	float w = (float)(rc.right - rc.left) - 1, h = (float)(rc.bottom - rc.top) - 1;
	if (radius * 2 > h) radius = h / 2;
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

static void fillRound(Gdiplus::Graphics& g, const RECT& rc, const float& radius, const COLORREF& fill, const COLORREF* stroke,
					  const bool& roundTop = true, const bool& roundBottom = true) {
	Gdiplus::GraphicsPath path;
	addRoundRect(path, rc, radius, roundTop, roundBottom);
	Gdiplus::SolidBrush brush(toColor(fill));
	g.FillPath(&brush, &path);
	if (stroke) {
		Gdiplus::Pen pen(toColor(*stroke), 1);
		g.DrawPath(&pen, &path);
	}
}

static void fillRect(Gdiplus::Graphics& g, const COLORREF& color, const RECT& rc) {
	Gdiplus::SolidBrush brush(toColor(color));
	g.FillRectangle(&brush, (INT)rc.left, (INT)rc.top, (INT)(rc.right - rc.left), (INT)(rc.bottom - rc.top));
}

static void drawLine(Gdiplus::Graphics& g, const COLORREF& color, const int& left, const int& right, const int& y) {
	RECT line = { left, y, right, y + 1 };
	fillRect(g, color, line);
}

static void drawChevron(Gdiplus::Graphics& g, const COLORREF& color, const float& cx, const float& cy, const float& size, const bool& up) {
	float half = size / 2, quarter = size / 4;
	float dir = up ? -1.0f : 1.0f;
	Gdiplus::PointF points[3] = {
		Gdiplus::PointF(cx - half, cy - quarter * dir),
		Gdiplus::PointF(cx, cy + quarter * dir),
		Gdiplus::PointF(cx + half, cy - quarter * dir)
	};
	Gdiplus::Pen pen(toColor(color), size / 8);
	pen.SetLineJoin(Gdiplus::LineJoinRound);
	pen.SetStartCap(Gdiplus::LineCapRound);
	pen.SetEndCap(Gdiplus::LineCapRound);
	g.DrawLines(&pen, points, 3);
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

/*-------------------------------- window --------------------------------*/

SettingsWindow::SettingsWindow(const HINSTANCE& hInstance) {
	this->hInstance = hInstance;
	SetRectEmpty(&expandArea);
	if (gdiplusUsers++ == 0) {
		Gdiplus::GdiplusStartupInput input;
		Gdiplus::GdiplusStartup(&gdiplusToken, &input, NULL);
	}
}

SettingsWindow::~SettingsWindow() {
	destroy();
	freeFonts();
	if (--gdiplusUsers == 0) {
		Gdiplus::GdiplusShutdown(gdiplusToken);
	}
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
	loadPalette();
	createFonts();
	ThemeHelper::applySolidWindowFrame(hWnd, pal.bg);
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

void SettingsWindow::loadPalette() {
	//Windows 11 Fluent colors, see the design mockup
	if (ThemeHelper::isAppDarkMode()) {
		pal.bg = RGB(0x20, 0x20, 0x20);
		pal.card = RGB(0x2B, 0x2B, 0x2B);
		pal.cardHover = RGB(0x32, 0x32, 0x32);
		pal.cardPressed = RGB(0x27, 0x27, 0x27);
		pal.stroke = RGB(0x1D, 0x1D, 0x1D);
		pal.divider = RGB(0x1D, 0x1D, 0x1D);
		pal.text = RGB(0xFF, 0xFF, 0xFF);
		pal.sub = RGB(0xC5, 0xC5, 0xC5);
		pal.disabled = RGB(0x78, 0x78, 0x78);
		pal.accent = RGB(0x60, 0xCD, 0xFF);
		pal.accentHover = RGB(0x5A, 0xBD, 0xEA);
		pal.onAccent = RGB(0x00, 0x00, 0x00);
		pal.field = RGB(0x37, 0x37, 0x37);
		pal.fieldHover = RGB(0x3D, 0x3D, 0x3D);
		pal.fieldStroke = RGB(0x45, 0x45, 0x45);
	} else {
		pal.bg = RGB(0xF3, 0xF3, 0xF3);
		pal.card = RGB(0xFF, 0xFF, 0xFF);
		pal.cardHover = RGB(0xF6, 0xF6, 0xF6);
		pal.cardPressed = RGB(0xF0, 0xF0, 0xF0);
		pal.stroke = RGB(0xE5, 0xE5, 0xE5);
		pal.divider = RGB(0xEB, 0xEB, 0xEB);
		pal.text = RGB(0x1B, 0x1B, 0x1B);
		pal.sub = RGB(0x5D, 0x5D, 0x5D);
		pal.disabled = RGB(0xA0, 0xA0, 0xA0);
		pal.accent = RGB(0x00, 0x5F, 0xB8);
		pal.accentHover = RGB(0x19, 0x6E, 0xBF);
		pal.onAccent = RGB(0xFF, 0xFF, 0xFF);
		pal.field = RGB(0xFB, 0xFB, 0xFB);
		pal.fieldHover = RGB(0xF3, 0xF3, 0xF3);
		pal.fieldStroke = RGB(0xDC, 0xDC, 0xDC);
	}
}

void SettingsWindow::createFonts() {
	freeFonts();
	LPCTSTR face = ThemeHelper::getFontFace();
	auto make = [&](const int& px, const int& weight) {
		return CreateFont(-s(px), 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, face);
	};
	fontBody = make(14, FW_NORMAL);
	fontCaption = make(12, FW_NORMAL);
	fontStrong = make(14, FW_SEMIBOLD);
	fontTitle = make(20, FW_SEMIBOLD);
	fontBadge = make(22, FW_SEMIBOLD);
}

void SettingsWindow::freeFonts() {
	HFONT* fonts[] = { &fontBody, &fontCaption, &fontStrong, &fontTitle, &fontBadge };
	for (HFONT* font : fonts) {
		if (*font) {
			DeleteObject(*font);
			*font = NULL;
		}
	}
}

int SettingsWindow::textWidth(const HFONT& font, LPCTSTR text) const {
	HDC hdc = GetDC(hWnd);
	HGDIOBJ old = SelectObject(hdc, font);
	SIZE size = { 0 };
	GetTextExtentPoint32(hdc, text, lstrlen(text), &size);
	SelectObject(hdc, old);
	ReleaseDC(hWnd, hdc);
	return size.cx;
}

/*-------------------------------- layout --------------------------------*/

void SettingsWindow::layout() {
	int width = s(WINDOW_WIDTH);
	int left = s(24), right = width - s(24);
	int y = s(8);
	items.clear();
	SetRectEmpty(&expandArea);
	stateWidth = max(textWidth(fontBody, TEXT_ON), textWidth(fontBody, TEXT_OFF));

	auto add = [&](const int& kind, const int& id, LPCTSTR label, LPCTSTR hint, const int& height) {
		Item item = {};
		item.kind = kind;
		item.id = id;
		item.label = label;
		item.hint = hint;
		item.rc = makeRect(left, y, right - left, height);
		item.textRight = right - s(16);
		items.push_back(item);
		y += height;
		return (int)items.size() - 1;
	};
	auto placeSwitch = [&](Item& item) {
		item.ctl = makeRect(item.rc.right - s(16) - s(40), centerY(item.rc) - s(10), s(40), s(20));
		item.textRight = item.ctl.left - s(12) - stateWidth - s(12);
	};
	auto placeField = [&](Item& item, const int& fieldWidth) {
		item.ctl = makeRect(item.rc.right - s(16) - fieldWidth, centerY(item.rc) - s(16), fieldWidth, s(32));
		item.textRight = item.ctl.left - s(12);
	};

	//current state and the Vietnamese switch
	int i = add(K_HERO, OPT_LANGUAGE, NULL, NULL, s(88));
	items[i].ctl = makeRect(items[i].rc.right - s(20) - s(40), centerY(items[i].rc) - s(10), s(40), s(20));
	y += s(12);

	add(K_SECTION, 0, _T("Cách gõ"), NULL, s(32));
	int comboWidth = s(160);
	for (LPCTSTR name : EasyKeyManager::getInputType())
		comboWidth = max(comboWidth, textWidth(fontBody, name) + s(11 + 36));
	for (LPCTSTR name : EasyKeyManager::getTableCode())
		comboWidth = max(comboWidth, textWidth(fontBody, name) + s(11 + 36));
	i = add(K_COMBO, OPT_INPUT_TYPE, _T("Kiểu gõ"), NULL, s(60));
	placeField(items[i], comboWidth);
	y += s(4);
	i = add(K_COMBO, OPT_CODE_TABLE, _T("Bảng mã"), NULL, s(60));
	placeField(items[i], comboWidth);
	y += s(4);
	i = add(K_HOTKEY, OPT_SWITCH_KEY, _T("Phím chuyển Việt/Anh"), capturing ? _T("Nhấn Esc để huỷ") : NULL, s(60));
	int hotKeyWidth = max(textWidth(fontBody, TEXT_PRESS_KEYS),
		textWidth(fontBody, EasyKeyHelper::getHotKeyString(capturing ? savedSwitchKey : vSwitchKeyStatus).c_str()));
	placeField(items[i], max(s(120), hotKeyWidth + s(32)));
	y += s(16);

	add(K_SECTION, 0, _T("Thường dùng"), NULL, s(32));
	for (const RowInfo& row : mainToggles) {
		i = add(row.kind, row.id, row.label, row.hint, s(60));
		placeSwitch(items[i]);
		if (row.id == OPT_MACRO) {
			items[i].linkAction = ACT_MACRO_TABLE;
			int linkWidth = textWidth(fontBody, TEXT_MACRO_TABLE) + s(16);
			items[i].link = makeRect(items[i].textRight - linkWidth, centerY(items[i].rc) - s(16), linkWidth, s(32));
			items[i].textRight = items[i].link.left - s(8);
		}
		y += s(4);
	}
	y += s(12);

	//everything else is folded into one expander
	int expander = add(K_EXPANDER, ACT_EXPAND, _T("Nâng cao"), _T("Chính tả, gõ nhanh, ứng dụng, hệ thống"), s(60));
	items[expander].ctl = makeRect(items[expander].rc.right - s(16) - s(16), centerY(items[expander].rc) - s(8), s(16), s(16));
	items[expander].textRight = items[expander].ctl.left - s(12);
	if (expanded) {
		bool afterLabel = false;
		int buttonWidth = max(max(textWidth(fontBody, TEXT_CHECK), textWidth(fontBody, TEXT_CHECKING)),
			textWidth(fontBody, TEXT_RESET)) + s(32);
		for (const RowInfo& row : advancedRows) {
			if (row.kind == K_GROUP) {
				add(row.kind, row.id, row.label, NULL, s(40));
				afterLabel = true;
				continue;
			}
			i = add(row.kind, row.id, row.label, row.hint, s(48));
			items[i].afterLabel = afterLabel;
			afterLabel = false;
			if (row.kind == K_SUBTOGGLE)
				placeSwitch(items[i]);
			else
				placeField(items[i], buttonWidth);
		}
		y += s(4);
		expandArea = makeRect(left, items[expander].rc.top, right - left, y - items[expander].rc.top);
	}
	y += s(16);

	//footer
	int x = left + s(2);
	LPCTSTR links[] = { _T("Công cụ chuyển mã"), _T("Giới thiệu") };
	int actions[] = { ACT_CONVERT_TOOL, ACT_ABOUT };
	for (int l = 0; l < 2; l++) {
		Item item = {};
		item.kind = K_LINK;
		item.id = actions[l];
		item.label = links[l];
		item.rc = makeRect(x, y, textWidth(fontBody, links[l]), s(32));
		item.textRight = item.rc.right;
		items.push_back(item);
		x = item.rc.right + s(24);
	}
	add(K_VERSION, 0, NULL, NULL, s(32));
	y += s(16);
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

void SettingsWindow::paintSwitch(Gdiplus::Graphics& g, const RECT& rc, const bool& on, const bool& hot, const bool& enabled) {
	Gdiplus::GraphicsPath track;
	addRoundRect(track, rc, (rc.bottom - rc.top) / 2.0f);
	float height = (float)(rc.bottom - rc.top);
	float knob = (float)s(hot && enabled ? 14 : 12);
	float cy = rc.top + height / 2;
	float cx;
	if (on) {
		Gdiplus::SolidBrush fill(toColor(!enabled ? pal.disabled : hot ? pal.accentHover : pal.accent));
		g.FillPath(&fill, &track);
		cx = rc.right - height / 2;
		Gdiplus::SolidBrush knobBrush(toColor(enabled ? pal.onAccent : pal.card));
		g.FillEllipse(&knobBrush, cx - knob / 2, cy - knob / 2, knob, knob);
	} else {
		COLORREF color = enabled ? pal.sub : pal.disabled;
		if (hot && enabled) {
			Gdiplus::SolidBrush fill(toColor(pal.fieldHover));
			g.FillPath(&fill, &track);
		}
		Gdiplus::Pen pen(toColor(color), 1);
		g.DrawPath(&pen, &track);
		cx = rc.left + height / 2;
		Gdiplus::SolidBrush knobBrush(toColor(color));
		g.FillEllipse(&knobBrush, cx - knob / 2, cy - knob / 2, knob, knob);
	}
}

void SettingsWindow::paintShapes(Gdiplus::Graphics& g, const Item& item, const int& index) {
	RECT rc = toView(item.rc);
	RECT ctl = toView(item.ctl);
	bool hot = index == hotItem && hotPart == 0;
	bool pressed = hot && index == pressedItem && pressedPart == 0;
	COLORREF cardColor = pressed ? pal.cardPressed : hot ? pal.cardHover : pal.card;

	switch (item.kind) {
	case K_HERO: {
		fillRound(g, rc, (float)s(8), cardColor, &pal.stroke);
		RECT badge = makeRect(rc.left + s(16), centerY(rc) - s(28), s(56), s(56));
		if (vLanguage)
			fillRound(g, badge, (float)s(12), pal.accent, NULL);
		else
			fillRound(g, badge, (float)s(12), pal.field, &pal.fieldStroke);
		paintSwitch(g, ctl, vLanguage != 0, hot, true);
		break;
	}
	case K_COMBO:
	case K_HOTKEY:
	case K_TOGGLE:
		fillRound(g, rc, (float)s(4), cardColor, &pal.stroke);
		if (item.kind == K_TOGGLE) {
			paintSwitch(g, ctl, getOption(item.id), hot, true);
			if (index == hotItem && hotPart == 1) {
				RECT link = toView(item.link);
				fillRound(g, link, (float)s(4), pal.fieldHover, NULL);
			}
		} else {
			bool active = item.kind == K_HOTKEY && capturing;
			fillRound(g, ctl, (float)s(4), active ? pal.card : hot ? pal.fieldHover : pal.field, &pal.fieldStroke);
			if (active) {
				RECT underline = { ctl.left + s(2), ctl.bottom - s(2), ctl.right - s(2), ctl.bottom };
				fillRect(g, pal.accent, underline);
			}
			if (item.kind == K_COMBO)
				drawChevron(g, pal.sub, (float)(ctl.right - s(18)), (float)centerY(ctl), (float)s(10), false);
		}
		break;
	case K_EXPANDER:
		if (expanded) {
			//the header and its rows share one card, see paint()
			if (hot) {
				RECT header = rc;
				InflateRect(&header, -1, 0);
				header.top += 1;
				fillRound(g, header, (float)s(4) - 1, cardColor, NULL, true, false);
			}
			drawLine(g, pal.divider, rc.left + 1, rc.right - 1, rc.bottom - 1);
		} else {
			fillRound(g, rc, (float)s(4), cardColor, &pal.stroke);
		}
		drawChevron(g, pal.text, (ctl.left + ctl.right) / 2.0f, (float)centerY(ctl), (float)s(10), expanded);
		break;
	case K_GROUP:
		break;
	case K_SUBTOGGLE:
	case K_SUBBUTTON:
		if (!item.afterLabel)
			drawLine(g, pal.divider, rc.left + 1, rc.right - 1, rc.top);
		if (item.kind == K_SUBTOGGLE) {
			if (hot) {
				RECT hover = { rc.left + 1, rc.top + 1, rc.right - 1, rc.bottom };
				fillRect(g, cardColor, hover);
			}
			paintSwitch(g, ctl, getOption(item.id), hot, isOptionEnabled(item.id));
		} else {
			bool enabled = isInteractive(index, 0);
			fillRound(g, ctl, (float)s(4), (hot && enabled) ? (pressed ? pal.field : pal.fieldHover) : pal.field, &pal.fieldStroke);
		}
		break;
	}

	//keyboard focus ring
	if (focusVisible && index == focusItem && GetFocus() == hWnd) {
		RECT ring = focusPart == 1 ? toView(item.link) : (item.kind == K_SUBBUTTON ? ctl : rc);
		InflateRect(&ring, s(2), s(2));
		Gdiplus::GraphicsPath path;
		RECT inner = ring;
		InflateRect(&inner, -1, -1); //centre the 2px pen on the ring
		addRoundRect(path, inner, (float)s(6));
		Gdiplus::Pen pen(toColor(pal.text), 2);
		g.DrawPath(&pen, &path);
	}
}

void SettingsWindow::drawText(HDC hdc, LPCTSTR text, RECT rc, const HFONT& font, const COLORREF& color, const UINT& format) {
	if (!text)
		return;
	SelectObject(hdc, font);
	SetTextColor(hdc, color);
	DrawText(hdc, text, -1, &rc, format | DT_SINGLELINE | DT_NOPREFIX);
}

void SettingsWindow::paintTexts(HDC hdc, const Item& item, const int& index) {
	RECT rc = toView(item.rc);
	RECT ctl = toView(item.ctl);
	int cy = centerY(rc);
	bool enabled = item.kind != K_SUBTOGGLE || isOptionEnabled(item.id);
	COLORREF textColor = enabled ? pal.text : pal.disabled;

	//label with an optional hint below it
	auto drawLabel = [&](const HFONT& font) {
		RECT label = { rc.left + s(16), rc.top, item.textRight, rc.bottom };
		if (item.hint) {
			RECT top = label, bottom = label;
			top.bottom = cy + s(1);
			bottom.top = cy + s(1);
			drawText(hdc, item.label, top, font, textColor, DT_BOTTOM | DT_END_ELLIPSIS);
			drawText(hdc, item.hint, bottom, fontCaption, enabled ? pal.sub : pal.disabled, DT_TOP | DT_END_ELLIPSIS);
		} else {
			drawText(hdc, item.label, label, font, textColor, DT_VCENTER | DT_END_ELLIPSIS);
		}
	};
	auto drawState = [&]() {
		RECT state = { ctl.left - s(12) - stateWidth, rc.top, ctl.left - s(12), rc.bottom };
		drawText(hdc, getOption(item.id) ? TEXT_ON : TEXT_OFF, state, fontBody, textColor, DT_VCENTER | DT_RIGHT);
	};

	switch (item.kind) {
	case K_HERO: {
		RECT badge = makeRect(rc.left + s(16), cy - s(28), s(56), s(56));
		drawText(hdc, vLanguage ? _T("V") : _T("E"), badge, fontBadge, vLanguage ? pal.onAccent : pal.text, DT_CENTER | DT_VCENTER);
		RECT title = { badge.right + s(16), rc.top, ctl.left - s(16), cy + s(2) };
		drawText(hdc, vLanguage ? _T("Đang gõ tiếng Việt") : _T("Đang gõ tiếng Anh"), title, fontTitle, pal.text, DT_BOTTOM | DT_END_ELLIPSIS);
		wstring hotKey = EasyKeyHelper::getHotKeyString(capturing ? savedSwitchKey : vSwitchKeyStatus);
		wstring subtitle = hotKey.empty() ? wstring(_T("Chưa đặt phím chuyển Việt/Anh")) : _T("Bấm ") + hotKey + _T(" để chuyển Việt/Anh");
		RECT sub = { title.left, cy + s(4), title.right, rc.bottom };
		drawText(hdc, subtitle.c_str(), sub, fontCaption, pal.sub, DT_TOP | DT_END_ELLIPSIS);
		break;
	}
	case K_SECTION: {
		RECT section = { rc.left + s(2), rc.top, rc.right, rc.bottom - s(8) };
		drawText(hdc, item.label, section, fontStrong, pal.text, DT_BOTTOM);
		break;
	}
	case K_COMBO: {
		drawLabel(fontBody);
		vector<LPCTSTR>& names = item.id == OPT_INPUT_TYPE ? EasyKeyManager::getInputType() : EasyKeyManager::getTableCode();
		int current = item.id == OPT_INPUT_TYPE ? vInputType : vCodeTable;
		RECT field = { ctl.left + s(11), ctl.top, ctl.right - s(32), ctl.bottom };
		if (current >= 0 && current < (int)names.size())
			drawText(hdc, names[current], field, fontBody, pal.text, DT_VCENTER | DT_END_ELLIPSIS);
		break;
	}
	case K_HOTKEY: {
		drawLabel(fontBody);
		wstring text;
		if (capturing) {
			text = captureModifiers ? EasyKeyHelper::getHotKeyString(captureModifiers | 0xFE) + _T(" + …") : TEXT_PRESS_KEYS;
		} else {
			text = EasyKeyHelper::getHotKeyString(vSwitchKeyStatus);
			if (text.empty())
				text = TEXT_NO_HOTKEY;
		}
		drawText(hdc, text.c_str(), ctl, fontBody, capturing && !captureModifiers ? pal.sub : pal.text, DT_CENTER | DT_VCENTER);
		break;
	}
	case K_TOGGLE:
		drawLabel(fontBody);
		drawState();
		if (item.linkAction) {
			bool linkHot = index == hotItem && hotPart == 1;
			drawText(hdc, TEXT_MACRO_TABLE, toView(item.link), fontBody, linkHot ? pal.accentHover : pal.accent, DT_CENTER | DT_VCENTER);
		}
		break;
	case K_EXPANDER:
		drawLabel(fontBody);
		break;
	case K_GROUP: {
		RECT group = { rc.left + s(16), rc.top, rc.right - s(16), rc.bottom - s(8) };
		drawText(hdc, item.label, group, fontStrong, pal.sub, DT_BOTTOM);
		break;
	}
	case K_SUBTOGGLE:
		drawLabel(fontBody);
		drawState();
		break;
	case K_SUBBUTTON: {
		drawLabel(fontBody);
		LPCTSTR text = item.id == ACT_CHECK_UPDATE ? (checkingUpdate ? TEXT_CHECKING : TEXT_CHECK) : TEXT_RESET;
		drawText(hdc, text, ctl, fontBody, isInteractive(index, 0) ? pal.text : pal.disabled, DT_CENTER | DT_VCENTER);
		break;
	}
	case K_LINK: {
		bool linkHot = index == hotItem;
		drawText(hdc, item.label, rc, fontBody, linkHot ? pal.accentHover : pal.accent, DT_VCENTER);
		break;
	}
	case K_VERSION: {
		static wstring version = _T("EasyKey ") + EasyKeyHelper::getVersionString();
		RECT text = { rc.left, rc.top, rc.right - s(2), rc.bottom };
		drawText(hdc, version.c_str(), text, fontCaption, pal.sub, DT_VCENTER | DT_RIGHT);
		break;
	}
	}
}

void SettingsWindow::paint(HDC hdc) {
	RECT client;
	GetClientRect(hWnd, &client);
	if (client.right <= 0 || client.bottom <= 0)
		return;

	//double buffered: shapes with GDI+ for anti-aliasing, then text with GDI for ClearType
	HDC mem = CreateCompatibleDC(hdc);
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, client.right, client.bottom);
	HGDIOBJ oldBitmap = SelectObject(mem, bitmap);
	HBRUSH background = CreateSolidBrush(pal.bg);
	FillRect(mem, &client, background);
	DeleteObject(background);

	RECT view = { 0, scrollY, client.right, scrollY + client.bottom };
	RECT visible;
	{
		Gdiplus::Graphics g(mem);
		g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
		if (expanded && IntersectRect(&visible, &expandArea, &view))
			fillRound(g, toView(expandArea), (float)s(4), pal.card, &pal.stroke);
		for (int i = 0; i < (int)items.size(); i++) {
			if (IntersectRect(&visible, &items[i].rc, &view))
				paintShapes(g, items[i], i);
		}
		RECT track, thumb;
		if (getThumb(track, thumb)) {
			Gdiplus::SolidBrush brush(Gdiplus::Color(hotScrollBar || draggingThumb ? 200 : 130,
				GetRValue(pal.sub), GetGValue(pal.sub), GetBValue(pal.sub)));
			Gdiplus::GraphicsPath path;
			addRoundRect(path, thumb, (thumb.right - thumb.left) / 2.0f);
			g.FillPath(&brush, &path);
		}
	}

	SetBkMode(mem, TRANSPARENT);
	HGDIOBJ oldFont = SelectObject(mem, fontBody);
	for (int i = 0; i < (int)items.size(); i++) {
		if (IntersectRect(&visible, &items[i].rc, &view))
			paintTexts(mem, items[i], i);
	}
	SelectObject(mem, oldFont);

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
	case K_HERO:
	case K_COMBO:
	case K_HOTKEY:
	case K_EXPANDER:
	case K_LINK:
		return part == 0;
	case K_TOGGLE:
		return part == 0 || (part == 1 && it.linkAction != 0);
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
	case K_HERO:
		setOption(OPT_LANGUAGE, !vLanguage);
		break;
	case K_COMBO:
		showDropDown(item);
		break;
	case K_HOTKEY:
		if (capturing)
			endCapture(false, 0);
		else
			startCapture();
		break;
	case K_TOGGLE:
	case K_SUBTOGGLE:
		if (part == 1)
			doAction(item.linkAction);
		else
			setOption(item.id, !getOption(item.id));
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
	vector<LPCTSTR>& names = item.id == OPT_INPUT_TYPE ? EasyKeyManager::getInputType() : EasyKeyManager::getTableCode();
	int current = item.id == OPT_INPUT_TYPE ? vInputType : vCodeTable;
	HMENU menu = CreatePopupMenu();
	for (int i = 0; i < (int)names.size(); i++)
		AppendMenu(menu, MF_STRING | (i == current ? MF_CHECKED : MF_UNCHECKED), i + 1, names[i]);

	RECT field = toView(item.ctl);
	MapWindowPoints(hWnd, NULL, (LPPOINT)&field, 2);
	TPMPARAMS params = { sizeof(params) };
	params.rcExclude = field;
	int command = (int)TrackPopupMenuEx(menu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_VERTICAL,
		field.left, field.bottom, hWnd, &params);
	DestroyMenu(menu);
	if (command <= 0 || command - 1 == current)
		return;

	if (item.id == OPT_INPUT_TYPE) {
		APP_SET_DATA(vInputType, command - 1);
	} else {
		APP_SET_DATA(vCodeTable, command - 1);
		if (vRememberCode) {
			setAppInputMethodStatus(EasyKeyHelper::getFrontMostAppExecuteName(), vLanguage | (vCodeTable << 1));
			saveSmartSwitchKeyData();
		}
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
			loadPalette();
			ThemeHelper::applySolidWindowFrame(hWnd, pal.bg);
			InvalidateRect(hWnd, NULL, FALSE);
		}
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
			moveFocus(1);
			return 0;
		case VK_UP:
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
