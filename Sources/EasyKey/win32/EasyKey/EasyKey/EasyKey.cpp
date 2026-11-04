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
#include "GameCandidateFinder.h"
#include "../../../app-core/GameModeState.h"
#include "../../../app-core/PasswordState.h"
#include "../../../app-core/ExeName.h"
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <ole2.h>
#include <UIAutomation.h>
#include <oleacc.h>

#pragma comment(lib, "imm32")
#pragma comment(lib, "oleacc.lib")
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
extern int vPasswordFieldBypass;
extern int vPasswordFieldUIA;

static HHOOK hKeyboardHook;
static HHOOK hMouseHook;
static DWORD _lastHookTick = 0; //last time either hook ran, see checkHooks
//key presses from a real keyboard, counted by the keyboard hook and by raw input, see onRawKeyboard
static DWORD _rawKeyDowns = 0, _hookKeyDowns = 0;
static HWINEVENTHOOK hSystemEvent;
static HWINEVENTHOOK hFocusEvent;
enum PasswordStateSource {
	PW_SRC_NONE = 0,
	PW_SRC_WIN32 = 1,
	PW_SRC_UIA = 2,
	PW_SRC_MSAA = 3
};

//the state word codec and the next-state decisions live in app-core/PasswordState
static std::atomic<uint32_t> _passwordStateWord{ makePasswordStateWord(PW_NORMAL, PW_SRC_NONE) };

static std::atomic<uint64_t> _probeGen{ 0 };
static std::atomic<uint64_t> _pendingDeadlineTick{ 0 };
static std::atomic<bool> _passwordStateDirty{ false };
static HANDLE _probeThread = NULL;
static HANDLE _probeWakeEvent = NULL;
static HANDLE _probeStopEvent = NULL;
static constexpr uint64_t PW_PENDING_TIMEOUT_MS = kPasswordPendingTimeoutMs; //kept for the worker note below; the value lives in app-core/PasswordState
static constexpr DWORD PW_WORKER_SHUTDOWN_TIMEOUT_MS = 200;
static void stopPasswordProbeWorker();
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
//the engine takes its settings as a vKeyConfig (engine/Engine.h); this layer
//keeps owning the settings as globals and copies them into one persistent
//config immediately before every engine call, so the engine always sees the
//latest values at the moment of the event (plan point 2)
static vKeyConfig _engineConfig;
static const vKeyConfig& currentEngineConfig() {
	_engineConfig.language = vLanguage;
	_engineConfig.mixedEnglish = vMixedEnglish;
	_engineConfig.mixedFixLast = vMixedFixLast;
	_engineConfig.inputType = vInputType;
	_engineConfig.freeMark = vFreeMark;
	_engineConfig.codeTable = vCodeTable;
	_engineConfig.switchKeyStatus = vSwitchKeyStatus;
	_engineConfig.checkSpelling = vCheckSpelling;
	_engineConfig.useModernOrthography = vUseModernOrthography;
	_engineConfig.quickTelex = vQuickTelex;
	_engineConfig.restoreIfWrongSpelling = vRestoreIfWrongSpelling;
	_engineConfig.fixRecommendBrowser = vFixRecommendBrowser;
	_engineConfig.useMacro = vUseMacro;
	_engineConfig.useMacroInEnglishMode = vUseMacroInEnglishMode;
	_engineConfig.autoCapsMacro = vAutoCapsMacro;
	_engineConfig.useSmartSwitchKey = vUseSmartSwitchKey;
	_engineConfig.upperCaseFirstChar = vUpperCaseFirstChar;
	_engineConfig.allowConsonantZFWJ = vAllowConsonantZFWJ;
	_engineConfig.quickStartConsonant = vQuickStartConsonant;
	_engineConfig.quickEndConsonant = vQuickEndConsonant;
	_engineConfig.rememberCode = vRememberCode;
	_engineConfig.otherLanguage = vOtherLanguage;
	_engineConfig.restoreOnEscape = vRestoreOnEscape;
	_engineConfig.passwordFieldBypass = vPasswordFieldBypass;
	return _engineConfig;
}

//game mode, see isGameApp; the binding/chat state machine itself lives in
//app-core/GameModeState (this layer keeps the HWND mapping and the side effects)
static vector<string> _gameApps;
static GameModeState _game;
static uint64_t gameWindowId(HWND hwnd) { return (uint64_t)(uintptr_t)hwnd; }
static HWND gameBoundHwnd() { return (HWND)(uintptr_t)_game.windowId; }
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
static bool SetModifierMask(const Uint16& vkCode);
static bool UnsetModifierMask(const Uint16& vkCode);
static void enterGameState();
static void leaveGameState();
static void clearGameBinding();
static void bindGameWindow(HWND hwnd, bool confirmed);

void EasyKeyFree() {
	stopPasswordProbeWorker();
	applyPasswordFeatureConfigChanges();
	clearGameBinding();
	UnhookWindowsHookEx(hMouseHook);
	UnhookWindowsHookEx(hKeyboardHook);
	UnhookWinEvent(hSystemEvent);
	UnhookWinEvent(hFocusEvent);
}

