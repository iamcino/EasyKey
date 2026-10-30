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
#include "stdafx.h"
#include "AppDelegate.h"
#include <algorithm>

#pragma comment(lib, "imm32")
#define IMC_GETOPENSTATUS 0x0005

#define MASK_SHIFT				0x01
#define MASK_CONTROL			0x02
#define MASK_ALT				0x04
#define MASK_CAPITAL			0x08
#define MASK_NUMLOCK			0x10
#define MASK_WIN				0x20
#define MASK_SCROLL				0x40

#define OTHER_CONTROL_KEY (_flag & MASK_ALT) || (_flag & MASK_CONTROL)
#define DYNA_DATA(macro, pos) (macro ? pData->macroData[pos] : pData->charData[pos])
#define EMPTY_HOTKEY 0xFE0000FE

//tag for events sent by EasyKey itself; other software (keyboard filter drivers, security suites)
//may set dwExtraInfo on real key events, so only skip events carrying this exact value
#define EASYKEY_EXTRA_INFO 0x4F4B4559

static vector<string> _chromiumBrowser = {
	"chrome.exe", "brave.exe", "msedge.exe"
};

extern int vSendKeyStepByStep;
extern int vShowOnStartUp;
extern int vShowSwitchFlyout;
extern int vSwitchInputType;
extern int vInputTypeHotKey;
extern int vRunWithWindows;

static HHOOK hKeyboardHook;
static HHOOK hMouseHook;
static DWORD _lastHookTick = 0; //last time either hook ran, see checkHooks
//key presses from a real keyboard, counted by the keyboard hook and by raw input, see onRawKeyboard
static DWORD _rawKeyDowns = 0, _hookKeyDowns = 0;
static HWINEVENTHOOK hSystemEvent;
static KBDLLHOOKSTRUCT* keyboardData;
static MSLLHOOKSTRUCT* mouseData;
static vKeyHookState* pData;
static vector<Uint16> _syncKey;
static Uint32 _flag = 0, _lastFlag = 0, _privateFlag;
static bool _flagChanged = false, _isFlagKey;
static Uint16 _keycode;
static Uint16 _newChar, _newCharHi;

static vector<Uint16> _newCharString;
static Uint16 _newCharSize;
static bool _willSendControlKey = false;

static Uint16 _uniChar[2];
static int _i, _j, _k;
static Uint32 _tempChar;

static string macroText, macroContent;
static int _languageTemp = 0; //use for smart switch key
static vector<Byte> savedSmartSwitchKeyData; ////use for smart switch key

static bool _hasJustUsedHotKey = false;
//game mode, see isGameApp
static vector<string> _gameApps;
static bool _inGame = false;   //the foreground app is in game mode
static bool _gameChat = false; //Enter opened the chat in it
//Esc that put back the typed keys: its key up is swallowed too, web chats (Facebook...) act on it
static bool _escapeRestored = false;

static INPUT backspaceEvent[2];
static INPUT keyEvent[2];

static int vDebugKeyLog = 0;
static FILE* _debugLogFile = NULL;
static int _debugLogCount = 0;

