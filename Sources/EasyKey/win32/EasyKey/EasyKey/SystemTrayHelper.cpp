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
#include "SystemTrayHelper.h"
#include "AppDelegate.h"
#include "ThemeHelper.h"
#include "Updater.h"
#include "TrayGlyph.h"
#include <cmath>

#define WM_TRAYMESSAGE (WM_USER + 1)
#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED 0x0320
#endif
#define TRAY_ICONUID 100
#define TIMER_TRAY_CLICK 1

#define POPUP_VIET_ON_OFF 900
#define POPUP_MIXED 901

#define POPUP_TELEX 910
#define POPUP_VNI 911
#define POPUP_SIMPLE_TELEX 912

#define POPUP_UNICODE 930
#define POPUP_TCVN3 931
#define POPUP_VNI_WINDOWS 932
#define POPUP_UNICODE_COMPOUND 933
#define POPUP_VN_LOCALE_1258 934

#define POPUP_GAME_MODE 960
#define POPUP_MACRO_TABLE 970
#define POPUP_CONVERT_TOOL 980
#define POPUP_CHECK_UPDATE 990
#define POPUP_CONTROL_PANEL 1000
#define POPUP_EASYKEY_EXIT 2000

#define MODIFY_MENU(MENU, COMMAND, DATA) ModifyMenu(MENU, COMMAND, \
											MF_BYCOMMAND | (DATA ? MF_CHECKED : MF_UNCHECKED), \
											COMMAND, \
											menuText(COMMAND));

static HMENU popupMenu;
static HMENU menuInputType;
static HMENU menuCodeTable;

static NOTIFYICONDATA nid;

static LPCTSTR menuText(const UINT& command) {
	switch (command) {
	case POPUP_VIET_ON_OFF: return TR("Gõ tiếng Việt", "Type Vietnamese");
	case POPUP_MIXED: return TR("Gõ Anh–Việt", "Type English and Vietnamese");
	case POPUP_TELEX: return _T("Telex");
	case POPUP_VNI: return _T("VNI");
	case POPUP_SIMPLE_TELEX: return _T("Simple Telex");
	case POPUP_UNICODE: return _T("Unicode");
	case POPUP_TCVN3: return _T("TCVN3 (ABC)");
	case POPUP_VNI_WINDOWS: return _T("VNI Windows");
	case POPUP_UNICODE_COMPOUND: return TR("Unicode tổ hợp", "Unicode combining");
	case POPUP_VN_LOCALE_1258: return _T("CP 1258");
	case POPUP_MACRO_TABLE: return TR("Bảng gõ tắt…", "Shortcut list…");
	case POPUP_CONVERT_TOOL: return TR("Công cụ chuyển mã…", "Convert text…");
	case POPUP_CHECK_UPDATE: return TR("Kiểm tra cập nhật…", "Check for updates…");
	case POPUP_CONTROL_PANEL: return TR("Cài đặt…", "Settings…");
	case POPUP_EASYKEY_EXIT: return TR("Thoát EasyKey", "Quit EasyKey");
	}
	return _T("");
}

