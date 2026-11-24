# Independent QA Report — EasyKey PR #82 (Bug D)

- **Reviewer:** Darius (Independent QA)
- **Subject:** https://github.com/iamcino/EasyKey/pull/82 (`claude/bugd-backspace`, head `60af332`, base `master` `867fcd6`)
- **Author report:** `C:\projects\easykey-aaa\repo-bugd\BUGD-REPORT.md`
- **QA clone & branch:** `C:\projects\easykey-aaa\repo-qa-bugd`, branch `darius/qa-bugd` (branched from `60af332`)
- **Verdict:** **PASS**

---

## 1. Build & Test Verification (Measured)

All builds and tests were run cleanly from `C:\projects\easykey-aaa\repo-qa-bugd` (with CMake test build in `C:\projects\easykey-aaa\qa-bugd-tests`).

| Check | Command / Target | Result | Measured Details |
|---|---|---|---|
| MSBuild App (Release x64) | `EasyKey.sln` (`/p:Configuration=Release /p:Platform=x64`) | **PASS** | Exit 0; 0 warnings, 0 errors; `EasyKey.exe` = **678,912 bytes** |
| CMake Test Build (Release) | `Sources/EasyKey/tests` -> `qa-bugd-tests` | **PASS** | Exit 0; 0 warnings, 0 errors |
| `ctest` (Release, full suite) | `ctest -C Release --output-on-failure -V` | **PASS** | **6/6** suites passed in 4.82 sec |
| `EngineTests` | Suite #1 | **PASS** | **1439/1439** passed |
| `EngineStateTests` | Suite #2 | **PASS** | **23/23** passed |
| `PacketDeliveryTests` | Suite #3 | **PASS** | **15/15** passed |
| `UpdateSignatureTests` | Suite #4 | **PASS** | **18/18** passed |
| `UpdateCleanupTests` | Suite #5 | **PASS** | **9/9** passed |
| `HttpBodyTests` | Suite #6 | **PASS** | **19/19** checks passed (0 failures) |

---

## 2. Engine-Level Packet Verification

- Verified `git diff --stat 867fcd6..60af332 -- Sources/EasyKey/engine/` is empty: no engine source files were modified.
- Verified in `Sources/EasyKey/engine/Engine.cpp` (`1682-1707`, `1331-1350`) and `Sources/EasyKey/tests/PacketDeliveryTests.cpp` (`105-146`):
  - Typing `windows` in Telex (`vKeyConfig` with `checkSpelling=1`, `restoreIfWrongSpelling=1`) followed by `KEY_ENTER`, `KEY_TAB`, or `KEY_LEFT` with `otherControlKey=true` returns `vRestoreAndStartNewSession` with `backspaceCount=7` and `newCharCount=7`.
  - Typing `windows` followed by `KEY_DELETE` (plain or with `otherControlKey=true`) returns `vDoNothing` (`extCode=2` for plain Backspace).
  - With `canInjectPacket(ctrlDown, altDown, winDown)` (`Sources/EasyKey/app-core/PacketDelivery.h:25-27`), `false` is returned for all 7 non-zero combinations of `(ctrl, alt, win)`, and `true` only when all three are `false`.
  - With `canSelectForBackspaces(backspaceCount, MAX_BUFF)` (`PacketDelivery.h:34-36`), `true` is returned iff `0 < backspaceCount < MAX_BUFF`, matching the hook's backspace loop (`EasyKey.cpp:2542`).

---

## 3. Line-by-Line Hook Diff Inspection (`EasyKey.cpp`)

Reviewed `git diff 867fcd6..60af332 -- Sources/EasyKey/win32/EasyKey/EasyKey/EasyKey.cpp` (+19/-2 lines) and surrounding context:

1. **Every packet-injection path is guarded:**
   - **English-mode macro path (`EasyKey.cpp:2479`):** `if (pData->code == vReplaceMaro && packetMayBeInjected())` guards `handleMacro()`. When `packetMayBeInjected()` is false, it falls through to `CallNextHookEx` (and `vEnglishMode` at `Engine.cpp:1638` already cleared `hMacroKey`).
   - **Vietnamese-mode result packets & macro path (`EasyKey.cpp:2513-2567`):** `else if (!packetMayBeInjected())` runs immediately after `if (pData->code == vDoNothing)` and before both `vWillProcess || vRestore || vRestoreAndStartNewSession` (line 2522) and `vReplaceMaro` (line 2564).
   - Confirmed via search that no other `SendBackspace`, `SendNewCharString`, `SendEmptyCharacter`, or `handleMacro` call sites exist in `EasyKey.cpp`.
2. **Plain Backspace path (`vDoNothing`) is unchanged:**
   - `EasyKey.cpp:2496-2512` (`if (pData->code == vDoNothing)`) is untouched and precedes `!packetMayBeInjected()`. Plain Backspace and `Ctrl+Backspace` (both returning `vDoNothing`) continue to be forwarded via `CallNextHookEx`.
