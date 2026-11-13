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

//Reader with the WinHttpReadData shape, so the updater passes that function
//directly and the tests can script every read outcome.
typedef BOOL (WINAPI *HttpReadDataFn)(void* request, void* buffer, DWORD capacity, DWORD* bytesRead);

//Reads a response body to its end. Success is the clean end of stream (TRUE
//with zero bytes read) and nothing else: a failed read, a read count above
//the buffer size, a body over maxBytes, or a final size that differs from
//the Content-Length the caller read (when haveLength) all fail. body is
//cleared on entry and again on every failure, so no caller sees a partial
//body. Runs on the update worker, never on the keyboard-hook/UI thread.
bool readHttpBody(void* request, HttpReadDataFn readData, size_t maxBytes, bool haveLength, DWORD contentLength, std::string& body);