LRESULT CALLBACK keyboardHookProcess(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK mouseHookProcess(int nCode, WPARAM wParam, LPARAM lParam);
VOID CALLBACK winEventProcCallback(HWINEVENTHOOK hWinEventHook, DWORD dwEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
static void watchRawKeyboard();
static void syncModifierMask();

void EasyKeyFree() {
	UnhookWindowsHookEx(hMouseHook);
	UnhookWindowsHookEx(hKeyboardHook);
	UnhookWinEvent(hSystemEvent);
}

void EasyKeyInit() {
	APP_GET_DATA(vLanguage, 1);
	vLanguage = vLanguage ? 1 : 0; //older versions could save the code table bits here too
	APP_GET_DATA(vMixedEnglish, 0);
	APP_GET_DATA(vInputType, 0);
	vFreeMark = 0;
	APP_GET_DATA(vCodeTable, 0);
	APP_GET_DATA(vCheckSpelling, 1);
	APP_GET_DATA(vUseModernOrthography, 0);
	APP_GET_DATA(vQuickTelex, 0);
	APP_GET_DATA(vSwitchKeyStatus, 0x5A00025A); //Alt + Z, same as the default config (0x7A000206 was a macOS key code)
	APP_GET_DATA(vRestoreIfWrongSpelling, 1);
	APP_GET_DATA(vUseMacro, 1);
	APP_GET_DATA(vUseMacroInEnglishMode, 0);
	APP_GET_DATA(vAutoCapsMacro, 0);
	APP_GET_DATA(vSendKeyStepByStep, 1);
	vShowOnStartUp = 0; //no longer an option, OpenKey's copied settings often had it on; open EasyKey again to show Settings
	APP_GET_DATA(vShowSwitchFlyout, 1);
	APP_GET_DATA(vSwitchInputType, 0);
	APP_GET_DATA(vInputTypeHotKey, EMPTY_HOTKEY);
	APP_GET_DATA(vUiLanguage, 0);
	APP_GET_DATA(vThemeMode, 0);
	APP_GET_DATA(vAccentColor, 0);
	APP_GET_DATA(vAccentLight, RGB(0x4F, 0x46, 0xE5));
	APP_GET_DATA(vAccentDark, RGB(0xA5, 0xB4, 0xFC));
	APP_GET_DATA(vAccentAutoDark, 1);
	APP_GET_DATA(vRunWithWindows, 1);
	EasyKeyHelper::registerRunOnStartup(vRunWithWindows);
	APP_GET_DATA(vUseSmartSwitchKey, 1);
	APP_GET_DATA(vUpperCaseFirstChar, 0);
	APP_GET_DATA(vAllowConsonantZFWJ, 0);
	APP_GET_DATA(vQuickStartConsonant, 0);
	APP_GET_DATA(vQuickEndConsonant, 0);
	APP_GET_DATA(vRunAsAdmin, 0);
	APP_GET_DATA(vCreateDesktopShortcut, 0);
	APP_GET_DATA(vCheckNewVersion, 1);
	APP_GET_DATA(vRememberCode, 1);
	APP_GET_DATA(vOtherLanguage, 1);
	APP_GET_DATA(vRestoreOnEscape, 1);
	APP_GET_DATA(vWarnOtherKeyboards, 1);
	APP_GET_DATA(vLastVersion, 0);
	//the browser fixes are always on; vFixBrowserOff = 1 in the registry turns them off for an app they break
	int vFixBrowserOff;
	APP_GET_DATA(vFixBrowserOff, 0);
	vFixRecommendBrowser = vFixChromiumBrowser = vFixBrowserOff ? 0 : 1;

	//init convert tool
	APP_GET_DATA(convertToolDontAlertWhenCompleted, 0);
	APP_GET_DATA(convertToolToAllCaps, 0);
	APP_GET_DATA(convertToolToAllNonCaps, 0);
	APP_GET_DATA(convertToolToCapsFirstLetter, 0);
	APP_GET_DATA(convertToolToCapsEachWord, 0);
	APP_GET_DATA(convertToolRemoveMark, 0);
	APP_GET_DATA(convertToolFromCode, 1); //TCVN3 to Unicode, the usual case
	APP_GET_DATA(convertToolToCode, 0);
	APP_GET_DATA(convertToolHotKey, EMPTY_HOTKEY);
	if (convertToolHotKey == 0) {
		convertToolHotKey = EMPTY_HOTKEY;
	}

	APP_GET_DATA(vDebugKeyLog, 0);

	pData = (vKeyHookState*)vKeyInit();

	//pre-create back key
	backspaceEvent[0].type = INPUT_KEYBOARD;
	backspaceEvent[0].ki.dwFlags = 0;
	backspaceEvent[0].ki.wVk = VK_BACK;
	backspaceEvent[0].ki.wScan = 0;
	backspaceEvent[0].ki.time = 0;
	backspaceEvent[0].ki.dwExtraInfo = EASYKEY_EXTRA_INFO;

	backspaceEvent[1].type = INPUT_KEYBOARD;
	backspaceEvent[1].ki.dwFlags = KEYEVENTF_KEYUP;
	backspaceEvent[1].ki.wVk = VK_BACK;
	backspaceEvent[1].ki.wScan = 0;
	backspaceEvent[1].ki.time = 0;
	backspaceEvent[1].ki.dwExtraInfo = EASYKEY_EXTRA_INFO;

	//get key state
	_flag = 0;
	syncModifierMask();
	if (GetKeyState(VK_NUMLOCK) < 0) _flag |= MASK_NUMLOCK;
	if (GetKeyState(VK_SCROLL) < 0) _flag |= MASK_SCROLL;

	//init and load macro data
	DWORD macroDataSize;
	BYTE* macroData = EasyKeyHelper::getRegBinary(_T("macroData"), macroDataSize);
	initMacroMap((Byte*)macroData, (int)macroDataSize);
	if (EasyKeyHelper::isFirstRun() && !macroData) {
		//a few common shortcuts to start with, so "ko → không" in Settings works out of the box
		const char* starters[][2] = { { "ko", "không" }, { "dc", "được" }, { "vs", "với" }, { "mn", "mọi người" }, { "ntn", "như thế nào" } };
		for (auto& starter : starters)
			addMacro(starter[0], starter[1]);
		vector<Byte> starterData;
		getMacroSaveData(starterData);
		EasyKeyHelper::setRegBinary(_T("macroData"), starterData.data(), (int)starterData.size());
	}

	//init and load smart switch key data
	DWORD smartSwitchKeySize;
	BYTE* data = EasyKeyHelper::getRegBinary(_T("smartSwitchKey"), smartSwitchKeySize);
	initSmartSwitchKey((Byte*)data, (int)smartSwitchKeySize);

	//game mode apps, exe names separated by new lines
	DWORD gameAppsSize;
	BYTE* gameApps = EasyKeyHelper::getRegBinary(_T("gameApps"), gameAppsSize);
	if (gameApps && gameAppsSize > 0) {
		string list((char*)gameApps, gameAppsSize), name;
		for (char c : list) {
			if (c == '\n') {
				if (!name.empty())
					_gameApps.push_back(name);
				name.clear();
			} else {
				name += c;
			}
		}
		if (!name.empty())
			_gameApps.push_back(name);
	}

	//init hook
	HINSTANCE hInstance = GetModuleHandle(NULL);
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHookProcess, hInstance, 0);
	hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseHookProcess, hInstance, 0);
	hSystemEvent = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, winEventProcCallback, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	watchRawKeyboard();
}

//exe name of a window, without the cache of getFrontMostAppExecuteName that keeps the previous app for Explorer
static string exeNameOfWindow(const HWND& hWnd) {
	DWORD processId = 0;
	GetWindowThreadProcessId(hWnd, &processId);
	HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
	if (!process)
		return "";
	TCHAR path[MAX_PATH];
	DWORD size = MAX_PATH;
	string name;
	if (QueryFullProcessImageName(process, 0, path, &size)) {
		LPCTSTR file = _tcsrchr(path, '\\');
		name = wideStringToUtf8(file ? file + 1 : path);
	}
	CloseHandle(process);
	return name;
}

