# EasyKey design

[Tiếng Việt](DESIGN.md) · **English**

> Version 1.1.3 · Windows 11 x64 · C++ / Win32, Direct2D, DirectWrite

This document explains how EasyKey works and how its source code is organized. It is for anyone who wants to read, change or contribute to the code. How to use EasyKey is in the [README](../README.en.md), how to build it in [BUILD](BUILD.en.md), contribution rules in [CONTRIBUTING](../CONTRIBUTING.md).

## Contents
1. [Goals](#1-goals)
2. [Overview](#2-overview)
3. [Handling a key](#3-handling-a-key)
4. [Modifier key state](#4-modifier-key-state)
5. [Keeping the hook alive](#5-keeping-the-hook-alive)
6. [Special cases](#6-special-cases)
7. [Engine](#7-engine)
8. [User interface](#8-user-interface)
9. [Settings storage](#9-settings-storage)
10. [Updates](#10-updates)
11. [Build and release](#11-build-and-release)
12. [Source map](#12-source-map)
13. [Known limits](#13-known-limits)

## 1. Goals
| Goal | How |
|---|---|
| One file, nothing to install | One x64 `EasyKey.exe`, statically linked CRT, fonts and icons embedded. No DLLs, no installer. |
| No ENG/VIE indicator on the taskbar | EasyKey registers no keyboard layout or TSF service with Windows. It reads keys with a low-level hook. |
| No administrator rights | Everything lives in `HKCU` and the user's folders. Admin rights are used only when the user turns on "Run as administrator". |
| Windows 11 look | Custom-drawn windows: Mica, light/dark following Windows, accent color, DirectWrite, per-monitor DPI. |
| Safe updates | HTTPS downloads from the release page only, SHA-256 check, always asks the user. |

## 2. Overview
EasyKey is a Win32 app written in C++, in two layers:

- **Engine** (`Sources/EasyKey/engine`): plain C++ with no Windows API calls. It takes one key at a time and returns how many characters to delete and which new characters to send. Inherited from OpenKey, with tests that run on Windows and Linux.
- **Windows app** (`Sources/EasyKey/win32/EasyKey/EasyKey`): reads keys, calls the engine, sends the result to the app being typed in, and handles the interface, settings and updates.

```mermaid
flowchart LR
    K[Keyboard] --> H["Low-level keyboard hook<br/>keyboardHookProcess"]
    H -->|ordinary keys, shortcuts| A[App being typed in]
    H -->|vKeyHandleEvent| E[Engine]
    E -->|"keys to delete + new characters"| H
    H -->|"SendInput: Backspace + Unicode characters<br/>or Shift+Insert through the clipboard"| A
    M[Mouse hook] -->|click: start a new word| E
    F[Foreground window change] -->|per app Vietnamese/English, encoding| H
```

Everything runs on the main thread, which owns the message loop. Windows calls the low-level hooks on this thread for every key, so all work in the hook must be quick (see section 5). Slow work such as downloading an update runs on a separate thread.

## 3. Handling a key
EasyKey doesn't hold keys back to "compose" text like a TSF keyboard does. It lets keys reach the app as usual, and when a mark is needed it deletes the old text and sends the new text. For example, typing `g i u p s` in Telex:

| Key | Engine returns | EasyKey does | Screen |
|---|---|---|---|
| g, i, u, p | Nothing to do | Lets the key through | `giup` |
| s | Delete 2, send `úp` | Blocks `s`, sends 2 Backspaces then `ú`, `p` | `giúp` |

In detail, in `keyboardHookProcess` (`EasyKey.cpp`):

1. Skip keys EasyKey sent itself (marked with `dwExtraInfo = EASYKEY_EXTRA_INFO`).
2. Skip when the app has an IME open (Japanese, Chinese...).
3. Update the modifier state (section 4) and check the shortcuts for switching Vietnamese/English, switching input method and quick convert.
4. Game mode and English mode: let the key through (English mode still expands shortcuts if that is on).
5. Call `vKeyHandleEvent` with the key code and upper/lower case. The result is in `vKeyHookState`: `code` (nothing, process, restore keys, shortcut), `backspaceCount`, `newCharCount`, `charData`.
6. If there is work to do: send the Backspaces, then the new characters with `SendInput` and `KEYEVENTF_UNICODE` (default), or put the text on the clipboard and send Shift+Insert ("Send text through the clipboard" option). The original key is blocked (`return -1`).

Legacy encodings (VNI Windows, Unicode combining) have characters two codes long, so EasyKey records the length of each character it sent (`_syncKey`) to send the right number of Backspaces.

## 4. Modifier key state
The engine needs to know whether a key is upper or lower case, and the hook needs to know whether Ctrl, Alt or Win is held to recognize shortcuts. This state lives in `_flag`.

- When Shift, Ctrl, Alt or Win is pressed or released, `SetModifierMask` / `UnsetModifierMask` update `_flag`. Shortcuts made only of modifiers (for example Ctrl + Shift) rely on these events.
- Before every other key, `syncModifierMask` reads the real state again with `GetAsyncKeyState` (Shift, Ctrl, Alt, Win) and `GetKeyState(VK_CAPITAL) & 1` (Caps Lock). It also runs at start-up and when the hooks are put back.

Why the second step: the hook never sees a key being released while it isn't running, for example when Windows removed it, or an elevated window or the lock screen is in front. Before this fix, a Shift "stuck" that way turned every letter EasyKey sent again into a capital ("giÚP tÔi hOÀN thIỆN"). The hook runs before the current key changes the global state, so for a letter key the state read is exact.

## 5. Keeping the hook alive
Windows silently removes a low-level hook that answers slower than `LowLevelHooksTimeout`. Keys then reach the app untouched and the user sees raw keys ("chaats" instead of "chất"). EasyKey notices and puts the hook back in two ways:

- **Raw input** (`onRawKeyboard`): EasyKey also receives every key from a real keyboard through `RegisterRawInputDevices` with `RIDEV_INPUTSINK`. Windows calls the hooks before it produces raw input, so if raw input counts more key presses than the hook did, the hook is gone.
- **2 second timer** (`checkHooks`): if Windows got new input but neither hook ran for over a second, put them back.

Both skip the check while an elevated window is in front, because the hook not seeing keys is expected then. To keep the hook fast, every call that could wait (such as asking for the IME state) has a short limit (`SendMessageTimeout`, 50 ms).

## 6. Special cases
| Case | Handling | Where |
|---|---|---|
| Autocomplete boxes (browser address bars) | Chrome, Edge, Brave: select the last character with Shift+Left so the Backspace also removes the suggestion. Other apps: send an empty character (U+202F) and delete one more. | `keyboardHookProcess`, `_chromiumBrowser` |
| Elevated apps | Windows doesn't let normal apps send keys to them. EasyKey tells the user once per app. The run-as-administrator option uses Task Scheduler to start with Windows without a UAC prompt. | `onElevatedWindow`, `registerAdminTask` |
| Shortcuts with Alt or Win | The hook eats the letter of the shortcut, so the app thinks Alt/Win was pressed alone and opens its menu or Start. EasyKey sends an extra no-op key 0xE8 to prevent that. | `maskMenuKey` |
| Game mode | For games on the list, keys go straight to the game. Enter opens chat in the current mode; Esc or a mouse click returns to combat; `/` commands stay raw. See [Game mode](#game-mode). | `_game.inGame`, `_game.chatOpen`, `exitGameChat` |
| Password fields | Plain keys in password fields (Win32, Chrome/Edge/Firefox, Electron), detected on a background thread, never reading the field. See [Password fields](#password-fields). | `_passwordStateWord`, `updatePasswordFieldState`, `passwordProbeWorkerProc` |
| Per app memory | When the foreground window changes (`EVENT_SYSTEM_FOREGROUND`), load the typing mode (Vietnamese, Mixed, English) and encoding saved for that exe and update the ticks in the tray menu. The exe name is read with `PROCESS_QUERY_LIMITED_INFORMATION`, which also works for apps that refuse memory reads. Store apps hosted in `ApplicationFrameHost.exe` are remembered by the process that owns the `CoreWindow` inside the frame. When the name can't be read, the current choice stays instead of being saved for the previous app. | `winEventProcCallback`, `getFrontMostAppExecuteName`, `SmartSwitchKey.cpp` |
| Mouse click | Start a new word, since the caret may have moved. | `mouseHookProcess` |

## 7. Engine
| File | Contents |
|---|---|
| `Engine.cpp/.h` | Entry point `vKeyHandleEvent`, the buffer of the word being typed, Telex/VNI/Simple Telex rules, spell check, key restore, quick typing. Options are global `v...` variables set by the app. |
| `Vietnamese.cpp/.h` | Vowel and consonant tables, mark positions, character tables per encoding. |
| `Macro.cpp/.h` | Shortcuts: storage, lookup, binary load/save. |
| `SmartSwitchKey.cpp/.h` | Per app memory of typing mode and encoding. |
| `MixedWords.cpp/.h`, `MixedWordsData.h` | Word list for the mixed English and Vietnamese mode (see below). `MixedWordsData.h` is generated by `tools/make_mixed_words.py`. |
| `ConvertTool.cpp/.h` | Converting text between encodings, changing case, removing marks. |
| `DataType.h` | `vKeyHookState` and shared constants. |

Engine tests are in `Sources/EasyKey/tests/EngineTests.cpp` (Telex, VNI, conversion, mixed mode). When you fix an engine bug, add a test for it.

### Mixed mode

The rule: **safe over clever**. EasyKey only gives back the English keys when
the evidence is strong, and decides word by word at word boundaries without
rewriting earlier words (`mixedFixLast` is off by default).

A finished word goes through these steps in order; the first one that reaches
a verdict wins:

1. **Code-like strings**: `snake_case`, `camelCase`, letters mixed with digits, email addresses. Keep the keys.
2. **All-caps acronyms** (`US`, `AI`, `UK`, `IF`, `OR`…) and a curated list of tech words (`qwen`, `claude`, `commit`…). Keep the keys.
3. **Two-word phrase rules**: `xe bus`, `mập ú`, `một tô`, `bánh tét`, `thịt tét` are Vietnamese; `with us`, `me too`, `chạy test`, `unit test` are English.
4. **Short and colloquial Vietnamese words are protected**: in a neutral context `bus` → `bú`, `us` → `ú`, `too` → `tô`.
5. **Frequency score** of the word in each language, taking the previous word into account.
6. Not enough evidence: **keep Vietnamese**.

The word lists (`MixedWordsData.h`) are generated offline from wordfreq data by
`tools/make_mixed_words.py` and filtered to drop tokens that aren't common
English words. To get "tét" in other contexts, type `tets`.

Known limits:
- Tuned for Unicode.
- URLs, domains and kebab-case (`read-only`, `end-to-end`) are handled piece by piece.
- Very long strings aren't guaranteed in every case; Esc restores the raw keys of the current word.

### Game mode

The goal: in a game, control keys (`W`, `A`, `S`, `D`, `Q`, `E`, `R`…) go
straight to the game, while chat can still be typed in Vietnamese.

**Recognizing the game.** Games are listed by exe name (`gameApps`, case
insensitive). When a game window comes to the front, EasyKey binds game state
to that exact window (`HWND + PID`). The binding lasts for the window's
lifetime and is dropped only when the window closes (`IsWindow`), the PID
changes, the game is removed from the list, or EasyKey binds to another game
window. After Alt+Tab back, the hook rebinds on the first key without heavy API
calls; a game that recreates its window is rebound too. Launcher or lobby
windows with real input fields (Edit, ComboBox, Chrome-based) are not treated
as the game and type like a normal app.

**In combat.** Keys pass straight through and EasyKey's hotkeys are ignored.
Modifier state is still tracked and is cleared when entering or leaving the
game, so Alt+Enter or Ctrl+Enter don't open chat by mistake.

**Chat.** Enter opens chat; from then on keys go through the engine in the
current mode (Vietnamese, Mixed or English). Enter is also passed through the
engine before chat closes so the last word gets its marks. Esc closes chat and
clears the buffer; a left, right or middle click returns keys to combat right
away (mouse moves, the wheel and side buttons don't). Commands starting with
`/` stay raw until Space or Enter. For games that ignore simulated keys
(`SendInput`), the user switches to English to chat.

**Kept apart from per-app memory.** Game mode writes nothing to
`smartSwitchKey`: changing mode in chat doesn't overwrite what other apps
remember, and Alt+Tab to Chrome or Word lands in the remembered mode.

**Game suggestions** (`GameCandidateFinder.cpp`). Run only when the user opens
the tray menu or Settings, never in the hook. Ranks three windows: recently in
front (+100), full screen (+30), excluding system processes and office apps.
Titles are read only for the final candidates, with a 50 ms
`SendMessageTimeout` and `SMTO_ABORTIFHUNG`, so a hung app can't freeze the
menu. Titles stay in memory and are never written to disk. Games running as
administrator are marked `[Admin required]` and EasyKey tells the user; it
never elevates itself. "Browse .exe file…" is the fallback, with a reminder to
pick the match executable rather than the launcher.

Known limits:
- Games that share an exe name share one setting.
- Some Microsoft Store games (hosted in `ApplicationFrameHost.exe`) may not be suggested, and the `WindowsApps` folder may not be browsable.
- Games with kernel anti-cheat, exclusive full screen or their own input handling can behave differently. Automated tests cover the state logic only; individual games are tested by hand.

### Password fields

When the cursor is in a password field, EasyKey sends plain keys: no marks,
Mixed mode, shortcuts, quick typing, auto-capitalization or buffer restore. Esc
and Enter go straight to the app.

How password fields are detected, without ever blocking the hook:

1. **Win32**: an `Edit` control with `ES_PASSWORD` is recognized immediately.
2. **Browsers, Electron**: a background thread (COM MTA) asks UI Automation (`UIA_IsPasswordPropertyId`), falling back to MSAA (`STATE_SYSTEM_PROTECTED`).
3. **Atomic state** `_passwordStateWord`: `PW_NORMAL`, `PW_PASSWORD` or `PW_PENDING`. During the 60 ms wait for an answer, the first key stays raw. A generation counter discards stale answers after focus moves.
4. **Sticky across Show/Hide password** on the same field.
5. **No probing during fast typing** (< 150 ms between keys, so search boxes stay snappy); re-probe on mouse-up in web content (100 ms throttle, 40 ms debounce).

The keyboard and mouse hooks only read a cached flag. EasyKey never reads the
field's contents, logs keys or stores state per app. Turn it off in Settings ›
Advanced › Typing (`vPasswordFieldBypass`); the browser part alone can be
turned off with the registry value `vPasswordFieldUIA = 0`.

Known limits: the Windows sign-in screen, UAC prompts and the secure desktop
are out of reach; custom-drawn fields, complex shadow DOM or cross-origin
iframes that don't expose the password property aren't detected.

### Tests

- Engine tests (`Sources/EasyKey/tests`) use only made-up data, make no network calls, don't read the real registry and write only to temp folders.
- When you fix an engine bug, add a test case for exactly that bug.

## 8. User interface
The interface doesn't use classic Win32 dialogs; it is custom drawn to look like Windows 11 with a compact layout (210px minimum height, single-line `V` · `EV` · `E` mode buttons that rebalance to 2 buttons when `vTwoModesOnly` is on, and a flat input method tab bar with a 2px accent underline):

| File | Role |
|---|---|
| `FluentWindow` | Base for windows: Mica, dark/light title bar, per-monitor DPI, buttons, toggles, choice boxes. |
| `FluentText` | Text drawn with DirectWrite (smooth at every scale), measure-to-fit font sizing with floor (`createFluentFontFit`), with the Be Vietnam Pro font embedded in the exe. |
| `FluentUi` | Shared colors, corner radii, sizes. |
| `ThemeHelper` | Reads Windows light/dark mode and accent color, accent presets, contrast checks. |
| `SettingsWindow` | The Settings window: main screen and Advanced (the item list is declared at the top of the file). |
| `FlyoutWindow` | The small popup when switching Vietnamese/English and for notices, drawn with Direct2D (the `EV` badge shrinks to fit via `IDWriteTextLayout::GetMetrics`). |
| `MessageWindow` | Question and notice dialogs in the same style. |
| `SystemTrayHelper`, `TrayGlyph` | The tray icon (a rounded key like the app icon, drawn at the exact pixel size for the DPI for `V`/`EV`/`E`) and the right-click menu. |
| `AboutWindow`, `MacroWindow`, `ConvertToolWindow` | About, Shortcut list, Convert text. |

Interface text comes in two languages, picked with the `TR("Vietnamese", "English")` macro or the `_T()` pairs in the `SettingsWindow` item table.

## 9. Settings storage
- All settings live in `HKCU\Software\EasyKey`. Each option is a `REG_DWORD` value named after its variable (for example `vInputType`, `vCodeTable`, `vSwitchKeyStatus`), read with `APP_GET_DATA` and written with `APP_SET_DATA`.
- Larger data is stored as binary: `macroData` (shortcut list), `smartSwitchKey` (per app memory), `gameApps` (game exe names, one per line).
- On the first start, if `HKCU\Software\TuyenMai\OpenKey` exists, EasyKey copies it.
- Start with Windows: an `EasyKey` value under `HKCU\...\CurrentVersion\Run`, or a Task Scheduler task when run as administrator is on.
- "Remove EasyKey from Windows" deletes `HKCU\Software\EasyKey`, the startup entry and the shortcut.

## 10. Updates
Updates live in `Updater.cpp` and run on a separate thread so the keyboard and mouse never stall:

1. Read `version.json` on the `master` branch (`latestWinVersion.versionName`) and compare it with the running version. Checked at start-up and every 6 hours when automatic checks are on.
2. When the user agrees, download `EasyKey.exe` and `EasyKey.exe.sha256` from `releases/download/<version>/`. Refuse if the SHA-256 doesn't match or the file doesn't start with `MZ`.
3. Write `EasyKey.new.<random>.exe` (created fresh, never over an existing file) and keep its handle open, denying writes and deletes. Check its Authenticode signature through that handle (`UpdateSignature.cpp`): exactly one SHA-256 signature, a signer certificate byte-identical to `docs/signing/EasyKey-codesign.cer` embedded in the exe, a chain built in memory that trusts only that certificate (nothing is added to the Windows certificate stores), and a mandatory RFC 3161 timestamp Windows trusts. On failure, delete the file and report it. The SHA-256 file sits next to the exe on GitHub, so it only catches a broken download; the signature is what stops a swapped exe.
4. Rename the running file to `EasyKey.old.exe` (Windows allows renaming a running exe but not overwriting it), then rename the verified file to `EasyKey.exe` through the same handle. If any step fails, put things back.
5. Remove the hooks, start the new version with an update argument and quit. The new version waits for the old process to exit; once its engine and tray are up it deletes `EasyKey.old.exe` on a background thread (retrying for up to a minute while the file is locked, e.g. by an antivirus scan, otherwise on the next start).

## 11. Build and release
- Visual Studio 2022 project (toolset v143); only `Release|x64` is released. Details in [BUILD](BUILD.en.md).
- CI (`.github/workflows/msbuild.yml`): runs the engine tests on Windows and Linux, builds x64, uploads the exe as an artifact and creates a provenance attestation.
- Releasing: raise `versionName` and `versionCode` in `version.json` and add a section to both CHANGELOG files, merge into `master`, then run the MSBuild workflow by hand (Actions › MSBuild › Run workflow). It tags `versionName`, builds, digitally signs the binary using an ECDSA P-256 Authenticode certificate (`tools/signing/sign.ps1`) with RFC 3161 timestamping, pins the thumbprint against `docs/signing/EasyKey-codesign.cer`, generates `signature.json`, computes `EasyKey.exe.sha256` after signing, packages the zip, creates provenance attestations, and publishes the release.
- Every release is published on both GitHub and GitLab (`gitlab.com/cino-group/easykey`). On GitLab, `.gitlab-ci.yml` runs the tests and a build on a self-hosted Windows runner (tag `easykey-windows`); the `release` job (manual on `master`, or automatic on a version tag) calls `tools\release-gitlab.ps1` to build, sign with the key from protected CI variables, check the signature the way the updater does, package and create the Release. By hand: `tools\release-gitlab.ps1 -FromGitHub` publishes exactly the GitHub release's files on GitLab, `-Ref <commit>` builds and signs locally; the script reads the GitLab token from `%LOCALAPPDATA%\EasyKey-signing\gitlab-token.clixml` or asks for it.
- Running copies read `version.json` on `master` too, so change the version there only when the matching release is about to be published.

## 12. Source map
```
Sources/EasyKey/
├── engine/                 Vietnamese typing engine (plain C++, shared)
├── tests/                  engine tests (CMake)
└── win32/EasyKey/
    ├── EasyKey.sln
    └── EasyKey/
        ├── main.cpp            entry point, restarts as administrator when that is on
        ├── AppDelegate         app lifetime, timers, first start, notices
        ├── EasyKey.cpp         keyboard/mouse hooks, sending keys, hook watchdog
        ├── EasyKeyManager      starts/stops the engine
        ├── EasyKeyHelper       registry, start with Windows, paths, clipboard
        ├── Updater             self update
        ├── *Window, Fluent*    interface (section 8)
        └── fonts/              Be Vietnam Pro (embedded in the exe)
docs/                       documentation and screenshots
tools/                      scripts that make the icons, the font and the mixed mode word list
version.json                latest version, read by running copies to find updates
```

License: the whole project is GPL-3.0. The newly written interface files (`Fluent*`, `FlyoutWindow`, `MessageWindow`, `ThemeHelper`, `TrayGlyph`, `Updater`) are also offered under MIT, as stated in the `SPDX-License-Identifier` line at the top of each file.

## 13. Known limits
- **Hook instead of TSF.** Deleting and resending text can show in apps that track every key themselves (typing tests, some games). In exchange there is no ENG/VIE indicator on the taskbar. TSF is kept for a separate product later.
- **Elevated apps** can be typed in only when EasyKey also runs as administrator.
- **Self-issued signing certificate.** Releases are signed with the project's own certificate and the updater pins it. Because it isn't a commercial certificate, SmartScreen may still say "Unknown publisher". The certificate expires on 2 November 2028; a release that accepts both the old and the new certificate must ship before then.
- **Windows x64 only.** No x86, ARM64, macOS or Linux build.
