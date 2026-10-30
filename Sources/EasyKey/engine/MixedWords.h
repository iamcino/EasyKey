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

#include <string>

/**
 * keys: the letters as typed, in lower case ("this", "sex")
 * english, vietnamese: Zipf frequency times 10 (70 = once in every 10 000 words, 0 = unknown)
 * return false when the word is not in the list
 */
bool findMixedWord(const std::string& keys, int& english, int& vietnamese);

#endif /* MixedWords_h */
