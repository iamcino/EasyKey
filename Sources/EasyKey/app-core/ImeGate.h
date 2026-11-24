//
//  ImeGate.h
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//
//  Decides whether the "IME open" flag a window reports should make EasyKey
//  step aside. Some apps (PowerPoint on Windows 11) report the IME as open
//  while the active keyboard is a plain layout such as en-US, which made every
//  key pass through untouched. The flag only means a composing IME is active
//  when the layout's language is one that uses such an IME.
//

#ifndef ImeGate_h
#define ImeGate_h

#include <cstdint>

namespace ImeGate {
    constexpr uint16_t kLangChinese  = 0x04;
    constexpr uint16_t kLangJapanese = 0x11;
    constexpr uint16_t kLangKorean   = 0x12;

    // keyboardLayout: the low word of the foreground thread's HKL (its language id).
    inline bool layoutUsesComposingIme(uint16_t keyboardLangId) {
        uint16_t primary = (uint16_t)(keyboardLangId & 0x3FF);
        return primary == kLangChinese || primary == kLangJapanese || primary == kLangKorean;
    }

    // imeOpen: the IMC_GETOPENSTATUS answer from the window's default IME window.
    inline bool shouldBypassForIme(bool imeOpen, uint16_t keyboardLangId) {
        return imeOpen && layoutUsesComposingIme(keyboardLangId);
    }
}

#endif /* ImeGate_h */
