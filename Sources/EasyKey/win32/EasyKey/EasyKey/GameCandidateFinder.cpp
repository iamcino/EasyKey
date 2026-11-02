/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only

This file is part of EasyKey, a modified version of OpenKey,
released under the GNU General Public License version 3.
See LICENSE and NOTICE.
-----------------------------------------------------------*/
#include "GameCandidateFinder.h"
#include <algorithm>
#include <cctype>
#include <shlobj.h>
#include <shlwapi.h>
#include "stdafx.h"
#include "EasyKeyHelper.h"

#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Shlwapi.lib")

static HWND _lastRealForegroundHwnd = NULL;

static std::string toLowerAscii(const std::string& str) {
	std::string res = str;
	for (char& c : res) {
		c = (char)std::tolower((unsigned char)c);
	}
	return res;
}

void GameCandidateFinder::recordForeground(HWND hwnd) {
	if (!hwnd || !IsWindow(hwnd))
		return;
	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == 0 || pid == GetCurrentProcessId())
		return;
	_lastRealForegroundHwnd = hwnd;
}

HWND GameCandidateFinder::getLastRealForeground() {
	if (_lastRealForegroundHwnd && IsWindow(_lastRealForegroundHwnd))
		return _lastRealForegroundHwnd;
	return NULL;
}

bool GameCandidateFinder::isSystemProcess(const std::string& exeName) {
	std::string lower = toLowerAscii(exeName);
	static const char* systemExes[] = {
		"explorer.exe",
		"dwm.exe",
		"svchost.exe",
		"taskhostw.exe",
		"conhost.exe",
		"searchhost.exe",
		"shellexperiencehost.exe",
		"startmenuexperiencehost.exe",
		"applicationframehost.exe",
		"textinputhost.exe",
		"ctfmon.exe",
		"systemsettings.exe",
		"taskmgr.exe",
		"easykey.exe",
		"cmd.exe",
		"powershell.exe",
		"windowsterminal.exe",
		"openkey.exe"
	};
	for (const char* sys : systemExes) {
		if (lower == sys)
			return true;
	}
	return false;
}

bool GameCandidateFinder::isOfficeApp(const std::string& exeName) {
	std::string lower = toLowerAscii(exeName);
	static const char* officeExes[] = {
		"notepad.exe",
		"calc.exe",
		"calculatorapp.exe",
		"winword.exe",
		"excel.exe",
		"powerpnt.exe",
		"outlook.exe",
		"onenote.exe"
	};
	for (const char* off : officeExes) {
		if (lower == off)
			return true;
	}
	return false;
}

bool GameCandidateFinder::isBlacklisted(const std::string& exeName) {
	if (exeName.empty())
		return true;
	return isSystemProcess(exeName) || isOfficeApp(exeName);
}

bool GameCandidateFinder::isWindowCloaked(HWND hwnd) {
	if (!hwnd)
		return true;
	int cloaked = 0;
	HRESULT hr = DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
	if (SUCCEEDED(hr)) {
		return cloaked != 0;
	}
	return false;
}

std::string GameCandidateFinder::getWindowExeName(HWND hwnd, DWORD* outPid) {
	if (!hwnd || !IsWindow(hwnd))
		return "";
	DWORD processId = 0;
	GetWindowThreadProcessId(hwnd, &processId);
	if (outPid)
		*outPid = processId;
	if (!processId)
		return "";

	HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
	if (!process)
		return "";

	TCHAR path[MAX_PATH];
	DWORD size = MAX_PATH;
	std::string name;
	if (QueryFullProcessImageName(process, 0, path, &size)) {
		LPCTSTR file = _tcsrchr(path, '\\');
		name = wideStringToUtf8(file ? file + 1 : path);
	}
	CloseHandle(process);
	return name;
}

std::wstring GameCandidateFinder::sanitizeTitle(const std::wstring& rawTitle) {
	std::wstring res;
	res.reserve(rawTitle.size());
	for (wchar_t ch : rawTitle) {
		// Filter control characters (< 32, DEL 127) and bidi overrides
		if (ch < 32 || ch == 127 ||
			(ch >= 0x202A && ch <= 0x202E) ||
			(ch >= 0x2066 && ch <= 0x2069) ||
			ch == 0x200E || ch == 0x200F) {
			res.push_back(L' ');
		} else {
			res.push_back(ch);
		}
	}
	// Trim leading and trailing whitespace
	size_t first = res.find_first_not_of(L" \t\r\n");
	if (first == std::wstring::npos)
		return L"";
	size_t last = res.find_last_not_of(L" \t\r\n");
	return res.substr(first, (last - first + 1));
}

static std::wstring getSafeWindowTitle(HWND hwnd) {
	if (!hwnd || !IsWindow(hwnd))
		return L"";
	// Do not send message if target window is hung
	if (IsHungAppWindow(hwnd))
		return L"";

	wchar_t titleBuf[128] = {};
	DWORD_PTR result = 0;
	// Use SendMessageTimeout with SMTO_ABORTIFHUNG only (do not use SMTO_BLOCK cross-process)
	// and 50ms timeout.
	LRESULT lr = SendMessageTimeoutW(
		hwnd,
		WM_GETTEXT,
		(WPARAM)(sizeof(titleBuf) / sizeof(titleBuf[0])),
		(LPARAM)titleBuf,
		SMTO_ABORTIFHUNG,
		50,
		&result
	);
	// Always guarantee null-termination
	titleBuf[sizeof(titleBuf) / sizeof(titleBuf[0]) - 1] = L'\0';
	if (lr && result > 0) {
		size_t len = (std::min)(static_cast<size_t>(result), sizeof(titleBuf) / sizeof(titleBuf[0]) - 1);
		titleBuf[len] = L'\0';
		return GameCandidateFinder::sanitizeTitle(titleBuf);
	}
	return L"";
}

