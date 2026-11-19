// Part B (plan v2.2) state characterization driver.
//
// The regression suite (EngineTests) asserts screens and event packets.
// The signed plan also requires the engine's session STATE at event
// boundaries: a CapsLock keydown must leave no hidden unit, no special
// chars, no typing states, no raw keys and no macro key, and it must
// preserve _mixedContext. Those fields are file-static inside
// Engine.cpp, so -- exactly like the debate probe harnesses -- this
// driver includes Engine.cpp in its own translation unit and reads
// them directly. Test-only: nothing here is linked into the app, and
// production sources are untouched.
//
// The CapsLock key only exists on Windows (VK_CAPITAL 0x14; the Linux
// platform header maps a different code), so the cells are _WIN32-only
// and the driver is a no-op elsewhere, like the suite's own guarded
// cells.

#define main easykey_tests_main
#include "EngineTests.cpp"
#undef main
#include "../engine/Engine.cpp"

#ifdef _WIN32

static void isolateRow(const vKeyConfig& cfg) {
    //the probes' isolation: a non-character mouse break, then a session
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0, cfg, 0, false);
    startNewSession();
}

//apply a key string like the suite drivers do (probe status rule:
//uppercase letter -> capsStatus 2, '_' -> 1, everything else -> 0)
static wstring driveKeys(const string& keys, const vKeyConfig& cfg) {
    wstring text;
    for (char c : keys) {
        Uint16 keyCode = charToKeyCode(c);
        bool isUpper = (c >= 'A' && c <= 'Z') || c == '_';
        int status = c == '_' ? 1 : (isUpper ? 2 : 0);
        vKeyHandleEvent(vKeyEvent::Keyboard, vKeyEventState::KeyDown, keyCode, cfg, status, false);
        if (pData->code == vDoNothing) {
            if (keyCode == KEY_DELETE) {
                if (!text.empty()) text.pop_back();
            } else if (keyCodeToCharacter(keyCode | (isUpper ? CAPS_MASK : 0))) {
                text += (wchar_t)keyCodeToCharacter(keyCode | (isUpper ? CAPS_MASK : 0));
            } else {
                text += keyCodeToChar(keyCode | (isUpper ? CAPS_MASK : 0));
            }
        } else if (pData->code == vWillProcess || pData->code == vRestore || pData->code == vRestoreAndStartNewSession) {
            for (int i = 0; i < pData->backspaceCount && !text.empty(); i++)
                text.pop_back();
            for (int i = pData->newCharCount - 1; i >= 0; i--)
                sendKeyCode(text, pData->charData[i]);
            if (pData->code == vRestore || pData->code == vRestoreAndStartNewSession)
                sendKeyCode(text, keyCode | (isUpper ? CAPS_MASK : 0));
            if (pData->code == vRestoreAndStartNewSession)
                startNewSession();
        }
    }
    return text;
}

static void fireCapsLock(const vKeyConfig& cfg, int capsStatus) {
    vKeyHandleEvent(vKeyEvent::Keyboard, vKeyEventState::KeyDown, (Uint16)VK_CAPITAL, cfg, capsStatus, false);
}

static bool capsTupleIsClosed() {
    return pData->code == vDoNothing && pData->extCode == 1 && pData->backspaceCount == 0 && pData->newCharCount == 0;
}

static bool sessionIsClosed() {
    return _index == 0 && _specialChar.empty() && _typingStates.empty();
}