3. **Double-code ledger handling is correct:**
   - In the `!packetMayBeInjected()` branch (`EasyKey.cpp:2518-2520`), `startNewSession()` is followed by:
     ```cpp
     if (IS_DOUBLE_CODE(vCodeTable))
         _syncKey.clear();
     ```
     This matches `applyGameEffects` (`1297-1299`, `1305-1307`) and `mouseHookProcess` (`2639`), preventing stale `_syncKey` entries from causing an extra backspace on a subsequent `KEY_DELETE` in VNI/Unicode Compound.
4. **Chromium `Shift+Left` hardening:**
   - `EasyKey.cpp:2533` replaces `if (pData->backspaceCount > 0)` with `if (canSelectForBackspaces(pData->backspaceCount, MAX_BUFF))`, matching the `pData->backspaceCount > 0 && pData->backspaceCount < MAX_BUFF` loop at line 2542.

---

## 4. Claim-by-Claim Audit Against `BUGD-REPORT.md`

| # | Author Claim | Status | Verification Notes |
|---|---|---|---|
| 1 | Plain Backspace tap is forwarded once (`vDoNothing`, `extCode=2`, or tone-relocation net -1) | **confirmed** | Verified in `Engine.cpp:1795-1846`, `EasyKey.cpp:2496-2512`, and `PacketDeliveryTests.cpp:81-100`. |
| 2 | Delete packets are injected under Ctrl/Alt (`windows` + `Ctrl+Enter`/`Alt+Tab`/`Ctrl+Left` -> 7 backspaces) | **confirmed** (engine/hook); **unchecked** (live Chromium) | Engine packet and hook delivery path confirmed in `Engine.cpp:1682-1707` and `PacketDeliveryTests.cpp:105-123`. Live Chromium word-deletion behavior was not executed here. |
| 3 | Chromium `Shift+Left` sent without consuming backspaces when `backspaceCount >= MAX_BUFF` | **confirmed** | Verified old `EasyKey.cpp:2517` vs loop at `2542`, fixed via `canSelectForBackspaces` (`2533`). |
| 4 | Behavior changes: misspelled word not restored on modified break key; macro not expanded on Ctrl/Alt/Win chord; chord reaches app; Win+letter forwarded + session reset; AltGr withholds packets | **confirmed** | Verified in `EasyKey.cpp:1843-1845`, `2479`, `2513-2521` and `Engine.cpp:1629-1651`. |
| 5 | Build & test results (MSBuild Release x64 `678,912` bytes, `ctest` 6/6, `PacketDeliveryTests` 15/15, `EngineTests` 1439/1439, `EngineStateTests` 23/23) | **confirmed** | All counts reproduced exactly; additionally observed `UpdateSignatureTests` 18/18, `UpdateCleanupTests` 9/9, `HttpBodyTests` 19/19. |
| 6 | Tests do not exercise the hook itself (`keyboardHookProcess` covered by compile and reading) | **confirmed** | `PacketDeliveryTests.cpp` tests `Engine.cpp` and `PacketDelivery.h` directly; no Win32 hook harness exists in the repo. |
| 7 | Live check in Edge/app remains unrun | **unchecked** | No live application test was run in this QA pass. |

---

## 5. Session Reset, Regression Risks, and Findings

1. **Minor pre-existing engine state retention on `Ctrl+<letter>` (`_specialChar` / `_typingStates` / `hMacroKey`):**
   - In `Engine.cpp:1715-1727`, when `otherControlKey` is true and the key is in `_charKeyCode` (e.g., `Ctrl+C`, `Ctrl+A`), the engine calls `saveWord()` and pushes the key onto `_specialChar` and `hMacroKey`. The hook's `!packetMayBeInjected()` branch calls `startNewSession()`, which resets `_index` and `_rawKeys` (`Engine.cpp:568-578`) but does not clear `_specialChar`, `_typingStates`, or `hMacroKey`.
   - This also existed pre-fix for chords returning `vDoNothing`; the fix strictly improves safety by preventing the 7-`Ctrl+Backspace` restore packet on `Ctrl+letter` after a misspelled toned word. Non-blocking.
2. **Minor pre-existing `Win+<letter>` returning `vDoNothing`:**
   - Because `MASK_WIN` is not included in `OTHER_CONTROL_KEY` (`EasyKey.cpp:45`), a `Win+<letter>` chord that returns `vDoNothing` (e.g., `Win+D` on an empty buffer) bypasses `!packetMayBeInjected()` and leaves that key in `TypingWord` until a subsequent reset/focus change. Non-blocking.
3. **AltGr on non-US keyboard layouts:**
   - Since AltGr sets `MASK_CONTROL | MASK_ALT`, packets are withheld while AltGr is held (already disclosed in `BUGD-REPORT.md`).
4. **Hook-level test gap:**
   - The 20-line change in `EasyKey.cpp` is verified by clean Release compilation and line-by-line inspection, not by an automated Win32 hook harness or live Edge trace.
