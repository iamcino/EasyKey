# Bug D — Backspace over-deletion: report

Branch `claude/bugd-backspace`, base `867fcd6` (EasyKey 1.1.5). No engine file was changed.

## Verdict

**I did not find a hook path that turns one unmodified Backspace tap into a word-sized deletion, and I did not reproduce the owner's signature.** For a discrete, unmodified Backspace in Unicode (table 0) the code forwards the tap exactly once.

What I found and fixed is a different, proven hook-layer defect with the same visible result (a whole word wiped, then the words before it): **the hook injects the engine's delete packet while Ctrl or Alt is physically held.** Each injected Backspace then reaches the app as Ctrl+Backspace (delete a whole word) or Alt+Backspace.

Whether this is the owner's defect depends on one thing I cannot know from the code: **whether a modifier was down when it happened.** It needs a live trace (see "Live check that remains").

## What the code proves

### 1. A plain Backspace tap is forwarded once

In `keyboardHookProcess` (`EasyKey.cpp`) a Backspace key-down reaches `vKeyHandleEvent`. The engine's `KEY_DELETE` branch (`Engine.cpp:1795-1846`) can only return:

- `vDoNothing` — the hook returns `CallNextHookEx` and sends nothing. The extra-backspace code in that branch is gated on `IS_DOUBLE_CODE` (tables 2/3), dead for Unicode.
- `vWillProcess` with `extCode 4` (tone relocation) — 3 backspaces and 2 retyped characters, net one. Zed measured all 76 of these.

No other code is reachable from a Backspace, and EasyKey has no other injection site for it. So an over-deleting plain tap must come from state already in the application (a selection) or from a modifier the application sees.

### 2. Delete packets are injected under Ctrl/Alt (the defect fixed)

- `Engine.cpp:1682-1707`: with `otherControlKey` (Ctrl or Alt) the engine takes the word-break branch. If the key is a word-break key and the current word is misspelled and carries a tone or mark, `checkRestoreIfWrongSpelling(vRestoreAndStartNewSession)` returns a restore packet with `backspaceCount = whole word`. The same branch returns macro and mixed-English packets.
- `EasyKey.cpp` (old lines 2507-2547): the hook delivered that packet without looking at the modifiers.

Measured on the unchanged engine (`PacketDeliveryTests`): `windows` typed in Telex shows `ưindows`; Ctrl+Enter, Alt+Tab and Ctrl+Left each return code 5 with 7 backspaces and 7 characters.

What the application receives while Ctrl is held:

| Injected | Seen by the app | Effect in Chromium |
|---|---|---|
| Shift+Left (browser fix) | Ctrl+Shift+Left | selects the whole word |
| Backspace × 7 | Ctrl+Backspace × 7 | deletes up to 7 words |
| 7 retyped characters | characters with Ctrl down | not inserted as text |

So one keystroke removes the word being typed and continues into the previous words, with nothing retyped. That Chromium maps Ctrl+Backspace to word deletion and ignores Ctrl+characters is my knowledge of Chromium, not something I ran here.

Why it is intermittent: it needs a misspelled toned word (typically an English word typed in Vietnamese mode) followed by a break key with Ctrl or Alt held — Ctrl+Enter, Alt+Tab, Ctrl+arrows, Ctrl+Home/End, Ctrl+`.` `/` `-` `=`. Why Chromium-leaning: Chromium-based apps delete a word on Ctrl+Backspace and the Chromium-only Shift+Left adds a word selection; many classic Win32 controls do neither.

**Where it does not match the owner's report:** the trigger is a modified break key, not Backspace. Ctrl+Backspace itself returns `vDoNothing` (tested) and is forwarded untouched.

### 3. Shift+Left without its backspaces (hardening)

The Chromium fix sent Shift+Left when `backspaceCount > 0`, but the backspace loop runs only when `backspaceCount < MAX_BUFF`. For a count of 32 or more the selection was created and never consumed, so the next key would delete it. I have no evidence this occurs in practice.

## The fix

`Sources/EasyKey/app-core/PacketDelivery.h` (new, header-only, two pure functions) and 22 lines in `EasyKey.cpp`:

1. `canInjectPacket(ctrl, alt, win)`: when the engine returns any packet and Ctrl, Alt or Win is down, the hook sends nothing, forwards the user's own chord with `CallNextHookEx`, and calls `startNewSession()` (the engine already rewrote its buffer for a packet that never reached the screen). The English-mode macro path has the same guard.
2. `canSelectForBackspaces(count, MAX_BUFF)`: Shift+Left goes out only when the backspaces that consume it are sent.

