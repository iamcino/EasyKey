/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
#include "UpdateCleanup.h"

bool cleanupOldUpdateFile(const std::wstring& path, DWORD attempts, DWORD retryDelayMs) {
    for (DWORD i = 0; i < attempts; ++i) {
        if (DeleteFileW(path.c_str()))
            return true;
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
            return true;
        if (error == ERROR_ACCESS_DENIED) {
            DWORD attributes = GetFileAttributesW(path.c_str());
            if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_READONLY) &&
                !(attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))) {
                DWORD writable = attributes & ~FILE_ATTRIBUTE_READONLY;
                if (SetFileAttributesW(path.c_str(), writable ? writable : FILE_ATTRIBUTE_NORMAL) &&
                    DeleteFileW(path.c_str()))
                    return true;
            }
        }
        if (i + 1 < attempts)
            Sleep(retryDelayMs);
    }
    return false;
}
