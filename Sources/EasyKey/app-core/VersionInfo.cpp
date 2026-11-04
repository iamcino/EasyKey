/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
#include "VersionInfo.h"

static bool parseComponent(const std::string& text, size_t& pos, uint32_t& value) {
    if (pos >= text.size() || text[pos] < '0' || text[pos] > '9')
        return false;
    uint32_t v = 0;
    while (pos < text.size() && text[pos] >= '0' && text[pos] <= '9') {
        v = v * 10 + (uint32_t)(text[pos] - '0');
        if (v > 65535) //the packed number has no room above this, for any component
            return false;
        pos++;
    }
    value = v;
    return true;
}

static bool splitVersionName(const std::string& version, uint32_t& major, uint32_t& minor, uint32_t& patch) {
    if (version.empty() || version.size() > 16)
        return false;
    size_t pos = 0;
    if (!parseComponent(version, pos, major))
        return false;
    if (pos >= version.size() || version[pos] != '.')
        return false;
    pos++;
    if (!parseComponent(version, pos, minor))
        return false;
    if (pos >= version.size() || version[pos] != '.')
        return false;
    pos++;
    if (!parseComponent(version, pos, patch))
        return false;
    if (pos != version.size())
        return false;
    return minor <= 255 && patch <= 255;
}

bool isVersionName(const std::string& version) {
    uint32_t major, minor, patch;
    return splitVersionName(version, major, minor, patch);
}

uint32_t versionNumber(const std::string& version) {
    uint32_t major, minor, patch;
    if (!splitVersionName(version, major, minor, patch))
        return 0;
    return (major << 16) | (minor << 8) | patch;
}

bool parseLatestVersionName(const std::string& data, std::string& version) {
    //"latestWinVersion": { "versionName": "1.0.1", ... }
    size_t pos = data.find("\"latestWinVersion\"");
    if (pos == std::string::npos)
        return false;
    pos = data.find("\"versionName\"", pos);
    if (pos == std::string::npos)
        return false;
    pos = data.find(':', pos);
    if (pos == std::string::npos)
        return false;
    pos = data.find('"', pos);
    if (pos == std::string::npos)
        return false;
    size_t end = data.find('"', pos + 1);
    if (end == std::string::npos)
        return false;
    std::string found = data.substr(pos + 1, end - pos - 1);
    if (!isVersionName(found))
        return false;
    version = found;
    return true;
}
