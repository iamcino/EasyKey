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

// The caller keeps this file handle open (and denies write/delete sharing) until
// the verified file is installed. The pin is DER from the compiled-in resource.
bool verifyUpdateSignature(const std::wstring& path, HANDLE file,
    const BYTE* pinnedDer, DWORD pinnedSize);
