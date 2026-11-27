# QA Report — EasyKey Shortcut-List Vietnamese Input Fix

**Verdict: PASS**

All checks (a)–(g) passed independently on branch `darius/qa-shortcut` (created from `garen/shortcut-vietnamese-input` tip `a217bc82de6a69ed4e8824e40ce76a7a6dfb40c9`) on `garen-pc`.

---

## (a) Git Branch & Signature Verification

- Base: `23e3f9b` (`master`, 1.1.10)
- Branch HEAD (`darius/qa-shortcut` at `garen/shortcut-vietnamese-input` tip): `a217bc82de6a69ed4e8824e40ce76a7a6dfb40c9`
- `git log -n 3 --show-signature`:
  - `a217bc82de6a69ed4e8824e40ce76a7a6dfb40c9` (`fix: pause the raw-keyboard watch while the shortcut list is in front`):
    `Good "git" signature for bmt.cino@gmail.com with ED25519 key SHA256:6k8r791+8hcblovdwr9y+tcdrC+Fr+OgAyIAXPNUYcs`
  - `96d40be1a2e49e6773e7ef70cdc3dbaadfd5d036` (`test: lock the raw-watch decision that leaves the shortcut list unhooked`):
    `Good "git" signature for bmt.cino@gmail.com with ED25519 key SHA256:6k8r791+8hcblovdwr9y+tcdrC+Fr+OgAyIAXPNUYcs`

---

## (b) Clean MSVC Release x64 Build

Built `Sources\EasyKey\win32\EasyKey\EasyKey.sln` with MSBuild 17.14.60 (`/t:Rebuild /p:Configuration=Release /p:Platform=x64`):

- **Result:** `Build succeeded. 0 Warning(s), 0 Error(s)` (Elapsed `00:00:42.99`)
- **Output:** `C:\projects\easykey-aaa\darius-qa\shortcut-qa\Sources\EasyKey\win32\EasyKey\x64\Release\EasyKey.exe`
- **Size:** `682,496` bytes
- **Timestamp:** `27/11/2026 23:38:38`

---

## (c) CTest Suite (Fresh Configure)

Configured fresh in `C:\projects\easykey-aaa\darius-qa\shortcut-qa-tests` (`cmake -S Sources/EasyKey/tests -B ...`, built `--config Release`, ran `ctest -C Release`):

- **Overall:** `100% tests passed, 0 tests failed out of 7`
- **Per-suite counts:**
  1. `EngineTests`: `3584/3584` passed
  2. `EngineStateTests`: `94/94` passed
  3. `PacketDeliveryTests`: `15/15` passed
  4. `OwnWindowInputTests`: `2/2` passed
  5. `UpdateSignatureTests`: `18/18` passed
  6. `UpdateCleanupTests`: `9/9` passed
  7. `HttpBodyTests`: `19 checks, 0 failures`

---

## (d) Live Typing in Shortcut List (Interactive Console Session)

Executed via one-shot scheduled task `DariusShortcutQA` (`/IT /RL HIGHEST`, `SessionId=2`) using `SendInput` hardware scan codes (`wVk` + `MapVirtualKey(vk, 0)`).

Vietnamese mode was forced on during the run (`vLanguage=1`, `vMixedEnglish=0`, `vUseSmartSwitchKey=0`, `vCodeTable=0`, `vInputType=0` for Telex / `1` for VNI).

| Box | Mode | Typed | Appeared | UTF-16 Codepoints |
|---|---|---|---|---|
| Shortcut (edit id 1) | Telex | `as` | `á` | `U+00E1` |
| Shortcut (edit id 1) | Telex | `dd` | `đ` | `U+0111` |
| Shortcut (edit id 1) | Telex | `vieetj` | `việt` | `U+0076 U+0069 U+1EC7 U+0074` |
| Text (edit id 2) | Telex | `as` | `á` | `U+00E1` |
| Text (edit id 2) | Telex | `dd` | `đ` | `U+0111` |
| Text (edit id 2) | Telex | `vieetj` | `việt` | `U+0076 U+0069 U+1EC7 U+0074` |
| Shortcut (edit id 1) | VNI | `a1` | `á` | `U+00E1` |
| Shortcut (edit id 1) | VNI | `d9` | `đ` | `U+0111` |
| Shortcut (edit id 1) | VNI | `vie6t5` | `việt` | `U+0076 U+0069 U+1EC7 U+0074` |
| Text (edit id 2) | VNI | `a1` | `á` | `U+00E1` |
| Text (edit id 2) | VNI | `d9` | `đ` | `U+0111` |
| Text (edit id 2) | VNI | `vie6t5` | `việt` | `U+0076 U+0069 U+1EC7 U+0074` |

