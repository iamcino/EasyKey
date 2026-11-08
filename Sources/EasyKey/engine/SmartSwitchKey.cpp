//
//  SmartSwitchKey.cpp
//  OpenKey
//
//  Created by Tuyen on 8/13/19.
//  Copyright © 2019 Tuyen Mai. All rights reserved.
//
//  Part of EasyKey, a modified version of OpenKey (https://github.com/tuyenvm/OpenKey).
//  SPDX-License-Identifier: GPL-3.0-only
//

#include "SmartSwitchKey.h"
#include <map>

#include <memory.h>

//main data, i use `map` because it has O(Log(n))
static map<string, Int8> _smartSwitchKeyData;
static string _cacheKey = ""; //use cache for faster
static Int8 _cacheData = 0; //use cache for faster

void initSmartSwitchKey(const Byte* pData, const int& size) {
    _smartSwitchKeyData.clear();
    if (pData == NULL) return;
    Uint16 count = 0;
    Uint32 cursor = 0;
    if (size >= 2) {
        memcpy(&count, pData + cursor, 2);
        cursor+=2;
    }
    Uint8 bundleIdSize;
    Uint8 value;
    for (int i = 0; i < count; i++) {
        //corrupt or truncated data: keep the entries that are still whole,
        //drop the cut one and stop (found by the step 3.4 sanitizer sweep)
        if ((int)cursor + 1 > size)
            break;
        bundleIdSize = pData[cursor++];
        if ((int)cursor + bundleIdSize + 1 > size)
            break;
        string bundleId((char*)pData + cursor, bundleIdSize);
        cursor += bundleIdSize;
        value = pData[cursor++];
        _smartSwitchKeyData[bundleId] = value;
    }
}

void getSmartSwitchKeySaveData(vector<Byte>& outData) {
    outData.clear();
    //build the entries first: the count must be the number of entries that
    //actually follow, and an entry whose id does not fit the one-byte length
    //is skipped instead of truncating the length and corrupting every entry
    //after it (finding F7)
    vector<Byte> body;
    Uint16 count = 0;
    for (std::map<string, Int8>::iterator it = _smartSwitchKeyData.begin(); it != _smartSwitchKeyData.end() && count < 0xFFFF; ++it) {
        if (it->first.length() > 0xFF)
            continue;
        body.push_back((Byte)it->first.length());
        for (int j = 0; j < it->first.length(); j++) {
            body.push_back(it->first[j]);
        }
        body.push_back(it->second);
        count++;
    }
    outData.push_back((Byte)count);
    outData.push_back((Byte)(count>>8));
    outData.insert(outData.end(), body.begin(), body.end());
}

int peekAppInputMethodStatus(const string& bundleId) {
    if (_cacheKey.compare(bundleId) == 0) {
        return _cacheData;
    }
    std::map<string, Int8>::iterator it = _smartSwitchKeyData.find(bundleId);
    if (it != _smartSwitchKeyData.end()) {
        _cacheKey = bundleId;
        _cacheData = it->second;
        return _cacheData;
    }
    return -1;
}

int getAppInputMethodStatus(const string& bundleId, const int& currentInputMethod) {
    if (_cacheKey.compare(bundleId) == 0) {
        return _cacheData;
    }
    if (_smartSwitchKeyData.find(bundleId) != _smartSwitchKeyData.end()) {
        _cacheKey = bundleId;
        _cacheData = _smartSwitchKeyData[bundleId];
        return _cacheData;
    }
    _cacheKey = bundleId;
    _cacheData = currentInputMethod;
    _smartSwitchKeyData[bundleId] = _cacheData;
    return -1;
}

void setAppInputMethodStatus(const string& bundleId, const int& language) {
    _smartSwitchKeyData[bundleId] = language;
    _cacheKey = bundleId;
    _cacheData = language;
}