const vector<string>& getGameApps() {
	return _gameApps;
}

bool isGameApp(const string& exe) {
	return std::find(_gameApps.begin(), _gameApps.end(), exe) != _gameApps.end();
}

void setGameApp(const string& exe, const bool& on) {
	_gameApps.erase(std::remove(_gameApps.begin(), _gameApps.end(), exe), _gameApps.end());
	if (on)
		_gameApps.push_back(exe);
	string list;
	for (auto& name : _gameApps)
		list += name + "\n";
	EasyKeyHelper::setRegBinary(_T("gameApps"), (BYTE*)list.data(), (int)list.size());
	//the app is in the background while the tray menu is open, it applies when it comes back
	_inGame = false;
	_gameChat = false;
}

void saveSmartSwitchKeyData() {
	getSmartSwitchKeySaveData(savedSmartSwitchKeyData);
	EasyKeyHelper::setRegBinary(_T("smartSwitchKey"), savedSmartSwitchKeyData.data(), (int)savedSmartSwitchKeyData.size());
}

static void InsertKeyLength(const Uint8& len) {
	_syncKey.push_back(len);
}

static inline void prepareKeyEvent(INPUT& input, const Uint16& keycode, const bool& isPress, const DWORD& flag=0) {
	input.type = INPUT_KEYBOARD;
	input.ki.dwFlags = isPress ? flag : flag|KEYEVENTF_KEYUP;
	input.ki.wVk = keycode;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = EASYKEY_EXTRA_INFO;
}

//a hotkey with Alt or Win eats its key, so the app sees Alt or Win pressed alone and opens its menu
//(or Start) when it is released. A key that does nothing, sent in between, stops that, as AutoHotkey does
static void maskMenuKey() {
	if (!(_flag & (MASK_ALT | MASK_WIN)))
		return;
	INPUT mask[2];
	prepareKeyEvent(mask[0], 0xE8, true); //0xE8 is unassigned
	prepareKeyEvent(mask[1], 0xE8, false);
	SendInput(2, mask, sizeof(INPUT));
}

static inline void prepareUnicodeEvent(INPUT& input, const Uint16& unicode, const bool& isPress) {
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = 0;
	input.ki.wScan = unicode;
	input.ki.time = 0;
	input.ki.dwFlags = (isPress ? 0 : KEYEVENTF_KEYUP) | KEYEVENTF_UNICODE;
	input.ki.dwExtraInfo = EASYKEY_EXTRA_INFO;
}

static void SendCombineKey(const Uint16& key1, const Uint16& key2, const DWORD& flagKey1=0, const DWORD& flagKey2 = 0) {
	prepareKeyEvent(keyEvent[0], key1, true, flagKey1);
	SendInput(1, keyEvent, sizeof(INPUT));

	prepareKeyEvent(keyEvent[0], key2, true, flagKey2);
	prepareKeyEvent(keyEvent[1], key2, false, flagKey2);
	SendInput(2, keyEvent, sizeof(INPUT));

	prepareKeyEvent(keyEvent[0], key1, false, flagKey1);
	SendInput(1, keyEvent, sizeof(INPUT));
}

static void SendKeyCode(Uint32 data) {
	_newChar = (Uint16)data;
	if (!(data & CHAR_CODE_MASK)) {
		if (IS_DOUBLE_CODE(vCodeTable)) //VNI
			InsertKeyLength(1);

		_newChar = keyCodeToCharacter(data);
		if (_newChar == 0) {
			_newChar = (Uint16)data;
			prepareKeyEvent(keyEvent[0], _newChar, true);
			prepareKeyEvent(keyEvent[1], _newChar, false);
			SendInput(2, keyEvent, sizeof(INPUT));
		} else {
			prepareUnicodeEvent(keyEvent[0], _newChar, true);
			prepareUnicodeEvent(keyEvent[1], _newChar, false);
			SendInput(2, keyEvent, sizeof(INPUT));
		}
	} else {
		if (vCodeTable == 0) { //unicode 2 bytes code
			prepareUnicodeEvent(keyEvent[0], _newChar, true);
			prepareUnicodeEvent(keyEvent[1], _newChar, false);
			SendInput(2, keyEvent, sizeof(INPUT));
		} else if (vCodeTable == 1 || vCodeTable == 2 || vCodeTable == 4) { //others such as VNI Windows, TCVN3: 1 byte code
			_newCharHi = HIBYTE(_newChar);
			_newChar = LOBYTE(_newChar);

			prepareUnicodeEvent(keyEvent[0], _newChar, true);
			prepareUnicodeEvent(keyEvent[1], _newChar, false);
			SendInput(2, keyEvent, sizeof(INPUT));

			if (_newCharHi > 32) {
				if (vCodeTable == 2) //VNI
					InsertKeyLength(2);
				prepareUnicodeEvent(keyEvent[0], _newCharHi, true);
				prepareUnicodeEvent(keyEvent[1], _newCharHi, false);
				SendInput(2, keyEvent, sizeof(INPUT));
			} else {
				if (vCodeTable == 2) //VNI
					InsertKeyLength(1);
			}
		} else if (vCodeTable == 3) { //Unicode Compound
			_newCharHi = (_newChar >> 13);
			_newChar &= 0x1FFF;
			_uniChar[0] = _newChar;
			_uniChar[1] = _newCharHi > 0 ? (_unicodeCompoundMark[_newCharHi - 1]) : 0;
			InsertKeyLength(_newCharHi > 0 ? 2 : 1);
			prepareUnicodeEvent(keyEvent[0], _uniChar[0], true);
			prepareUnicodeEvent(keyEvent[1], _uniChar[0], false);
			SendInput(2, keyEvent, sizeof(INPUT));
			if (_newCharHi > 0) {
				prepareUnicodeEvent(keyEvent[0], _uniChar[1], true);
				prepareUnicodeEvent(keyEvent[1], _uniChar[1], false);
				SendInput(2, keyEvent, sizeof(INPUT));
			}
		}
	}
}

