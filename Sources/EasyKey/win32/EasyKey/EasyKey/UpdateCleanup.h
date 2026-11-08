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
#include <windows.h>
#include <string>

// Runs on the cleanup worker, never on the keyboard-hook/UI thread. A locked
// backup is retained on timeout and retried on the next successful startup.
bool cleanupOldUpdateFile(const std::wstring& path, DWORD attempts = 120, DWORD retryDelayMs = 500);
