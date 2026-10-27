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

#define WM_TRAYMESSAGE (WM_USER + 1)
#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED 0x0320
#endif
#define TRAY_ICONUID 100

#define POPUP_VIET_ON_OFF 900

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
		DestroyIcon((HICON)lParam); //a tinted icon the tray no longer shows
		break;
	case WM_DWMCOLORIZATIONCOLORCHANGED:
		//the Windows accent changed
		SystemTrayHelper::updateData();
		break;
	case WM_USER+2019:
		AppDelegate::getInstance()->onControlPanel();
		break;
	case WM_TRAYMESSAGE: {
		if (lParam == WM_LBUTTONDBLCLK) {
			AppDelegate::getInstance()->onControlPanel();
		}
		if (lParam == WM_LBUTTONUP) {
			AppDelegate::getInstance()->onToggleVietnamese();
			SystemTrayHelper::updateData();
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
			case POPUP_VIET_ON_OFF:
				AppDelegate::getInstance()->onToggleVietnamese();
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

//the same shape as source, filled with color; the caller destroys it
static HICON tintIcon(const HICON& source, const COLORREF& color) {
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
	bool read = GetDIBits(dc, info.hbmColor, 0, bm.bmHeight, pixels.data(), &bmi, DIB_RGB_COLORS) != 0;
	HICON result = NULL;
	if (read) {
		//keep only the alpha, which is the glyph; icons take straight, not premultiplied, alpha
		DWORD rgb = (GetRValue(color) << 16) | (GetGValue(color) << 8) | GetBValue(color);
		for (DWORD& pixel : pixels)
			pixel = (pixel & 0xFF000000) | rgb;
		ICONINFO tinted = {};
		tinted.fIcon = TRUE;
		tinted.hbmColor = CreateDIBitmap(dc, &bmi.bmiHeader, CBM_INIT, pixels.data(), &bmi, DIB_RGB_COLORS);
		tinted.hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);
		if (tinted.hbmColor && tinted.hbmMask)
			result = CreateIconIndirect(&tinted);
		if (tinted.hbmColor) DeleteObject(tinted.hbmColor);
		if (tinted.hbmMask) DeleteObject(tinted.hbmMask);
	}
	ReleaseDC(NULL, dc);
	DeleteObject(info.hbmColor);
	DeleteObject(info.hbmMask);
	return result;
}

static void loadTrayIcon() {
	static HICON tinted = NULL;
	int icon = 0;
	//tray icon is a monochrome glyph, white on a dark taskbar and black on a light one;
	//in Vietnamese mode it takes the accent color so the mode shows at a glance
	bool darkTaskbar = ThemeHelper::isTaskbarDarkMode();
	if (vLanguage) {
		icon = darkTaskbar ? IDI_ICON_STATUS_VIET_DARK : IDI_ICON_STATUS_VIET_LIGHT;
		lstrcpyn(nid.szTip, TR("EasyKey: đang gõ tiếng Việt", "EasyKey: typing Vietnamese"), 128);
	}
	else {
		icon = darkTaskbar ? IDI_ICON_STATUS_ENG_DARK : IDI_ICON_STATUS_ENG_LIGHT;
		lstrcpyn(nid.szTip, TR("EasyKey: đang gõ tiếng Anh", "EasyKey: typing English"), 128);
	}
	//load the small icon size directly so the tray does not downscale the 32px one
	HICON glyph = (HICON)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(icon), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
	HICON previous = tinted;
	tinted = vLanguage ? tintIcon(glyph, ThemeHelper::getAccentColor(darkTaskbar, false)) : NULL;
	nid.hIcon = tinted ? tinted : glyph;
	//the tray copies the icon, so the old one can go once the new one is set
	if (previous)
		PostMessage(nid.hWnd, WM_APP + 7, 0, (LPARAM)previous);
}

//"Game mode for <app>", checked when the app is in game mode; grayed when there is no app to name
static void updateGameItem(const string& exe) {
	bool known = !exe.empty() && exe != "UnknownProgram";
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

	//show the switch key next to the Vietnamese item, like a menu accelerator
	wstring vietnamese = menuText(POPUP_VIET_ON_OFF);
	wstring hotKey = EasyKeyHelper::getHotKeyString(vSwitchKeyStatus);
	if (!hotKey.empty())
		vietnamese += _T("\t") + hotKey;
	ModifyMenu(popupMenu, POPUP_VIET_ON_OFF, MF_BYCOMMAND | (vLanguage ? MF_CHECKED : MF_UNCHECKED),
		POPUP_VIET_ON_OFF, vietnamese.c_str());

	MODIFY_MENU(menuInputType, POPUP_TELEX, vInputType == 0);
	MODIFY_MENU(menuInputType, POPUP_VNI, vInputType == 1);
	MODIFY_MENU(menuInputType, POPUP_SIMPLE_TELEX, vInputType == 2);
	MODIFY_MENU(menuCodeTable, POPUP_UNICODE, vCodeTable == 0);
	MODIFY_MENU(menuCodeTable, POPUP_TCVN3, vCodeTable == 1);
	MODIFY_MENU(menuCodeTable, POPUP_VNI_WINDOWS, vCodeTable == 2);
	MODIFY_MENU(menuCodeTable, POPUP_UNICODE_COMPOUND, vCodeTable == 3);
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