static void SendBackspace() {
	SendInput(2, backspaceEvent, sizeof(INPUT));
	if (IS_DOUBLE_CODE(vCodeTable)) { //VNI or Unicode Compound
		if (_syncKey.back() > 1) {
			/*if (!(vCodeTable == 3 && containUnicodeCompoundApp(FRONT_APP))) {
				SendInput(2, backspaceEvent, sizeof(INPUT));
			}*/
			SendInput(2, backspaceEvent, sizeof(INPUT));
		}
		_syncKey.pop_back();
	}
}

static void SendEmptyCharacter() {
	if (IS_DOUBLE_CODE(vCodeTable)) //VNI or Unicode Compound
		InsertKeyLength(1);

	_newChar = 0x202F; //empty char

	prepareUnicodeEvent(keyEvent[0], _newChar, true);
	prepareUnicodeEvent(keyEvent[1], _newChar, false);
	SendInput(2, keyEvent, sizeof(INPUT));
}

static void SendNewCharString(const bool& dataFromMacro = false) {
	_j = 0;
	_newCharSize = dataFromMacro ? (Uint16)pData->macroData.size() : pData->newCharCount;
	if (_newCharString.size() < _newCharSize) {
		_newCharString.resize(_newCharSize);
	}
	_willSendControlKey = false;
	
	if (_newCharSize > 0) {
		for (_k = dataFromMacro ? 0 : pData->newCharCount - 1;
			dataFromMacro ? _k < pData->macroData.size() : _k >= 0;
			dataFromMacro ? _k++ : _k--) {

			_tempChar = DYNA_DATA(dataFromMacro, _k);
			if (_tempChar & PURE_CHARACTER_MASK) {
				_newCharString[_j++] = _tempChar;
				if (IS_DOUBLE_CODE(vCodeTable)) {
					InsertKeyLength(1);
				}
			} else if (!(_tempChar & CHAR_CODE_MASK)) {
				if (IS_DOUBLE_CODE(vCodeTable)) //VNI
					InsertKeyLength(1);
				_newCharString[_j++] = keyCodeToCharacter(_tempChar);
			} else {
				_newChar = _tempChar;
				if (vCodeTable == 0) {  //unicode 2 bytes code
					_newCharString[_j++] = _newChar;
				} else if (vCodeTable == 1 || vCodeTable == 2 || vCodeTable == 4) { //others such as VNI Windows, TCVN3: 1 byte code
					_newCharHi = HIBYTE(_newChar);
					_newChar = LOBYTE(_newChar);
					_newCharString[_j++] = _newChar;

					if (_newCharHi > 32) {
						if (vCodeTable == 2) //VNI
							InsertKeyLength(2);
						_newCharString[_j++] = _newCharHi;
						_newCharSize++;
					}
					else {
						if (vCodeTable == 2) //VNI
							InsertKeyLength(1);
					}
				} else if (vCodeTable == 3) { //Unicode Compound
					_newCharHi = (_newChar >> 13);
					_newChar &= 0x1FFF;

					InsertKeyLength(_newCharHi > 0 ? 2 : 1);
					_newCharString[_j++] = _newChar;
					if (_newCharHi > 0) {
						_newCharSize++;
						_newCharString[_j++] = _unicodeCompoundMark[_newCharHi - 1];
					}

				}
			}
		}//end for
	}

	if (pData->code == vRestore || pData->code == vRestoreAndStartNewSession) { //if is restore
		if (keyCodeToCharacter(_keycode) != 0) {
			_newCharSize++;
			_newCharString[_j++] = keyCodeToCharacter(_keycode | ((_flag & MASK_SHIFT) || (_flag & MASK_CAPITAL) ? CAPS_MASK : 0));
		} else {
			_willSendControlKey = true;
		}
	}
	if (pData->code == vRestoreAndStartNewSession) {
		startNewSession();
	}

	EasyKeyHelper::setClipboardText((LPCTSTR)_newCharString.data(), _newCharSize + 1, CF_UNICODETEXT);

	//Send shift + insert
	SendCombineKey(KEY_LEFT_SHIFT, VK_INSERT, 0, KEYEVENTF_EXTENDEDKEY);
	
	//the case when hCode is vRestore or vRestoreAndStartNewSession,
	//the word is invalid and last key is control key such as TAB, LEFT ARROW, RIGHT ARROW,...
	if (_willSendControlKey) {
		SendKeyCode(_keycode);
	}
}

bool checkHotKey(int hotKeyData, bool checkKeyCode = true) {
	if ((hotKeyData & (~0x8000)) == EMPTY_HOTKEY)
		return false;
	if (HAS_CONTROL(hotKeyData) ^ GET_BOOL(_lastFlag & MASK_CONTROL))
		return false;
	if (HAS_OPTION(hotKeyData) ^ GET_BOOL(_lastFlag & MASK_ALT))
		return false;
	if (HAS_COMMAND(hotKeyData) ^ GET_BOOL(_lastFlag & MASK_WIN))
		return false;
	if (HAS_SHIFT(hotKeyData) ^ GET_BOOL(_lastFlag & MASK_SHIFT))
		return false;
	if (checkKeyCode) {
		if (GET_SWITCH_KEY(hotKeyData) != _keycode)
			return false;
	}
	return true;
}

