#pragma once
#include <windows.h>
#include <string>

// Runs on the cleanup worker, never on the keyboard-hook/UI thread. A locked
// backup is retained on timeout and retried on the next successful startup.
bool cleanupOldUpdateFile(const std::wstring& path, DWORD attempts = 120, DWORD retryDelayMs = 500);
