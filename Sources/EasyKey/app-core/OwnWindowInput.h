//
//  OwnWindowInput.h
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//
//  Whether the raw-keyboard watchdog stays registered.
//
//  EasyKey registers for raw keyboard input so a low-level hook that Windows
//  has removed can be noticed on the next key. A process that is registered
//  for raw keyboard input does not get its own WH_KEYBOARD_LL calls while one
//  of its windows is in front. The Shortcut list is such a window, so Telex
//  and VNI never run there and the two edits show the raw letters.
//
//  rawWatchShouldRun is the decision the watchdog uses. These tests lock what
//  it does today: the watch stays registered no matter which window is in front.
//

#ifndef OwnWindowInput_h
#define OwnWindowInput_h

// ownTypingWindowActive: an EasyKey window whose edits take Vietnamese
// (the Shortcut list) is the foreground window.
inline bool rawWatchShouldRun(bool ownTypingWindowActive) {
    (void)ownTypingWindowActive;
    return true;
}

#endif /* OwnWindowInput_h */
