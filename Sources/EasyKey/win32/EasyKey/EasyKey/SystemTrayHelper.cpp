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
#include "SystemTrayHelper.h"
#include "AppDelegate.h"
#include "ThemeHelper.h"

#define WM_TRAYMESSAGE (WM_USER + 1)
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

#define POPUP_CONVERT_TOOL 980
#define POPUP_CONTROL_PANEL 1000
#define POPUP_EASYKEY_EXIT 2000

#define MODIFY_MENU(MENU, COMMAND, DATA) ModifyMenu(MENU, COMMAND, \
											MF_BYCOMMAND | (DATA ? MF_CHECKED : MF_UNCHECKED), \
											COMMAND, \
											menuData[COMMAND]);

static HMENU popupMenu;
static HMENU menuInputType;
static HMENU menuCodeTable;

static NOTIFYICONDATA nid;

map<UINT, LPCTSTR> menuData = {
	{POPUP_VIET_ON_OFF, _T("Gõ tiếng Việt")},
	{POPUP_TELEX, _T("Telex")},
	{POPUP_VNI, _T("VNI")},
	{POPUP_SIMPLE_TELEX, _T("Simple Telex")},
	{POPUP_UNICODE, _T("Unicode")},
	{POPUP_TCVN3, _T("TCVN3 (ABC)")},
	{POPUP_VNI_WINDOWS, _T("VNI Windows")},
	{POPUP_UNICODE_COMPOUND, _T("Unicode tổ hợp")},
	{POPUP_VN_LOCALE_1258, _T("CP 1258")},
	{POPUP_CONVERT_TOOL, _T("Công cụ chuyển mã")},
	{POPUP_CONTROL_PANEL, _T("Cài đặt…")},
	{POPUP_EASYKEY_EXIT, _T("Thoát EasyKey")},
};

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
	popupMenu = CreatePopupMenu();
	AppendMenu(popupMenu, MF_CHECKED, POPUP_VIET_ON_OFF, menuData[POPUP_VIET_ON_OFF]);
	AppendMenu(popupMenu, MF_SEPARATOR, 0, 0);

	menuInputType = CreatePopupMenu();
	AppendMenu(menuInputType, MF_UNCHECKED, POPUP_TELEX, menuData[POPUP_TELEX]);
	AppendMenu(menuInputType, MF_UNCHECKED, POPUP_VNI, menuData[POPUP_VNI]);
	AppendMenu(menuInputType, MF_UNCHECKED, POPUP_SIMPLE_TELEX, menuData[POPUP_SIMPLE_TELEX]);
	AppendMenu(popupMenu, MF_POPUP, (UINT_PTR)menuInputType, _T("Kiểu gõ"));

	menuCodeTable = CreatePopupMenu();
	//same order as the settings window, CP 1258 is added by updateData only while it is in use
	AppendMenu(menuCodeTable, MF_UNCHECKED, POPUP_UNICODE, menuData[POPUP_UNICODE]);
	AppendMenu(menuCodeTable, MF_UNCHECKED, POPUP_UNICODE_COMPOUND, menuData[POPUP_UNICODE_COMPOUND]);
	AppendMenu(menuCodeTable, MF_UNCHECKED, POPUP_TCVN3, menuData[POPUP_TCVN3]);
	AppendMenu(menuCodeTable, MF_UNCHECKED, POPUP_VNI_WINDOWS, menuData[POPUP_VNI_WINDOWS]);
	AppendMenu(popupMenu, MF_POPUP, (UINT_PTR)menuCodeTable, _T("Bảng mã"));

	AppendMenu(popupMenu, MF_UNCHECKED, POPUP_CONVERT_TOOL, menuData[POPUP_CONVERT_TOOL]);
	AppendMenu(popupMenu, MF_SEPARATOR, 0, 0);
	AppendMenu(popupMenu, MF_STRING, POPUP_CONTROL_PANEL, menuData[POPUP_CONTROL_PANEL]);
	AppendMenu(popupMenu, MF_UNCHECKED, POPUP_EASYKEY_EXIT, menuData[POPUP_EASYKEY_EXIT]);

	SetMenuDefaultItem(popupMenu, POPUP_CONTROL_PANEL, false);
}

static void loadTrayIcon() {
	int icon = 0;
	//tray icon is a monochrome glyph, white on a dark taskbar and black on a light one
	bool darkTaskbar = ThemeHelper::isTaskbarDarkMode();
	if (vLanguage) {
		icon = darkTaskbar ? IDI_ICON_STATUS_VIET_DARK : IDI_ICON_STATUS_VIET_LIGHT;
		LoadString(GetModuleHandle(0), IDS_TRAY_TITLE_2, nid.szTip, 128);
	}
	else {
		icon = darkTaskbar ? IDI_ICON_STATUS_ENG_DARK : IDI_ICON_STATUS_ENG_LIGHT;
		LoadString(GetModuleHandle(0), IDS_TRAY_TITLE, nid.szTip, 128);
	}
	//load the small icon size directly so the tray does not downscale the 32px one
	nid.hIcon = (HICON)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(icon), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
}

void SystemTrayHelper::updateData() {
	loadTrayIcon();
	Shell_NotifyIcon(NIM_MODIFY, &nid);

	//show the switch key next to the Vietnamese item, like a menu accelerator
	wstring vietnamese = menuData[POPUP_VIET_ON_OFF];
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
		AppendMenu(menuCodeTable, MF_CHECKED, POPUP_VN_LOCALE_1258, menuData[POPUP_VN_LOCALE_1258]);
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