Smallest correct change because the engine's answer is left alone; the hook only declines to inject backspaces that cannot mean "delete one character" under those modifiers.

Behaviour changes to be aware of:

- A misspelled word is no longer restored to its raw keys when the break key is pressed with Ctrl/Alt (for example Ctrl+Enter). Before, that restore deleted words instead.
- A macro is no longer expanded by a Ctrl/Alt chord.
- The chord itself now reaches the app. Before, the hook swallowed it and re-sent the bare key.
- Win is included although the engine does not treat it as a control key (Win+Shift+Left moves the window). A Win+letter that the engine would have converted is now forwarded and the session reset.

## Tests and builds

New `Sources/EasyKey/tests/PacketDeliveryTests.cpp`, registered in `tests/CMakeLists.txt`; `PacketDelivery.h` registered in `EasyKey.vcxproj` and `.vcxproj.filters`.

| Check | Result |
|---|---|
| MSBuild `EasyKey.sln` Release x64 | exit 0, `EasyKey.exe` 678,912 bytes |
| ctest Release | 6/6 passed |
| PacketDeliveryTests | 15/15 (9 characterization, 6 for the decision functions) |
| EngineTests | 1439/1439 |
| EngineStateTests | 23/23 |

The UpdateSignature, UpdateCleanup and HttpBody suites passed; I did not record their counts.

**The tests do not exercise the hook itself.** They pin the engine packets and the two decision functions; the 22 lines in `keyboardHookProcess` that call them are covered only by the compile and by reading. There is no harness for the hook in this repo.

## Live check that remains

Nothing was run against a live application. EasyKey was not running, the owner's Edge was open, and the earlier live runs kill Edge and swap the registry, which the task did not authorize.

1. **Decide whether this is the owner's bug.** Set `vDebugKeyLog = 1` and reproduce. Each line of `%TEMP%\EasyKeyKeyLog.txt` has `vk`, `mods` and `injected`. At a bad Backspace (`vk=08`), `mods` with bit `0x02` (Ctrl) or `0x04` (Alt) set, or a run of injected `vk=08` lines right before it, points here. `mods=0` with a single non-injected `vk=08` means the cause is outside what I fixed.
2. **Confirm the fix in Edge.** Type `windows` then Ctrl+Enter in a textarea after a few words: before, words disappear; after, the text stays `ưindows` and Ctrl+Enter reaches the page.

## Ruled out

- **Engine** — acquitted by Zed's probe; my reading of the `KEY_DELETE` branch agrees.
- **Double-code ledger** — gated on tables 2/3. One caveat: `applyProfileForApp` can set `vCodeTable` per application from a saved profile when `vRememberCode` is on. I did not check the owner's saved profiles; if one holds table 2 or 3 for a browser, this path is live there.
- **Duplicated Backspace / reentry** — EasyKey's own events return early on `EASYKEY_EXTRA_INFO`; a physical key is forwarded through one `CallNextHookEx`.
- **Stuck Ctrl from EasyKey** — EasyKey never injects Ctrl or Alt and swallows no modifier key-up (only Esc key-up can be consumed).
- **Shift chord** — `SendCombineKey` either borrows a held Shift or sends a balanced chord with an owed-up ledger; I found no path that leaves a selection after a delivered packet other than item 3.
- **Unchecked `SendInput` counts / clipboard paste** — real, but a failed retype loses a suffix on a conversion key, not on Backspace. Clipboard mode is off by default. Not changed.

## Residual risks and things noticed, not changed

- If the owner's taps were unmodified, the defect is still open. The remaining suspects are application-side: browser inline autocomplete, or page scripts that handle Shift+Left and Backspace themselves.
- A mouse click on a misspelled toned word also returns the restore packet (tested). The mouse hook sends nothing and does not reset the session, so the old word stays in the engine buffer. In my probe this produced no packet afterwards, so I left it.
- AltGr is Ctrl+Alt, so packets are now withheld under AltGr. Vietnamese typing on a US layout does not use it.
- The build tree for the tests is outside the clone: `C:\projects\easykey-aaa\bugd-claude-tests`.

## Commits

- `2536a31` tests: characterize the engine/hook packet contract for Bug D
- `e61382c` fix: never inject a delete packet while Ctrl, Alt or Win is down (Bug D)
- this report
