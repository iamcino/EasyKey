#pragma once
#include <windows.h>
#include <string>

// The caller keeps this file handle open (and denies write/delete sharing) until
// the verified file is installed. The pin is DER from the compiled-in resource.
bool verifyUpdateSignature(const std::wstring& path, HANDLE file,
    const BYTE* pinnedDer, DWORD pinnedSize);