static void updateGameItem(const string& exe);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static UINT taskbarCreated;

	switch (message) {
	case WM_CREATE:
		taskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));
		break;
	case WM_SETTINGCHANGE:
		//taskbar switched between light and dark
		if (ThemeHelper::isThemeChangedMessage(message, lParam)) {
			ThemeHelper::applyMenuTheme();
			SystemTrayHelper::updateData();
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_APP + 7:
		DestroyIcon((HICON)lParam); //an icon the tray no longer shows
		break;
	case WM_DPICHANGED:
	case WM_DISPLAYCHANGE:
		//the screen scale changed: draw the icon again at the new size
		SystemTrayHelper::updateData();
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_DWMCOLORIZATIONCOLORCHANGED:
		//the Windows accent changed
		SystemTrayHelper::updateData();
		break;
	case WM_USER+2019:
		AppDelegate::getInstance()->onControlPanel();
		break;
	case WM_TIMER:
		//a single click: no second one came, switch Vietnamese/English
		if (wParam == TIMER_TRAY_CLICK) {
			KillTimer(hWnd, TIMER_TRAY_CLICK);
			AppDelegate::getInstance()->onToggleVietnamese();
			SystemTrayHelper::updateData();
		}
		break;
	case WM_TRAYMESSAGE: {
		//a double click opens Settings without switching twice on the way
		static bool doubleClicked = false;
		if (lParam == WM_LBUTTONDBLCLK) {
			KillTimer(hWnd, TIMER_TRAY_CLICK);
			doubleClicked = true;
			AppDelegate::getInstance()->onControlPanel();
		}
		if (lParam == WM_LBUTTONUP) {
			if (doubleClicked)
				doubleClicked = false;
			else
				SetTimer(hWnd, TIMER_TRAY_CLICK, GetDoubleClickTime(), NULL);
		} else if (lParam == WM_RBUTTONDOWN) {
			POINT curPoint;
			GetCursorPos(&curPoint);
			string gameApp = EasyKeyHelper::getFrontMostAppExecuteName(); //the app the user came from
			updateGameItem(gameApp);
			SetForegroundWindow(hWnd);
			UINT commandId = TrackPopupMenu(
				popupMenu,
				TPM_RETURNCMD | TPM_NONOTIFY,
				curPoint.x,
				curPoint.y,
				0,
				hWnd,
				NULL
			);
			switch (commandId) {
			case POPUP_VIET_ON_OFF: //picking the checked mode goes back to English
				AppDelegate::getInstance()->setTypingMode(TYPING_MODE == 1 ? 0 : 1);
				break;
			case POPUP_MIXED:
				AppDelegate::getInstance()->setTypingMode(TYPING_MODE == 2 ? 0 : 2);
				break;
			case POPUP_GAME_MODE: {
				bool on = !isGameApp(gameApp);
				setGameApp(gameApp, on);
				wstring app = utf8ToWideString(gameApp);
				if (on)
					AppDelegate::getInstance()->showNotice(TR("Đã bật chế độ game", "Game mode on"),
						app + TR(": bấm Enter để chat tiếng Việt", ": press Enter to chat in Vietnamese"));
				else
					AppDelegate::getInstance()->showNotice(TR("Đã tắt chế độ game", "Game mode off"), app);
				break;
			}
			case POPUP_MACRO_TABLE:
				AppDelegate::getInstance()->onMacroTable();
				break;
			case POPUP_CONVERT_TOOL:
				AppDelegate::getInstance()->onConvertTool();
				break;
			case POPUP_TELEX:
				AppDelegate::getInstance()->onInputType(0);
				break;
			case POPUP_VNI:
				AppDelegate::getInstance()->onInputType(1);
				break;
			case POPUP_SIMPLE_TELEX:
				AppDelegate::getInstance()->onInputType(2);
				break;
			case POPUP_UNICODE:
				AppDelegate::getInstance()->onTableCode(0);
				break;
			case POPUP_TCVN3:
				AppDelegate::getInstance()->onTableCode(1);
				break;
			case POPUP_VNI_WINDOWS:
				AppDelegate::getInstance()->onTableCode(2);
				break;
			case POPUP_UNICODE_COMPOUND:
				AppDelegate::getInstance()->onTableCode(3);
				break;
			case POPUP_VN_LOCALE_1258:
				AppDelegate::getInstance()->onTableCode(4);
				break;
			case POPUP_CHECK_UPDATE:
				Updater::checkAndOffer(NULL, false);
				break;
			case POPUP_CONTROL_PANEL:
				AppDelegate::getInstance()->onControlPanel();
				break;
			case POPUP_EASYKEY_EXIT:
				AppDelegate::getInstance()->onEasyKeyExit();
				break;
			}
			SystemTrayHelper::updateData();
		}
	}
	break;
	default:
		// if the taskbar is restarted, add the system tray icon again
		if (message == taskbarCreated) {
			Shell_NotifyIcon(NIM_ADD, &nid);
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

HWND SystemTrayHelper::createFakeWindow(const HINSTANCE & hIns) {
	//create fake window
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hIns;
	wcex.hIcon = LoadIcon(hIns, MAKEINTRESOURCE(IDI_APP_ICON));
	wcex.hCursor = NULL;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = APP_CLASS;
	wcex.hIconSm = NULL;
	ATOM atom = RegisterClassExW(&wcex);
	HWND hWnd = CreateWindowW(APP_CLASS, _T(""), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hIns, nullptr);
	if (!hWnd) {
		return NULL;
	}
	ShowWindow(hWnd, 0);
	UpdateWindow(hWnd);
	return hWnd;
}

void SystemTrayHelper::createPopupMenu() {
	if (popupMenu)
		DestroyMenu(popupMenu); //submenus go with it
	popupMenu = CreatePopupMenu();
	AppendMenu(popupMenu, MF_CHECKED, POPUP_VIET_ON_OFF, menuText(POPUP_VIET_ON_OFF));
	AppendMenu(popupMenu, MF_UNCHECKED, POPUP_MIXED, menuText(POPUP_MIXED));
	AppendMenu(popupMenu, MF_SEPARATOR, 0, 0);

	menuInputType = CreatePopupMenu();
	AppendMenu(menuInputType, MF_UNCHECKED, POPUP_TELEX, menuText(POPUP_TELEX));
	AppendMenu(menuInputType, MF_UNCHECKED, POPUP_VNI, menuText(POPUP_VNI));
	AppendMenu(menuInputType, MF_UNCHECKED, POPUP_SIMPLE_TELEX, menuText(POPUP_SIMPLE_TELEX));
	AppendMenu(popupMenu, MF_POPUP, (UINT_PTR)menuInputType, TR("Kiểu gõ", "Input method"));

	menuCodeTable = CreatePopupMenu();
	//same order as the settings window, CP 1258 is added by updateData only while it is in use
	AppendMenu(menuCodeTable, MF_UNCHECKED, POPUP_UNICODE, menuText(POPUP_UNICODE));
	AppendMenu(menuCodeTable, MF_UNCHECKED, POPUP_UNICODE_COMPOUND, menuText(POPUP_UNICODE_COMPOUND));
	AppendMenu(menuCodeTable, MF_UNCHECKED, POPUP_TCVN3, menuText(POPUP_TCVN3));
	AppendMenu(menuCodeTable, MF_UNCHECKED, POPUP_VNI_WINDOWS, menuText(POPUP_VNI_WINDOWS));
	AppendMenu(popupMenu, MF_POPUP, (UINT_PTR)menuCodeTable, TR("Bảng mã", "Character encoding"));
	AppendMenu(popupMenu, MF_STRING, POPUP_GAME_MODE, TR("Chế độ game", "Game mode")); //named after the app by updateGameItem

	//text only, checkmarks show what is in use
	AppendMenu(popupMenu, MF_STRING, POPUP_MACRO_TABLE, menuText(POPUP_MACRO_TABLE));
	AppendMenu(popupMenu, MF_STRING, POPUP_CONVERT_TOOL, menuText(POPUP_CONVERT_TOOL));
	AppendMenu(popupMenu, MF_STRING, POPUP_CONTROL_PANEL, menuText(POPUP_CONTROL_PANEL));
	AppendMenu(popupMenu, MF_SEPARATOR, 0, 0);
	AppendMenu(popupMenu, MF_STRING, POPUP_CHECK_UPDATE, menuText(POPUP_CHECK_UPDATE));
	AppendMenu(popupMenu, MF_STRING, POPUP_EASYKEY_EXIT, menuText(POPUP_EASYKEY_EXIT));

	SetMenuDefaultItem(popupMenu, POPUP_CONTROL_PANEL, false);
}

typedef UINT(WINAPI *GetDpiForWindowFunc)(HWND);
typedef int(WINAPI *GetSystemMetricsForDpiFunc)(int, UINT);

//the size the taskbar shows tray icons at, for the DPI of the screen it is on now
static int trayIconSize() {
	//both only exist on Windows 10 1607 and later
	static GetDpiForWindowFunc getDpiForWindow =
		(GetDpiForWindowFunc)GetProcAddress(GetModuleHandle(_T("user32.dll")), "GetDpiForWindow");
	static GetSystemMetricsForDpiFunc getSystemMetricsForDpi =
		(GetSystemMetricsForDpiFunc)GetProcAddress(GetModuleHandle(_T("user32.dll")), "GetSystemMetricsForDpi");
	HWND taskbar = FindWindow(_T("Shell_TrayWnd"), NULL);
	UINT dpi = getDpiForWindow && taskbar ? getDpiForWindow(taskbar) : 0;
	if (dpi > 0 && getSystemMetricsForDpi)
		return getSystemMetricsForDpi(SM_CXSMICON, dpi);
	return GetSystemMetrics(SM_CXSMICON);
}

//the tray glyph drawn for this exact size, a solid tile with the letter in its own color; the caller destroys it
static HICON createTrayIcon(const int& size, const bool& vietnamese, const COLORREF& tileColor, const COLORREF& letterColor) {
	vector<BYTE> tile, letter;
	drawTrayGlyph(size, vietnamese, tile, letter);
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = size;
	bmi.bmiHeader.biHeight = -size;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	vector<DWORD> pixels(size * size);
	for (int i = 0; i < size * size; i++) {
		//icons take straight, not premultiplied, alpha
		int k = letter[i];
		auto mix = [&](const int& from, const int& to) { return (DWORD)((from * (255 - k) + to * k + 127) / 255); };
		pixels[i] = ((DWORD)tile[i] << 24) | (mix(GetRValue(tileColor), GetRValue(letterColor)) << 16) |
			(mix(GetGValue(tileColor), GetGValue(letterColor)) << 8) | mix(GetBValue(tileColor), GetBValue(letterColor));
	}
	HDC dc = GetDC(NULL);
	ICONINFO info = {};
	info.fIcon = TRUE;
	info.hbmColor = CreateDIBitmap(dc, &bmi.bmiHeader, CBM_INIT, pixels.data(), &bmi, DIB_RGB_COLORS);
	info.hbmMask = CreateBitmap(size, size, 1, 1, NULL);
	HICON icon = info.hbmColor && info.hbmMask ? CreateIconIndirect(&info) : NULL;
	if (info.hbmColor) DeleteObject(info.hbmColor);
	if (info.hbmMask) DeleteObject(info.hbmMask);
	ReleaseDC(NULL, dc);
	return icon;
}

//the green "update available" arrow, in a unit square: a head on top of a stem
static bool insideUpArrow(const float& x, const float& y) {
	if (y >= 0.08f && y <= 0.56f) //head, widest at its base
		return fabsf(x - 0.5f) <= (y - 0.08f) / 0.48f * 0.46f;
	return y > 0.56f && y <= 0.96f && x >= 0.32f && x <= 0.68f; //stem
}

//how much of a pixel the arrow covers, 0..1, sampled 4x4; grow widens the shape (for the halo around it)
static float upArrowCoverage(const int& px, const int& py, const int& size, const float& grow = 0) {
	int hits = 0;
	for (int sy = 0; sy < 4; sy++) {
		for (int sx = 0; sx < 4; sx++) {
			float x = (px + (sx + 0.5f) / 4) / size, y = (py + (sy + 0.5f) / 4) / size;
			bool in = insideUpArrow(x, y);
			for (int d = 0; !in && grow > 0 && d < 8; d++) {
				float a = d * 3.14159265f / 4;
				in = insideUpArrow(x + grow * cosf(a), y + grow * sinf(a));
			}
			hits += in ? 1 : 0;
		}
	}
	return hits / 16.0f;
}

static COLORREF updateGreen() {
	return ThemeHelper::isTaskbarDarkMode() ? RGB(0x4A, 0xDE, 0x80) : RGB(0x16, 0xA3, 0x4A);
}

//source with a small green up arrow in its lower right corner; the caller destroys it
static HICON withUpdateArrow(const HICON& source) {
	ICONINFO info;
	if (!GetIconInfo(source, &info))
		return NULL;
	BITMAP bm = {};
	GetObject(info.hbmColor, sizeof(bm), &bm);
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = bm.bmWidth;
	bmi.bmiHeader.biHeight = -bm.bmHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	vector<DWORD> pixels(bm.bmWidth * bm.bmHeight);
	HDC dc = GetDC(NULL);
	HICON result = NULL;
	if (GetDIBits(dc, info.hbmColor, 0, bm.bmHeight, pixels.data(), &bmi, DIB_RGB_COLORS)) {
		COLORREF green = updateGreen();
		int size = bm.bmWidth * 5 / 8, ox = bm.bmWidth - size, oy = bm.bmHeight - size;
		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				DWORD& pixel = pixels[(oy + y) * bm.bmWidth + ox + x];
				//clear a thin halo so the arrow stands apart from the glyph, then lay the arrow over it
				float halo = upArrowCoverage(x, y, size, 1.5f / size);
				float base = (pixel >> 24) / 255.0f * (1 - halo);
				float arrow = upArrowCoverage(x, y, size);
				float alpha = arrow + base * (1 - arrow);
				if (alpha <= 0) {
					pixel = 0;
					continue;
				}
				//icons take straight, not premultiplied, alpha
				auto mix = [&](const int& a, const int& b) { return (DWORD)((a * arrow + b * base * (1 - arrow)) / alpha + 0.5f); };
				pixel = ((DWORD)(alpha * 255 + 0.5f) << 24) | (mix(GetRValue(green), (pixel >> 16) & 0xFF) << 16) |
					(mix(GetGValue(green), (pixel >> 8) & 0xFF) << 8) | mix(GetBValue(green), pixel & 0xFF);
			}
		}
		ICONINFO badged = {};
		badged.fIcon = TRUE;
		badged.hbmColor = CreateDIBitmap(dc, &bmi.bmiHeader, CBM_INIT, pixels.data(), &bmi, DIB_RGB_COLORS);
		badged.hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);
		if (badged.hbmColor && badged.hbmMask)
			result = CreateIconIndirect(&badged);
		if (badged.hbmColor) DeleteObject(badged.hbmColor);
		if (badged.hbmMask) DeleteObject(badged.hbmMask);
	}
	ReleaseDC(NULL, dc);
	DeleteObject(info.hbmColor);
	DeleteObject(info.hbmMask);
	return result;
}

