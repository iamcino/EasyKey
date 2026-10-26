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
	//the foreground window runs with more rights than EasyKey, so typing there is not seen
	void onElevatedWindow(const HWND& hWnd);
	//display language or accent color changed
	void onAppearanceChanged();
	//a setting changed outside the settings window
	void onSettingsChanged();
	//a short notice in the flyout
	void showNotice(const wstring& title, const wstring& detail);
	void onUninstall();
	void onDefaultConfig();

	void onToggleVietnamese();

	void onMacroTable();
	void onConvertTool();
	void onQuickConvert();

	void onInputType(const int& type);
	void onTableCode(const int& code);

	void onControlPanel();
	void onEasyKeyAbout();
	void onEasyKeyExit();
};