void switchLanguage() {
	if (vLanguage == 0)
		vLanguage = 1;
	else
		vLanguage = 0;
	if (HAS_BEEP(vSwitchKeyStatus))
		MessageBeep(MB_OK);
	AppDelegate::getInstance()->onInputMethodChangedFromHotKey();
	AppDelegate::getInstance()->showSwitchFlyout();
	if (vUseSmartSwitchKey) {
		setAppInputMethodStatus(EasyKeyHelper::getFrontMostAppExecuteName(), vLanguage | (vCodeTable << 1));
		saveSmartSwitchKeyData();
	}
	startNewSession();
}

//Telex, VNI, Simple Telex in turn; only while typing Vietnamese
static bool isInputTypeHotKey(const bool& checkKeyCode) {
	return vSwitchInputType && checkHotKey(vInputTypeHotKey, checkKeyCode && GET_SWITCH_KEY(vInputTypeHotKey) != 0xFE);
}

void switchInputType() {
	vInputType = (vInputType + 1) % (int)EasyKeyManager::getInputType().size();
	if (HAS_BEEP(vSwitchKeyStatus))
		MessageBeep(MB_OK);
	AppDelegate::getInstance()->onInputTypeFromHotKey();
	startNewSession();
}

static void SendPureCharacter(const Uint16& ch) {
	if (ch < 128)
		SendKeyCode(ch);
	else {
		prepareUnicodeEvent(keyEvent[0], ch, true);
		prepareUnicodeEvent(keyEvent[1], ch, false);
		SendInput(2, keyEvent, sizeof(INPUT));
		if (IS_DOUBLE_CODE(vCodeTable)) {
			InsertKeyLength(1);
		}
	}
}

static void handleMacro() {
	//fix autocomplete
	if (vFixRecommendBrowser) {
		SendEmptyCharacter();
		pData->backspaceCount++;
	}

	//send backspace
	if (pData->backspaceCount > 0) {
		for (int i = 0; i < pData->backspaceCount; i++) {
			SendBackspace();
		}
	}
	//send real data
	if (!vSendKeyStepByStep) {
		SendNewCharString(true);
	} else {
		for (int i = 0; i < pData->macroData.size(); i++) {
			if (pData->macroData[i] & PURE_CHARACTER_MASK) {
				SendPureCharacter(pData->macroData[i]);
			} else {
				SendKeyCode(pData->macroData[i]);
			}
		}
	}
	SendKeyCode(_keycode | (_flag & MASK_SHIFT ? CAPS_MASK : 0));
}

//Shift, Ctrl, Alt, Win and Caps Lock as they really are now. The hook follows them from their own key
//events, but it never sees a key up while it is gone (removed by Windows until the watchdog puts it back,
//an elevated window or the lock screen in front): a Shift left down that way made every rewritten letter
//upper case, "giÚP tÔi hOÀN thIỆN" in Firefox. The hook runs before the key changes the async state, so
//this is right for any key but the modifier being pressed.
static void syncModifierMask() {
	_flag &= ~(MASK_SHIFT | MASK_CONTROL | MASK_ALT | MASK_WIN | MASK_CAPITAL);
	if (GetAsyncKeyState(VK_SHIFT) < 0) _flag |= MASK_SHIFT;
	if (GetAsyncKeyState(VK_CONTROL) < 0) _flag |= MASK_CONTROL;
	if (GetAsyncKeyState(VK_MENU) < 0) _flag |= MASK_ALT;
	if (GetAsyncKeyState(VK_LWIN) < 0 || GetAsyncKeyState(VK_RWIN) < 0) _flag |= MASK_WIN;
	// For caps lock case, toggling the flag isn't enough. We need to check the actual state, which should be done before each key press.
	// Example: the caps lock state can be changed without the key being pressed, or the key toggle is made with admin privilege, making the app not able to detect the change.
	if (GetKeyState(VK_CAPITAL) & 1) _flag |= MASK_CAPITAL;
}

static bool SetModifierMask(const Uint16& vkCode) {
	if (vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) _flag |= MASK_SHIFT;
	else if (vkCode == VK_LCONTROL || vkCode == VK_RCONTROL) _flag |= MASK_CONTROL;
	else if (vkCode == VK_LMENU || vkCode == VK_RMENU) _flag |= MASK_ALT;
	else if (vkCode == VK_LWIN || vkCode == VK_RWIN) _flag |= MASK_WIN;
	else if (vkCode == VK_NUMLOCK) _flag |= MASK_NUMLOCK;
	else if (vkCode == VK_SCROLL) _flag |= MASK_SCROLL;
	else { 
		syncModifierMask();
		_isFlagKey = false;
		return false; 
	}
	_isFlagKey = true;
	return true;
}

static bool UnsetModifierMask(const Uint16& vkCode) {
	if (vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) _flag &= ~MASK_SHIFT;
	else if (vkCode == VK_LCONTROL || vkCode == VK_RCONTROL) _flag &= ~MASK_CONTROL;
	else if (vkCode == VK_LMENU || vkCode == VK_RMENU) _flag &= ~MASK_ALT;
	else if (vkCode == VK_LWIN || vkCode == VK_RWIN) _flag &= ~MASK_WIN;
	else if (vkCode == VK_NUMLOCK) _flag &= ~MASK_NUMLOCK;
	else if (vkCode == VK_SCROLL) _flag &= ~MASK_SCROLL;
	else { 
		_isFlagKey = false;
		return false; 
	}
	_isFlagKey = true;
	return true;
}

