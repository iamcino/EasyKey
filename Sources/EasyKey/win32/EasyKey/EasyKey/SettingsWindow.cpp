/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

Based on OpenKey - The Cross platform Open source Vietnamese
Keyboard application.
Copyright (C) 2019 Mai Vu Tuyen
Contact: maivutuyen.91@gmail.com
Github: https://github.com/tuyenvm/OpenKey

SPDX-License-Identifier: GPL-3.0-only

This file is part of EasyKey, a modified version of OpenKey,
released under the GNU General Public License version 3.
See LICENSE and NOTICE.
-----------------------------------------------------------*/
//GDI+ needs the full windows.h and objidl.h, include them before stdafx.h trims windows.h down
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <Shlobj.h>
#include <cmath>
#include <cwctype>
#include "SettingsWindow.h"
#include "AppDelegate.h"
#include "MessageWindow.h"
#include "Updater.h"
#include "ThemeHelper.h"

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED 0x0320
#endif

#define SETTINGS_CLASS _T("EasyKeySettingsWindow")
#define WINDOW_STYLE (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME)
#define WINDOW_WIDTH 440 //client width in DIPs
#define MIN_WINDOW_HEIGHT 240
#define MAX_FIT_HEIGHT 640 //opening Nâng cao grows the window up to this, then it scrolls
#define SIDE 16 //space left and right of the cards
#define SPELLING_GUIDE_URL _T("https://github.com/iamcino/EasyKey#ki%E1%BB%83m-tra-ch%C3%ADnh-t%E1%BA%A3-ti%E1%BA%BFng-vi%E1%BB%87t")

enum ItemKind {
	K_LANG, K_HOTKEY, K_SECTION, K_SEGMENT, K_SEGBUTTON, K_EXPANDER,
	K_GROUP, K_SUBTOGGLE, K_SUBCOMBO, K_SUBBUTTON, K_SUBSEG, K_UILANG, K_THEME, K_SUBLABEL, K_SWATCH, K_HEXFIELD, K_NOTE,
	K_LINK, K_VERSION
};

enum Option {
	OPT_NONE, OPT_LANGUAGE, OPT_CODE_TABLE,
	OPT_SPELLING, OPT_MODERN, OPT_MACRO, OPT_RUN_WITH_WINDOWS,
	OPT_ESCAPE, OPT_INPUT_HOTKEY, OPT_ZWJF,
	OPT_QUICK_TELEX, OPT_QUICK_START, OPT_QUICK_END, OPT_CAPS_FIRST, OPT_MACRO_ENGLISH, OPT_MACRO_AUTO_CAPS,
	OPT_SMART_SWITCH, OPT_REMEMBER_CODE,
	OPT_CLIPBOARD, OPT_FLYOUT, OPT_BEEP,
	OPT_WARN_CONFLICT, OPT_RUN_AS_ADMIN, OPT_DESKTOP_SHORTCUT, OPT_CHECK_UPDATE,
	OPT_AUTO_DARK
};

enum Action {
	ACT_EXPAND = 100, ACT_CAPTURE, ACT_ABOUT, ACT_DEFAULT_CONFIG,
	ACT_SPELLING_GUIDE, ACT_UNINSTALL, ACT_MACRO_TABLE, ACT_CAPTURE_INPUT, ACT_UPDATE, ACT_GAME_APPS
};

#define CUSTOM_ACCENT (ThemeHelper::ACCENT_PRESETS + 1)

struct RowInfo {
	int kind;
	int id;
	LPCTSTR vi, en;
	LPCTSTR hintVi, hintEn;
};

//K_SUBLABEL marks where the appearance rows (language, accent swatches, custom color) go
static const RowInfo advancedRows[] = {
	{ K_GROUP, 1, _T("Gõ"), _T("Typing") },
	{ K_SUBTOGGLE, OPT_SPELLING, _T("Gõ thông minh"), _T("Smart typing"), _T("Chỉ bỏ dấu khi đúng chính tả, gõ sai thì trả lại phím"), _T("Marks only real words, puts the keys back otherwise") },
	{ K_SUBTOGGLE, OPT_MODERN, _T("Dấu kiểu mới: hoà, thuý"), _T("Modern marks: hoà, thuý") },
	{ K_SUBTOGGLE, OPT_MACRO, _T("Gõ tắt: ko → không"), _T("Shortcuts: ko → không") },
	{ K_SUBBUTTON, ACT_MACRO_TABLE, _T("Bảng gõ tắt"), _T("Shortcut list") },
	{ K_SUBTOGGLE, OPT_ESCAPE, _T("Esc trả lại phím vừa gõ: việt → vieejt"), _T("Esc puts back the keys typed: việt → vieejt") },
	{ K_SUBTOGGLE, OPT_INPUT_HOTKEY, _T("Đổi kiểu gõ bằng phím tắt"), _T("Switch input method with keys"), _T("Telex → VNI → Simple Telex"), _T("Telex → VNI → Simple Telex") },
	{ K_GROUP, 2, _T("Giao diện"), _T("Appearance") },
	{ K_SUBLABEL, 0, NULL, NULL },
	{ K_GROUP, 3, _T("Bảng mã"), _T("Character encoding") },
	{ K_SUBCOMBO, OPT_CODE_TABLE, _T("Bảng mã khi gõ"), _T("Typing encoding"), _T("Bảng mã cũ chỉ dùng cho font cũ"), _T("Legacy ones are for old fonts") },
	{ K_SUBTOGGLE, OPT_REMEMBER_CODE, _T("Nhớ bảng mã theo từng ứng dụng"), _T("Remember the encoding per app") },
	{ K_GROUP, 4, _T("Chính tả"), _T("Spelling") },
	{ K_SUBTOGGLE, OPT_ZWJF, _T("Cho phép z, w, j, f làm phụ âm đầu"), _T("Allow z, w, j, f as first consonants") },
	{ K_SUBBUTTON, ACT_SPELLING_GUIDE, _T("Windows gạch đỏ chữ Việt?"), _T("Red underlines under Vietnamese?") },
	{ K_GROUP, 5, _T("Gõ nhanh và gõ tắt"), _T("Quick typing and shortcuts") },
	{ K_SUBTOGGLE, OPT_QUICK_TELEX, _T("Gõ nhanh: cc → ch, gg → gi, nn → ng…"), _T("Quick typing: cc → ch, gg → gi, nn → ng…") },
	{ K_SUBTOGGLE, OPT_QUICK_START, _T("Phụ âm đầu: f → ph, j → gi, w → qu"), _T("First consonants: f → ph, j → gi, w → qu") },
	{ K_SUBTOGGLE, OPT_QUICK_END, _T("Phụ âm cuối: g → ng, h → nh, k → ch"), _T("Last consonants: g → ng, h → nh, k → ch") },
	{ K_SUBTOGGLE, OPT_CAPS_FIRST, _T("Viết hoa chữ cái đầu câu"), _T("Capitalize the first letter of sentences") },
	{ K_SUBTOGGLE, OPT_MACRO_ENGLISH, _T("Gõ tắt cả khi đang gõ tiếng Anh"), _T("Also use shortcuts in English mode") },
	{ K_SUBTOGGLE, OPT_MACRO_AUTO_CAPS, _T("Tự viết hoa theo từ gõ tắt"), _T("Match the capitalization of shortcuts") },
	{ K_GROUP, 6, _T("Ứng dụng"), _T("Apps") },
	{ K_SUBTOGGLE, OPT_SMART_SWITCH, _T("Nhớ chế độ Việt/Anh theo từng ứng dụng"), _T("Remember Vietnamese/English per app") },
	{ K_SUBBUTTON, ACT_GAME_APPS, _T("Chế độ game"), _T("Game mode"), _T("Phím vào game, Enter để gõ tiếng Việt"), _T("Keys go to the game, Enter types Vietnamese") },
	{ K_SUBTOGGLE, OPT_CLIPBOARD, _T("Gửi chữ qua Clipboard"), _T("Send text through the clipboard") },
	{ K_SUBTOGGLE, OPT_FLYOUT, _T("Hiện thông báo khi chuyển Việt/Anh"), _T("Show a notice when switching") },
	{ K_SUBTOGGLE, OPT_BEEP, _T("Kêu bíp khi chuyển Việt/Anh"), _T("Beep when switching") },
	{ K_GROUP, 7, _T("Hệ thống"), _T("System") },
	{ K_SUBTOGGLE, OPT_RUN_WITH_WINDOWS, _T("Chạy cùng Windows"), _T("Start with Windows") },
	{ K_SUBTOGGLE, OPT_WARN_CONFLICT, _T("Cảnh báo khi bộ gõ khác đang chạy"), _T("Warn when another Vietnamese keyboard runs") },
	{ K_SUBTOGGLE, OPT_RUN_AS_ADMIN, _T("Chạy với quyền Admin"), _T("Run as administrator"), _T("Để gõ được trong ứng dụng chạy quyền Admin"), _T("To type in apps that run as administrator") },
	{ K_SUBTOGGLE, OPT_DESKTOP_SHORTCUT, _T("Lối tắt trên màn hình nền"), _T("Desktop shortcut") },
	{ K_SUBTOGGLE, OPT_CHECK_UPDATE, _T("Tự kiểm tra bản mới"), _T("Check for updates automatically") },
	{ K_SUBBUTTON, ACT_DEFAULT_CONFIG, _T("Đưa mọi cài đặt về mặc định"), _T("Reset all settings") },
	{ K_SUBBUTTON, ACT_UNINSTALL, _T("Gỡ EasyKey khỏi Windows"), _T("Remove EasyKey from Windows"), _T("Xoá chạy cùng Windows, lối tắt và cài đặt"), _T("Removes startup, shortcut and settings") },
};

