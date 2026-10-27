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
#include <algorithm>
#include "AppDelegate.h"
#include "ThemeHelper.h"
#include "Updater.h"
#include "MessageWindow.h"
#include "AboutWindow.h"
#include "ConvertToolWindow.h"
#include "MacroWindow.h"
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

//shown once after updating to this version; change it with each release
#define WHATS_NEW_VI L"• Sửa lỗi bảng mã VNI Windows, TCVN3, Unicode tổ hợp… tự quay về Unicode sau khi chọn trong Cài đặt hay menu khay.\n" \
	L"• Popup kiểu gõ cũng hiện khi đổi kiểu gõ từ menu khay hay trong Cài đặt.\n" \
	L"• Chạy với quyền Admin: tác vụ khởi động không còn bị Windows dừng sau 72 giờ và vẫn chạy khi dùng pin."
#define WHATS_NEW_EN L"• Fixed VNI Windows, TCVN3, Unicode combining… going back to Unicode after picking them in Settings or the tray menu.\n" \
	L"• The input method popup also shows when the input method is changed from the tray menu or Settings.\n" \
	L"• Run as administrator: the startup task is no longer stopped after 72 hours and also runs on battery."

static AppDelegate* _instance;

//see document in Engine.h
int vLanguage = 1;
int vInputType = 0;
int vFreeMark = 0;
int vCodeTable = 0;
int vCheckSpelling = 1;
int vUseModernOrthography = 1;
int vQuickTelex = 0;
#define DEFAULT_SWITCH_STATUS 0x5A00025A //default option + z
int vSwitchKeyStatus = DEFAULT_SWITCH_STATUS;
int vRestoreIfWrongSpelling = 1;
int vFixRecommendBrowser = 0;
int vUseMacro = 1;
int vUseMacroInEnglishMode = 1;
int vAutoCapsMacro = 0;
int vSendKeyStepByStep = 1;
int vUseSmartSwitchKey = 1;
int vUpperCaseFirstChar = 0;
int vAllowConsonantZFWJ = 0;
int vQuickStartConsonant = 0;
int vQuickEndConsonant = 0;
int vOtherLanguage = 1;
int vRememberCode = 1;
int vRestoreOnEscape = 1;
int vWarnOtherKeyboards = 1;
int vLastVersion = 0;

int vShowOnStartUp = 0;
int vShowSwitchFlyout = 1;
int vSwitchInputType = 0;
int vInputTypeHotKey = 0xFE0000FE; //no key set
int vUiLanguage = 0;
int vThemeMode = 0;
int vAccentColor = 0;
int vAccentLight = RGB(0x4F, 0x46, 0xE5);
int vAccentDark = RGB(0xA5, 0xB4, 0xFC);
int vAccentAutoDark = 1;
int vRunWithWindows = 1;

int vCreateDesktopShortcut = 0;
int vRunAsAdmin = 0;
int vCheckNewVersion = 0;
int vFixChromiumBrowser = 1;

AppDelegate::AppDelegate() {
	_instance = this;
}

AppDelegate * AppDelegate::getInstance() {
	return _instance;
}

