//
//  PacketDelivery.h
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//
//  Whether the Windows hook may inject an engine packet (backspaces, then the
//  retyped characters). Pure decisions, no Win32 calls, so the tests can
//  exercise them.
//
//  A packet's backspaces are plain key events. The application combines them
//  with the modifiers that are down at that moment: with Ctrl down every
//  injected Backspace is Ctrl+Backspace (delete the whole word before the
//  caret), with Alt down it is Alt+Backspace (undo), and the Shift+Left the
//  Chromium fix sends first becomes Ctrl+Shift+Left (select the whole word) or
//  Win+Shift+Left (move the window). A packet of N backspaces then removes up
//  to N words instead of N characters, and its retyped characters are dropped
//  as shortcuts. So with Ctrl, Alt or Win down a packet is never injected.
//

#ifndef PacketDelivery_h
#define PacketDelivery_h

//true when a packet may be injected with these modifiers physically down
inline bool canInjectPacket(bool ctrlDown, bool altDown, bool winDown) {
    return !ctrlDown && !altDown && !winDown;
}

//the Chromium fix selects the last character (Shift+Left) so that the first
//backspace takes the browser's suggestion with it. That selection is only
//safe when the backspaces that consume it are really sent: the same range
//the hook's backspace loop uses. Outside it the selection would stay behind
//for the next key to delete.
inline bool canSelectForBackspaces(int backspaceCount, int maxBackspaces) {
    return backspaceCount > 0 && backspaceCount < maxBackspaces;
}

#endif /* PacketDelivery_h */