void EasyKeyInit() {
	APP_GET_DATA(vLanguage, 1);
	vLanguage = vLanguage ? 1 : 0; //older versions could save the code table bits here too
	APP_GET_DATA(vMixedEnglish, 0);
	APP_GET_DATA(vMixedFixLast, 0);
	APP_GET_DATA(vTwoModesOnly, 0);
	if (vTwoModesOnly && vMixedEnglish)
		vMixedEnglish = 0;
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
	APP_GET_DATA(vPasswordFieldBypass, 1);
	APP_GET_DATA(vPasswordFieldUIA, 1);
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
			if (c == '\n' || c == '\r') {
				if (!name.empty() && name.length() <= MAX_PATH)
					_gameApps.push_back(name);
				name.clear();
			} else if ((unsigned char)c >= 32 && (unsigned char)c != 127) {
				name += c;
			}
		}
		if (!name.empty() && name.length() <= MAX_PATH)
			_gameApps.push_back(name);
	}

	//init hook
	HINSTANCE hInstance = GetModuleHandle(NULL);
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHookProcess, hInstance, 0);
	hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseHookProcess, hInstance, 0);
	hSystemEvent = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, winEventProcCallback, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	hFocusEvent = SetWinEventHook(EVENT_OBJECT_FOCUS, EVENT_OBJECT_FOCUS, NULL, winEventProcCallback, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	updatePasswordFieldState(GetForegroundWindow());
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
	if (exe.empty())
		return false;
	return std::find_if(_gameApps.begin(), _gameApps.end(), [&](const string& item) {
		return isSameExe(item, exe);
	}) != _gameApps.end();
}

struct FindGameWindowCtx {
	const string* targetExe = nullptr;
	DWORD selfPid = 0;
	HWND found = NULL;
};

static BOOL CALLBACK findGameWindowEnumProc(HWND hwnd, LPARAM lParam) {
	if (!IsWindowVisible(hwnd) || IsIconic(hwnd))
		return TRUE;
	FindGameWindowCtx* ctx = reinterpret_cast<FindGameWindowCtx*>(lParam);
	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (!pid || pid == ctx->selfPid)
		return TRUE;
	if (isSameExe(exeNameOfWindow(hwnd), *ctx->targetExe)) {
		ctx->found = hwnd;
		return FALSE;
	}
	return TRUE;
}

static HWND findRunningWindowForExe(const string& exe) {
	if (exe.empty())
		return NULL;
	FindGameWindowCtx ctx{ &exe, GetCurrentProcessId(), NULL };
	EnumWindows(findGameWindowEnumProc, reinterpret_cast<LPARAM>(&ctx));
	return ctx.found;
}

void setGameApp(const string& exe, const bool& on, HWND candidateHwnd) {
	if (exe.empty() || exe.length() > MAX_PATH)
		return;
	for (char ch : exe) {
		if ((unsigned char)ch < 32 || (unsigned char)ch == 127)
			return;
	}
	_gameApps.erase(std::remove_if(_gameApps.begin(), _gameApps.end(), [&](const string& item) {
		return isSameExe(item, exe);
	}), _gameApps.end());
	if (on)
		_gameApps.push_back(exe);
	string list;
	for (auto& name : _gameApps)
		list += name + "\n";
	EasyKeyHelper::setRegBinary(_T("gameApps"), (BYTE*)list.data(), (int)list.size());
	if (on) {
		HWND fg = GetForegroundWindow();
		if (fg && isSameExe(exeNameOfWindow(fg), exe)) {
			bindGameWindow(fg, true);
			return;
		}
		if (candidateHwnd && IsWindow(candidateHwnd)) {
			bindGameWindow(candidateHwnd, false);
			return;
		}
		HWND lastFg = GameCandidateFinder::getLastRealForeground();
		if (lastFg && IsWindow(lastFg) && isSameExe(exeNameOfWindow(lastFg), exe)) {
			bindGameWindow(lastFg, false);
			return;
		}
		HWND found = findRunningWindowForExe(exe);
		if (found) {
			bindGameWindow(found, false);
			return;
		}
		if (_game.windowId != 0 && IsWindow(gameBoundHwnd()) && isGameApp(exeNameOfWindow(gameBoundHwnd()))) {
			return;
		}
		clearGameBinding();
		return;
	}
	if (_game.windowId != 0 && (!IsWindow(gameBoundHwnd()) || !isGameApp(exeNameOfWindow(gameBoundHwnd())))) {
		clearGameBinding();
	}
}

bool isPasswordFieldActive() {
	return getPasswordState(_passwordStateWord.load(std::memory_order_acquire)) != PW_NORMAL;
}

static HWND getFocusedControlHandle(HWND foregroundHwnd) {
	if (!foregroundHwnd || !IsWindow(foregroundHwnd))
		return NULL;

	DWORD threadId = GetWindowThreadProcessId(foregroundHwnd, NULL);
	if (!threadId)
		return foregroundHwnd;

	GUITHREADINFO gti = { sizeof(GUITHREADINFO) };
	if (GetGUIThreadInfo(threadId, &gti) && gti.hwndFocus) {
		return gti.hwndFocus;
	}
	return foregroundHwnd;
}

static bool isWin32PasswordField(HWND hwnd) {
	if (!hwnd || !IsWindow(hwnd))
		return false;

	TCHAR cls[64] = { 0 };
	if (!GetClassName(hwnd, cls, 64))
		return false;

	if (_tcsicmp(cls, _T("Edit")) != 0)
		return false;

	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (style & ES_PASSWORD)
		return true;

	DWORD_PTR passChar = 0;
	if (SendMessageTimeout(hwnd, EM_GETPASSWORDCHAR, 0, 0,
		SMTO_ABORTIFHUNG | SMTO_BLOCK, 50, &passChar) && passChar != 0) {
		return true;
	}

	return false;
}


struct PasswordFocusKey {
	HWND hwnd = NULL;
	DWORD pid = 0;
	LONG idObject = 0;
	LONG idChild = 0;

	bool operator==(const PasswordFocusKey& other) const {
		return hwnd == other.hwnd &&
			pid == other.pid &&
			idObject == other.idObject &&
			idChild == other.idChild;
	}

