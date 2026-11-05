//
//  ExeName.h
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//
//  Case-insensitive exe name equality, extracted from the Win32 layer without
//  behavior changes. Callers pass base names (the Win32 layer strips paths
//  before comparing); this function itself does no path handling.
//

#ifndef ExeName_h
#define ExeName_h

#include <string>
#ifdef _WIN32
#include <cstring>
#else
#include <algorithm>
#include <cctype>
#endif

inline bool isSameExe(const std::string& a, const std::string& b) {
#ifdef _WIN32
    return _stricmp(a.c_str(), b.c_str()) == 0;
#else
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char c1, char c2) {
        return std::tolower((unsigned char)c1) == std::tolower((unsigned char)c2);
    });
#endif
}

#endif /* ExeName_h */
