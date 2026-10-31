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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <vector>

#include <shellapi.h>
#include <Commctrl.h>
#include <psapi.h>

#include "resource.h"

#include "../../../engine/Engine.h"

#include "EasyKeyManager.h"
#include "EasyKeyHelper.h"
#include "SystemTrayHelper.h"

using namespace std;

extern wchar_t _logBuffer[1024];
#define LOG(...)  wsprintfW(_logBuffer, __VA_ARGS__); \
					OutputDebugString(_logBuffer);

#define APP_SET_DATA(KEY, VAL) KEY = VAL; EasyKeyHelper::setRegInt(_T(#KEY), KEY)
#define APP_GET_DATA(KEY, DEFAULT_VAL) KEY = EasyKeyHelper::getRegInt(_T(#KEY), DEFAULT_VAL)

#define APP_CLASS _T("EasyKeyVietnameseInputMethod")

//text in the display language the user picked, Vietnamese or English
#define TR(VI, EN) (EasyKeyHelper::isEnglishUi() ? _T(EN) : _T(VI))

extern void saveSmartSwitchKeyData();
//game mode: in these apps keys go straight to the game, Enter opens the chat for Vietnamese
extern bool isGameApp(const string& exe);
extern void setGameApp(const string& exe, const bool& on);
extern const vector<string>& getGameApps();
extern void checkHooks();

extern int vLanguage;
extern int vMixedEnglish;
extern int vTwoModesOnly;
//0 English, 1 Vietnamese, 2 English and Vietnamese mixed (Vietnamese on, English words kept)
#define TYPING_MODE (vLanguage ? ((vTwoModesOnly || !vMixedEnglish) ? 1 : 2) : 0)
extern int vInputType;
extern int vFreeMark;
extern int vCodeTable;
extern int vCheckSpelling;
extern int vUseModernOrthography;
extern int vQuickTelex;
extern int vSwitchKeyStatus;
extern int vRestoreIfWrongSpelling;
extern int vFixRecommendBrowser;
extern int vUseMacro;
extern int vUseMacroInEnglishMode;
extern int vAutoCapsMacro;
extern int vSendKeyStepByStep;
extern int vUseSmartSwitchKey;
extern int vUpperCaseFirstChar;
extern int vShowOnStartUp;
extern int vShowSwitchFlyout;
extern int vSwitchInputType;  //the input method hotkey is on
extern int vInputTypeHotKey;  //next input method while typing Vietnamese, same format as vSwitchKeyStatus
extern int vUiLanguage;       //0 follow Windows, 1 Vietnamese, 2 English
extern int vThemeMode;        //0 follow Windows, 1 light, 2 dark
extern int vAccentColor;      //0 follow Windows, 1-7 presets, 8 custom
extern int vAccentLight;      //custom accent for the light theme, COLORREF
extern int vAccentDark;       //custom accent for the dark theme, COLORREF
extern int vAccentAutoDark;   //derive vAccentDark from vAccentLight
extern int vRunWithWindows;
extern int vCreateDesktopShortcut;
extern int vRunAsAdmin;
extern int vCheckNewVersion;
extern int vRememberCode;
extern int vOtherLanguage;
extern int vRestoreOnEscape;
extern int vWarnOtherKeyboards; //warn at start when UniKey, EVKey... also runs
extern int vLastVersion;        //version that ran last, to show what is new once after an update
extern int vFixChromiumBrowser;