	bool operator!=(const PasswordFocusKey& other) const {
		return !(*this == other);
	}
};

static constexpr uint64_t PW_TYPING_SUPPRESS_MS = 150;
static constexpr uint64_t PW_NAVIGATION_SIGNAL_TTL_MS = 250;
static constexpr uint64_t PW_MOUSE_REPROBE_THROTTLE_MS = 100;
static constexpr DWORD PW_MOUSE_REPROBE_DEBOUNCE_MS = 40;
static constexpr DWORD PW_PROBE_RETRY_DELAY_MS = 10;
static PasswordFocusKey _lastFocusKey{};
static PasswordFocusKey _lastCharFocusKey{};
static uint64_t _lastCharKeyTick = 0;
static bool _webInputContext = false;
static bool _hadNavigationSignal = false;
static uint64_t _lastNavigationTick = 0;
static uint64_t _lastProbeTick = 0;
static std::atomic<uintptr_t> _probeTargetHwnd{ 0 };
static std::atomic<LONG> _probeTargetObjectId{ 0 };
static std::atomic<LONG> _probeTargetChildId{ 0 };
static std::atomic<DWORD> _probePreDelayMs{ 0 };

static void recordNavigationSignal(uint64_t nowTick) {
	_hadNavigationSignal = true;
	_lastNavigationTick = nowTick;
}

static void clearNavigationSignal() {
	_hadNavigationSignal = false;
	_lastNavigationTick = 0;
}

static bool hasFreshNavigationSignal(uint64_t nowTick) {
	return _hadNavigationSignal &&
		nowTick >= _lastNavigationTick &&
		(nowTick - _lastNavigationTick) < PW_NAVIGATION_SIGNAL_TTL_MS;
}

static bool isAcceptedFocusObjectId(LONG idObject) {
	return idObject == OBJID_WINDOW ||
		idObject == OBJID_CLIENT ||
		idObject < -12;
}

static bool isKnownWebContentWidgetClass(HWND hwnd) {
	if (!hwnd || !IsWindow(hwnd))
		return false;

	TCHAR cls[64] = { 0 };
	if (!GetClassName(hwnd, cls, 64))
		return false;

	return _tcsicmp(cls, _T("Chrome_RenderWidgetHostHWND")) == 0 ||
		_tcsicmp(cls, _T("MozillaWindowClass")) == 0;
}

static bool isKnownWebBrowserContainerClass(HWND hwnd) {
	if (!hwnd || !IsWindow(hwnd))
		return false;

	TCHAR cls[64] = { 0 };
	if (!GetClassName(hwnd, cls, 64))
		return false;

	return _tcsicmp(cls, _T("Chrome_WidgetWin_1")) == 0 ||
		_tcsicmp(cls, _T("Chrome_WidgetWin_0")) == 0;
}

static bool isWebInputContextForKey(const PasswordFocusKey& key) {
	if (!isAcceptedFocusObjectId(key.idObject))
		return false;

	// 1. Dedicated content widget (Chrome_RenderWidgetHostHWND or MozillaWindowClass)
	if (isKnownWebContentWidgetClass(key.hwnd))
		return true;

	// 2. Container window (Chrome_WidgetWin_1/0) with explicit DOM element indicators:
	//    - idObject < -12 (Chromium internal accessibility node ID)
	//    - OR idChild != CHILDID_SELF and idChild != 0 (DOM child element index)
	if (isKnownWebBrowserContainerClass(key.hwnd)) {
		if (key.idObject < -12)
			return true;
		if (key.idChild != CHILDID_SELF && key.idChild != 0)
			return true;
	}

	return false;
}

static bool isFocusNavigationKey(DWORD vkCode) {
	switch (vkCode) {
	case VK_TAB:
	case VK_RETURN:
	case VK_ESCAPE:
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_HOME:
	case VK_END:
		return true;
	default:
		return false;
	}
}

static bool isPrintableContentKey(DWORD vkCode) {
	if (vkCode >= '0' && vkCode <= '9') return true;
	if (vkCode >= 'A' && vkCode <= 'Z') return true;
	if (vkCode >= VK_NUMPAD0 && vkCode <= VK_DIVIDE) return true;
	if (vkCode >= VK_OEM_1 && vkCode <= VK_OEM_3) return true;
	if (vkCode >= VK_OEM_4 && vkCode <= VK_OEM_8) return true;
	if (vkCode == VK_SPACE || vkCode == VK_BACK) return true;
	return false;
}

static HRESULT queryUIAIsPassword(IUIAutomation* pUIA, IUIAutomationCacheRequest* pCacheReq, bool* outIsPass) {
	*outIsPass = false;
	if (!pUIA)
		return E_POINTER;

	IUIAutomationElement* pEl = NULL;
	HRESULT hr = E_FAIL;
	if (pCacheReq) {
		hr = pUIA->GetFocusedElementBuildCache(pCacheReq, &pEl);
		if (SUCCEEDED(hr) && pEl) {
			BOOL isPass = FALSE;
			if (SUCCEEDED(pEl->get_CachedIsPassword(&isPass)) && isPass) {
				*outIsPass = true;
			}
			pEl->Release();
			return hr;
		}
	}
	hr = pUIA->GetFocusedElement(&pEl);
	if (SUCCEEDED(hr) && pEl) {
		BOOL isPass = FALSE;
		if (SUCCEEDED(pEl->get_CurrentIsPassword(&isPass)) && isPass) {
			*outIsPass = true;
		}
		pEl->Release();
	}
	return hr;
}