//code tables in the order they are offered, CP 1258 only for people who already use it
static const struct { int code; LPCTSTR vi, en; } codeTables[] = {
	{ 0, _T("Unicode"), _T("Unicode") },
	{ 3, _T("Unicode tổ hợp"), _T("Unicode combining") },
	{ 1, _T("TCVN3 (ABC)"), _T("TCVN3 (ABC)") },
	{ 2, _T("VNI Windows"), _T("VNI Windows") },
	{ 4, _T("CP 1258"), _T("CP 1258") },
};
#define CP1258_CODE 4

typedef BOOL(WINAPI *AdjustWindowRectExForDpiFunc)(LPRECT, DWORD, BOOL, DWORD, UINT);

static LPCTSTR T(LPCTSTR vi, LPCTSTR en) {
	return EasyKeyHelper::isEnglishUi() && en ? en : vi;
}

/*-------------------------------- helpers --------------------------------*/

static wstring hexString(const COLORREF& c) {
	TCHAR text[8];
	wsprintf(text, _T("%02X%02X%02X"), GetRValue(c), GetGValue(c), GetBValue(c));
	return text;
}

static COLORREF customDark() {
	return vAccentAutoDark ? ThemeHelper::deriveDarkAccent((COLORREF)vAccentLight) : (COLORREF)vAccentDark;
}

static LPCTSTR codeTableName(const int& code) {
	for (auto& table : codeTables) {
		if (table.code == code)
			return T(table.vi, table.en);
	}
	return codeTables[0].vi;
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
	applyTheme();
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
		vInputTypeHotKey = savedInputKey;
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
	fillPalette(pal, dark, translucent, solid);
}