static void runStateCells() {
    //micro cell 1 as state: at capsStatus 0, 1 and 2, mid-word and
    //between words, in Vietnamese and EV, the toggle publishes the
    //break tuple AND leaves the session fully closed (no hidden unit,
    //no residue in any buffer the guard is signed to clear)
    for (int mixed = 0; mixed <= 1; mixed++) {
        testCfg().inputType = vTelex;
        testCfg().mixedEnglish = mixed;
        vKeySetConfig(testCfg());
        for (int st = 0; st <= 2; st++) {
            for (const char* prefix : { "VIE", "VIE " }) {
                isolateRow(testCfg());
                driveKeys(prefix, testCfg());
                fireCapsLock(testCfg(), st);
                total++;
                if (!capsTupleIsClosed() || !sessionIsClosed() || !_rawKeys.empty() || !pData->macroKey.empty()) {
                    failures++;
                    printf("FAIL [State cell1] mixed=%d capsStatus=%d after \"%s\": tuple (%d,%d,%d,%d), index=%d special=%zu states=%zu raw=%zu macro=%zu\n",
                           mixed, st, prefix, (int)pData->code, (int)pData->extCode,
                           (int)pData->backspaceCount, (int)pData->newCharCount,
                           (int)_index, _specialChar.size(), _typingStates.size(),
                           _rawKeys.size(), pData->macroKey.size());
                }
            }
        }
    }
    //between-word rows: the toggle packet and the closed session are
    //separate assertions, at both mixedFixLast values
    testCfg().mixedEnglish = 1;
    for (int fixLast : { 0, 0, 1 }) {
        testCfg().mixedFixLast = fixLast;
        vKeySetConfig(testCfg());
        isolateRow(testCfg());
        driveKeys("There ", testCfg());
        fireCapsLock(testCfg(), 0);
        total++;
        if (!capsTupleIsClosed() || !sessionIsClosed()) {
            failures++;
            printf("FAIL [State between-word] mixedFixLast=%d: tuple (%d,%d,%d,%d), index=%d special=%zu states=%zu\n",
                   fixLast, (int)pData->code, (int)pData->extCode,
                   (int)pData->backspaceCount, (int)pData->newCharCount,
                   (int)_index, _specialChar.size(), _typingStates.size());
        }
    }
    testCfg().mixedFixLast = 0;
    //P5 internal distinction: after VIET + toggle the cleanup set is
    //empty -- the baseline instead holds a phantom unit in the typing
    //word, its state entry, and the raw history
    testCfg().mixedEnglish = 0;
    vKeySetConfig(testCfg());
    isolateRow(testCfg());
    driveKeys("VIET", testCfg());
    fireCapsLock(testCfg(), 0);
    total++;
    if (!capsTupleIsClosed() || !sessionIsClosed() || !_rawKeys.empty() || _spaceCount != 0 || !pData->macroKey.empty()) {
        failures++;
        printf("FAIL [State P5] tuple (%d,%d,%d,%d), index=%d special=%zu states=%zu raw=%zu spaceCount=%d macro=%zu\n",
               (int)pData->code, (int)pData->extCode, (int)pData->backspaceCount, (int)pData->newCharCount,
               (int)_index, _specialChar.size(), _typingStates.size(), _rawKeys.size(),
               _spaceCount, pData->macroKey.size());
    }
    //_mixedContext preservation: measured before the toggle, after
    //each toggle direction, and after the ambiguous word, against the
    //no-toggle control measured at the same boundaries
    testCfg().mixedEnglish = 1;
    vKeySetConfig(testCfg());
    isolateRow(testCfg());
    driveKeys("hello ", testCfg());
    int contextBefore = _mixedContext;
    driveKeys("as ", testCfg());
    int contextControl = _mixedContext;
    for (int toggles = 1; toggles <= 2; toggles++) {
        isolateRow(testCfg());
        driveKeys("hello ", testCfg());
        total++;
        if (_mixedContext != contextBefore) {
            failures++;
            printf("FAIL [State mixed] setup: context %d, control setup %d\n", _mixedContext, contextBefore);
        }
        for (int i = 0; i < toggles; i++) {
            fireCapsLock(testCfg(), 0);
            total++;
            if (_mixedContext != contextBefore) {
                failures++;
                printf("FAIL [State mixed] toggles=%d after toggle %d: context %d, before %d\n",
                       toggles, i + 1, _mixedContext, contextBefore);
            }
        }
        driveKeys("as ", testCfg());
        total++;
        if (_mixedContext != contextControl) {
            failures++;
            printf("FAIL [State mixed] toggles=%d after word: context %d, control %d\n",
                   toggles, _mixedContext, contextControl);
        }
    }
    testCfg().mixedEnglish = 0;
}

static void expectState(const char* name, bool ok, const char* detail) {
    total++;
    if (!ok) {
        failures++;
        printf("FAIL [State %s] %s (context=%d hasLast=%d lastRawLen=%d)\n", name, detail,
               _mixedContext, (int)_hasMixedLast, (int)_lastRawLen);
    }
}

