/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only

This file is part of EasyKey, a modified version of OpenKey,
released under the GNU General Public License version 3.
See LICENSE and NOTICE.
-----------------------------------------------------------*/
#pragma once

#include <dwmapi.h>
#include <windows.h>
#include <string>
#include <vector>

struct GameWindowCandidate {
	HWND hwnd = NULL;
	DWORD pid = 0;
	std::string exeBaseName; // lowercase utf-8, e.g. "game.exe"
	std::wstring title;      // transient, UI display only, not persisted
	bool elevated = false;
	bool lockedByPrivilege = false;
	int score = 0;
};

class GameCandidateFinder {
public:
	// Record foreground window as it changes (called from winEventProcCallback)
	static void recordForeground(HWND hwnd);

	// Get the last real foreground window (before user switched to EasyKey)
	static HWND getLastRealForeground();

	// Check if an exe name is a known Windows system process
	static bool isSystemProcess(const std::string& exeName);

	// Check if an exe name is a known office/productivity app
	static bool isOfficeApp(const std::string& exeName);

	// Check if an exe name is blacklisted from candidate suggestions
	static bool isBlacklisted(const std::string& exeName);

	// Check if a window is cloaked by DWM (virtual desktop, suspended UWP, etc.)
	static bool isWindowCloaked(HWND hwnd);

	// Extract exe basename from a window handle
	static std::string getWindowExeName(HWND hwnd, DWORD* outPid = nullptr);

	// Sanitize window title (strip control chars, bidi overrides, trim whitespace)
	static std::wstring sanitizeTitle(const std::wstring& rawTitle);

	// Find top N candidate game windows
	static std::vector<GameWindowCandidate> findTopCandidates(size_t maxCount = 3);
};
