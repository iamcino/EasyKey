//
//  MixedModeDecider.cpp
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//

#include "MixedModeDecider.h"
#include "MixedWords.h"
#include "Vietnamese.h"

//Zipf x10 points: a word reads as English when its English value beats its Vietnamese one by this much,
//the language of the last words moves that by up to MIXED_CONTEXT_WEIGHT,
//and a word that leads by MIXED_CLEAR or more can settle the word before it
#define MIXED_VIETNAMESE_LEAN 5
#define MIXED_CONTEXT_WEIGHT 20
#define MIXED_CLEAR 20

static int mixedBias(const int& context) {
    return MIXED_CONTEXT_WEIGHT * context / 100;
}

//token classifier for code-like identifiers, camelCase, URLs, and emails in Mixed mode
bool isCodeLikeToken(const vector<Uint32>& rawKeys, int inputType) {
    if (rawKeys.empty())
        return false;
    bool hasLowerToUpper = false;
    bool hasLetter = false, hasDigit = false;
    bool hasUnderscore = false, hasAtSign = false;
    bool prevWasLower = false;
    for (size_t i = 0; i < rawKeys.size(); i++) {
        Uint16 key = rawKeys[i] & CHAR_MASK;
        bool isCaps = (rawKeys[i] & CAPS_MASK) != 0;
        Uint16 c = keyCodeToCharacter(key);
        if (key == KEY_MINUS) {
            if (isCaps) {
                hasUnderscore = true;
            }
            prevWasLower = false;
        } else if (key == KEY_2 && isCaps) {
            hasAtSign = true;
            prevWasLower = false;
        } else if (c >= 'a' && c <= 'z') {
            hasLetter = true;
            if (isCaps) {
                if (prevWasLower) {
                    hasLowerToUpper = true;
                }
                prevWasLower = false;
            } else {
                prevWasLower = true;
            }
        } else if (c >= '0' && c <= '9' && !isCaps) {
            hasDigit = true;
            prevWasLower = false;
        } else {
            prevWasLower = false;
        }
    }
    if (hasUnderscore)
        return true;
    if (hasAtSign)
        return true;
    if (hasLowerToUpper)
        return true;
    if (inputType != vVNI && hasLetter && hasDigit)
        return true;
    return false;
}
void MixedModeDecider::invalidateCandidate() {
    _hasLast = false;
}

void MixedModeDecider::forgetLast() {
    _hasLast = false;
    _lastRawLen = 0;
    _lastRaw[0] = '\0';
}

void MixedModeDecider::forgetAll() {
    forgetLast();
    _context = 0;
}

#ifdef EASYKEY_TEST
void MixedModeDecider::setMixedLastForTest(const MixedWord& word) {
    _last = word;
    _hasLast = true;
    _lastRawLen = 0;
    _lastRaw[0] = '\0';
}
#endif

MixedDecision MixedModeDecider::decide(const MixedWordInput& in) {
    MixedDecision decision;
    bool hadLast = _hasLast;
    if (!in.valid) {
        //the entry guard refused this word (empty, over-long or helper-backed):
        //it still consumes the stored candidate, but nothing else runs and
        //phrase memory and context are kept
        invalidateCandidate();
        return decision;
    }
    _hasLast = false;
    int english = 0, vietnamese = 0, score = 0, language = 0; //1: English, -1: Vietnamese, 0: can't tell
    bool codeLike = in.codeLike;
    bool curatedEnglish = !codeLike && !in.keys.empty() &&
        (isCuratedSafeEnglish(in.keys) || isAllCapsEnglishAcronym(in.keys, in.allCaps));
    int phraseMatch = (!codeLike && !in.keys.empty() && !in.allCaps) ? checkMixedPhrase(_lastRaw, _lastRawLen, in.keys) : 0;
    bool listed = !codeLike && !in.keys.empty() && findMixedWord(in.keys, english, vietnamese), clear = false;
    bool protectedVietnamese = in.changed && in.spellingOk && !in.allCaps &&
        isProtectedShortVietnamese(in.keys, english, vietnamese, _context);
    // Explicit phrase/curated/protection decisions must survive the next word.
    bool reconsider = listed && !curatedEnglish && phraseMatch == 0 && !protectedVietnamese;
    if (codeLike) {
        language = 1;
        clear = false;
    } else if (curatedEnglish) {
        language = 1;
        clear = true;
    } else if (phraseMatch != 0) {
        language = phraseMatch;
        clear = true;
    } else if (protectedVietnamese) {
        language = -1;
        clear = false;
    } else if (listed) {
        score = english - vietnamese - MIXED_VIETNAMESE_LEAN;
        language = score + mixedBias(_context) > 0 ? 1 : -1;
        clear = score >= MIXED_CLEAR || score <= -MIXED_CLEAR;
    } else if (in.changed && in.spellingOk) {
        language = -1; //a Vietnamese word the list doesn't have
        clear = true;
    }
    int contextBefore = _context;
    bool restore = language == 1 && in.changed;

    //read the last word again with this one as its context
    //(not with VNI Windows and compound Unicode, there a letter on screen may take two backspaces)
    bool fixLast = false;
    vector<Uint32> lastText; //the last word as it should be now
    size_t lastShown = 0; //the shown length of the last word that a fix would replace
    if (in.fixLastEnabled && hadLast && clear && !in.doubleCode) {
        bool english = _last.score + mixedBias((_last.contextBefore + language * 100) / 2) > 0;
        lastText = english ? _last.keys : _last.vietnamese;
        size_t shown = english ? _last.vietnamese.size() : _last.keys.size();
        if (english != _last.english && shown + 1 + in.shownLength < MAX_BUFF &&
            lastText.size() + 1 + (restore ? in.rawCount : in.shownLength) <= MAX_BUFF) {
            fixLast = true;
            lastShown = shown;
            _last.english = english;
            contextBefore = (_last.contextBefore + (english ? 100 : -100)) / 2;
        }
    }
    if (language != 0 && !codeLike)
        _context = (contextBefore + language * 100) / 2;
    //this word may be the next one's "last word"
    MixedWord word;
    if (in.spaceFollows && reconsider) {
        word.keys.assign(in.rawKeys, in.rawKeys + in.rawCount);
        word.vietnamese.assign(in.vietnamese, in.vietnamese + in.vietnameseCount);
        word.score = score;
        word.contextBefore = contextBefore;
        word.english = language == 1;
    }
    if (in.spaceFollows && reconsider) {
        _last = word;
        _hasLast = true;
    }
    if (in.spaceFollows) {
        _lastRawLen = 0;
        for (size_t k = 0; k < in.rawCount && k < 15; k++) {
            Uint16 c = keyCodeToCharacter(in.rawKeys[k] & CHAR_MASK);
            if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
            _lastRaw[_lastRawLen++] = (char)c;
        }
        _lastRaw[_lastRawLen] = '\0';
    } else {
        _lastRawLen = 0;
        _lastRaw[0] = '\0';
    }
    decision.language = language;
    decision.clear = clear;
    decision.restore = restore;
    decision.fixLast = fixLast;
    if (fixLast) {
        decision.lastText = lastText;
        decision.lastShownCount = (Byte)lastShown;
    }
    return decision;
}
