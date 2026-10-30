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
#pragma once
#include "stdafx.h"
#include "SettingsWindow.h"
#include "FlyoutWindow.h"
#include "FluentWindow.h"

class AppDelegate {
private:
	HINSTANCE hInstance;
	SettingsWindow* settingsWindow = NULL;
	FlyoutWindow* flyout = NULL;
	FluentWindow* aboutWindow = NULL, *macroWindow = NULL, *convertWindow = NULL;
private:
	//once at start: a welcome notice, what is new after an update, other Vietnamese keyboards
	void greet();
	void warnOtherKeyboard(const wstring& name);
	//true when EasyKey moved itself and is exiting
	bool offerToMoveApp();
	void showWhatsNew();
public:
	AppDelegate();
	static AppDelegate* getInstance();
	int run(HINSTANCE hInstance);
	void createMainDialog();
	void closeWindow(FluentWindow* window);
	void closeSettingsWindow();
public: //event
	void onInputMethodChangedFromHotKey();
	void onInputTypeFromHotKey();
	void showSwitchFlyout();
	//the popup with the input method, after it changed from the hotkey, the tray menu or Settings
	void showInputTypeFlyout();
	//the popup with the code table, after it was picked in the tray menu or Settings
	void showCodeTableFlyout();
	//the foreground window runs with more rights than EasyKey, so typing there is not seen
	void onElevatedWindow(const HWND& hWnd);
	//display language or accent color changed
	void onAppearanceChanged();
	//a setting changed outside the settings window
	void onSettingsChanged();
	//a short notice in the flyout
	void showNotice(const wstring& title, const wstring& detail);
	//a background check found a newer version: mark the tray icon, the tray menu and Settings
	void onUpdateAvailable();
	void onUninstall();
	void onDefaultConfig();

	//the tray icon was clicked: the next typing mode, in the order of the switch key
	void onNextTypingMode();
	//TYPING_MODE: 0 English, 1 Vietnamese, 2 English and Vietnamese mixed
	void setTypingMode(const int& mode);

	void onMacroTable();
	void onConvertTool();
	void onQuickConvert();

	void onInputType(const int& type);
	//showFlyout is off when EasyKey restores the code table remembered for an app
	void onTableCode(const int& code, const bool& showFlyout = true);

	void onControlPanel();
	void onEasyKeyAbout();
	void onEasyKeyExit();
};