void SettingsWindow::createFonts() {
	freeFonts();
	fontBody = ThemeHelper::createFont(false, (float)s(14), Gdiplus::FontStyleRegular);
	fontCaption = ThemeHelper::createFont(false, (float)s(12), Gdiplus::FontStyleRegular);
	fontSmall = ThemeHelper::createFont(false, (float)s(13), Gdiplus::FontStyleRegular);
	fontStrong = ThemeHelper::createFont(true, (float)s(13), Gdiplus::FontStyleRegular);
	fontTitle = ThemeHelper::createFont(true, (float)s(15), Gdiplus::FontStyleRegular);
	fontBadge = ThemeHelper::createFont(false, (float)s(15), Gdiplus::FontStyleBold);
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
		item.textRight = rc.right - s(14);
		items.push_back(item);
		return (int)items.size() - 1;
	};
	auto add = [&](const int& kind, const int& id, LPCTSTR label, LPCTSTR hint, const int& height) {
		int i = push(kind, id, label, hint, makeRect(left, y, right - left, height));
		y += height;
		return i;
	};
	auto placeSwitch = [&](Item& item) {
		item.ctl = makeRect(item.rc.right - s(14) - s(40), centerY(item.rc) - s(10), s(40), s(20));
		item.textRight = item.ctl.left - s(14);
	};
	auto placeField = [&](Item& item, const int& fieldWidth) {
		item.ctl = makeRect(item.rc.right - s(14) - fieldWidth, centerY(item.rc) - s(15), fieldWidth, s(30));
		item.textRight = item.ctl.left - s(12);
	};

	//Vietnamese or English, two big buttons and the hotkey below them
	int pad = s(4), gap = s(4);
	int inner = right - left - pad * 2;
	int half = (inner - gap) / 2;
	heroArea = makeRect(left, y, right - left, pad + s(56) + gap + s(32) + pad);
	push(K_LANG, 1, T(_T("Tiếng Việt"), _T("Vietnamese")), NULL, makeRect(left + pad, y + pad, half, s(56)));
	push(K_LANG, 0, T(_T("Tiếng Anh"), _T("English")), NULL, makeRect(left + pad + half + gap, y + pad, inner - half - gap, s(56)));
	int i = push(K_HOTKEY, 0, T(_T("Chuyển nhanh"), _T("Switch with")), NULL, makeRect(left + pad, y + pad + s(56) + gap, inner, s(32)));
	items[i].linkAction = ACT_CAPTURE;
	LPCTSTR linkText = capturing && captureTarget == 0 ? T(_T("Huỷ"), _T("Cancel")) : T(_T("Đổi phím"), _T("Change"));
	int linkWidth = textWidth(fontSmall, linkText) + s(16);
	items[i].link = makeRect(items[i].rc.right - linkWidth, centerY(items[i].rc) - s(13), linkWidth, s(26));
	y = heroArea.bottom + s(14);

	//input type as a segmented control
	add(K_SECTION, 0, T(_T("Kiểu gõ"), _T("Input method")), NULL, s(18));
	y += s(6);
	vector<LPCTSTR>& inputTypes = EasyKeyManager::getInputType();
	int segment = add(K_SEGMENT, 0, NULL, NULL, s(36));
	int count = (int)inputTypes.size();
	int segmentWidth = (right - left - s(6) - s(3) * (count - 1)) / max(1, count);
	for (int t = 0; t < count; t++) {
		int x = left + s(3) + t * (segmentWidth + s(3));
		int w = t == count - 1 ? right - s(3) - x : segmentWidth;
		push(K_SEGBUTTON, t, inputTypes[t], NULL, makeRect(x, items[segment].rc.top + s(3), w, s(30)));
	}
	y += s(14);

	//everything else is folded into one expander
	int expander = add(K_EXPANDER, ACT_EXPAND, T(_T("Nâng cao"), _T("Advanced")),
		T(_T("Gõ, giao diện, bảng mã, hệ thống"), _T("Typing, appearance, encoding, system")), s(48));
	items[expander].ctl = makeRect(items[expander].rc.right - s(14) - s(16), centerY(items[expander].rc) - s(8), s(16), s(16));
	items[expander].textRight = items[expander].ctl.left - s(12);
	if (expanded) {
		bool afterLabel = false;
		int buttonWidth = s(64);
		LPCTSTR buttonTexts[] = { T(_T("Mở"), _T("Open")),
			T(_T("Khôi phục"), _T("Reset")), T(_T("Xem cách sửa"), _T("How to fix")), T(_T("Gỡ"), _T("Remove")), T(_T("Chọn game"), _T("Games")) };
		for (LPCTSTR text : buttonTexts)
			buttonWidth = max(buttonWidth, textWidth(fontSmall, text) + s(28));
		int comboWidth = s(140);
		for (auto& table : codeTables)
			comboWidth = max(comboWidth, textWidth(fontSmall, T(table.vi, table.en)) + s(10 + 32));
		bool groupOpen = true;
		for (const RowInfo& row : advancedRows) {
			if (row.kind == K_GROUP) {
				//each group folds; its header is a row with a chevron
				groupOpen = (openGroups & (1 << row.id)) != 0;
				i = add(row.kind, row.id, T(row.vi, row.en), NULL, s(44));
				items[i].ctl = makeRect(items[i].rc.right - s(14) - s(16), centerY(items[i].rc) - s(8), s(16), s(16));
				items[i].textRight = items[i].ctl.left - s(12);
				afterLabel = groupOpen;
				continue;
			}
			if (!groupOpen)
				continue;
			if (row.kind == K_SUBLABEL) {
				//display language and color mode: three choices each in a small segmented control
				auto addChoices = [&](const int& id, const int& kind, LPCTSTR label, LPCTSTR* names) {
					i = add(K_SUBSEG, id, label, NULL, s(44));
					items[i].afterLabel = afterLabel;
					afterLabel = false;
					int widths[3], total = s(4);
					for (int n = 0; n < 3; n++) {
						widths[n] = textWidth(fontCaption, names[n]) + s(16);
						total += widths[n] + s(2);
					}
					items[i].ctl = makeRect(items[i].rc.right - s(14) - total, centerY(items[i].rc) - s(15), total, s(30));
					items[i].textRight = items[i].ctl.left - s(10);
					int x = items[i].ctl.left + s(2);
					for (int n = 0; n < 3; n++) {
						push(kind, n, names[n], NULL, makeRect(x, items[i].ctl.top + s(2), widths[n], s(26)));
						x += widths[n] + s(2);
					}
				};
				LPCTSTR languages[] = { T(_T("Theo Windows"), _T("Windows")), _T("Tiếng Việt"), _T("English") };
				addChoices(0, K_UILANG, T(_T("Ngôn ngữ giao diện"), _T("Display language")), languages);
				LPCTSTR modes[] = { T(_T("Theo Windows"), _T("Windows")), T(_T("Sáng"), _T("Light")), T(_T("Tối"), _T("Dark")) };
				addChoices(1, K_THEME, T(_T("Chế độ màu"), _T("Color mode")), modes);

				//accent: label row, then the swatches, five to a row
				add(K_SUBLABEL, 0, T(_T("Màu nhấn"), _T("Accent color")), T(_T("Trái: chế độ sáng · Phải: tối"), _T("Left: light mode · Right: dark")), s(36));
				int cell = (right - left - s(20)) / 5;
				for (int n = 0; n <= CUSTOM_ACCENT; n++) {
					LPCTSTR name = n == CUSTOM_ACCENT ? T(_T("Tuỳ chỉnh"), _T("Custom")) : ThemeHelper::getPresetName(n);
					push(K_SWATCH, n, name, NULL, makeRect(left + s(10) + (n % 5) * cell, y + (n / 5) * s(54), cell, s(52)));
				}
				y += s(54) * 2 + s(4);

				if (vAccentColor == CUSTOM_ACCENT) {
					//two hex fields, each with its label above
					int fieldWidth = (right - left - s(28) - s(10)) / 2;
					push(K_HEXFIELD, 0, T(_T("Chế độ sáng"), _T("Light mode")), NULL, makeRect(left + s(14), y, fieldWidth, s(52)));
					push(K_HEXFIELD, 1, T(_T("Chế độ tối"), _T("Dark mode")), NULL, makeRect(left + s(14) + fieldWidth + s(10), y, fieldWidth, s(52)));
					for (int n = 0; n < 2; n++) {
						Item& field = items[items.size() - 2 + n];
						field.ctl = makeRect(field.rc.left, field.rc.top + s(20), field.rc.right - field.rc.left, s(30));
					}
					y += s(56);
					i = add(K_SUBTOGGLE, OPT_AUTO_DARK, T(_T("Tự tạo màu cho chế độ tối"), _T("Make the dark mode shade for me")), NULL, s(40));
					items[i].afterLabel = true;
					placeSwitch(items[i]);
					add(K_NOTE, 0, NULL, NULL, s(26));
				}
				afterLabel = false;
				continue;
			}
			i = add(row.kind, row.id, T(row.vi, row.en), T(row.hintVi, row.hintEn), s(row.hintVi ? 52 : 40));
			items[i].afterLabel = afterLabel;
			afterLabel = false;
			if (row.kind == K_SUBTOGGLE)
				placeSwitch(items[i]);
			else
				placeField(items[i], row.kind == K_SUBCOMBO ? comboWidth : buttonWidth);
			if (row.id == OPT_INPUT_HOTKEY && vSwitchInputType) {
				//the keys, with a link to change them, under the switch
				i = add(K_HOTKEY, 1, T(_T("Phím tắt"), _T("Keys")), NULL, s(40));
				items[i].linkAction = ACT_CAPTURE_INPUT;
				LPCTSTR text = capturing && captureTarget == 1 ? T(_T("Huỷ"), _T("Cancel")) : T(_T("Đổi phím"), _T("Change"));
				int width = textWidth(fontSmall, text) + s(16);
				items[i].link = makeRect(items[i].rc.right - s(8) - width, centerY(items[i].rc) - s(13), width, s(26));
			}
		}
		y += s(6);
		expandArea = makeRect(left, items[expander].rc.top, right - left, y - items[expander].rc.top);
	}
	y += s(10);

	//footer; the shortcut list and the converter open from the tray menu
	LPCTSTR about = T(_T("Giới thiệu"), _T("About"));
	push(K_LINK, ACT_ABOUT, about, NULL, makeRect(left + s(2), y, textWidth(fontSmall, about), s(28)));
	if (!Updater::availableVersion().empty()) {
		//a background check found a newer version: offer it where the version number sits
		static wstring update;
		update = T(_T("↑ Cập nhật lên "), _T("↑ Update to ")) + utf8ToWideString(Updater::availableVersion());
		int w = textWidth(fontSmall, update.c_str());
		push(K_LINK, ACT_UPDATE, update.c_str(), NULL, makeRect(right - s(2) - w, y, w, s(28)));
		y += s(28);
	} else {
		add(K_VERSION, 0, NULL, NULL, s(28));
	}
	y += s(10);
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
	height = min(height, min(workHeight - s(48), s(MAX_FIT_HEIGHT)));

	RECT current;
	GetWindowRect(hWnd, &current);
	int x = current.left, y = current.top;
	if (center) {
		x = mi.rcWork.left + (workWidth - width) / 2;
		y = mi.rcWork.top + (workHeight - height) / 2;
	} else if (y + height > mi.rcWork.bottom) {
		y = max(mi.rcWork.top, mi.rcWork.bottom - height);
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

void SettingsWindow::revealGroup(const int& group) {
	int header = -1;
	for (int i = 0; i < (int)items.size(); i++) {
		if (items[i].kind == K_GROUP && items[i].id == group) {
			header = i;
			break;
		}
	}
	if (header < 0)
		return;
	int bottom = expandArea.bottom;
	for (int i = header + 1; i < (int)items.size(); i++) {
		if (items[i].kind == K_GROUP) {
			bottom = items[i].rc.top;
			break;
		}
	}
	//scroll down until the group's last row shows, keeping its header in view
	int top = items[header].rc.top - s(8);
	if (bottom + s(8) > scrollY + viewHeight())
		scrollTo(min(top, bottom + s(8) - viewHeight()));
	else if (top < scrollY)
		scrollTo(top);
}

bool SettingsWindow::getThumb(RECT& track, RECT& thumb) const {
	int view = viewHeight();
	if (contentHeight <= view || view <= 0)
		return false;
	RECT client;
	GetClientRect(hWnd, &client);
	track = makeRect(client.right - s(12), s(4), s(10), view - s(8));
	int trackHeight = track.bottom - track.top;
	int thumbHeight = max(s(32), trackHeight * view / contentHeight);
	int thumbTop = track.top + (trackHeight - thumbHeight) * scrollY / max(1, maxScroll());
	int thumbWidth = (hotScrollBar || draggingThumb) ? s(6) : s(3);
	thumb = makeRect(track.right - s(2) - thumbWidth, thumbTop, thumbWidth, thumbHeight);
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
	auto divider = [&]() {
		if (!item.afterLabel)
			drawLine(g, pal.divider, rc.left + 1, rc.right - 1, rc.top);
	};
	//keycaps with a thicker bottom edge, like the keys they stand for
	auto drawKeys = [&](const wstring& hotKey, int x) {
		vector<wstring> keys = splitHotKey(hotKey);
		for (size_t k = 0; k < keys.size(); k++) {
			if (k > 0) {
				int plus = textWidth(fontCaption, _T("+"));
				drawText(g, _T("+"), makeRect(x, rc.top, plus, rc.bottom - rc.top), fontCaption, pal.sub, 0, 1);
				x += plus + s(5);
			}
			int w = textWidth(fontCaption, keys[k].c_str()) + s(14);
			RECT cap = makeRect(x, cy - s(10), w, s(20));
			RECT edge = cap;
			OffsetRect(&edge, 0, 1);
			fillRound(g, edge, (float)s(5), pal.stroke);
			fillRound(g, cap, (float)s(5), pal.field, pal.stroke);
			drawText(g, keys[k].c_str(), cap, fontCaption, pal.text, 1, 1);
			x = cap.right + s(5);
		}
		return x;
	};

	switch (item.kind) {
	case K_LANG: {
		bool on = (vLanguage != 0) == (item.id != 0);
		if (on)
			fillRound(g, rc, (float)s(9), pal.accent);
		else if (hot)
			fillRound(g, rc, (float)s(9), pressed ? pal.cardPressed : pal.cardHover);
		RECT badge = makeRect(rc.left + s(10), cy - s(16), s(32), s(32));
		if (on)
			fillRound(g, badge, (float)s(8), pal.onAccent == 0xFFFFFFFF ? fade(0xFFFFFFFF, 22) : 0x24000000);
		else
			fillRound(g, badge, (float)s(8), pal.field, pal.stroke);
		DWORD color = on ? pal.onAccent : pal.text;
		drawText(g, item.id ? _T("V") : _T("E"), badge, fontBadge, color, 1, 1);
		RECT title = { badge.right + s(10), rc.top, rc.right - s(6), cy + s(2) };
		drawText(g, item.label, title, fontTitle, color, 0, 2);
		RECT state = { title.left, cy + s(2), title.right, rc.bottom };
		drawText(g, on ? T(_T("Đang dùng"), _T("In use")) : T(_T("Bấm để chuyển"), _T("Click to switch")), state, fontCaption, fade(color, 80), 0, 0);
		break;
	}
	case K_HOTKEY: {
		//0 the switch key under the language buttons, 1 the input method key in Nâng cao
		bool mine = capturing && captureTarget == item.id;
		if (item.id == 1)
			divider();
		int x = rc.left + s(item.id == 1 ? 14 : 8);
		int labelWidth = textWidth(fontCaption, item.label);
		drawText(g, item.label, makeRect(x, rc.top, labelWidth, rc.bottom - rc.top), fontCaption, pal.sub, 0, 1);
		x += labelWidth + s(8);
		RECT link = toView(item.link);
		if (mine) {
			if (captureModifiers) {
				x = drawKeys(EasyKeyHelper::getHotKeyString(captureModifiers | 0xFE), x);
				drawText(g, _T("+ …"), makeRect(x, rc.top, link.left - x, rc.bottom - rc.top), fontCaption, pal.sub, 0, 1);
			} else {
				drawText(g, T(_T("Nhấn tổ hợp phím mới…"), _T("Press the new keys…")), makeRect(x, rc.top, link.left - x, rc.bottom - rc.top), fontCaption, pal.text, 0, 1);
			}
		} else {
			wstring hotKey = EasyKeyHelper::getHotKeyString(item.id == 1 ? vInputTypeHotKey : vSwitchKeyStatus);
			if (hotKey.empty())
				drawText(g, T(_T("Chưa đặt phím"), _T("No key set")), makeRect(x, rc.top, link.left - x, rc.bottom - rc.top), fontCaption, pal.text, 0, 1);
			else
				drawKeys(hotKey, x);
		}
		bool linkHot = index == hotItem && hotPart == 1;
		if (linkHot)
			fillRound(g, link, (float)s(6), pal.cardHover);
		drawText(g, mine ? T(_T("Huỷ"), _T("Cancel")) : T(_T("Đổi phím"), _T("Change")), link, fontSmall, linkHot ? pal.linkHover : pal.link, 1, 1);
		break;
	}
	case K_SECTION:
		drawText(g, item.label, makeRect(rc.left + s(2), rc.top, rc.right - rc.left, rc.bottom - rc.top), fontStrong, pal.text, 0, 2);
		break;
	case K_SEGMENT:
		fillRound(g, rc, (float)s(10), pal.track, pal.stroke);
		break;
	case K_SEGBUTTON: {
		bool on = item.id == vInputType;
		if (on)
			fillRound(g, rc, (float)s(7), pal.field, pal.stroke);
		else if (hot)
			fillRound(g, rc, (float)s(7), pressed ? pal.cardPressed : pal.cardHover);
		drawText(g, item.label, rc, on ? fontStrong : fontSmall, on || hot ? pal.text : pal.sub, 1, 1);
		break;
	}
	case K_EXPANDER:
		if (expanded) {
			//the header and its rows share one card, see paint()
			if (hot)
				fillRect(g, pressed ? pal.cardPressed : pal.cardHover, rc);
			drawLine(g, pal.divider, rc.left + 1, rc.right - 1, rc.bottom - 1);
		} else {
			fillRound(g, rc, (float)s(10), pressed ? pal.cardPressed : hot ? pal.cardHover : pal.card, pal.stroke);
		}
		drawLabel(fontBody, rc.left + s(14), pal.text, pal.sub);
		drawChevron(g, pal.text, (ctl.left + ctl.right) / 2.0f, (float)centerY(ctl), (float)s(10), expanded);
		break;
	case K_GROUP: {
		bool open = (openGroups & (1 << item.id)) != 0;
		divider();
		rowHover();
		drawText(g, item.label, makeRect(rc.left + s(14), rc.top, item.textRight - rc.left - s(14), rc.bottom - rc.top),
			fontTitle, pal.text, 0, 1);
		drawChevron(g, pal.sub, (ctl.left + ctl.right) / 2.0f, (float)centerY(ctl), (float)s(10), open);
		break;
	}
	case K_SUBTOGGLE:
		divider();
		if (enabled)
			rowHover();
		drawLabel(fontBody, rc.left + s(14), textColor, enabled ? pal.sub : pal.disabled);
		paintSwitch(g, ctl, getOption(item.id), hot, enabled);
		break;
	case K_SUBCOMBO: {
		divider();
		rowHover();
		drawLabel(fontBody, rc.left + s(14), pal.text, pal.sub);
		fillRound(g, ctl, (float)s(6), hot ? pal.fieldHover : pal.field, pal.stroke);
		RECT field = { ctl.left + s(10), ctl.top, ctl.right - s(30), ctl.bottom };
		drawText(g, codeTableName(vCodeTable), field, fontSmall, pal.text, 0, 1);
		drawChevron(g, pal.sub, (float)(ctl.right - s(16)), (float)centerY(ctl), (float)s(10), false);
		break;
	}
	case K_SUBBUTTON: {
		divider();
		bool active = isInteractive(index, 0);
		drawLabel(fontBody, rc.left + s(14), pal.text, pal.sub);
		fillRound(g, ctl, (float)s(6), (hot && active) ? (pressed ? pal.field : pal.fieldHover) : pal.field, pal.stroke);
		LPCTSTR text = T(_T("Khôi phục"), _T("Reset"));
		if (item.id == ACT_SPELLING_GUIDE)
			text = T(_T("Xem cách sửa"), _T("How to fix"));
		else if (item.id == ACT_UNINSTALL)
			text = T(_T("Gỡ"), _T("Remove"));
		else if (item.id == ACT_MACRO_TABLE)
			text = T(_T("Mở"), _T("Open"));
		else if (item.id == ACT_GAME_APPS)
			text = T(_T("Chọn game"), _T("Games"));
		drawText(g, text, ctl, fontSmall, active ? pal.text : pal.disabled, 1, 1);
		break;
	}
	case K_SUBSEG:
		divider();
		drawLabel(fontBody, rc.left + s(14), pal.text, pal.sub);
		fillRound(g, ctl, (float)s(8), pal.track, pal.stroke);
		break;
	case K_UILANG:
	case K_THEME: {
		bool on = item.id == (item.kind == K_UILANG ? vUiLanguage : vThemeMode);
		if (on)
			fillRound(g, rc, (float)s(6), pal.field, pal.stroke);
		else if (hot)
			fillRound(g, rc, (float)s(6), pal.cardHover);
		drawText(g, item.label, rc, fontCaption, on || hot ? pal.text : pal.sub, 1, 1);
		break;
	}
	case K_SUBLABEL: {
		drawLine(g, pal.divider, rc.left + 1, rc.right - 1, rc.top);
		RECT label = { rc.left + s(14), rc.top, rc.right - s(14), rc.bottom };
		drawText(g, item.label, label, fontBody, pal.text, 0, 1);
		drawText(g, item.hint, label, fontCaption, pal.sub, 2, 1);
		break;
	}
	case K_SWATCH: {
		bool on = item.id == vAccentColor;
		COLORREF light, dark;
		if (item.id == CUSTOM_ACCENT) {
			light = (COLORREF)vAccentLight;
			dark = customDark();
		} else {
			light = ThemeHelper::getPresetAccent(item.id, false);
			dark = ThemeHelper::getPresetAccent(item.id, true);
		}
		float size = (float)s(28), cx = (rc.left + rc.right) / 2.0f, top = (float)(rc.top + s(5));
		//left half the light theme shade, right half the dark one
		Gdiplus::RectF circle(cx - size / 2, top, size, size);
		Gdiplus::SolidBrush lightBrush((Gdiplus::ARGB)argb(255, light)), darkBrush((Gdiplus::ARGB)argb(255, dark));
		g.FillPie(&lightBrush, circle, 90, 180);
		g.FillPie(&darkBrush, circle, 270, 180);
		if (on || hot) {
			Gdiplus::Pen ring((Gdiplus::ARGB)(on ? pal.text : pal.sub), (float)s(2));
			float r = size / 2 + s(4);
			g.DrawEllipse(&ring, cx - r, top + size / 2 - r, r * 2, r * 2);
		}
		RECT name = { rc.left, rc.top + s(38), rc.right, rc.bottom };
		drawText(g, item.label, name, fontCaption, on ? pal.text : pal.sub, 1, 1);
		break;
	}
	case K_HEXFIELD: {
		bool editing = hexEditing == item.id;
		bool active = isInteractive(index, 0);
		drawText(g, item.label, makeRect(rc.left, rc.top, rc.right - rc.left, s(18)), fontCaption, pal.sub, 0, 1);
		fillRound(g, ctl, (float)s(6), editing ? pal.field : hot ? pal.fieldHover : pal.field, pal.stroke);
		if (editing) {
			RECT underline = { ctl.left + s(2), ctl.bottom - s(2), ctl.right - s(2), ctl.bottom };
			fillRect(g, pal.accent, underline);
		}
		COLORREF color = item.id == 0 ? (COLORREF)vAccentLight : customDark();
		RECT chip = makeRect(ctl.left + s(8), centerY(ctl) - s(7), s(14), s(14));
		fillRound(g, chip, (float)s(3), argb(255, color));
		wstring text = _T("#") + (editing ? hexBuffer : hexString(color));
		RECT value = { chip.right + s(8), ctl.top, ctl.right - s(8), ctl.bottom };
		drawText(g, text.c_str(), value, fontSmall, active || editing ? pal.text : pal.disabled, 0, 1);
		if (editing) {
			//caret after the text
			int caret = value.left + textWidth(fontSmall, text.c_str()) + s(1);
			RECT bar = { caret, centerY(ctl) - s(8), caret + max(1, s(1)), centerY(ctl) + s(8) };
			fillRect(g, pal.text, bar);
		}
		break;
	}
	case K_NOTE: {
		//white text on the light shade and black text on the dark one must stay readable
		double light = ThemeHelper::getContrast((COLORREF)vAccentLight, RGB(255, 255, 255));
		double dark = ThemeHelper::getContrast(customDark(), RGB(0, 0, 0));
		bool good = light >= 4.5 && dark >= 4.5;
		TCHAR text[160];
		swprintf_s(text, good ? T(_T("Tương phản: sáng %.1f:1, tối %.1f:1, dễ đọc"), _T("Contrast: light %.1f:1, dark %.1f:1, easy to read"))
			: T(_T("Tương phản: sáng %.1f:1, tối %.1f:1, khó đọc"), _T("Contrast: light %.1f:1, dark %.1f:1, hard to read")),
			light, dark);
		drawText(g, text, makeRect(rc.left + s(14), rc.top, rc.right - rc.left - s(28), rc.bottom - rc.top), fontCaption, good ? pal.good : pal.warning, 0, 1);
		break;
	}
	case K_LINK:
		if (item.id == ACT_UPDATE)
			drawText(g, item.label, rc, fontSmall, pal.good, 0, 1); //green like the arrow on the tray icon
		else
			drawText(g, item.label, rc, fontSmall, hot ? pal.linkHover : pal.link, 0, 1);
		break;
	case K_VERSION: {
		static wstring version = _T("EasyKey ") + EasyKeyHelper::getVersionString();
		RECT text = { rc.left, rc.top, rc.right - s(2), rc.bottom };
		drawText(g, version.c_str(), text, fontCaption, pal.sub, 2, 1);
		break;
	}
	}

	//keyboard focus ring
	if (focusVisible && index == focusItem && GetFocus() == hWnd) {
		RECT ring = focusPart == 1 ? toView(item.link) : ((item.kind == K_SUBBUTTON || item.kind == K_HEXFIELD) ? ctl : rc);
		InflateRect(&ring, s(2), s(2));
		Gdiplus::GraphicsPath path;
		RECT inner = ring;
		InflateRect(&inner, -1, -1); //centre the 2px pen on the ring
		bool large = item.kind == K_LANG || item.kind == K_EXPANDER;
		addRoundRect(path, inner, (float)s(large ? 12 : 8));
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
			fillRound(g, toView(heroArea), (float)s(12), pal.card, pal.stroke);
		Gdiplus::GraphicsPath expandPath;
		if (expanded) {
			addRoundRect(expandPath, toView(expandArea), (float)s(10));
			if (IntersectRect(&visible, &expandArea, &view))
				fillRound(g, toView(expandArea), (float)s(10), pal.card, pal.stroke);
		}
		for (int i = 0; i < (int)items.size(); i++) {
			if (!IntersectRect(&visible, &items[i].rc, &view))
				continue;
			//rows of the open expander stay inside its rounded card
			bool clip = expanded && items[i].kind >= K_EXPANDER && items[i].kind <= K_NOTE;
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
	case K_UILANG:
		return part == 0 && it.id != vUiLanguage;
	case K_THEME:
		return part == 0 && it.id != vThemeMode;
	case K_SWATCH:
		return part == 0 && it.id != vAccentColor;
	case K_HEXFIELD:
		return part == 0 && (it.id == 0 || !vAccentAutoDark);
	case K_EXPANDER:
	case K_GROUP:
	case K_SUBCOMBO:
	case K_LINK:
		return part == 0;
	case K_SUBTOGGLE:
		return part == 0 && isOptionEnabled(it.id);
	case K_SUBBUTTON:
		return part == 0;
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
		const RECT& area = (it.kind == K_SUBBUTTON || it.kind == K_HEXFIELD) ? it.ctl : it.rc;
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
		AppDelegate::getInstance()->showInputTypeFlyout();
		layout();
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case K_UILANG:
		APP_SET_DATA(vUiLanguage, item.id);
		AppDelegate::getInstance()->onAppearanceChanged();
		break;
	case K_THEME:
		APP_SET_DATA(vThemeMode, item.id);
		ThemeHelper::applyMenuTheme();
		AppDelegate::getInstance()->onAppearanceChanged();
		break;
	case K_SWATCH:
		APP_SET_DATA(vAccentColor, item.id);
		AppDelegate::getInstance()->onAppearanceChanged();
		break;
	case K_HEXFIELD:
		startHexEdit(item.id);
		break;
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
	case K_GROUP: {
		//one group open at a time; opening one shows all of it
		int group = item.id;
		bool opening = (openGroups & (1 << group)) == 0;
		openGroups = opening ? 1 << group : 0;
		endHexEdit();
		fitWindow(false);
		if (opening)
			revealGroup(group);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	}
}

void SettingsWindow::doAction(const int& action) {
	switch (action) {
	case ACT_EXPAND:
		expanded = !expanded;
		if (!expanded)
			openGroups = 0; //Nâng cao opens again with every group closed
		endHexEdit();
		fitWindow(false);
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
	case ACT_CAPTURE_INPUT: {
		int target = action == ACT_CAPTURE ? 0 : 1;
		bool same = capturing && captureTarget == target;
		endCapture(false, 0);
		if (!same)
			startCapture(target);
		break;
	}
	case ACT_ABOUT:
		AppDelegate::getInstance()->onEasyKeyAbout();
		break;
	case ACT_UPDATE:
		Updater::checkAndOffer(hWnd, false);
		break;
	case ACT_GAME_APPS:
		for (const Item& item : items) {
			if (item.id == ACT_GAME_APPS && item.kind == K_SUBBUTTON)
				showGameApps(item);
		}
		break;
	case ACT_SPELLING_GUIDE:
		ShellExecute(NULL, _T("open"), SPELLING_GUIDE_URL, NULL, NULL, SW_SHOWNORMAL);
		break;
	case ACT_MACRO_TABLE:
		AppDelegate::getInstance()->onMacroTable();
		break;
	case ACT_DEFAULT_CONFIG:
		if (MessageWindow::ask(hWnd, T(_T("Đưa mọi cài đặt về mặc định?"), _T("Reset every setting to its default?")),
			T(_T("Bảng gõ tắt vẫn được giữ nguyên."), _T("Your shortcut list is kept.")),
			T(_T("Đặt lại"), _T("Reset")), T(_T("Huỷ"), _T("Cancel")))) {
			AppDelegate::getInstance()->onDefaultConfig();
		}
		break;
	case ACT_UNINSTALL:
		if (MessageWindow::ask(hWnd, T(_T("Gỡ EasyKey khỏi Windows?"), _T("Remove EasyKey from Windows?")),
			T(_T("EasyKey sẽ thoát, không còn chạy cùng Windows, lối tắt trên Desktop và mọi cài đặt, bảng gõ tắt sẽ bị xoá."),
				_T("EasyKey quits and no longer starts with Windows. Its desktop shortcut, settings and shortcut list are deleted.")),
			T(_T("Gỡ EasyKey"), _T("Remove")), T(_T("Huỷ"), _T("Cancel")))) {
			AppDelegate::getInstance()->onUninstall();
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
	case OPT_ESCAPE: return vRestoreOnEscape != 0;
	case OPT_WARN_CONFLICT: return vWarnOtherKeyboards != 0;
	case OPT_ZWJF: return vAllowConsonantZFWJ != 0;
	case OPT_QUICK_TELEX: return vQuickTelex != 0;
	case OPT_QUICK_START: return vQuickStartConsonant != 0;
	case OPT_QUICK_END: return vQuickEndConsonant != 0;
	case OPT_CAPS_FIRST: return vUpperCaseFirstChar != 0;
	case OPT_MACRO_ENGLISH: return vUseMacroInEnglishMode != 0;
	case OPT_MACRO_AUTO_CAPS: return vAutoCapsMacro != 0;
	case OPT_SMART_SWITCH: return vUseSmartSwitchKey != 0;
	case OPT_REMEMBER_CODE: return vRememberCode != 0;
	case OPT_CLIPBOARD: return vSendKeyStepByStep == 0;
	case OPT_FLYOUT: return vShowSwitchFlyout != 0;
	case OPT_INPUT_HOTKEY: return vSwitchInputType != 0;
	case OPT_BEEP: return HAS_BEEP(switchKey) != 0;
	case OPT_RUN_AS_ADMIN: return vRunAsAdmin != 0;
	case OPT_DESKTOP_SHORTCUT: return EasyKeyManager::hasDesktopShortcut();
	case OPT_CHECK_UPDATE: return vCheckNewVersion != 0;
	case OPT_AUTO_DARK: return vAccentAutoDark != 0;
	}
	return false;
}

bool SettingsWindow::isOptionEnabled(const int& id) const {
	switch (id) {
	case OPT_ZWJF:
		return vCheckSpelling != 0;
	case OPT_MACRO_ENGLISH:
	case OPT_MACRO_AUTO_CAPS:
		return vUseMacro != 0;
	}
	return true;
}

void SettingsWindow::setOption(const int& id, const bool& on) {
	int val = on ? 1 : 0;
	switch (id) {
	case OPT_LANGUAGE:
		APP_SET_DATA(vLanguage, val);
		AppDelegate::getInstance()->showSwitchFlyout();
		if (vUseSmartSwitchKey) {
			setAppInputMethodStatus(EasyKeyHelper::getFrontMostAppExecuteName(), vLanguage | (vCodeTable << 1));
			saveSmartSwitchKeyData();
		}
		break;
	case OPT_SPELLING:
		//smart typing: spell check and putting the keys back go together
		APP_SET_DATA(vCheckSpelling, val);
		APP_SET_DATA(vRestoreIfWrongSpelling, val);
		break;
	case OPT_MODERN: APP_SET_DATA(vUseModernOrthography, val); break;
	case OPT_MACRO: APP_SET_DATA(vUseMacro, val); break;
	case OPT_RUN_WITH_WINDOWS:
		APP_SET_DATA(vRunWithWindows, val);
		EasyKeyHelper::registerRunOnStartup(vRunWithWindows);
		break;
	case OPT_ESCAPE: APP_SET_DATA(vRestoreOnEscape, val); break;
	case OPT_WARN_CONFLICT: APP_SET_DATA(vWarnOtherKeyboards, val); break;
	case OPT_ZWJF: APP_SET_DATA(vAllowConsonantZFWJ, val); break;
	case OPT_QUICK_TELEX: APP_SET_DATA(vQuickTelex, val); break;
	case OPT_QUICK_START: APP_SET_DATA(vQuickStartConsonant, val); break;
	case OPT_QUICK_END: APP_SET_DATA(vQuickEndConsonant, val); break;
	case OPT_CAPS_FIRST: APP_SET_DATA(vUpperCaseFirstChar, val); break;
	case OPT_MACRO_ENGLISH: APP_SET_DATA(vUseMacroInEnglishMode, val); break;
	case OPT_MACRO_AUTO_CAPS: APP_SET_DATA(vAutoCapsMacro, val); break;
	case OPT_SMART_SWITCH: APP_SET_DATA(vUseSmartSwitchKey, val); break;
	case OPT_REMEMBER_CODE: APP_SET_DATA(vRememberCode, val); break;
	case OPT_CLIPBOARD: APP_SET_DATA(vSendKeyStepByStep, on ? 0 : 1); break;
	case OPT_FLYOUT: APP_SET_DATA(vShowSwitchFlyout, val); break;
	case OPT_INPUT_HOTKEY:
		if (!on && capturing && captureTarget == 1)
			endCapture(false, 0);
		APP_SET_DATA(vSwitchInputType, val);
		if (on && vInputTypeHotKey == 0xFE0000FE)
			startCapture(1); //no keys yet, ask for them now
		break;
	case OPT_BEEP:
		vSwitchKeyStatus = on ? (vSwitchKeyStatus | 0x8000) : (vSwitchKeyStatus & ~0x8000);
		APP_SET_DATA(vSwitchKeyStatus, vSwitchKeyStatus);
		break;
	case OPT_RUN_AS_ADMIN:
		APP_SET_DATA(vRunAsAdmin, val);
		requestRestartAsAdmin();
		break;
	case OPT_DESKTOP_SHORTCUT:
		//the switch shows whether the shortcut is there, so off removes it
		if (on)
			EasyKeyManager::createDesktopShortcut();
		else
			EasyKeyManager::removeDesktopShortcut();
		break;
	case OPT_CHECK_UPDATE: APP_SET_DATA(vCheckNewVersion, val); break;
	case OPT_AUTO_DARK:
		if (!on)
			APP_SET_DATA(vAccentDark, (int)customDark()); //start from the shade in use
		APP_SET_DATA(vAccentAutoDark, val);
		applyTheme();
		break;
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
		AppendMenu(menu, MF_STRING | (table.code == vCodeTable ? MF_CHECKED : MF_UNCHECKED), table.code + 1, T(table.vi, table.en));
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
	AppDelegate::getInstance()->showCodeTableFlyout();
	InvalidateRect(hWnd, NULL, FALSE);
}

//the apps in game mode, checked; picking one takes it out. The app used before Settings can be added
void SettingsWindow::showGameApps(const Item& item) {
	vector<string> apps = getGameApps();
	string last = EasyKeyHelper::getFrontMostAppExecuteName(); //Settings keeps the app the user came from
	bool canAdd = EasyKeyHelper::isRealApp(last) && !isGameApp(last);
	HMENU menu = CreatePopupMenu();
	if (canAdd) {
		wstring add = T(_T("Bật cho "), _T("Turn on for ")) + utf8ToWideString(last);
		AppendMenu(menu, MF_STRING, 1, add.c_str());
		AppendMenu(menu, MF_SEPARATOR, 0, 0);
	}
	if (apps.empty()) {
		AppendMenu(menu, MF_STRING | MF_GRAYED, 0,
			T(_T("Chưa có game nào. Bật ở menu khay khi đang chơi"), _T("No games yet. Turn it on from the tray menu while playing")));
	} else {
		for (size_t n = 0; n < apps.size(); n++)
			AppendMenu(menu, MF_STRING | MF_CHECKED, 100 + n, utf8ToWideString(apps[n]).c_str());
	}

	RECT field = toView(item.ctl);
	MapWindowPoints(hWnd, NULL, (LPPOINT)&field, 2);
	TPMPARAMS params = { sizeof(params) };
	params.rcExclude = field;
	int command = (int)TrackPopupMenuEx(menu, TPM_RETURNCMD | TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_VERTICAL,
		field.right, field.bottom, hWnd, &params);
	DestroyMenu(menu);
	if (command == 1 && canAdd) {
		setGameApp(last, true);
		AppDelegate::getInstance()->showNotice(T(_T("Đã bật chế độ game"), _T("Game mode on")),
			utf8ToWideString(last) + T(_T(": bấm Enter để chat tiếng Việt"), _T(": press Enter to chat in Vietnamese")));
	} else if (command >= 100 && command - 100 < (int)apps.size()) {
		setGameApp(apps[command - 100], false);
		AppDelegate::getInstance()->showNotice(T(_T("Đã tắt chế độ game"), _T("Game mode off")), utf8ToWideString(apps[command - 100]));
	}
}

void SettingsWindow::requestRestartAsAdmin() {
	EasyKeyHelper::registerRunOnStartup(false);
	if (vRunAsAdmin && !IsUserAnAdmin()) {
		if (MessageWindow::ask(hWnd, T(_T("Khởi động lại EasyKey?"), _T("Restart EasyKey?")),
			T(_T("EasyKey cần khởi động lại để chạy với quyền Admin."), _T("EasyKey needs to restart to run as administrator.")),
			T(_T("Khởi động lại"), _T("Restart")), T(_T("Để sau"), _T("Later")))) {
			PostQuitMessage(0);
			ShellExecute(0, L"runas", EasyKeyHelper::getFullPath().c_str(), 0, 0, SW_SHOWNORMAL);
		}
	} else {
		EasyKeyHelper::registerRunOnStartup(vRunWithWindows);
	}
}

/*-------------------------------- custom accent --------------------------------*/

void SettingsWindow::startHexEdit(const int& field) {
	hexEditing = field;
	hexBuffer = hexString(field == 0 ? (COLORREF)vAccentLight : customDark());
	SetFocus(hWnd);
	InvalidateRect(hWnd, NULL, FALSE);
}

void SettingsWindow::endHexEdit() {
	if (hexEditing < 0)
		return;
	hexEditing = -1;
	hexBuffer.clear();
	if (hWnd)
		InvalidateRect(hWnd, NULL, FALSE);
}

//save the field once it holds a full color
void SettingsWindow::applyHex() {
	if (hexBuffer.size() != 6)
		return;
	int value = (int)wcstol(hexBuffer.c_str(), NULL, 16);
	COLORREF color = RGB((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);
	if (hexEditing == 0) {
		APP_SET_DATA(vAccentLight, (int)color);
	} else {
		APP_SET_DATA(vAccentDark, (int)color);
	}
	applyTheme();
	SystemTrayHelper::updateData();
}

bool SettingsWindow::onHexKey(const UINT& uMsg, const WPARAM& key) {
	if (uMsg == WM_CHAR) {
		if (key == 0x16) { //Ctrl+V: keep the hex digits of the clipboard
			wstring pasted = EasyKeyHelper::getClipboardText(CF_UNICODETEXT);
			hexBuffer.clear();
			for (wchar_t c : pasted) {
				if (iswxdigit(c) && hexBuffer.size() < 6)
					hexBuffer += (wchar_t)towupper(c);
			}
		} else if (iswxdigit((wint_t)key) && hexBuffer.size() < 6) {
			hexBuffer += (wchar_t)towupper((wint_t)key);
		} else if (key == VK_BACK && !hexBuffer.empty()) {
			hexBuffer.pop_back();
		} else {
			return true;
		}
		applyHex();
		InvalidateRect(hWnd, NULL, FALSE);
		return true;
	}
	if (uMsg == WM_KEYDOWN && (key == VK_RETURN || key == VK_ESCAPE || key == VK_TAB)) {
		endHexEdit();
		return key != VK_TAB;
	}
	return uMsg == WM_KEYDOWN && key != VK_TAB;
}

/*-------------------------------- switch key capture --------------------------------*/

void SettingsWindow::startCapture(const int& target) {
	if (capturing)
		return;
	endHexEdit();
	capturing = true;
	captureTarget = target;
	savedSwitchKey = vSwitchKeyStatus;
	savedInputKey = vInputTypeHotKey;
	captureModifiers = 0;
	//keep the keyboard hook from acting on the old hotkeys while the new one is typed
	vSwitchKeyStatus = (savedSwitchKey & 0x8000) | 0xFE0000FE;
	vInputTypeHotKey = 0xFE0000FE;
	SetFocus(hWnd);
	layout();
	InvalidateRect(hWnd, NULL, FALSE);
}

void SettingsWindow::endCapture(const bool& save, const int& hotKey) {
	if (!capturing)
		return;
	capturing = false;
	vSwitchKeyStatus = savedSwitchKey;
	vInputTypeHotKey = savedInputKey;
	if (save && captureTarget == 0) {
		vSwitchKeyStatus = (savedSwitchKey & 0x8000) | hotKey;
		APP_SET_DATA(vSwitchKeyStatus, vSwitchKeyStatus);
		SystemTrayHelper::updateData();
	} else if (save) {
		APP_SET_DATA(vInputTypeHotKey, hotKey);
	} else if (captureTarget == 1 && vInputTypeHotKey == 0xFE0000FE) {
		//turned on but no keys given: off again
		APP_SET_DATA(vSwitchInputType, 0);
	}
	if (hWnd) {
		layout();
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

//the other hotkey already uses these keys
bool SettingsWindow::isHotKeyTaken(const int& hotKey) const {
	int other = captureTarget == 0 ? (vSwitchInputType ? savedInputKey : 0xFE0000FE) : (savedSwitchKey & ~0x8000);
	return other != 0xFE0000FE && (other & ~0x8000) == hotKey;
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
			int hotKey = mods | (int)vk | ((int)vk << 24);
			if (isHotKeyTaken(hotKey))
				MessageBeep(MB_OK);
			else
				endCapture(true, hotKey);
		} else {
			MessageBeep(MB_OK); //a hotkey needs at least one modifier and a typing key
		}
		return true;
	}
	//modifiers only, like Ctrl + Shift: saved when the first of them is released
	if (mask && (captureModifiers & mask)) {
		int hotKey = captureModifiers | 0xFE | (0xFE << 24);
		if (countModifiers(captureModifiers) >= 2 && !isHotKeyTaken(hotKey)) {
			endCapture(true, hotKey);
		} else {
			if (isHotKeyTaken(hotKey))
				MessageBeep(MB_OK);
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
			bool onField = hotItem >= 0 && hotItem < (int)items.size() && items[hotItem].kind == K_HEXFIELD;
			SetCursor(LoadCursor(NULL, onLink ? IDC_HAND : onField ? IDC_IBEAM : IDC_ARROW));
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
		if (hexEditing >= 0 && (item < 0 || items[item].kind != K_HEXFIELD || items[item].id != hexEditing))
			endHexEdit();
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
		if (hexEditing >= 0 && onHexKey(uMsg, wParam))
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
		if (hexEditing >= 0 && uMsg == WM_CHAR) {
			onHexKey(uMsg, wParam);
			return 0;
		}
		break;
	case WM_SETFOCUS:
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_KILLFOCUS:
		endCapture(false, 0);
		endHexEdit();
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_CLOSE:
		//deletes this window, do not touch members afterwards
		AppDelegate::getInstance()->closeSettingsWindow();
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