static HRESULT queryMSAAProtected(HWND hwnd, LONG idObject, LONG idChild, bool* outIsPass) {
	*outIsPass = false;
	if (!hwnd || !IsWindow(hwnd))
		return E_INVALIDARG;

	IAccessible* pAcc = NULL;
	VARIANT varChild;
	VariantInit(&varChild);
	HRESULT hr = AccessibleObjectFromEvent(hwnd, idObject, idChild, &pAcc, &varChild);
	if (SUCCEEDED(hr) && pAcc) {
		VARIANT varState;
		VariantInit(&varState);
		hr = pAcc->get_accState(varChild, &varState);
		if (SUCCEEDED(hr) && varState.vt == VT_I4) {
			if ((varState.lVal & STATE_SYSTEM_PROTECTED) != 0) {
				*outIsPass = true;
			}
		}
		VariantClear(&varState);
		pAcc->Release();
	}
	VariantClear(&varChild);
	return hr;
}

static void applyPasswordProbeResultFromWorker(uint64_t resultGen, bool isPass, PasswordStateSource source) {
	if (resultGen != _probeGen.load(std::memory_order_acquire))
		return;

	uint32_t oldWord = _passwordStateWord.load(std::memory_order_acquire);
	for (;;) {
		//the decision itself is pure (app-core/PasswordState); this loop only
		//races it against the atomic word. PW_PASSWORD never reaches a change:
		//sticky password on same focus key/generation, do not downgrade
		PasswordTransition transition = passwordAfterProbeResult(getPasswordState(oldWord), isPass, source);
		if (!transition.changesWord)
			break;
		uint32_t nextWord = makePasswordStateWord(transition.nextState, transition.nextSource);
		if (_passwordStateWord.compare_exchange_weak(oldWord, nextWord, std::memory_order_acq_rel)) {
			if (transition.markDirty)
				_passwordStateDirty.store(true, std::memory_order_release);
			break;
		}
	}
}

// Note: UIA GetFocusedElementBuildCache and MSAA AccessibleObjectFromEvent are synchronous
// cross-process COM calls on this background worker thread. If a target accessibility provider
// hangs, this worker thread may block on IPC; however, the keyboard hook thread never blocks
// and automatically expires PW_PENDING -> PW_NORMAL after PW_PENDING_TIMEOUT_MS (60 ms),
// incrementing _probeGen so any late worker completion is safely discarded.
static DWORD WINAPI passwordProbeWorkerProc(LPVOID) {
	HRESULT hrCom = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	bool comInitialized = SUCCEEDED(hrCom);

	IUIAutomation* pUIA = NULL;
	IUIAutomationCacheRequest* pCacheReq = NULL;
	if (comInitialized) {
		if (SUCCEEDED(CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pUIA)) && pUIA) {
			if (SUCCEEDED(pUIA->CreateCacheRequest(&pCacheReq)) && pCacheReq) {
				pCacheReq->AddProperty(UIA_IsPasswordPropertyId);
				pCacheReq->put_TreeScope(TreeScope_Element);
				pCacheReq->put_AutomationElementMode(AutomationElementMode_None);
			}
		}
	}

	HANDLE waitHandles[2] = { _probeStopEvent, _probeWakeEvent };
	for (;;) {
		DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);
		if (waitResult == WAIT_OBJECT_0 || waitResult != WAIT_OBJECT_0 + 1) {
			break;
		}

		uint64_t gen = _probeGen.load(std::memory_order_acquire);
		DWORD preDelayMs = _probePreDelayMs.exchange(0, std::memory_order_acq_rel);
		if (preDelayMs > 0) {
			if (WaitForSingleObject(_probeStopEvent, preDelayMs) == WAIT_OBJECT_0) {
				break;
			}
			// Coalesce with any newer probe scheduled during the debounce window
			gen = _probeGen.load(std::memory_order_acquire);
		}

		HWND targetHwnd = (HWND)_probeTargetHwnd.load(std::memory_order_acquire);
		LONG targetObj = _probeTargetObjectId.load(std::memory_order_acquire);
		LONG targetChild = _probeTargetChildId.load(std::memory_order_acquire);

		bool isPass = false;
		PasswordStateSource passSource = PW_SRC_NONE;
		for (int attempt = 0; attempt < 2; attempt++) {
			if (gen != _probeGen.load(std::memory_order_acquire))
				break;

			bool uiaPass = false;
			HRESULT hrUia = queryUIAIsPassword(pUIA, pCacheReq, &uiaPass);
			if (SUCCEEDED(hrUia) && uiaPass) {
				isPass = true;
				passSource = PW_SRC_UIA;
				break;
			}

			// When UIA returns false (or error) in web context, always fall back to MSAA STATE_SYSTEM_PROTECTED
			bool msaaPass = false;
			HRESULT hrMsaa = queryMSAAProtected(targetHwnd, targetObj, targetChild, &msaaPass);
			if (SUCCEEDED(hrMsaa) && msaaPass) {
				isPass = true;
				passSource = PW_SRC_MSAA;
				break;
			}

			if (SUCCEEDED(hrUia) || SUCCEEDED(hrMsaa)) {
				break; // Definitive non-password result
			}

			// Both UIA and MSAA failed with error: retry once after short wait unless stopped
			if (attempt == 0 && WaitForSingleObject(_probeStopEvent, PW_PROBE_RETRY_DELAY_MS) == WAIT_OBJECT_0) {
				break;
			}
		}

		applyPasswordProbeResultFromWorker(gen, isPass, passSource);
	}

	if (pCacheReq) {
		pCacheReq->Release();
	}
	if (pUIA) {
		pUIA->Release();
	}
	if (comInitialized) {
		CoUninitialize();
	}
	return 0;
}