//the green arrow for the update item of the tray menu, premultiplied as menus want
static HBITMAP updateArrowBitmap() {
	int size = GetSystemMetrics(SM_CXMENUCHECK);
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = size;
	bmi.bmiHeader.biHeight = -size;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	void* bits = NULL;
	HBITMAP bitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (!bitmap)
		return NULL;
	//the menu follows the app theme, not the taskbar: the darker green reads on both
	COLORREF green = RGB(0x16, 0xA3, 0x4A);
	DWORD* pixels = (DWORD*)bits;
	int inset = size / 8, arrowSize = size - 2 * inset;
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			float a = (x < inset || y < inset || x >= inset + arrowSize || y >= inset + arrowSize) ? 0 :
				upArrowCoverage(x - inset, y - inset, arrowSize);
			pixels[y * size + x] = ((DWORD)(a * 255 + 0.5f) << 24) | ((DWORD)(GetRValue(green) * a) << 16) |
				((DWORD)(GetGValue(green) * a) << 8) | (DWORD)(GetBValue(green) * a);
		}
	}
	return bitmap;
}

static void loadTrayIcon() {
	static HICON glyph = NULL, badged = NULL;
	//a solid tile drawn for the size the taskbar wants, so no scaling blurs it:
	//V on the accent color in Vietnamese mode, E in white on a dark taskbar and in black on a light one
	bool darkTaskbar = ThemeHelper::isTaskbarDarkMode();
	COLORREF tileColor, letterColor;
	if (vLanguage) {
		//the accent of light mode is the deep one, white text on it reads on either taskbar
		tileColor = ThemeHelper::getAccentColor(false, false);
		letterColor = RGB(0xFF, 0xFF, 0xFF);
		lstrcpyn(nid.szTip, vMixedEnglish ? TR("EasyKey: đang gõ Anh–Việt", "EasyKey: typing English and Vietnamese") :
			TR("EasyKey: đang gõ tiếng Việt", "EasyKey: typing Vietnamese"), 128);
	}
	else {
		tileColor = darkTaskbar ? RGB(0xFF, 0xFF, 0xFF) : RGB(0x1C, 0x1C, 0x1C);
		letterColor = darkTaskbar ? RGB(0x1C, 0x1C, 0x1C) : RGB(0xFF, 0xFF, 0xFF);
		lstrcpyn(nid.szTip, TR("EasyKey: đang gõ tiếng Anh", "EasyKey: typing English"), 128);
	}
	HICON previous = glyph, previousBadged = badged;
	glyph = createTrayIcon(trayIconSize(), vLanguage != 0, tileColor, letterColor);
	nid.hIcon = glyph;
	//a newer version is out: a small green arrow on the icon
	const string& update = Updater::availableVersion();
	badged = update.empty() ? NULL : withUpdateArrow(nid.hIcon);
	if (badged) {
		nid.hIcon = badged;
		wstring tip = wstring(nid.szTip) + TR(" · có bản ", " · version ") + utf8ToWideString(update) + TR("", " is out");
		lstrcpyn(nid.szTip, tip.c_str(), 128);
	}
	//the tray copies the icon, so the old one can go once the new one is set
	if (previous)
		PostMessage(nid.hWnd, WM_APP + 7, 0, (LPARAM)previous);
	if (previousBadged)
		PostMessage(nid.hWnd, WM_APP + 7, 0, (LPARAM)previousBadged);
}