//diagnostic log, enabled by setting registry value vDebugKeyLog = 1. It keeps going so a rare
//wrong word can be caught: every 2000 lines the file moves to EasyKeyKeyLog.old.txt
static bool openDebugLog() {
	if (_debugLogFile != NULL && _debugLogCount < 2000)
		return true;
	WCHAR path[MAX_PATH], oldPath[MAX_PATH];
	GetTempPath(MAX_PATH, path);
	wcscpy_s(oldPath, path);
	wcscat_s(path, L"EasyKeyKeyLog.txt");
	wcscat_s(oldPath, L"EasyKeyKeyLog.old.txt");
	if (_debugLogFile != NULL) {
		fclose(_debugLogFile);
		_debugLogFile = NULL;
		MoveFileEx(path, oldPath, MOVEFILE_REPLACE_EXISTING);
	}
	_debugLogCount = 0;
	return _wfopen_s(&_debugLogFile, path, L"w") == 0;
}

static void writeDebugLine(const char* text) {
	if (!openDebugLog())
		return;
	fprintf(_debugLogFile, "t=%lu %s\n", GetTickCount(), text);
	fflush(_debugLogFile);
	_debugLogCount++;
}

static void writeDebugKeyLog(WPARAM wParam, DWORD_PTR isImeON) {
	if (!openDebugLog())
		return;
	fprintf(_debugLogFile, "t=%lu msg=%04X vk=%02X scan=%02X flags=%02X injected=%d extra=%llX ime=%d mods=%X lang=%d app=%s\n",
		GetTickCount(), (unsigned)wParam, keyboardData->vkCode, keyboardData->scanCode, keyboardData->flags,
		(keyboardData->flags & LLKHF_INJECTED) ? 1 : 0, (unsigned long long)keyboardData->dwExtraInfo,
		(int)isImeON, _flag, vLanguage, EasyKeyHelper::getLastAppExecuteName().c_str());
	fflush(_debugLogFile);
	_debugLogCount++;
}

static void reinstallHooks(const char* reason) {
	HINSTANCE hInstance = GetModuleHandle(NULL);
	UnhookWindowsHookEx(hKeyboardHook);
	UnhookWindowsHookEx(hMouseHook);
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHookProcess, hInstance, 0);
	hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseHookProcess, hInstance, 0);
	_lastHookTick = GetTickCount();
	if ((int)(_rawKeyDowns - _hookKeyDowns) > 0)
		_hookKeyDowns = _rawKeyDowns;
	//the keys typed while the hook was gone reached the app as they were, start over
	startNewSession();
	syncModifierMask();
	if (vDebugKeyLog)
		writeDebugLine(reason);
}

//Windows silently removes a low-level hook that once answered too slowly (LowLevelHooksTimeout),
//after which keys reach the app untouched ("chaats" instead of "chất") until the hook is back.
//Raw keyboard input (onRawKeyboard) finds a lost keyboard hook on the next key; this timer is
//the fallback: input that keeps arriving without reaching the hooks means they are gone.
void checkHooks() {
	LASTINPUTINFO info = { sizeof(info) };
	if (!GetLastInputInfo(&info))
		return;
	DWORD now = GetTickCount();
	if (now - info.dwTime > 3000 || (int)(info.dwTime - _lastHookTick) < 1000)
		return;
	//an elevated window gets input the hooks are not allowed to see
	HWND foreground = GetForegroundWindow();
	if (foreground && EasyKeyHelper::isWindowElevated(foreground))
		return;
	reinstallHooks("hooks reinstalled (timer)");
}

//raw input gets every key from a keyboard, hook or not. Windows calls the low-level hooks before
//it makes the raw input, so a key press the hook did not see means the keyboard hook is gone.
//checkHooks alone missed it while the mouse moved: the mouse hook still ran and looked alive
static void onRawKeyboard(HRAWINPUT input) {
	RAWINPUT raw;
	UINT size = sizeof(raw);
	if (GetRawInputData(input, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) == (UINT)-1 ||
		raw.header.dwType != RIM_TYPEKEYBOARD || raw.header.hDevice == NULL) //no device: sent by software
		return;
	if (raw.data.keyboard.Message != WM_KEYDOWN && raw.data.keyboard.Message != WM_SYSKEYDOWN)
		return;
	//the hook runs first, so its count is ahead or equal unless it missed keys; allow one to be safe
	if ((int)(++_rawKeyDowns - _hookKeyDowns) < 2)
		return;
	//an elevated window gets input the hooks are not allowed to see
	HWND foreground = GetForegroundWindow();
	if (foreground && EasyKeyHelper::isWindowElevated(foreground)) {
		_hookKeyDowns = _rawKeyDowns;
		return;
	}
	reinstallHooks("hooks reinstalled (raw input)");
}

static LRESULT CALLBACK rawInputProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_INPUT)
		onRawKeyboard((HRAWINPUT)lParam);
	return DefWindowProc(hWnd, message, wParam, lParam);
}

static void watchRawKeyboard() {
	WNDCLASS wc = {};
	wc.lpfnWndProc = rawInputProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = _T("EasyKeyRawInput");
	RegisterClass(&wc);
	HWND window = CreateWindow(wc.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, wc.hInstance, NULL);
	if (!window)
		return;
	RAWINPUTDEVICE device = {};
	device.usUsagePage = 0x01; //generic desktop
	device.usUsage = 0x06;     //keyboard
	device.dwFlags = RIDEV_INPUTSINK; //also while other apps are in front
	device.hwndTarget = window;
	RegisterRawInputDevices(&device, 1, sizeof(device));
}