static void startPasswordProbeWorkerIfNeeded() {
	if (!vPasswordFieldBypass || !vPasswordFieldUIA)
		return;
	if (_probeThread != NULL)
		return;
	if (!_probeStopEvent)
		_probeStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	else
		ResetEvent(_probeStopEvent);
	if (!_probeWakeEvent)
		_probeWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	else
		ResetEvent(_probeWakeEvent);
	if (_probeStopEvent && _probeWakeEvent) {
		_probeThread = CreateThread(NULL, 0, passwordProbeWorkerProc, NULL, 0, NULL);
	}
}

static void stopPasswordProbeWorker() {
	_probeGen.fetch_add(1, std::memory_order_acq_rel);
	uint32_t prevWord = _passwordStateWord.exchange(makePasswordStateWord(PW_NORMAL, PW_SRC_NONE), std::memory_order_acq_rel);
	if (getPasswordState(prevWord) != PW_NORMAL) {
		_passwordStateDirty.store(true, std::memory_order_release);
	}
	if (_probeThread) {
		if (_probeStopEvent) {
			SetEvent(_probeStopEvent);
		}
		WaitForSingleObject(_probeThread, PW_WORKER_SHUTDOWN_TIMEOUT_MS);
		CloseHandle(_probeThread);
		_probeThread = NULL;
	}
	if (_probeWakeEvent) {
		CloseHandle(_probeWakeEvent);
		_probeWakeEvent = NULL;
	}
	if (_probeStopEvent) {
		CloseHandle(_probeStopEvent);
		_probeStopEvent = NULL;
	}
}

static void resetTypingSessionOnHookThread() {
	exitGameChat();
	_escapeRestored = false;
	_lastFlag = 0;
	_hasJustUsedHotKey = false;
	startNewSession();
	vStartSentence();
	if (IS_DOUBLE_CODE(vCodeTable)) {
		_syncKey.clear();
	}
}

static bool isPasswordRawActiveOnHookThread(uint64_t nowTick) {
	uint32_t word = _passwordStateWord.load(std::memory_order_acquire);
	int state = getPasswordState(word);
	if (state == PW_PENDING) {
		uint64_t deadline = _pendingDeadlineTick.load(std::memory_order_acquire);
		PasswordTransition transition = passwordAfterPendingTimeout(state, nowTick, deadline);
		if (transition.changesWord) {
			uint32_t expected = word;
			uint32_t normalWord = makePasswordStateWord(transition.nextState, transition.nextSource);
			if (_passwordStateWord.compare_exchange_strong(expected, normalWord, std::memory_order_acq_rel)) {
				if (transition.bumpGeneration)
					_probeGen.fetch_add(1, std::memory_order_acq_rel);
				if (transition.markDirty)
					_passwordStateDirty.store(true, std::memory_order_release);
				state = transition.nextState;
			} else {
				state = getPasswordState(_passwordStateWord.load(std::memory_order_acquire));
			}
		}
	}
	if (_passwordStateDirty.exchange(false, std::memory_order_acq_rel)) {
		resetTypingSessionOnHookThread();
	}
	return state != PW_NORMAL;
}

void applyPasswordFeatureConfigChanges() {
	static uint64_t _lastRegCheckTick = 0;
	uint64_t now = GetTickCount64();
	if (_lastRegCheckTick == 0 || (now - _lastRegCheckTick) > 200) {
		_lastRegCheckTick = now;
		vPasswordFieldUIA = EasyKeyHelper::getRegInt(_T("vPasswordFieldUIA"), 1);
		vPasswordFieldBypass = EasyKeyHelper::getRegInt(_T("vPasswordFieldBypass"), 1);
	}

	if (!vPasswordFieldBypass) {
		stopPasswordProbeWorker();
		invalidatePasswordFieldCache();
		return;
	}

	if (!vPasswordFieldUIA) {
		stopPasswordProbeWorker();
		uint32_t oldWord = _passwordStateWord.load(std::memory_order_acquire);
		for (;;) {
			int state = getPasswordState(oldWord);
			int src = getPasswordSource(oldWord);
			if (passwordResetWhenUiaDisabled(state, src)) {
				uint32_t normalWord = makePasswordStateWord(PW_NORMAL, PW_SRC_NONE);
				if (_passwordStateWord.compare_exchange_weak(oldWord, normalWord, std::memory_order_acq_rel)) {
					_probeGen.fetch_add(1, std::memory_order_acq_rel);
					_passwordStateDirty.store(true, std::memory_order_release);
					break;
				}
			} else {
				break;
			}
		}
	}
}

static bool schedulePasswordProbeIfNeeded(const PasswordFocusKey& key, bool force, DWORD preDelayMs = 0) {
	if (!vPasswordFieldBypass || !vPasswordFieldUIA)
		return false;
	if (!isWebInputContextForKey(key))
		return false;

	startPasswordProbeWorkerIfNeeded();
	if (!_probeThread || !_probeWakeEvent)
		return false;

	int oldState = getPasswordState(_passwordStateWord.load(std::memory_order_acquire));
	uint64_t nowTick = GetTickCount64();
	PasswordSchedule schedule = passwordOnProbeScheduled(oldState, force, nowTick);
	if (!schedule.schedule) {
		// Sticky password on same focus key: do not downgrade
		return true;
	}

	_lastProbeTick = nowTick;
	_probeTargetHwnd.store((uintptr_t)key.hwnd, std::memory_order_release);
	_probeTargetObjectId.store(key.idObject, std::memory_order_release);
	_probeTargetChildId.store(key.idChild, std::memory_order_release);
	_probePreDelayMs.store(preDelayMs, std::memory_order_release);
	_pendingDeadlineTick.store(schedule.deadline, std::memory_order_release);
	_probeGen.fetch_add(1, std::memory_order_acq_rel);

	uint32_t pendingWord = makePasswordStateWord(PW_PENDING, PW_SRC_NONE);
	uint32_t prevWord = _passwordStateWord.exchange(pendingWord, std::memory_order_acq_rel);
	if (getPasswordState(prevWord) != PW_PENDING) {
		_passwordStateDirty.store(true, std::memory_order_release);
	}

	SetEvent(_probeWakeEvent);
	return true;
}