//"Game mode for <app>", checked when the app is in game mode; grayed when there is no app to name
static void updateGameItem(const string& exe) {
	bool known = EasyKeyHelper::isRealApp(exe);
	wstring text = TR("Chế độ game", "Game mode");
	if (known)
		text += TR(" cho ", " for ") + utf8ToWideString(exe);
	ModifyMenu(popupMenu, POPUP_GAME_MODE, MF_BYCOMMAND | MF_STRING |
		(known && isGameApp(exe) ? MF_CHECKED : MF_UNCHECKED) | (known ? MF_ENABLED : MF_GRAYED),
		POPUP_GAME_MODE, text.c_str());
}

void SystemTrayHelper::rebuildMenu() {
	createPopupMenu();
	updateData();
}

void SystemTrayHelper::updateData() {
	loadTrayIcon();
	Shell_NotifyIcon(NIM_MODIFY, &nid);

	//show the switch key next to the Vietnamese mode it goes back to, like a menu accelerator
	wstring hotKey = EasyKeyHelper::getHotKeyString(vSwitchKeyStatus);
	for (UINT command : { POPUP_VIET_ON_OFF, POPUP_MIXED }) {
		wstring text = menuText(command);
		if (!hotKey.empty() && (command == POPUP_MIXED) == (vMixedEnglish != 0))
			text += _T("\t") + hotKey;
		ModifyMenu(popupMenu, command, MF_BYCOMMAND | (TYPING_MODE == (command == POPUP_MIXED ? 2 : 1) ? MF_CHECKED : MF_UNCHECKED),
			command, text.c_str());
	}

	MODIFY_MENU(menuInputType, POPUP_TELEX, vInputType == 0);
	MODIFY_MENU(menuInputType, POPUP_VNI, vInputType == 1);
	MODIFY_MENU(menuInputType, POPUP_SIMPLE_TELEX, vInputType == 2);
	MODIFY_MENU(menuCodeTable, POPUP_UNICODE, vCodeTable == 0);
	MODIFY_MENU(menuCodeTable, POPUP_TCVN3, vCodeTable == 1);
	MODIFY_MENU(menuCodeTable, POPUP_VNI_WINDOWS, vCodeTable == 2);
	MODIFY_MENU(menuCodeTable, POPUP_UNICODE_COMPOUND, vCodeTable == 3);
	//"Update to 1.0.6" with a green arrow once a newer version is out
	static HBITMAP arrow = NULL;
	const string& update = Updater::availableVersion();
	wstring updateText = update.empty() ? wstring(menuText(POPUP_CHECK_UPDATE)) :
		TR("Cập nhật lên ", "Update to ") + utf8ToWideString(update);
	if (!update.empty() && !arrow)
		arrow = updateArrowBitmap();
	MENUITEMINFO updateItem = {};
	updateItem.cbSize = sizeof(updateItem);
	updateItem.fMask = MIIM_STRING | MIIM_BITMAP;
	updateItem.dwTypeData = (LPTSTR)updateText.c_str();
	updateItem.hbmpItem = update.empty() ? NULL : arrow;
	SetMenuItemInfo(popupMenu, POPUP_CHECK_UPDATE, FALSE, &updateItem);
	DeleteMenu(menuCodeTable, POPUP_VN_LOCALE_1258, MF_BYCOMMAND);
	if (vCodeTable == 4)
		AppendMenu(menuCodeTable, MF_CHECKED, POPUP_VN_LOCALE_1258, menuText(POPUP_VN_LOCALE_1258));
}

