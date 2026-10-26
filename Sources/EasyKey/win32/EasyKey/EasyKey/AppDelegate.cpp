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
#include <algorithm>
#include "AppDelegate.h"
#include "ThemeHelper.h"
#include "Updater.h"
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

//shown once after updating to this version; change it with each release
#define WHATS_NEW_VI L"• Cửa sổ cài đặt gọn hơn, có giao diện tiếng Anh và chọn màu nhấn.\n" \
	L"• Gõ thông minh: một công tắc cho kiểm tra chính tả và trả lại phím khi gõ sai.\n" \
	L"• Bấm Esc ngay sau một từ để lấy lại phím vừa gõ, ví dụ việt → vieejt.\n" \
	L"• Báo khi UniKey, EVKey... đang chạy cùng lúc.\n" \
	L"• Tự cập nhật ngay trong EasyKey."
#define WHATS_NEW_EN L"• A more compact settings window, in English too, with your own accent color.\n" \
	L"• Smart typing: one switch for spell check and putting the keys back on misspelled words.\n" \
	L"• Press Esc right after a word to get back the keys typed, e.g. việt → vieejt.\n" \
	L"• A warning when UniKey, EVKey... run at the same time.\n" \
	L"• Updates install from inside EasyKey."
#define RELEASE_NOTES_URL L"https://github.com/iamcino/EasyKey/releases/tag/"

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
int vTempOffSpelling = 0;
int vAllowConsonantZFWJ = 0;
int vQuickStartConsonant = 0;
int vQuickEndConsonant = 0;
int vOtherLanguage = 1;
int vRememberCode = 1;
int vTempOffOpenKey = 0;
int vRestoreOnEscape = 1;
int vWarnOtherKeyboards = 1;
int vLastVersion = 0;

int vShowOnStartUp = 0;
int vShowSwitchFlyout = 1;
int vUiLanguage = 0;
int vAccentColor = 0;
int vAccentLight = RGB(0x4F, 0x46, 0xE5);
int vAccentDark = RGB(0xA5, 0xB4, 0xFC);
int vAccentAutoDark = 1;
int vRunWithWindows = 1;

int vCreateDesktopShortcut = 0;
int vRunAsAdmin = 0;
int vCheckNewVersion = 0;
int vFixChromiumBrowser = 1;