struct EnumCandidateContext {
	std::vector<GameWindowCandidate> candidates;
	HWND lastForeground;
	bool isEasyKeyAdmin;
};

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	EnumCandidateContext* ctx = (EnumCandidateContext*)lParam;

	// Must be valid visible non-minimized window
	if (!IsWindow(hwnd) || !IsWindowVisible(hwnd) || IsIconic(hwnd))
		return TRUE;

	// Filter cloaked windows (virtual desktops, suspended UWP)
	if (GameCandidateFinder::isWindowCloaked(hwnd))
		return TRUE;

	// Filter tool windows
	LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	if (exStyle & WS_EX_TOOLWINDOW)
		return TRUE;

	// Must have meaningful dimensions
	RECT r;
	if (!GetWindowRect(hwnd, &r))
		return TRUE;
	int width = r.right - r.left;
	int height = r.bottom - r.top;
	if (width <= 150 || height <= 150)
		return TRUE;

	DWORD pid = 0;
	std::string exe = GameCandidateFinder::getWindowExeName(hwnd, &pid);
	if (exe.empty() || pid == GetCurrentProcessId() || GameCandidateFinder::isBlacklisted(exe))
		return TRUE;

	// Note: We intentionally do NOT fetch window titles during enumeration
	// to completely prevent hung external windows from delaying or locking UI.

	bool elevated = EasyKeyHelper::isWindowElevated(hwnd);
	bool locked = elevated && !ctx->isEasyKeyAdmin;

	int score = 0;
	if (hwnd == ctx->lastForeground)
		score += 100;

	// Check multi-monitor bounds for fullscreen / near-fullscreen scoring
	int monWidth = 0;
	int monHeight = 0;
	HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	if (hMon) {
		MONITORINFO mi = {};
		mi.cbSize = sizeof(mi);
		if (GetMonitorInfoW(hMon, &mi)) {
			monWidth = mi.rcMonitor.right - mi.rcMonitor.left;
			monHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
		}
	}
	// Fallback to primary screen metrics if monitor query failed
	if (monWidth <= 0 || monHeight <= 0) {
		monWidth = GetSystemMetrics(SM_CXSCREEN);
		monHeight = GetSystemMetrics(SM_CYSCREEN);
	}

	if (monWidth > 0 && monHeight > 0) {
		int64_t winArea = (int64_t)width * height;
		int64_t monArea = (int64_t)monWidth * monHeight;
		// Fullscreen / borderless windowed (area >= 90% monitor area or within 4px of monitor dimensions)
		if (winArea >= (monArea * 9 / 10) || (width >= monWidth - 4 && height >= monHeight - 4)) {
			score += 30;
		}
	}

	if (!isGameApp(exe))
		score += 5;

	if (locked)
		score -= 50;

	GameWindowCandidate cand;
	cand.hwnd = hwnd;
	cand.pid = pid;
	cand.exeBaseName = exe;
	cand.title = L""; // will be safely fetched only for top candidates
	cand.elevated = elevated;
	cand.lockedByPrivilege = locked;
	cand.score = score;

	ctx->candidates.push_back(cand);
	return TRUE;
}

std::vector<GameWindowCandidate> GameCandidateFinder::findTopCandidates(size_t maxCount) {
	EnumCandidateContext ctx;
	ctx.lastForeground = getLastRealForeground();
	ctx.isEasyKeyAdmin = IsUserAnAdmin() != FALSE;

	EnumWindows(EnumWindowsProc, (LPARAM)&ctx);

	// Deduplicate by exeBaseName (case-insensitive), keeping candidate with highest score
	std::vector<GameWindowCandidate> uniqueCandidates;
	for (const auto& c : ctx.candidates) {
		auto it = std::find_if(uniqueCandidates.begin(), uniqueCandidates.end(), [&](const GameWindowCandidate& item) {
			return _stricmp(item.exeBaseName.c_str(), c.exeBaseName.c_str()) == 0;
		});
		if (it == uniqueCandidates.end()) {
			uniqueCandidates.push_back(c);
		} else if (c.score > it->score) {
			*it = c;
		}
	}

	// Sort descending by score
	std::sort(uniqueCandidates.begin(), uniqueCandidates.end(), [](const GameWindowCandidate& a, const GameWindowCandidate& b) {
		return a.score > b.score;
	});

	if (uniqueCandidates.size() > maxCount) {
		uniqueCandidates.resize(maxCount);
	}

	// Safely fetch title ONLY for final top candidates with timeout and hung guard
	for (auto& cand : uniqueCandidates) {
		cand.title = getSafeWindowTitle(cand.hwnd);
	}

	return uniqueCandidates;
}
