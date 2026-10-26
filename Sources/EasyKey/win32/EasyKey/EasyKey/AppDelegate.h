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