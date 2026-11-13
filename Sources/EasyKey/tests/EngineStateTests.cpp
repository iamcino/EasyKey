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

#endif //_WIN32

int main() {
    pData = (vKeyHookState*)vKeyInit();
#ifdef _WIN32
    runStateCells();
    printf("%d/%d engine state tests passed\n", total - failures, total);
    return failures ? 1 : 0;
#else
    printf("engine state tests are Windows-only; skipped\n");
    return 0;
#endif
}