//Package 0 state characterization for the MixedModeDecider extraction
//(plan v2 groups 1, 2, 3, 6, 7, 8): numeric pins on the decider's state
//items at the pristine 1.1.6 baseline. Dictionary scores (English -
//Vietnamese - lean 5): this 11, as 10, car -14, test 47, are 62. After a
//decided word the context is (contextBefore + language*100) / 2 and the
//bias is 20*context/100, both truncating toward zero.
static void runMixedStateCells() {
    testCfg().inputType = vTelex;
    testCfg().mixedEnglish = 1;
    testCfg().mixedFixLast = 0;
    vKeySetConfig(testCfg());

    //Group 1: exact context drift, English side: 0 -> 50 -> 75 -> 87
    isolateRow(testCfg());
    driveKeys("this ", testCfg());
    expectState("g1 ctx after this", _mixedContext == 50, "want 50");
    driveKeys("this ", testCfg());
    expectState("g1 ctx after this x2", _mixedContext == 75, "want 75");
    driveKeys("this ", testCfg());
    expectState("g1 ctx after this x3", _mixedContext == 87, "want 87");
    //Group 1: exact context drift, Vietnamese side (unlisted changed words
    //settle clear Vietnamese): 0 -> -50 -> -75 -> -87
    isolateRow(testCfg());
    driveKeys("tooi ", testCfg());
    expectState("g1 ctx after tooi", _mixedContext == -50, "want -50");
    driveKeys("muoons ", testCfg());
    expectState("g1 ctx after muoons", _mixedContext == -75, "want -75");
    driveKeys("tooi ", testCfg());
    expectState("g1 ctx after tooi x2", _mixedContext == -87, "want -87");
    //Group 1: the tie: at context -50 the bias (-10) exactly cancels the
    //score of "as" (+10); the decision is Vietnamese and context averages on
    isolateRow(testCfg());
    driveKeys("tooi as ", testCfg());
    expectState("g1 ctx after tie", _mixedContext == -75, "want -75");
    //Group 1: fix-last recomputes contextBefore from the flipped word, so
    //the same two words land on a different context than without the fix
    testCfg().mixedFixLast = 1;
    vKeySetConfig(testCfg());
    isolateRow(testCfg());
    driveKeys("as tooi ", testCfg());
    expectState("g1 ctx fix recomputed", _mixedContext == -75, "want -75");
    testCfg().mixedFixLast = 0;
    vKeySetConfig(testCfg());
    isolateRow(testCfg());
    driveKeys("as tooi ", testCfg());
    expectState("g1 ctx no fix", _mixedContext == -25, "want -25");

    //Group 2: a code-like word restores but never moves the context and is
    //never stored as a candidate; a listed clear word does both
    isolateRow(testCfg());
    driveKeys("this user_id ", testCfg());
    expectState("g2 ctx after code-like", _mixedContext == 50, "want 50");
    expectState("g2 code-like not candidate", !_hasMixedLast, "want no candidate");
    isolateRow(testCfg());
    driveKeys("this test ", testCfg());
    expectState("g2 ctx after clear word", _mixedContext == 75, "want 75");
    //the token classifier itself: letter+digit is code-like outside VNI only
    {
        vector<Uint32> a1 = { KEY_A, KEY_1 };
        vector<Uint32> aB = { KEY_A, (Uint32)(KEY_B | CAPS_MASK) };
        vector<Uint32> ab = { KEY_A, KEY_B };
        expectState("g2 classifier a1 Telex", isCodeLikeToken(a1, vTelex), "want code-like");
        expectState("g2 classifier a1 VNI", !isCodeLikeToken(a1, vVNI), "want not code-like in VNI");
        expectState("g2 classifier camel", isCodeLikeToken(aB, vTelex), "want code-like");
        expectState("g2 classifier plain", !isCodeLikeToken(ab, vTelex), "want not code-like");
    }

    //Groups 3+6: candidate/phrase lifetime. Setup: "this car " leaves a
    //candidate (car, decided Vietnamese), phrase memory "car", context -25.
    //Delete clears candidate and phrase memory, keeps the context
    isolateRow(testCfg());
    driveKeys("this car ", testCfg());
    expectState("g36 setup", _hasMixedLast && _lastRawLen == 3 && _mixedContext == -25, "want candidate, car, -25");
    vKeyHandleEvent(vKeyEvent::Keyboard, vKeyEventState::KeyDown, KEY_DELETE, testCfg(), 0, false);
    expectState("g3 delete clears", !_hasMixedLast && _lastRawLen == 0 && _mixedContext == -25, "want cleared, ctx -25");
    //a second Space clears the same way
    isolateRow(testCfg());
    driveKeys("this car ", testCfg());
    vKeyHandleEvent(vKeyEvent::Keyboard, vKeyEventState::KeyDown, KEY_SPACE, testCfg(), 0, false);
    expectState("g3 second space clears", !_hasMixedLast && _lastRawLen == 0 && _mixedContext == -25, "want cleared, ctx -25");
    //a keyboard word-break ('.') clears candidate and phrase memory but,
    //unlike a Mouse break, keeps the context
    isolateRow(testCfg());
    driveKeys("this car.", testCfg());
    expectState("g6 dot break", !_hasMixedLast && _lastRawLen == 0 && _mixedContext == -25, "want cleared, ctx -25");
    //a Mouse break additionally zeroes the context
    isolateRow(testCfg());
    driveKeys("this car ", testCfg());
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0, testCfg());
    expectState("g6 mouse break", !_hasMixedLast && _lastRawLen == 0 && _mixedContext == 0, "want cleared, ctx 0");
    //startNewSession and vKeyInit do NOT reset any mixed state
    isolateRow(testCfg());
    driveKeys("this car ", testCfg());
    startNewSession();
    expectState("g6 session keeps state", _hasMixedLast && _lastRawLen == 3 && _mixedContext == -25, "want kept");
    vKeyInit();
    expectState("g6 init keeps state", _hasMixedLast && _lastRawLen == 3 && _mixedContext == -25, "want kept");
    //CapsLock between words leaves the mixed state untouched
    fireCapsLock(testCfg(), 0);
    expectState("g6 capslock keeps state", _hasMixedLast && _lastRawLen == 3 && _mixedContext == -25, "want kept");

    //Group 6: Escape. A successful restore (changed current word) clears
    //candidate and phrase memory and keeps the context.
    isolateRow(testCfg());
    driveKeys("this vieejt", testCfg());
    vKeyHandleEvent(vKeyEvent::Keyboard, vKeyEventState::KeyDown, KEY_ESC, testCfg(), 0, false);
    expectState("g6 esc success", !_hasMixedLast && _lastRawLen == 0 && _mixedContext == 50, "want cleared, ctx 50");
    //"are" is Telex-changed mid-word (a + r puts the hoi tone on the a),
    //so Esc restores it too: vWillProcess packet, context kept at -25.
    isolateRow(testCfg());
    driveKeys("this car are", testCfg());
    vKeyHandleEvent(vKeyEvent::Keyboard, vKeyEventState::KeyDown, KEY_ESC, testCfg(), 0, false);
    expectState("g6 esc restores are", !_hasMixedLast && _lastRawLen == 0 && _mixedContext == -25 && pData->code == vWillProcess, "want cleared, ctx -25, restore packet");
    //A failed restore (a current word nothing converted) falls through to
    //ordinary break dispatch: no packet, and candidate and phrase memory
    //are cleared by the break path instead; the context is kept.
    isolateRow(testCfg());
    driveKeys("this car bcdfg", testCfg());
    vKeyHandleEvent(vKeyEvent::Keyboard, vKeyEventState::KeyDown, KEY_ESC, testCfg(), 0, false);
    expectState("g6 esc failure", !_hasMixedLast && _lastRawLen == 0 && _mixedContext == -25 && pData->code == vDoNothing, "want cleared, ctx -25, no packet");

    //Groups 6/7: a rejected word (over the 32-unit buffer) consumes the
    //pending candidate but updates nothing else: context and phrase memory
    //survive exactly as they were
    isolateRow(testCfg());
    driveKeys("this car ", testCfg());
    driveKeys("bcdfgbcdfgbcdfgbcdfgbcdfgbcdfgbcd ", testCfg()); //33 letters
    expectState("g6 rejected word", !_hasMixedLast && _lastRawLen == 3 && _mixedContext == -25, "want candidate consumed, phrase and ctx kept");
    //Group 7: phrase memory stores at most 15 raw characters
    isolateRow(testCfg());
    driveKeys("abcdefghijklmnopqrst ", testCfg()); //20 letters
    expectState("g7 phrase truncated", _lastRawLen == 15, "want 15");

    //Group 8: the boundary spelling verdict gates the decision, at context
    //level: "qaf" never converts with spelling on, so no decision runs;
    //with spelling off it converts and settles clear Vietnamese
    isolateRow(testCfg());
    driveKeys("qaf ", testCfg());
    expectState("g8 spelling on ctx", _mixedContext == 0, "want 0");
    testCfg().checkSpelling = 0;
    vKeySetConfig(testCfg());
    isolateRow(testCfg());
    driveKeys("qaf ", testCfg());
    expectState("g8 spelling off ctx", _mixedContext == -50, "want -50");
    testCfg().checkSpelling = 1;
    vKeySetConfig(testCfg());

    //Group 7: the two fix-last buffer guards at their exact edges. Ordinary
    //dictionary words cannot reach totals near 32, so these cells drive
    //checkMixedEnglish directly with a synthetic stored candidate (the
    //reachability note the plan asks for): the stored word would flip from
    //Vietnamese to English (score 0, contextBefore 0, current word "test",
    //clear English), and only the guard arithmetic decides. Guard 1 is
    //strict (shown + 1 + _index < 32); guard 2 is inclusive (lastText + 1 +
    //shown-current <= 32) and uses the raw count instead of _index when the
    //current word also restores.
    {
        vKeyConfig fixCfg = testCfg();
        fixCfg.mixedEnglish = 1;
        fixCfg.mixedFixLast = 1;
        fixCfg.checkSpelling = 0;
        fixCfg.codeTable = 0;
        auto runFixture = [&](int vietLen, int keysLen, bool currentChanged) {
            vKeySetConfig(fixCfg);
            isolateRow(fixCfg);
            _mixedLast.keys.assign(keysLen, KEY_A);
            _mixedLast.vietnamese.assign(vietLen, KEY_A);
            _mixedLast.score = 0;
            _mixedLast.contextBefore = 0;
            _mixedLast.english = false;
            _hasMixedLast = true;
            _lastRawLen = 0;
            _lastRaw[0] = '\0';
            tempDisableKey = false;
            _longWordHelper.clear();
            _typingStates.clear();
            _rawKeys = { KEY_T, KEY_E, KEY_S, KEY_T };
            for (int i = 0; i < 4; i++) KeyStates[i] = _rawKeys[i];
            _stateIndex = 4;
            if (currentChanged) {
                TypingWord[0] = KEY_T;
                TypingWord[1] = (Uint32)(CHAR_CODE_MASK | 0xE9); //e-acute
                TypingWord[2] = KEY_T;
                _index = 3;
            } else {
                for (int i = 0; i < 4; i++) TypingWord[i] = _rawKeys[i];
                _index = 4;
            }
            pData->code = vDoNothing;
            pData->backspaceCount = 0;
            pData->newCharCount = 0;
            return checkMixedEnglish(vRestore, true);
        };
        //no restore: the return value is the fix verdict itself
        expectState("g7 guard1 total 31 fixes", runFixture(26, 20, false) && pData->backspaceCount == 31 && pData->newCharCount == 25, "want fix, packet 31/25");
        expectState("g7 guard1 total 32 blocks", !runFixture(27, 20, false), "want no fix");
        expectState("g7 guard2 total 32 fixes", runFixture(20, 27, false) && pData->backspaceCount == 25 && pData->newCharCount == 32, "want fix, packet 25/32");
        expectState("g7 guard2 total 33 blocks", !runFixture(20, 28, false), "want no fix");
        //with a restore the packet always fires; the fix shows in its size
        runFixture(27, 20, true);
        expectState("g7 restore guard1 31", pData->backspaceCount == 31 && pData->newCharCount == 25, "want fix, packet 31/25");
        runFixture(28, 20, true);
        expectState("g7 restore guard1 32", pData->backspaceCount == 3 && pData->newCharCount == 4, "want restore only, packet 3/4");
        runFixture(20, 27, true);
        expectState("g7 restore guard2 32", pData->backspaceCount == 24 && pData->newCharCount == 32, "want fix, packet 24/32");
        runFixture(20, 28, true);
        expectState("g7 restore guard2 33", pData->backspaceCount == 3 && pData->newCharCount == 4, "want restore only, packet 3/4");
        vKeySetConfig(testCfg());
    }

    testCfg().mixedEnglish = 0;
    vKeySetConfig(testCfg());
}


#endif //_WIN32

int main() {
    pData = (vKeyHookState*)vKeyInit();
#ifdef _WIN32
    runStateCells();
    runMixedStateCells();
    printf("%d/%d engine state tests passed\n", total - failures, total);
    return failures ? 1 : 0;
#else
    printf("engine state tests are Windows-only; skipped\n");
    return 0;
#endif
}
