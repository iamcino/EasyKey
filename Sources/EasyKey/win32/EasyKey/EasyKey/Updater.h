/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
#pragma once
#include "stdafx.h"

//notes of one release, followed by its version
#define RELEASE_NOTES_URL L"https://github.com/iamcino/EasyKey/releases/tag/"

/*
 * Updates EasyKey.exe in place from GitHub Releases. The new exe is downloaded
 * over HTTPS and checked against the SHA-256 published next to it. The running
 * exe is renamed to EasyKey.old.exe, the new one takes its name and starts, and
 * the old file is deleted on the next start. Nothing is installed without asking.
 */
class Updater {
public:
	//latest version from version.json, true when it is newer than this exe
	static bool check(string& newVersion);
	//check, then ask and install, in the background. With silent, say nothing when already up to date or offline
	static void checkAndOffer(const HWND& owner, const bool& silent);
	//on start: remove the previous exe and wait for it to exit after an update
	static void finishPreviousUpdate();
};