static HINSTANCE ins;
static int recreateCount = 0;

void SystemTrayHelper::_createSystemTrayIcon(const HINSTANCE& hIns) {
	HWND hWnd = createFakeWindow(ins);
	
	if (hWnd == NULL) { //Use timer to create
		if (recreateCount >= 5) {
			PostQuitMessage(0);
			return;
		}
		ins = hIns;
		SetTimer(NULL, 0, 1000 * 3, (TIMERPROC)&WaitToCreateFakeWindow);
		++recreateCount;
		return;
	}
	createPopupMenu();

	//create system tray
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = TRAY_ICONUID;
	nid.uVersion = NOTIFYICON_VERSION;
	nid.uCallbackMessage = WM_TRAYMESSAGE;
	loadTrayIcon();
	LoadString(ins, IDS_APP_TITLE, nid.szTip, 128);
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

	// Shell_NotifyIcon may fail if the system tray icon is not fully initialized
	const int maxRetries = 5;
	for (int attempt = 0; attempt < maxRetries; ++attempt) {
		if (Shell_NotifyIcon(NIM_ADD, &nid)) {
			break;
		}
		Sleep(1000);
	}
}


void CALLBACK SystemTrayHelper::WaitToCreateFakeWindow(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime) {
	_createSystemTrayIcon(ins);
	KillTimer(0, timerId);
}

void SystemTrayHelper::createSystemTrayIcon(const HINSTANCE& hIns) {
	_createSystemTrayIcon(hIns);
}

void SystemTrayHelper::removeSystemTray() {
	Shell_NotifyIcon(NIM_DELETE, &nid);
}