static void scheduleMouseUpReprobeIfNeeded(uint64_t nowTick, POINT pt) {
	if (!vPasswordFieldBypass || !vPasswordFieldUIA)
		return;

	// Sticky password on same focus key: never downgrade on mouse click (e.g. "Show password")
	if (getPasswordState(_passwordStateWord.load(std::memory_order_acquire)) == PW_PASSWORD)
		return;

	// Throttle mouse-up reprobes to at most 1 per PW_MOUSE_REPROBE_THROTTLE_MS (100 ms)
	if (_lastProbeTick != 0 && nowTick >= _lastProbeTick &&
		(nowTick - _lastProbeTick) < PW_MOUSE_REPROBE_THROTTLE_MS) {
		return;
	}

	HWND hClicked = WindowFromPoint(pt);
	if (!hClicked || !IsWindow(hClicked))
		return;

	// Reject clicks on browser toolbars, tabs, menus (Chrome_WidgetWin_1 itself)
	// ONLY probe if clicked directly on a web content widget
	if (!isKnownWebContentWidgetClass(hClicked))
		return;

	DWORD pid = 0;
	GetWindowThreadProcessId(hClicked, &pid);
	PasswordFocusKey targetKey{ hClicked, pid, OBJID_CLIENT, 0 };

	schedulePasswordProbeIfNeeded(targetKey, false, PW_MOUSE_REPROBE_DEBOUNCE_MS);
}

void invalidatePasswordFieldCache() {
	_lastFocusKey = PasswordFocusKey{};
	_lastCharFocusKey = PasswordFocusKey{};
	_lastCharKeyTick = 0;
	_lastProbeTick = 0;
	_webInputContext = false;
	clearNavigationSignal();
	_probePreDelayMs.store(0, std::memory_order_release);
	_probeGen.fetch_add(1, std::memory_order_acq_rel);
	uint32_t normalWord = makePasswordStateWord(PW_NORMAL, PW_SRC_NONE);
	uint32_t prevWord = _passwordStateWord.exchange(normalWord, std::memory_order_acq_rel);
	if (getPasswordState(prevWord) != PW_NORMAL) {
		_passwordStateDirty.store(true, std::memory_order_release);
	}
}

void updatePasswordFieldState(HWND hwnd, LONG idObject, LONG idChild, bool isForegroundChange) {
	applyPasswordFeatureConfigChanges();

	if (!vPasswordFieldBypass) {
		invalidatePasswordFieldCache();
		return;
	}

	HWND focusedControl = getFocusedControlHandle(hwnd);
	DWORD pid = 0;
	if (focusedControl) {
		GetWindowThreadProcessId(focusedControl, &pid);
	}

	PasswordFocusKey currentKey{ focusedControl, pid, idObject, idChild };
	if (!isForegroundChange && currentKey == _lastFocusKey)
		return;

	bool isWin32Pass = isWin32PasswordField(focusedControl);
	if (isWin32Pass) {
		_lastFocusKey = currentKey;
		_webInputContext = false;
		clearNavigationSignal();
		_probeGen.fetch_add(1, std::memory_order_acq_rel);
		uint32_t win32PassWord = makePasswordStateWord(PW_PASSWORD, PW_SRC_WIN32);
		uint32_t prevWord = _passwordStateWord.exchange(win32PassWord, std::memory_order_acq_rel);
		if (getPasswordState(prevWord) != PW_PASSWORD) {
			_passwordStateDirty.store(false, std::memory_order_release);
			resetTypingSessionOnHookThread();
		}
		return;
	}

	bool isWebCtx = isWebInputContextForKey(currentKey);
	_webInputContext = isWebCtx;

	uint64_t nowTick = GetTickCount64();
	bool sameHostWindow = (_lastCharFocusKey.hwnd != NULL &&
		focusedControl == _lastCharFocusKey.hwnd &&
		pid == _lastCharFocusKey.pid);
	bool suppressContinuousTyping = !isForegroundChange &&
		!hasFreshNavigationSignal(nowTick) &&
		isWebCtx &&
		sameHostWindow &&
		_lastCharKeyTick != 0 &&
		nowTick >= _lastCharKeyTick &&
		(nowTick - _lastCharKeyTick) < PW_TYPING_SUPPRESS_MS;

	_lastFocusKey = currentKey;
	if (suppressContinuousTyping) {
		return;
	}

	clearNavigationSignal();

	if (isWebCtx && vPasswordFieldUIA) {
		if (schedulePasswordProbeIfNeeded(currentKey, true)) {
			return;
		}
	}

	_probeGen.fetch_add(1, std::memory_order_acq_rel);
	uint32_t normalWord = makePasswordStateWord(PW_NORMAL, PW_SRC_NONE);
	uint32_t prevWord = _passwordStateWord.exchange(normalWord, std::memory_order_acq_rel);
	if (getPasswordState(prevWord) != PW_NORMAL) {
		_passwordStateDirty.store(false, std::memory_order_release);
		resetTypingSessionOnHookThread();
	}
}