LRESULT CALLBACK keyboardHookProcess(int nCode, WPARAM wParam, LPARAM lParam) {
	_lastHookTick = GetTickCount();
	keyboardData = (KBDLLHOOKSTRUCT *)lParam;
	if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && !(keyboardData->flags & LLKHF_INJECTED))
		_hookKeyDowns++;
	//ignore my event
	if (keyboardData->dwExtraInfo == EASYKEY_EXTRA_INFO) {
		return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
	}
	
	//ignore if IME pad is open when typing Japanese/Chinese...
	HWND hWnd = GetForegroundWindow();
	DWORD_PTR isImeON = 0;
	DWORD foregroundProcess = 0;
	GetWindowThreadProcessId(hWnd, &foregroundProcess);
	//EasyKey's own windows (the shortcut list) take Vietnamese and never an IME
	if (foregroundProcess != GetCurrentProcessId()) {
		HWND hIME = ImmGetDefaultIMEWnd(hWnd);
		//don't block the hook on a hung window, Windows removes hooks that exceed LowLevelHooksTimeout
		SendMessageTimeout(hIME, WM_IME_CONTROL, IMC_GETOPENSTATUS, 0, SMTO_ABORTIFHUNG, 50, &isImeON);
	}
	if (vDebugKeyLog) {
		writeDebugKeyLog(wParam, isImeON);
	}
	if (isImeON) {
		return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
	}
	
	//check modifier key
	if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
		//LOG(L"Key down: %d\n", keyboardData->vkCode);
		SetModifierMask((Uint16)keyboardData->vkCode);
		if (keyboardData->vkCode == VK_ESCAPE)
			_escapeRestored = false;
	} else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
		//LOG(L"Key up: %d\n", keyboardData->vkCode);
		UnsetModifierMask((Uint16)keyboardData->vkCode);
		if (keyboardData->vkCode == VK_ESCAPE && _escapeRestored) {
			_escapeRestored = false;
			return -1;
		}
	}
	if (!_isFlagKey && wParam != WM_KEYUP && wParam != WM_SYSKEYUP)
		_keycode = (Uint16)keyboardData->vkCode;

	//switch language shortcut; convert hotkey
	if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && !_isFlagKey && _keycode != 0) {
		if (GET_SWITCH_KEY(vSwitchKeyStatus) != _keycode && GET_SWITCH_KEY(convertToolHotKey) != _keycode &&
			!(vSwitchInputType && GET_SWITCH_KEY(vInputTypeHotKey) == _keycode)) {
			_lastFlag = 0;
		} else {
			if (GET_SWITCH_KEY(vSwitchKeyStatus) == _keycode && checkHotKey(vSwitchKeyStatus, GET_SWITCH_KEY(vSwitchKeyStatus) != 0xFE)) {
				switchLanguage();
				maskMenuKey();
				_hasJustUsedHotKey = true;
				_keycode = 0;
				return -1;
			}
			if (GET_SWITCH_KEY(convertToolHotKey) == _keycode && checkHotKey(convertToolHotKey, GET_SWITCH_KEY(convertToolHotKey) != 0xFE)) {
				AppDelegate::getInstance()->onQuickConvert();
				maskMenuKey();
				_hasJustUsedHotKey = true;
				_keycode = 0;
				return -1;
			}
			if (GET_SWITCH_KEY(vInputTypeHotKey) == _keycode && isInputTypeHotKey(true)) {
				switchInputType();
				maskMenuKey();
				_hasJustUsedHotKey = true;
				_keycode = 0;
				return -1;
			}
		}
		_hasJustUsedHotKey = _lastFlag != 0;
	} else if (_isFlagKey) {
		if (_lastFlag == 0 || _lastFlag < _flag)
			_lastFlag = _flag;
		else if (_lastFlag > _flag) {
			//check switch
			if (checkHotKey(vSwitchKeyStatus, GET_SWITCH_KEY(vSwitchKeyStatus) != 0xFE)) {
				switchLanguage();
				_hasJustUsedHotKey = true;
			}
			if (checkHotKey(convertToolHotKey, GET_SWITCH_KEY(convertToolHotKey) != 0xFE)) {
				AppDelegate::getInstance()->onQuickConvert();
				_hasJustUsedHotKey = true;
			}
			if (isInputTypeHotKey(true)) {
				switchInputType();
				_hasJustUsedHotKey = true;
			}
			_lastFlag = _flag;
			_hasJustUsedHotKey = false;
		}
		_keycode = 0;
		return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
	}

	//game mode: keys go straight to the game (skills on Q, W, E, R...) until Enter opens the chat;
	//Enter (send) or Esc (close) ends the chat
	if (_inGame && vLanguage && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
		if ((_keycode == VK_RETURN || _keycode == VK_ESCAPE) && !(_flag & (MASK_ALT | MASK_CONTROL))) { //not Alt + Enter
			_gameChat = _keycode == VK_RETURN && !_gameChat;
			startNewSession();
			return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
		}
		if (!_gameChat)
			return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
	}

	//if is in english mode
	if (vLanguage == 0) {
		if (vUseMacro && vUseMacroInEnglishMode && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
			vEnglishMode(((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) ? vKeyEventState::KeyDown : vKeyEventState::MouseDown),
				_keycode,
				(_flag & MASK_SHIFT) || (_flag & MASK_CAPITAL),
				OTHER_CONTROL_KEY);

			if (pData->code == vReplaceMaro) { //handle macro in english mode
				handleMacro();
				return NULL;
			}
		}
		return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
	}

	//handle keyboard
	if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
		//send event signal to Engine
		vKeyHandleEvent(vKeyEvent::Keyboard,
						vKeyEventState::KeyDown,
						_keycode,
						(_flag & MASK_SHIFT && _flag & MASK_CAPITAL) ? 0 : (_flag & MASK_SHIFT ? 1 : (_flag & MASK_CAPITAL ? 2 : 0)),
						OTHER_CONTROL_KEY);
		if (pData->code == vDoNothing) { //do nothing
			if (IS_DOUBLE_CODE(vCodeTable)) { //VNI
				if (pData->extCode == 1) { //break key
					_syncKey.clear();
				} else if (pData->extCode == 2) { //delete key
					if (_syncKey.size() > 0) {
						if (_syncKey.back() > 1 && (vCodeTable == 2 || vCodeTable == 3)) {
							//send one more backspace
							SendInput(2, backspaceEvent, sizeof(INPUT));
						}
						_syncKey.pop_back();
					}
				} else if (pData->extCode == 3) { //normal key
					InsertKeyLength(1);
				}
			}
			return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
		} else if (pData->code == vWillProcess || pData->code == vRestore || pData->code == vRestoreAndStartNewSession) { //handle result signal
			_escapeRestored = _keycode == VK_ESCAPE;
			//fix autocomplete
			if (vFixRecommendBrowser && pData->extCode != 4) {
				if (vFixChromiumBrowser && 
					std::find(_chromiumBrowser.begin(), _chromiumBrowser.end(), EasyKeyHelper::getLastAppExecuteName()) != _chromiumBrowser.end()) {
					//select the last character so the backspace takes the suggestion with it; with nothing to delete
					//(e.g. upper case first char) selecting would eat the character before the caret.
					//Always send the backspaces: pages that track keys themselves (typing tests) ignore the
					//selection, so typing over it would leave the old letter there ("so6" gave "soô")
					if (pData->backspaceCount > 0)
						SendCombineKey(KEY_LEFT_SHIFT, KEY_LEFT, 0, KEYEVENTF_EXTENDEDKEY);
				} else {
					SendEmptyCharacter();
					pData->backspaceCount++;
				}
			}
			
			//send backspace
			if (pData->backspaceCount > 0 && pData->backspaceCount < MAX_BUFF) {
				for (_i = 0; _i < pData->backspaceCount; _i++) {
					SendBackspace();
				}
			}

			//send new character
			if (!vSendKeyStepByStep) {
				SendNewCharString();
			} else {
				if (pData->newCharCount > 0 && pData->newCharCount <= MAX_BUFF) {
					for (int i = pData->newCharCount - 1; i >= 0; i--) {
						SendKeyCode(pData->charData[i]);
					}
				}
				if (pData->code == vRestore || pData->code == vRestoreAndStartNewSession) {
					SendKeyCode(_keycode | ((_flag & MASK_CAPITAL) || (_flag & MASK_SHIFT) ? CAPS_MASK : 0));
				}
				if (pData->code == vRestoreAndStartNewSession) {
					startNewSession();
				}
			}
		} else if (pData->code == vReplaceMaro) { //MACRO
			handleMacro();
		}
		return -1; //consume event
	}
	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK mouseHookProcess(int nCode, WPARAM wParam, LPARAM lParam) {
	_lastHookTick = GetTickCount();
	mouseData = (MSLLHOOKSTRUCT *)lParam;
	switch (wParam) {
	case WM_LBUTTONDOWN:
	
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_NCXBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	case WM_NCXBUTTONUP:
		//send event signal to Engine
		vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
		if (IS_DOUBLE_CODE(vCodeTable)) { //VNI
			_syncKey.clear();
		}
		break;
	}
	return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

VOID CALLBACK winEventProcCallback(HWINEVENTHOOK hWinEventHook, DWORD dwEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
	if (hwnd && EasyKeyHelper::isWindowElevated(hwnd))
		AppDelegate::getInstance()->onElevatedWindow(hwnd);
	//typing in another window usually starts a new sentence
	startNewSession();
	vStartSentence();
	_inGame = !_gameApps.empty() && hwnd && isGameApp(exeNameOfWindow(hwnd));
	_gameChat = false;
	//smart switch key
	if (vUseSmartSwitchKey || vRememberCode) {
		string& exe = EasyKeyHelper::getFrontMostAppExecuteName();
		//Explorer counts as the app the user came from; an app whose name can't be read
		//keeps what is on now instead of sharing one entry with every other such app
		if (!EasyKeyHelper::isRealApp(exe))
			return;
		_languageTemp = getAppInputMethodStatus(exe, vLanguage | (vCodeTable << 1));
		if (vUseSmartSwitchKey && (_languageTemp & 0x01) != vLanguage) {
			if (_languageTemp != -1) {
				vLanguage = _languageTemp & 0x01; //the other bits hold the code table
				AppDelegate::getInstance()->onInputMethodChangedFromHotKey();
			} else {
				saveSmartSwitchKeyData();
			}
		}
		startNewSession();
		if (vRememberCode && (_languageTemp >> 1) != vCodeTable) { //for remember table code feature
			if (_languageTemp != -1) {
				AppDelegate::getInstance()->onTableCode(_languageTemp >> 1, false);
			} else {
				saveSmartSwitchKeyData();
			}
		}
	}
}
