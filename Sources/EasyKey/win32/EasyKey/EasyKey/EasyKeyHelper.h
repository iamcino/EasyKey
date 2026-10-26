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

extern int CF_RTF;
extern int CF_HTML;
extern int CF_EASYKEY;

class EasyKeyHelper {
private:
	static void openRegKey();
public:
	static void setRegInt(LPCTSTR key, const int& val);
	static int getRegInt(LPCTSTR key, const int& defaultValue);

	static void setRegBinary(LPCTSTR key, const BYTE* pData, const int& size);
	static BYTE* getRegBinary(LPCTSTR key, DWORD& outSize);

	static void registerRunOnStartup(const int& val);

	static LPTSTR getExecutePath();

	static string& getFrontMostAppExecuteName();
	static string& getLastAppExecuteName();

	static wstring getFullPath();

	static wstring getClipboardText(const int& type);
	static void setClipboardText(LPCTSTR data, const int& len, const int& type);

	static bool quickConvert();

	static DWORD getVersionNumber();
	static wstring getVersionString();

	//"Ctrl + Shift + Z" for a hotkey stored like vSwitchKeyStatus, empty if none is set
	static wstring getHotKeyString(const int& hotKey);

	//vUiLanguage resolved, following the Windows display language when it is 0
	static bool isEnglishUi();
	//replace the Vietnamese text of a dialog and its controls with English when the UI is English
	static void translateWindow(const HWND& hWnd);
	//true when the window belongs to a process with a higher integrity level than EasyKey,
	//whose keystrokes the keyboard hook never sees
	static bool isWindowElevated(const HWND& hWnd);
	//remove what EasyKey added to Windows: startup entry, admin task, shortcut and settings
	static void removeFromSystem();
};