void setPasswordFieldBypass(bool enabled) {
	vPasswordFieldBypass = enabled ? 1 : 0;
	applyPasswordFeatureConfigChanges();
	invalidatePasswordFieldCache();
	HWND fg = GetForegroundWindow();
	updatePasswordFieldState(fg, OBJID_WINDOW, CHILDID_SELF, true);
	resetTypingSessionOnHookThread();
}

void setPasswordFieldUIA(bool enabled) {
	vPasswordFieldUIA = enabled ? 1 : 0;
	applyPasswordFeatureConfigChanges();
	if (vPasswordFieldBypass && vPasswordFieldUIA && _webInputContext && _lastFocusKey.hwnd != NULL) {
		schedulePasswordProbeIfNeeded(_lastFocusKey, true);
	}
}

static void applyGameEffects(const GameModeEffects& fx);

void exitGameChat() {
	applyGameEffects(_game.closeChat());
}

static void resetModifierState() {
	syncModifierMask();
	_lastFlag = 0;
	_isFlagKey = false;
	_hasJustUsedHotKey = false;
	_keycode = 0;
}

static void updateModifierMaskOnly(WPARAM wParam) {
	if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
		SetModifierMask((Uint16)keyboardData->vkCode);
		if (keyboardData->vkCode == VK_ESCAPE)
			_escapeRestored = false;
	} else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
		UnsetModifierMask((Uint16)keyboardData->vkCode);
		if (keyboardData->vkCode == VK_ESCAPE)
			_escapeRestored = false;
	}
}

//runs the side effects a game state transition asks for, in the order the
//original code ran them: the chat-close session reset, then the enter/leave
//reset (modifier state included), then the chat-open session start
static void applyGameEffects(const GameModeEffects& fx) {
	if (fx.chatClosed) {
		startNewSession();
		vStartSentence();
		if (IS_DOUBLE_CODE(vCodeTable)) {
			_syncKey.clear();
		}
	}
	if (fx.sessionReset) {
		resetModifierState();
		startNewSession();
		vStartSentence();
		if (IS_DOUBLE_CODE(vCodeTable)) {
			_syncKey.clear();
		}
	}
	if (fx.chatOpened) {
		startNewSession();
	}
}

static void enterGameState() {
	applyGameEffects(_game.enterGame());
}

static void leaveGameState() {
	applyGameEffects(_game.leaveGame());
}

static void clearGameBinding() {
	applyGameEffects(_game.clearBinding());
}