int AppDelegate::run(HINSTANCE hInstance) {
	this->hInstance = hInstance;

	//after an update the previous EasyKey is still closing
	Updater::finishPreviousUpdate();

	//check app has already run or not
	HWND previousInstance = FindWindow(APP_CLASS, NULL);
	if (previousInstance) {
		MessageBeep(MB_OK);
		SendMessage(previousInstance, WM_USER + 2019, 0, 0);
		PostQuitMessage(0);
		return 0;
	}

	//init EasyKey Engine
	EasyKeyManager::initEngine();

	//dark or light popup menus, before the tray menu is built
	ThemeHelper::applyMenuTheme();

	//shown when the switch key is pressed
	flyout = new FlyoutWindow(hInstance);

	//put the keyboard hooks back if Windows drops them
	SetTimer(NULL, 0, 2000, [](HWND, UINT, UINT_PTR, DWORD) { checkHooks(); });

	//create system tray
	SystemTrayHelper::createSystemTrayIcon(hInstance);
	SystemTrayHelper::updateData();

	//create main control
	if (vShowOnStartUp)
		createMainDialog();
	MessageBeep(MB_OK);
	greet();

	//check update
	if (vCheckNewVersion)
		Updater::checkAndOffer(NULL, true);

	MSG msg;
	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))	{
		if (msg.message == WM_KEYDOWN) {
			EasyKeyManager::_lastKeyCode = (UINT16)msg.wParam;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

void AppDelegate::createMainDialog() {
	if (settingsWindow == NULL) {
		settingsWindow = new SettingsWindow(hInstance);
		settingsWindow->show();
	} else {
		settingsWindow->bringOnTop();
	}
}

void AppDelegate::closeWindow(FluentWindow* window) {
	window->destroy();
	if (aboutWindow == window)
		aboutWindow = NULL;
	else if (macroWindow == window)
		macroWindow = NULL;
	else if (convertWindow == window)
		convertWindow = NULL;
	delete window;
}

void AppDelegate::closeSettingsWindow() {
	if (settingsWindow) {
		settingsWindow->destroy();
		delete settingsWindow;
		settingsWindow = NULL;
	}
}

void AppDelegate::onInputMethodChangedFromHotKey() {
	APP_SET_DATA(vLanguage, vLanguage);
	if (settingsWindow) {
		settingsWindow->fillData();
	}
	SystemTrayHelper::updateData();
}

void AppDelegate::onInputTypeFromHotKey() {
	onInputType(vInputType);
}

void AppDelegate::showSwitchFlyout() {
	if (flyout)
		flyout->notify(FLYOUT_LANGUAGE);
}

void AppDelegate::showInputTypeFlyout() {
	if (flyout)
		flyout->notify(FLYOUT_INPUT_TYPE);
}

void AppDelegate::showCodeTableFlyout() {
	if (flyout)
		flyout->notify(FLYOUT_CODE_TABLE);
}

void AppDelegate::onElevatedWindow(const HWND& hWnd) {
	//once per app, only while typing Vietnamese
	static vector<DWORD> warned;
	DWORD processId = 0;
	GetWindowThreadProcessId(hWnd, &processId);
	if (!vLanguage || !flyout || find(warned.begin(), warned.end(), processId) != warned.end())
		return;
	warned.push_back(processId);
	flyout->showMessage(TR("Ứng dụng này chạy với quyền Admin", "This app runs as administrator"),
		TR("EasyKey không gõ được ở đây. Bật \"Chạy với quyền Admin\" trong Nâng cao.",
			"EasyKey can't type here. Turn on \"Run as administrator\" in Advanced."));
}

void AppDelegate::greet() {
	DWORD version = EasyKeyHelper::getVersionNumber();
	bool updated = vLastVersion != 0 && (DWORD)vLastVersion != version;
	if ((DWORD)vLastVersion != version)
		APP_SET_DATA(vLastVersion, (int)version);
	if (EasyKeyHelper::isFirstRun()) {
		//no questions on the first run, the defaults work: just say how to switch
		flyout->showMessage(TR("EasyKey đang chạy", "EasyKey is running"),
			TR("Bấm ", "Press ") + EasyKeyHelper::getHotKeyString(vSwitchKeyStatus) + TR(" để chuyển Việt/Anh", " to switch Vietnamese/English"), false);
	} else if (updated) {
		showWhatsNew();
	}

	if (vWarnOtherKeyboards) {
		wstring other = EasyKeyHelper::findOtherKeyboard();
		if (!other.empty())
			warnOtherKeyboard(other);
	}
}

void AppDelegate::warnOtherKeyboard(const wstring& name) {
	//never closes the other keyboard, that is the user's call
	bool windowsKeyboard = name == TR("Bàn phím tiếng Việt của Windows", "The Windows Vietnamese keyboard");
	MessageWindow::Options options;
	options.heading = name + TR(" đang chạy", " is running");
	options.text = windowsKeyboard
		? TR("Hai bộ gõ cùng lúc dễ làm chữ bị bỏ dấu hai lần, ví dụ \"vieejt\". Bấm Windows + Space để chuyển về bàn phím tiếng Anh.",
			"Two Vietnamese keyboards at once can add marks twice, as in \"vieejt\". Press Windows + Space to go back to the English keyboard.")
		: TR("Hai bộ gõ chạy cùng lúc dễ làm chữ bị bỏ dấu hai lần, ví dụ \"vieejt\". Hãy tắt một trong hai.",
			"Two Vietnamese keyboards at once can add marks twice, as in \"vieejt\". Please quit one of them.");
	options.buttons.push_back(TR("OK", "OK"));
	options.check = TR("Không nhắc lại", "Don't remind me again");
	MessageWindow::show(NULL, options);
	if (options.checked) {
		APP_SET_DATA(vWarnOtherKeyboards, 0);
		onSettingsChanged();
	}
}

void AppDelegate::showWhatsNew() {
	wstring version = EasyKeyHelper::getVersionString();
	MessageWindow::Options options;
	options.heading = TR("Có gì mới trong EasyKey ", "What's new in EasyKey ") + version;
	options.text = EasyKeyHelper::isEnglishUi() ? WHATS_NEW_EN : WHATS_NEW_VI;
	options.link = TR("Xem đầy đủ các thay đổi", "See all changes");
	options.linkUrl = wstring(RELEASE_NOTES_URL) + version;
	options.buttons.push_back(TR("OK", "OK"));
	MessageWindow::show(NULL, options);
}

void AppDelegate::onAppearanceChanged() {
	SystemTrayHelper::rebuildMenu();
	onSettingsChanged();
}

void AppDelegate::onSettingsChanged() {
	if (settingsWindow)
		settingsWindow->fillData();
	FluentWindow* windows[] = { aboutWindow, macroWindow, convertWindow };
	for (FluentWindow* window : windows)
		if (window)
			window->fillData();
}

void AppDelegate::showNotice(const wstring& title, const wstring& detail) {
	if (flyout)
		flyout->showMessage(title, detail, false);
}

void AppDelegate::onUninstall() {
	EasyKeyManager::freeEngine();
	EasyKeyHelper::removeFromSystem();
	//one literal per language: _T() only widens the first piece of a concatenated string
	MessageWindow::notice(NULL, TR("Đã gỡ EasyKey khỏi Windows", "EasyKey was removed from Windows"),
		TR("EasyKey không còn chạy cùng Windows, lối tắt và cài đặt đã xoá. Thư mục chứa EasyKey.exe sẽ mở ra để bạn xoá file nếu muốn.",
			"It no longer starts with Windows, and its shortcut and settings are gone. The folder with EasyKey.exe opens next, so you can delete the file if you like."));
	wstring select = L"/select,\"" + EasyKeyHelper::getFullPath() + L"\"";
	ShellExecute(NULL, L"open", L"explorer.exe", select.c_str(), NULL, SW_SHOWNORMAL);
	SystemTrayHelper::removeSystemTray();
	PostQuitMessage(0);
}

void AppDelegate::onDefaultConfig() {
	APP_SET_DATA(vLanguage, 1);
	APP_SET_DATA(vInputType, 0);
	vFreeMark = 0;
	APP_SET_DATA(vCodeTable, 0);
	APP_SET_DATA(vCheckSpelling, 1);
	APP_SET_DATA(vUseModernOrthography, 0);
	APP_SET_DATA(vQuickTelex, 0);
	APP_SET_DATA(vSwitchKeyStatus, DEFAULT_SWITCH_STATUS);
	APP_SET_DATA(vRestoreIfWrongSpelling, 1);
	APP_SET_DATA(vFixRecommendBrowser, 1);
	APP_SET_DATA(vUseMacro, 1);
	APP_SET_DATA(vUseMacroInEnglishMode, 0);
	APP_SET_DATA(vSendKeyStepByStep, 1);
	APP_SET_DATA(vUseSmartSwitchKey, 1);
	APP_SET_DATA(vUpperCaseFirstChar, 0);
	APP_SET_DATA(vAllowConsonantZFWJ, 0);

	APP_SET_DATA(vShowOnStartUp, 0);
	APP_SET_DATA(vShowSwitchFlyout, 1);
	APP_SET_DATA(vSwitchInputType, 0);
	APP_SET_DATA(vInputTypeHotKey, 0xFE0000FE);
	APP_SET_DATA(vUiLanguage, 0);
	APP_SET_DATA(vThemeMode, 0);
	APP_SET_DATA(vAccentColor, 0);
	APP_SET_DATA(vAccentAutoDark, 1);
	APP_SET_DATA(vRunWithWindows, 1);
	EasyKeyHelper::registerRunOnStartup(vRunWithWindows);

	APP_SET_DATA(vRememberCode, 1);
	APP_SET_DATA(vOtherLanguage, 1);
	APP_SET_DATA(vRestoreOnEscape, 1);
	APP_SET_DATA(vWarnOtherKeyboards, 1);
	APP_SET_DATA(vFixChromiumBrowser, 1);

	SystemTrayHelper::rebuildMenu();
	if (settingsWindow) {
		settingsWindow->fillData();
	}
}

void AppDelegate::onToggleVietnamese() {
	APP_SET_DATA(vLanguage, vLanguage ? 0 : 1);
	if (settingsWindow) {
		settingsWindow->fillData();
	}
	showSwitchFlyout();
	
	if (vUseSmartSwitchKey) {
		string& exe = EasyKeyHelper::getLastAppExecuteName();
		setAppInputMethodStatus(exe, vLanguage | (vCodeTable << 1));
		saveSmartSwitchKeyData();
	}
}

void AppDelegate::onMacroTable() {
	if (macroWindow == NULL) {
		MacroWindow* window = new MacroWindow(hInstance);
		macroWindow = window;
		window->show();
	} else {
		macroWindow->bringOnTop();
	}
}

void AppDelegate::onConvertTool() {
	if (convertWindow == NULL) {
		ConvertToolWindow* window = new ConvertToolWindow(hInstance);
		convertWindow = window;
		window->show();
	} else {
		convertWindow->bringOnTop();
	}
}

void AppDelegate::onQuickConvert() {
	if (EasyKeyHelper::quickConvert()) {
		//alert when complete
		if (!convertToolDontAlertWhenCompleted) {
			showNotice(TR("Đã chuyển mã", "Converted"),
				TR("Kết quả đã nằm trong Clipboard", "The result is in the clipboard"));
		}
	}
}

void AppDelegate::onInputType(const int & type) {
	APP_SET_DATA(vInputType, type);
	if (settingsWindow) {
		settingsWindow->fillData();
	}
	SystemTrayHelper::updateData();
	showInputTypeFlyout();
}

void AppDelegate::onTableCode(const int & code, const bool& showFlyout) {
	APP_SET_DATA(vCodeTable, code);
	if (settingsWindow) {
		settingsWindow->fillData();
	}
	if (vRememberCode) {
		setAppInputMethodStatus(EasyKeyHelper::getFrontMostAppExecuteName(), vLanguage | (vCodeTable << 1));
		saveSmartSwitchKeyData();
	}
	if (showFlyout)
		showCodeTableFlyout();
}

void AppDelegate::onControlPanel() {
	createMainDialog();
}

void AppDelegate::onEasyKeyAbout() {
	if (aboutWindow == NULL) {
		AboutWindow* window = new AboutWindow(hInstance);
		aboutWindow = window;
		window->show();
	} else {
		aboutWindow->bringOnTop();
	}
}

void AppDelegate::onEasyKeyExit() {
	EasyKeyManager::freeEngine();
	SystemTrayHelper::removeSystemTray();
	PostQuitMessage(0);
}
