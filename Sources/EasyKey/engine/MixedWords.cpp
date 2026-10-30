//
//  MixedWords.cpp
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//

#include <unordered_map>
#include "MixedWords.h"
#include "MixedWordsData.h"

using namespace std;

static unordered_map<string, unsigned short> _mixedWords;

//each entry of MIXED_WORDS is the word, then the English and the Vietnamese value as one character each ('0' + value), then a line break
static void initMixedWords() {
    _mixedWords.reserve(MIXED_WORD_COUNT);
    const char* p = MIXED_WORDS;
    while (*p) {
        const char* end = p;
        while (*end != '\n') end++;
        if (end - p > 2)
            _mixedWords[string(p, end - p - 2)] = (unsigned short)(((end[-2] - '0') << 8) | (end[-1] - '0'));
        p = end + 1;
    }
}

bool findMixedWord(const string& keys, int& english, int& vietnamese) {
    if (_mixedWords.empty())
        initMixedWords();
    auto it = _mixedWords.find(keys);
    if (it == _mixedWords.end())
        return false;
    english = it->second >> 8;
    vietnamese = it->second & 0xFF;
    return true;
}