static void bindGameWindow(HWND hwnd, bool confirmed) {
	if (!hwnd || !IsWindow(hwnd))
		return;
	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (!pid || pid == GetCurrentProcessId())
		return;
	applyGameEffects(_game.bind(gameWindowId(hwnd), pid, confirmed, GetTickCount64(), GetForegroundWindow() == hwnd));
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

//Vietnamese, English and Vietnamese mixed, English in turn, the order of the Settings buttons
void switchLanguage() {
	if (vTwoModesOnly) {
		if (vLanguage == 0) {
			vLanguage = 1;
			vMixedEnglish = 0;
		} else {
			vLanguage = 0;
			vMixedEnglish = 0;
		}
	} else {
		if (vLanguage == 0) {
			vLanguage = 1;
			vMixedEnglish = 0;
		} else if (!vMixedEnglish) {
			vMixedEnglish = 1;
		} else {
			vLanguage = 0;
		}
	}
	if (HAS_BEEP(vSwitchKeyStatus))
		MessageBeep(MB_OK);
	AppDelegate::getInstance()->onInputMethodChangedFromHotKey();
	AppDelegate::getInstance()->showSwitchFlyout();
	if (vUseSmartSwitchKey && !_game.inGame) {
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
	if (nCode < 0)
		return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);

	_lastHookTick = GetTickCount();
	keyboardData = (KBDLLHOOKSTRUCT *)lParam;
	if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && !(keyboardData->flags & LLKHF_INJECTED))
		_hookKeyDowns++;
	//ignore my event
	if (keyboardData->dwExtraInfo == EASYKEY_EXTRA_INFO) {
		return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
	}

	uint64_t nowTick = GetTickCount64();
	if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && !(keyboardData->flags & LLKHF_INJECTED)) {
		if (isFocusNavigationKey(keyboardData->vkCode)) {
			recordNavigationSignal(nowTick);
		} else if (!(OTHER_CONTROL_KEY) && isPrintableContentKey(keyboardData->vkCode)) {
			_lastCharKeyTick = nowTick;
			_lastCharFocusKey = _lastFocusKey;
			clearNavigationSignal();
		}
	}

	if (isPasswordRawActiveOnHookThread(nowTick)) {
		return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
	}

	//fast-path game window binding check (confirmed bindings persist for the lifetime of the game window;
	//only unconfirmed armed bindings expire after the armed TTL)
	HWND fg = NULL;
	if (_game.windowId != 0) {
		bool boundAlive = IsWindow(gameBoundHwnd());
		HWND fgHwnd = NULL;
		uint64_t fgId = 0;
		bool fgPidMatches = false;
		if (boundAlive && !_game.armedExpired(nowTick)) {
			fgHwnd = GetForegroundWindow();
			fgId = gameWindowId(fgHwnd);
			if (fgId == _game.windowId && (!_game.confirmed || !_game.inGame)) {
				DWORD pid = 0;
				GetWindowThreadProcessId(fgHwnd, &pid);
				fgPidMatches = (pid == _game.processId && IsWindow(fgHwnd));
			}
		}
		GameModeState::BindingCheck check = _game.onKeyboardBindingCheck(nowTick, boundAlive, fgId, fgPidMatches);
		applyGameEffects(check.effects);
		if (check.useForeground)
			fg = fgHwnd;
	}

	//game mode: keys go straight to the game (skills on Q, W, E, R, Alt+W...) until Enter opens the chat;
	//Enter (send) or Esc (close) ends the chat. All key-up events and modifier combinations in game mode
	//bypass hotkey/language-switch evaluation completely.
	if (_game.inGame) {
		updateModifierMaskOnly(wParam);
		_lastFlag = 0;
		_hasJustUsedHotKey = false;
		if (wParam != WM_KEYDOWN && wParam != WM_SYSKEYDOWN) {
			return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
		}
		if (!_isFlagKey)
			_keycode = (Uint16)keyboardData->vkCode;
		//the state machine decides chat open/close and the / raw command; every
		//in-game key then goes straight to the game (Model W: game chat is raw
		//by default without mutating normal app language)
		GameModeState::KeyInput keyInput;
		keyInput.vkCode = _keycode;
		keyInput.altOrCtrl = (_flag & (MASK_ALT | MASK_CONTROL)) != 0;
		applyGameEffects(_game.onGameKeyDown(keyInput));
		return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
	}

	//ignore if IME pad is open when typing Japanese/Chinese...
	HWND hWnd = (fg != NULL) ? fg : GetForegroundWindow();
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
		if (_flag == 0 && !_isFlagKey) {
			_lastFlag = 0;
			_hasJustUsedHotKey = false;
		}
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
						currentEngineConfig(),
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
	if (nCode < 0)
		return CallNextHookEx(hMouseHook, nCode, wParam, lParam);

	_lastHookTick = GetTickCount();
	mouseData = (MSLLHOOKSTRUCT *)lParam;

	uint64_t nowTick = GetTickCount64();
	if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN || wParam == WM_LBUTTONUP) {
		recordNavigationSignal(nowTick);
	}
	if (wParam == WM_LBUTTONUP) {
		scheduleMouseUpReprobeIfNeeded(nowTick, mouseData->pt);
	}

	if (isPasswordRawActiveOnHookThread(nowTick))
		return CallNextHookEx(hMouseHook, nCode, wParam, lParam);

	if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN) {
		if (_game.processId != 0 && !_game.inGame) {
			bool boundAlive = _game.windowId != 0 && IsWindow(gameBoundHwnd());
			uint64_t fgId = 0;
			bool fgIsBoundProcess = false;
			if (!((_game.windowId != 0 && !boundAlive) || (!_game.confirmed && _game.lastForegroundTick != 0 && _game.armedExpired(nowTick)))) {
				HWND fg = GetForegroundWindow();
				fgId = gameWindowId(fg);
				if (fg != NULL) {
					DWORD pid = 0;
					GetWindowThreadProcessId(fg, &pid);
					fgIsBoundProcess = (pid == _game.processId && pid != GetCurrentProcessId() && IsWindow(fg) && IsWindowVisible(fg) && GetAncestor(fg, GA_ROOT) == fg);
				}
			}
			applyGameEffects(_game.onMouseBindingCheck(nowTick, boundAlive, fgId, fgIsBoundProcess));
		}
	}

	//game mode: click to exit chat
	if (_game.inGame && _game.chatOpen) {
		if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN) {
			exitGameChat();
		}
		// In game chat, do not forward other mouse events (XBUTTON, button-up) to typing engine
	} else {
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
			vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0, currentEngineConfig());
			if (IS_DOUBLE_CODE(vCodeTable)) { //VNI
				_syncKey.clear();
			}
			break;
		}
	}
	return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

VOID CALLBACK winEventProcCallback(HWINEVENTHOOK hWinEventHook, DWORD dwEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
	if (dwEvent == EVENT_SYSTEM_FOREGROUND) {
		GameCandidateFinder::recordForeground(hwnd);
		if (hwnd && EasyKeyHelper::isWindowElevated(hwnd))
			AppDelegate::getInstance()->onElevatedWindow(hwnd);
		//typing in another window usually starts a new sentence
		startNewSession();
		vStartSentence();
		uint64_t nowTick = GetTickCount64();
		if (!_gameApps.empty() && hwnd && isGameApp(exeNameOfWindow(hwnd))) {
			bindGameWindow(hwnd, true);
			_game.forceInGame();
		} else {
			applyGameEffects(_game.onForegroundChangedAway(nowTick, _game.windowId != 0 && IsWindow(gameBoundHwnd())));
		}
		recordNavigationSignal(nowTick);
		updatePasswordFieldState(hwnd, OBJID_WINDOW, CHILDID_SELF, true);
		//smart switch key
		if (!_game.inGame && (vUseSmartSwitchKey || vRememberCode)) {
			string& exe = EasyKeyHelper::getFrontMostAppExecuteName();
			//Explorer counts as the app the user came from; an app whose name can't be read
			//keeps what is on now instead of sharing one entry with every other such app
			if (!EasyKeyHelper::isRealApp(exe))
				return;
			_languageTemp = getAppInputMethodStatus(exe, vLanguage | (vCodeTable << 1));
			if (vUseSmartSwitchKey && (_languageTemp & 0x01) != vLanguage) {
				if (_languageTemp != -1) {
					vLanguage = _languageTemp & 0x01; //the other bits hold the code table
					if (vTwoModesOnly)
						vMixedEnglish = 0;
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
	} else if (dwEvent == EVENT_OBJECT_FOCUS) {
		if (isAcceptedFocusObjectId(idObject)) {
			updatePasswordFieldState(hwnd, idObject, idChild, false);
		}
	}
}
