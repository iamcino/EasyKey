/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only

Bug D (Backspace over-deletion). The engine hands the Windows hook a packet:
"send N backspaces, then retype these characters". These tests pin the engine
side of that contract at the three places the hook depends on it:

 1. a plain Backspace tap is never a packet (the hook forwards the tap once);
 2. a word-break key pressed with Ctrl or Alt held still returns a restore
    packet whose backspace count is the whole word;
 3. a mouse click returns the same restore packet, which no one delivers, and
    leaves the old word in the engine buffer;
 4. the hook's own decision (app-core/PacketDelivery.h) not to inject a packet
    while Ctrl, Alt or Win is down.
-----------------------------------------------------------*/
#include <cstdio>
#include <string>
#include "../engine/Engine.h"
#include "../app-core/PacketDelivery.h"

static int total = 0, failures = 0;
static void check(bool ok, const char* label) {
    ++total;
    if (!ok) {
        ++failures;
        std::printf("FAIL [%s]\n", label);
    }
}

static vKeyHookState* hook = nullptr;

static vKeyConfig telexConfig() {
    vKeyConfig cfg;
    cfg.language = 1;
    cfg.inputType = 0; //Telex
    cfg.codeTable = 0; //Unicode
    cfg.checkSpelling = 1;
    cfg.restoreIfWrongSpelling = 1;
    cfg.useModernOrthography = 0;
    cfg.useMacro = 0;
    cfg.mixedEnglish = 0;
    cfg.freeMark = 0;
    cfg.quickTelex = 0;
    cfg.fixRecommendBrowser = 1;
    return cfg;
}

static void key(Uint16 code, bool otherControlKey = false) {
    vKeyHandleEvent(vKeyEvent::Keyboard, vKeyEventState::KeyDown, code, telexConfig(), 0, otherControlKey);
}
static void keys(const char* text) {
    for (const char* p = text; *p; ++p)
        key((Uint16)(*p >= 'a' && *p <= 'z' ? *p - 'a' + 'A' : *p));
}
static void click() {
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0, telexConfig());
}
static void fresh() {
    //a click clears the line cache; then drop whatever word is left
    click();
    startNewSession();
}

//a plain Backspace is either forwarded untouched (code 0) or, for a tone that
//has to move, a grammar packet that nets exactly one deleted character
static bool backspaceIsSingleDelete() {
    if (hook->code == vDoNothing)
        return hook->extCode == 2;
    return hook->code == vWillProcess && hook->extCode == 4 &&
           hook->backspaceCount - hook->newCharCount == 1;
}

int main() {
    hook = (vKeyHookState*)vKeyInit();

    //1. plain Backspace taps: never more than one character, also across the
    //word boundary where the saved word comes back
    fresh();
    keys("windows"); //shows as "ưindows": wrong spelling, toned
    key(KEY_DELETE);
    check(hook->code == vDoNothing && hook->extCode == 2, "plain Backspace on a misspelled toned word is forwarded");
    fresh();
    keys("vieejt");
    key(KEY_SPACE);
    keys("hoans");
    bool allSingle = true;
    for (int n = 0; n < 14; n++) {
        key(KEY_DELETE);
        allSingle = allSingle && backspaceIsSingleDelete();
    }
    check(allSingle, "14 Backspace taps over two words each delete one character");

    //Ctrl+Backspace is the user's own word delete: forwarded, never a packet
    fresh();
    keys("windows");
    key(KEY_DELETE, true);
    check(hook->code == vDoNothing, "Ctrl+Backspace is forwarded");

    //2. a word break with Ctrl or Alt held still asks for the whole word to be
    //deleted and retyped. Injected under Ctrl those backspaces are
    //Ctrl+Backspace, one word each; under Alt they are Alt+Backspace (undo)
    fresh();
    keys("windows");
    key(KEY_ENTER, true);
    check(hook->code == vRestoreAndStartNewSession, "Ctrl+Enter on a misspelled toned word returns a restore packet");
    check(hook->backspaceCount == 7 && hook->newCharCount == 7, "the Ctrl+Enter packet deletes and retypes all 7 characters");
    fresh();
    keys("windows");
    key(KEY_TAB, true);
    check(hook->code == vRestoreAndStartNewSession && hook->backspaceCount == 7, "Alt+Tab returns the same 7-backspace packet");
    fresh();
    keys("windows");
    key(KEY_LEFT, true);
    check(hook->code == vRestoreAndStartNewSession && hook->backspaceCount == 7, "Ctrl+Left returns the same 7-backspace packet");
    //a correctly spelled word gives no packet under Ctrl
    fresh();
    keys("vieejt");
    key(KEY_ENTER, true);
    check(hook->code == vDoNothing, "Ctrl+Enter on a correct word is forwarded");

    //3. a mouse click returns the restore packet too; the mouse hook sends nothing
    fresh();
    keys("windows");
    click();
    check(hook->code == vRestoreAndStartNewSession && hook->backspaceCount == 7, "a click on a misspelled toned word returns an undelivered restore packet");

    //4. the hook's decision (app-core/PacketDelivery.h): no packet is injected while
    //Ctrl, Alt or Win is down, in any combination; Shift alone never blocks one
    check(canInjectPacket(false, false, false), "no modifier: the packet is injected");
    bool blockedAll = true;
    for (int m = 1; m < 8; m++)
        blockedAll = blockedAll && !canInjectPacket((m & 1) != 0, (m & 2) != 0, (m & 4) != 0);
    check(blockedAll, "Ctrl, Alt or Win down, alone or together: nothing is injected");
    //the packets of section 2 are exactly the ones the decision now stops
    fresh();
    keys("windows");
    key(KEY_ENTER, true);
    check(hook->code != vDoNothing && !canInjectPacket(true, false, false), "the Ctrl+Enter restore packet is not injected");

    //the Chromium Shift+Left goes out only together with the backspaces that consume it
    check(!canSelectForBackspaces(0, MAX_BUFF), "no backspace: no selection");
    check(canSelectForBackspaces(1, MAX_BUFF) && canSelectForBackspaces(MAX_BUFF - 1, MAX_BUFF), "backspaces sent: selection allowed");
    check(!canSelectForBackspaces(MAX_BUFF, MAX_BUFF) && !canSelectForBackspaces(255, MAX_BUFF), "backspaces not sent: no selection left behind");

    std::printf("%d/%d packet delivery tests passed\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
