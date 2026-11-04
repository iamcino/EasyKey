//
//  MixedWords.h
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//
//  Word list for the mixed English and Vietnamese mode: for the keys of a word as typed,
//  how common the word is in English and how common the Vietnamese word is that Telex makes of the same keys.
//

#ifndef MixedWords_h
#define MixedWords_h

#include <cstdint>
#include <string>

/**
 * keys: the letters as typed, in lower case ("this", "sex")
 * english, vietnamese: Zipf frequency times 10 (70 = once in every 10 000 words, 0 = unknown)
 * return false when the word is not in the list
 */
bool findMixedWord(const std::string& keys, int& english, int& vietnamese);

/**
 * Curated AI/tech proper nouns and developer terms that should always stay English in Mixed mode.
 */
bool isCuratedSafeEnglish(const std::string& keys);

/**
 * Short ALL-CAPS English acronyms (e.g. "US", "UK", "EU", "AI", "ER", "IT", "QA", "API", "GPU")
 * that should stay English when typed in all uppercase in Mixed mode.
 */
bool isAllCapsEnglishAcronym(const std::string& keys, bool isAllCaps);

/**
 * 2-word collocation/phrase check for Mixed mode:
 * returns 1 for an explicit English phrase (e.g. "xe bus", "with us", "me too"),
 * -1 for an explicit Vietnamese phrase (e.g. "mập ú", "bánh tét", "một tô"),
 * or 0 when no phrase rule matches.
 */
int checkMixedPhrase(const char* prevRaw, uint8_t prevLen, const std::string& currentRaw);

/**
 * Returns true when a short Telex-modified token represents a common Vietnamese word
 * (such as "bú" from "bus", "ú" from "us", "tô" from "too") that must be protected
 * in standalone or Vietnamese/neutral context unless matched by an English phrase.
 */
bool isProtectedShortVietnamese(const std::string& keys, int english, int vietnamese, int contextBefore);

#endif /* MixedWords_h */
