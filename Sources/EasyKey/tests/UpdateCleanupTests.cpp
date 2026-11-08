/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
#include <windows.h>
#include <cstdio>
#include <string>
#include "../win32/EasyKey/EasyKey/UpdateCleanup.h"

static int total = 0, failures = 0;
static void check(bool ok, const char* label) {
    ++total;
    if (!ok) {
        ++failures;
        std::printf("FAIL [%s]\n", label);
    }
}

struct CleanupJob {
    std::wstring path;
    HANDLE started;
    bool removed = false;
};

static DWORD WINAPI runCleanup(LPVOID param) {
    auto job = static_cast<CleanupJob*>(param);
    SetEvent(job->started);
    job->removed = cleanupOldUpdateFile(job->path, 200, 10);
    return 0;
}

int main() {
    wchar_t temp[MAX_PATH] = {}, name[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, temp);
    if (!GetTempFileNameW(temp, L"eko", 0, name)) return 1;
    const std::wstring path = name;
    check(cleanupOldUpdateFile(path, 1, 0), "ordinary backup removed");
    check(cleanupOldUpdateFile(path, 1, 0), "already absent backup is success");

    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
        CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    check(file != INVALID_HANDLE_VALUE, "locked backup created");
    if (file != INVALID_HANDLE_VALUE) {
        check(!cleanupOldUpdateFile(path, 2, 1) && GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES,
            "timeout preserves locked backup for next startup");
        CleanupJob job = {path, CreateEventW(nullptr, TRUE, FALSE, nullptr)};
        HANDLE thread = job.started ? CreateThread(nullptr, 0, runCleanup, &job, 0, nullptr) : nullptr;
        check(thread != nullptr, "background cleanup started");
        if (thread) {
            WaitForSingleObject(job.started, INFINITE);
            // Keep the lock through several failed delete attempts.
            Sleep(80);
            CloseHandle(file);
            file = INVALID_HANDLE_VALUE;
            bool finished = WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0;
            check(finished && job.removed && GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES,
                "backup removed after transient lock released");
            // Keep stack-owned job alive until the worker is definitely done.
            if (!finished) WaitForSingleObject(thread, INFINITE);
            CloseHandle(thread);
        }
        if (job.started) CloseHandle(job.started);
        if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
    }
    DeleteFileW(path.c_str());

    file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    check(file != INVALID_HANDLE_VALUE, "read-only fixture created");
    if (file != INVALID_HANDLE_VALUE) {
        CloseHandle(file);
        bool readOnly = SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_READONLY) != FALSE;
        check(readOnly && cleanupOldUpdateFile(path, 1, 0), "read-only backup removed");
    }
    SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
    DeleteFileW(path.c_str());
    bool directory = CreateDirectoryW(path.c_str(), nullptr) != FALSE;
    check(directory && !cleanupOldUpdateFile(path, 1, 0), "directory is never recursively removed");
    if (directory) RemoveDirectoryW(path.c_str());

    std::printf("%d/%d updater cleanup tests passed\n", total - failures, total);
    return failures ? 1 : 0;
}