### Registry Values Before & After

- **Before:** `vLanguage=0x0`, `vMixedEnglish=0x1`, `vCodeTable=0x0`, `vThemeMode=0x2`, `vLastVersion=0x90101` (`vInputType` and `vUseSmartSwitchKey` absent); `AppsUseLightTheme=0x0`, `SystemUsesLightTheme=0x0`, `EnableTransparency=0x1`.
- **After (restored):** `vLanguage=0x0`, `vMixedEnglish=0x1`, `vCodeTable=0x0`, `vThemeMode=0x2`, `vLastVersion=0x90101` (`vInputType` and `vUseSmartSwitchKey` deleted); `AppsUseLightTheme=0x0`, `SystemUsesLightTheme=0x0`, `EnableTransparency=0x1`.

---

## (e) Hook Alive for Other Apps After Leaving Shortcut List

- After closing Shortcut list (`MACRO_STILL_OPEN=False`), typing `as` in Notepad (Telex) produced `á` (`U+00E1`) — observed both in the Telex pass (`NOTEPAD_TYPED=as`, `U+00E1`) and immediately after the dark-mode Shortcut list closed (`IMMEDIATE NOTEPAD_TYPED=as`, `U+00E1`).
- After closing Shortcut list in VNI mode, typing `a1` in Notepad produced `á` (`U+00E1`).

---

## (f) Layered-Edit Regression Check (Light & Dark Modes)

- **Light mode** (`AppsUseLightTheme=1`, `SystemUsesLightTheme=1`, `vThemeMode=0`, `EnableTransparency=1`), typed `as` -> `á` in Box 1 and `vieetj` -> `việt` in Box 2:
  - Screenshot: `C:\projects\easykey-aaa\darius-qa\shortcut-qa-work\shots\light_mode_macro.png`
  - `EDIT1_INNER`: `total=3072 darkest=(27,27,27) brightest=(255,255,255) mode=(255,255,255)[3009/3072] bleach_243_254_gray=0`
  - `EDIT2_INNER`: `total=4320 darkest=(0,0,0) brightest=(255,255,255) mode=(255,255,255)[4122/4320] bleach_243_254_gray=0`
  - `EDIT1_SURROUND_LEFT`: `mode=(255,255,255)[100/100]`
  - `EDIT2_SURROUND_LEFT`: `mode=(255,255,255)[100/100]`
  - Crisp dark-on-light glyphs, no white-on-white or bleaching.
- **Dark mode** (`AppsUseLightTheme=0`, `SystemUsesLightTheme=0`, `vThemeMode=2`, `EnableTransparency=1`), typed `as` -> `á` in Box 1 and `vieetj` -> `việt` in Box 2:
  - Screenshot: `C:\projects\easykey-aaa\darius-qa\shortcut-qa-work\shots\dark_mode_macro.png`
  - `EDIT1_INNER`: `total=3072 darkest=(45,45,45) brightest=(255,255,255) mode=(45,45,45)[3009/3072] bleach_243_254_gray=0`
  - `EDIT2_INNER`: `total=4320 darkest=(45,45,45) brightest=(255,255,255) mode=(45,45,45)[4122/4320] bleach_243_254_gray=0`
  - `EDIT1_SURROUND_LEFT`: `darkest=(45,45,45) mode=(45,45,45)[100/100]`
  - `EDIT2_SURROUND_LEFT`: `darkest=(45,45,45) mode=(45,45,45)[100/100]`
  - Edit inner box matches `(45,45,45)` with no lighter rectangle.

---

## (g) Smoke & Restore

- **App launch & tray:** `EasyKey.exe` launched cleanly; tray icon detected (`EasyKey - Bộ gõ Tiếng Việt - 1 running window`).
- **Settings:** Settings window opened via tray menu (`SETTINGS_OPENED`, `size=442x268`, `settings_real.png`).
- **Open/close & live theme toggle:** Opening/closing Shortcut list and toggling Light -> Dark -> Light while Shortcut list was open completed without crash (`EasyKey_Alive_After_Toggle=YES pid=15500`).
- **Immediate external typing:** Typing `as` in Notepad immediately after closing Shortcut list produced `á`.
- **Restore:** `garen-pc` restored to Windows dark (`AppsUseLightTheme=0`, `SystemUsesLightTheme=0`), `vThemeMode=2`, `EnableTransparency=1`, registry values restored, `DariusShortcutQA` scheduled task deleted, no `EasyKey` process running.