bool AppDelegate::isDialogMsg(MSG & msg) const {
	return (macroDialog != NULL && IsDialogMessage(macroDialog->getHwnd(), &msg)) || 
		(convertDialog != NULL && IsDialogMessage(convertDialog->getHwnd(), &msg)) || 
		(aboutDialog != NULL && IsDialogMessage(aboutDialog->getHwnd(), &msg));
}

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
		if (!isDialogMsg(msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
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

void AppDelegate::closeDialog(BaseDialog * dialog) {
	dialog->closeDialog();
	if (aboutDialog == dialog) {
		delete aboutDialog;
		aboutDialog = NULL;
	} else if (macroDialog == dialog) {
		delete macroDialog;
		macroDialog = NULL;
	} else if (convertDialog == dialog) {
		delete convertDialog;
		convertDialog = NULL;
	}
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

void AppDelegate::showSwitchFlyout() {
	if (flyout)
		flyout->notify();
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
	wstring title = name + TR(" đang chạy", " is running");
	TASKDIALOGCONFIG config = { sizeof(config) };
	config.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION;
	config.dwCommonButtons = TDCBF_OK_BUTTON;
	config.pszWindowTitle = _T("EasyKey");
	config.pszMainIcon = TD_WARNING_ICON;
	config.pszMainInstruction = title.c_str();
	bool windowsKeyboard = name == TR("Bàn phím tiếng Việt của Windows", "The Windows Vietnamese keyboard");
	config.pszContent = windowsKeyboard
		? TR("Hai bộ gõ cùng lúc dễ làm chữ bị bỏ dấu hai lần, ví dụ \"vieejt\". Bấm Windows + Space để chuyển về bàn phím tiếng Anh.",
			"Two Vietnamese keyboards at once can add marks twice, as in \"vieejt\". Press Windows + Space to go back to the English keyboard.")
		: TR("Hai bộ gõ chạy cùng lúc dễ làm chữ bị bỏ dấu hai lần, ví dụ \"vieejt\". Hãy tắt một trong hai.",
			"Two Vietnamese keyboards at once can add marks twice, as in \"vieejt\". Please quit one of them.");
	config.pszVerificationText = TR("Không nhắc lại", "Don't remind me again");
	BOOL dontRemind = FALSE;
	if (SUCCEEDED(TaskDialogIndirect(&config, NULL, NULL, &dontRemind)) && dontRemind) {
		APP_SET_DATA(vWarnOtherKeyboards, 0);
		if (settingsWindow)
			settingsWindow->fillData();
	}
}

void AppDelegate::showWhatsNew() {
	wstring version = EasyKeyHelper::getVersionString();
	wstring title = TR("Có gì mới trong EasyKey ", "What's new in EasyKey ") + version;
	wstring link = wstring(L"<a href=\"") + RELEASE_NOTES_URL + version + L"\">" + TR("Xem đầy đủ các thay đổi", "See all changes") + L"</a>";
	TASKDIALOGCONFIG config = { sizeof(config) };
	config.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_ENABLE_HYPERLINKS;
	config.dwCommonButtons = TDCBF_OK_BUTTON;
	config.pszWindowTitle = _T("EasyKey");
	config.pszMainIcon = TD_INFORMATION_ICON;
	config.pszMainInstruction = title.c_str();
	config.pszContent = EasyKeyHelper::isEnglishUi() ? WHATS_NEW_EN : WHATS_NEW_VI;
	config.pszFooter = link.c_str();
	config.pfCallback = [](HWND, UINT msg, WPARAM, LPARAM lParam, LONG_PTR) -> HRESULT {
		if (msg == TDN_HYPERLINK_CLICKED)
			ShellExecute(NULL, _T("open"), (LPCWSTR)lParam, NULL, NULL, SW_SHOWNORMAL);
		return S_OK;
	};
	TaskDialogIndirect(&config, NULL, NULL, NULL);
}

void AppDelegate::onAppearanceChanged() {
	SystemTrayHelper::rebuildMenu();
	if (settingsWindow)
		settingsWindow->fillData();
}

void AppDelegate::onUninstall() {
	EasyKeyManager::freeEngine();
	EasyKeyHelper::removeFromSystem();
	//one literal per language: _T() only widens the first piece of a concatenated string
	MessageBox(NULL, TR("Đã gỡ EasyKey khỏi Windows: không còn chạy cùng Windows, lối tắt và cài đặt đã xoá.\nThư mục chứa EasyKey.exe sẽ mở ra để bạn xoá file nếu muốn.",
		"EasyKey was removed from Windows: it no longer starts with Windows, and its shortcut and settings are gone.\n"
		"The folder with EasyKey.exe opens next, so you can delete the file if you like."), _T("EasyKey"), MB_ICONINFORMATION | MB_OK);
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
	APP_SET_DATA(vTempOffSpelling, 0);

	APP_SET_DATA(vShowOnStartUp, 0);
	APP_SET_DATA(vShowSwitchFlyout, 1);
	APP_SET_DATA(vUiLanguage, 0);
	APP_SET_DATA(vAccentColor, 0);
	APP_SET_DATA(vAccentAutoDark, 1);
	APP_SET_DATA(vRunWithWindows, 1);
	EasyKeyHelper::registerRunOnStartup(vRunWithWindows);

	APP_SET_DATA(vRememberCode, 1);
	APP_SET_DATA(vOtherLanguage, 1);
	APP_SET_DATA(vTempOffOpenKey, 0);
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
	
	if (vUseSmartSwitchKey) {
		string& exe = EasyKeyHelper::getLastAppExecuteName();
		setAppInputMethodStatus(exe, vLanguage | (vCodeTable << 1));
		saveSmartSwitchKeyData();
	}
}

void AppDelegate::onMacroTable() {
	if (macroDialog == NULL) {
		macroDialog = new MacroDialog(hInstance, IDD_DIALOG_MACRO);
		macroDialog->show();
	} else {
		macroDialog->bringOnTop();
	}
}

void AppDelegate::onConvertTool() {
	if (convertDialog == NULL) {
		convertDialog = new ConvertToolDialog(hInstance, IDD_DIALOG_CONVERT_TOOL);
		convertDialog->show();
	} else {
		convertDialog->bringOnTop();
	}
}

void AppDelegate::onQuickConvert() {
	if (EasyKeyHelper::quickConvert()) {
		//alert when complete
		if (!convertToolDontAlertWhenCompleted) {
			LPCTSTR msg = TR("Chuyển mã thành công!\nKết quả đã được lưu trong Clipboard!",
				"Converted.\nThe result is in the clipboard.");
			MessageBox(NULL, msg, _T("EasyKey"), MB_OK);
		}
	}
}

void AppDelegate::onInputType(const int & type) {
	APP_SET_DATA(vInputType, type);
	if (settingsWindow) {
		settingsWindow->fillData();
	}
}

void AppDelegate::onTableCode(const int & code) {
	APP_SET_DATA(vCodeTable, code);
	if (settingsWindow) {
		settingsWindow->fillData();
	}
	if (vRememberCode) {
		setAppInputMethodStatus(EasyKeyHelper::getFrontMostAppExecuteName(), vLanguage | (vCodeTable << 1));
		saveSmartSwitchKeyData();
	}
}

void AppDelegate::onControlPanel() {
	createMainDialog();
}

void AppDelegate::onEasyKeyAbout() {
	if (aboutDialog == NULL) {
		aboutDialog = new AboutDialog(hInstance, IDD_ABOUTBOX);
		aboutDialog->show();
	} else {
		aboutDialog->bringOnTop();
	}
}

void AppDelegate::onEasyKeyExit() {
	EasyKeyManager::freeEngine();
	SystemTrayHelper::removeSystemTray();
	PostQuitMessage(0);
}
