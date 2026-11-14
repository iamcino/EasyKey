# Changelog

[Tiếng Việt](CHANGELOG.md) · **English**

## 1.1.6 - 2026-11-14

### Fixed

- **Central Highlands names could not be typed**: names such as Krông Bông, Adrơng, Ađrơng, Blơh, Chư Prông and Chư Păh now type correctly in Telex, VNI and mixed-English mode.
- **Backspace occasionally deleted too much**: when a word-break key was pressed together with Ctrl, Alt or Windows while the word being typed was marked misspelled, EasyKey deleted the whole word and could keep eating earlier words (most visible in Chromium); those key chords are now passed through to the application untouched.

## 1.1.5 - 2026-11-14

### Security

- **Rollback protection**: when updating, EasyKey no longer accepts any version older than 1.1.4 — an old release with known flaws cannot be pushed to users by a compromised or mistaken distribution host, even if it is properly signed.

## 1.1.4 - 2026-11-14

### Fixed

- **Update downloads**: a file cut off mid-transfer is no longer accepted — EasyKey checks the received length against the server's, and fetches from the other host when it does not match.
- **Update checks**: GitHub is always tried first, then GitLab, and every step now has a time limit, so a host that does not answer no longer stalls the check.

## 1.1.3 - 2026-11-14

### Fixed

- **Message windows**: dialogs such as "What's new" no longer open much taller than their content (on high-resolution displays they could stretch to the full screen height).

## 1.1.2 - 2026-11-14

### Fixed
- **Typing in Chromium/Electron apps**: holding Shift no longer drops the capitals after the first character (typing AAA no longer comes out as Aaa).
- **Caps Lock**: typing accents with Caps Lock on, then turning Caps Lock off and deleting, no longer corrupts the word being typed or the one after it.
- **Place names**: Vietnamese place names such as Đắk Lắk, Đak Pơ, Ea Rốk, Krông Búk and Đăk Pék can now be typed — tone marks after a final k were refused before.

## 1.1.1 - 2026-11-09

### Changed
- **Updates**: when the check fails, the notice also suggests allowing EasyKey in your antivirus (Bitdefender can block it from the network).
- **About**: shows the EasyKey logo instead of the V badge.

## 1.1.0 - 2026-11-09

### Added
- **Updates**: when GitHub can't be reached, EasyKey checks for and downloads new versions from GitLab. Downloads must still match their SHA-256 and the EasyKey signature.
- From this version on, every release is on both [GitHub](https://github.com/iamcino/EasyKey/releases) and [GitLab](https://gitlab.com/cino-group/easykey/-/releases).
- **About**: adds the GitLab home page and a link to the releases on GitLab.

### Notes
- 1.0.23 and older look for updates on GitHub only. If GitHub can't be reached, download 1.1.0 by hand from GitLab; from then on EasyKey updates from whichever site works.

## 1.0.23 - 2026-11-08

### Fixed
- Alt+Tab no longer mixes up the input mode (Vietnamese/English) and character encoding remembered for each app. Mixed English–Vietnamese mode is now remembered per app too.
- **Game Mode**: no longer stops working after Alt+Tab away and back, even when the game recreates its window. Launcher/lobby windows with text fields are no longer taken for the game.

### Added
- **Game Mode**: in-game chat can type Vietnamese. Press Enter to open chat; keys go through the input method in the current mode (Vietnamese, mixed or English). Commands starting with `/` stay raw until a space, and Esc closes chat. Outside chat, keys go straight to the game as before.
- **Updates**: EasyKey only installs an update signed with the EasyKey certificate; the old `EasyKey.old.exe` is cleaned up after start.
- **Macros**: over-long names or content are refused when added instead of being lost on save.

### Notes
- After using 1.0.23, going back to 1.0.22 or older may mix up the per-app character encoding; pick the encoding again for that app.

## 1.0.22 - 2026-11-05

### Fixed
- Fixed out-of-bounds reads when macro data or the per-app remembered list is corrupted.
- Fixed EasyKey possibly shutting down unexpectedly when quick end consonant is on and a very long word is typed.
- Fixed entries being lost when saving macros or the per-app remembered list with over-long names.

## 1.0.21 - 2026-11-04

### Fixed
- **Game Mode**: keep confirmed game window bindings alive for the lifetime of the window, preventing game mode from being lost after mouse-only gameplay or Alt+Tab away/return. Control keys such as `W`, `Q`, `E`, `R`, and `Alt+W` pass directly to the game.
- **Game Mode**: prevent accidental hotkey/language switching when releasing `Alt` / SysKey modifiers, and reset modifier state when entering or leaving game mode. `Alt+Enter` and `Ctrl+Enter` no longer toggle game chat accidentally.
- **Typing engine**: fix `q` + `w` producing invalid `qư`, preserving technical/proper names such as `qwen` while keeping valid Vietnamese words such as `quơ`, `quở`, `quờ`, `quốc`, `quân`, `quên`, and `quê` intact.

### Added
- **Mixed mode**: protect short colloquial Vietnamese words in neutral contexts, for example `bus` → `bú`, `us` → `ú`, `too` → `tô`, and `er` → `ẻ`.
- **Mixed mode**: recognize two-word phrases to preserve the correct English or Vietnamese context, such as `xe bus`, `tuyến bus`, `with us`, `contact us`, `me too`, `mập ú`, `béo ú`, `một tô`, and `đi ẻ`.
- **Mixed mode**: respect ALL-CAPS acronyms such as `US`, `AI`, `ER`, `UK`, `AS`, `IS`, `IF`, and `OR`, while lowercase forms still follow Vietnamese-friendly behavior, for example `us` → `ú`, `ai` → `ai`, and `er` → `ẻ`.
- **Mixed mode**: add curated common AI/developer proper nouns such as `qwen`, `deepseek`, `claude`, `gemini`, `chatgpt`, `openai`, `llama`, `mistral`, `ollama`, `vllm`, `cursor`, `perplexity`, `anthropic`, and `midjourney`.

## 1.0.20 - 2026-11-03

### Added
- **Game Mode window binding**: Game Mode binds directly to the active game window (`HWND` + PID); preserves raw input in game and restores per-application remembering (SmartSwitchKey) when switching out; fast-path rebind within 5 seconds without hook latency.
- **Game window picker**: Automatically suggests the top 3 candidate game windows in the tray menu and Settings, ranking by recent foreground MRU and multi-monitor fullscreen bounds; provides match binary `.exe` browse fallback; enforces clear privilege boundaries and strict privacy (window titles remain transient in RAM only, never logged, never persisted, never transmitted).
- **VirusTotal Reference Sidecar**: Integrates an independent, decoupled VirusTotal API v3 sidecar workflow downstream after release publication, verifies SHA-256 integrity via `SHA256SUMS.txt`, and attaches structured report `virustotal-<tag>.json` to the release.

### Notes
- Game Mode targets applications by executable base name (`exeBaseName`); if different applications share the same exe name, the configuration applies to both.
- Certain UWP / Microsoft Store games or titles hosted by specialized shells (`ApplicationFrameHost.exe`) may require manual executable browsing if permitted by Windows.
- EasyKey does not inspect game memory or input contents, does not log keystrokes, and does not send data off-device.

## 1.0.19 - 2026-11-02

### Added
- **Browser password field bypass**: EasyKey can detect standard password inputs in Chrome, Edge, Firefox and Electron, then switch to raw typing to avoid Vietnamese marks, macros, Quick Telex, and auto-capitalization.
- **Non-blocking detection**: modern password-field detection runs asynchronously on a background worker; keyboard and mouse hooks only read cached state.
- **Sticky password**: once a field is identified as a password field, EasyKey keeps raw input until focus leaves that field, even when the user toggles “Show password”.
- **Kill switch**: browser detection can be disabled with registry `vPasswordFieldUIA = 0`; classic Win32 password fields continue to work.

### Notes
- Support is best-effort for standard password controls. Canvas inputs, virtual keyboards, complex shadow DOM, cross-origin iframes, or custom controls may not be detected.
- Querying accessibility APIs may cause browsers to activate their accessibility tree, increasing local resource usage.
- EasyKey does not read field contents, does not log keystrokes, and does not send data off-device.

## 1.0.18 - 2026-11-02

### Added
- **Game Mode**: protect control keys while playing; game chat defaults to raw input to avoid turning fast chat words such as `afk`, `gg`, `wp`, `how`, or `as` into Vietnamese marks.
- **Game Mode**: Esc and left/right/middle mouse click exit chat state immediately, returning WASD/QWE control to the game; mouse movement, wheel, and extra mouse buttons do not exit chat.
- **Game Mode**: commands starting with `/`, such as `/ff`, `/w`, `/give`, and `/tp`, are preserved as raw input until space or Enter.
- **Password field bypass**: when focus enters a clearly identified password field, EasyKey switches to raw typing to avoid Vietnamese marks, macros, Quick Telex, and auto-capitalization.
- **Password field bypass**: detection runs only on focus change; keyboard and mouse hooks read a cached flag with constant cost; field contents are not read, keystrokes are not logged, and no data is sent off-device.
- **Code signing**: add Authenticode signing, timestamping, signature verification, and a public certificate for binary integrity checks.

### Notes
- Password field bypass is enabled by default and can be turned off in Settings › Advanced › Typing.
- Windows logon, UAC secure prompts, secure desktop, and some custom controls may be outside the supported scope of password field bypass.
- Game Mode state-machine logic is covered by automated tests; real-world behavior in individual games is covered by manual testing.

## 1.0.17 (Windows 11 x64)
- More compact Settings window: tighter spacing and type sizes, single-line **Vietnamese** · **Mixed** · **English** mode buttons with centered badge and label (both in 3-mode and 2-mode layouts), a flat input method tab bar with an accent underline, and a slimmer **Advanced** header.
- The **EV** badge in Settings, the switch flyout and the tray icon is sized to fit with clear margins at every scale; at 16 px the three bars of E stay separate.
- Shorter labels: **Shortcut** › **Edit** (was "Switch with › Change"), tray menu items **Vietnamese** and **Mixed**, plus a separator above **Shortcut list…**.
- Mixed mode: typing `dư dả` or `già dả` no longer turns "dả" into "dar".

## 1.0.16 (Windows 11 x64)
- `EasyKey.exe` is about 32% smaller, from 939 KB to 636 KB: text conversion and file reading/writing now use built-in Windows APIs instead of heavier C++ components.
- Fixed a crash that could happen when a shortcut file contains invalid UTF-8. Malformed bytes now show as "�" and the shortcuts still load.
- Rewrote shortcut file reading and writing, with no limit on line length. Existing shortcut data keeps working as before, no conversion needed.
- Telex, VNI, Simple Telex and Mixed mode typing are unchanged.

## 1.0.15 (Windows 11 x64)
- New **Only 2 modes (English and Vietnamese)** option in Settings › Advanced › Typing: for users who only want to toggle between Vietnamese and English. When enabled, Mixed mode is completely hidden from the UI, hotkey (Alt + Z), tray icon clicks, and the flyout popup only cycle between the 2 modes.
- Fixed an issue where using Backspace and Space after mistyping swallowed the previous word or dropped characters in the typing sequence.
- Mixed mode: typing "fix" and its forms now works as expected when "Allow typing z, f, w, j" is enabled (no longer turns into "fĩ").
- Quick Telex:
  - Initial consonants `pp` (ph), `tt` (th), `gg` (gi), `kk` (kh), `qq` (qu) now only convert at the beginning of a word; English words with double letters such as `app`, `apple`, `happy`, `attack`, `egg` are no longer altered.
  - Added triple-tap undo (e.g. `ppp` becomes `pp`, `ccc` becomes `cc`, `ttt` becomes `tt`).

## 1.0.14 (Windows 11 x64)
- Mixed mode: typing `test` right after "bánh" or "thịt" now gives "tét" (bánh tét, thịt tét) instead of keeping the English word. Other sentences such as "chạy test" or "test đầu tiên" keep "test". To get "tét" anywhere else, type `tets`.

## 1.0.13 (Windows 11 x64)
- Mixed mode keeps code names and emails as typed: snake_case (`max_size`, `sort_desc`, which used to become "mã_size", "sort_déc"), camelCase (`parseJSON`, `iPhone`), letters mixed with digits, and email addresses. In this mode `_` and `@` no longer end a word. Capitals on the first two keys such as "DDi" or "VIeejt" still give "Đi" and "VIệt".
- Mixed mode no longer corrects the word before (for example "Thể are" into "There are"): erasing and retyping the earlier word often went wrong in slow apps and rich editors, so it is now off by default.
- 39 common tech and chat words join the English list of Mixed mode, such as install, update, software, warning, merge, commit, token, btw.

## 1.0.12 (Windows 11 x64)
- The mode switch key (Alt + Z by default) and a click on the tray icon now go through all three modes in turn: Vietnamese → Mixed → English → Vietnamese.
- The tray icon shows **EV** while typing Mixed, instead of the V used for Vietnamese.
- The key on the tray icon follows the accent color again: the Windows accent, or the color picked in Settings. It keeps the rounded key shape of the app icon.
- The update-available arrow is now a glowing green arrow in the top right corner of the tray icon; the update item of the tray menu uses the same arrow.

## 1.0.11 (Windows 11 x64)
- New **Mixed** mode next to Vietnamese and English: type Vietnamese as usual, and English words get their keys back when the word ends, even words that are also Vietnamese such as "this" (thí) or "test" (tét). EasyKey weighs how common the word is in each language and the words around it, so "tôi sẽ đi" stays Vietnamese and "There are" does not turn into "Thể are". Turn it on with the Mixed button in Settings or "Type English and Vietnamese" in the tray menu; Alt + Z switches between English and the Vietnamese mode used last.
- Fixed the per-app encoding: switching to an app on VNI Windows or TCVN3 left the tray menu ticked on Unicode, so it looked reset to Unicode. Apps whose name could not be read had the encoding picked there saved for the previous app; Microsoft Store apps are now remembered under their own name instead of sharing ApplicationFrameHost.exe.
- The tray icon is drawn like the key in the EasyKey icon: a blue key with V for Vietnamese, the same key shape in a neutral color with E for English. It no longer follows the Windows accent color.
- Rewritten documentation: a README that lays out EasyKey's strengths and all its features, plus a privacy policy (PRIVACY.md), security policy (SECURITY.md), contributing guide (CONTRIBUTING.md) and disclaimer (DISCLAIMER.md).

## 1.0.10 (Windows 11 x64)
- Fixed letters EasyKey rewrites coming out in capitals, for example "giúp tôi hoàn thiện" turning into "giÚP tÔi hOÀN thIỆN" (seen in Gemini on Firefox). EasyKey had missed a Shift key release, for example while the screen was locked or an administrator window was in front, and kept treating Shift as held. EasyKey now reads the real state of Shift, Ctrl, Alt, Win and Caps Lock before every key.
- The README is rewritten to be clearer (a table of everyday actions, updating and removing, troubleshooting, how to record a key log for a bug report). New design document for developers (docs/DESIGN.en.md). The "Red underlines under Vietnamese?" button in Settings opens the right guide again.

## 1.0.9 (Windows 11 x64)
- New tray icon: a solid tile with V or E, drawn at the exact pixel size the taskbar uses at every scale (100%, 125%, 150%, 200%…), so it stays sharp instead of blurring from a scaled-down image. Vietnamese is a tile in the accent color with a white letter; English is a white tile on a dark taskbar and a black one on a light taskbar.
- The interface font is now Be Vietnam Pro (OFL license), carried inside EasyKey.exe and not installed into Windows. Windows 10 before 1703 keeps Segoe UI.

## 1.0.8 (Windows 11 x64)
- The mouse and keyboard no longer stall while updating: EasyKey writes the new file on a separate thread and removes its hooks before it starts the new version (that is when antivirus scans the new file).
- Text in every window (Settings, About, shortcut list, convert tool, dialogs) is drawn with DirectWrite like the popup: smooth, and unbroken when Windows scales the screen above 100%.

## 1.0.7 (Windows 11 x64)
- Fix occasional raw keys in the text (for example "chaats" instead of "chất"), seen most in Discord and other Electron apps: when Windows quietly removes EasyKey's keyboard hook, EasyKey now notices on the next key and puts it back, even while the mouse is in use (it used to take a few seconds, or not notice at all).
- Alt + Z (or any hotkey with Alt or Win) no longer makes the app open its menu bar, so what you type right after it is not lost to the menu.
- The convert tool keeps capitals when the case option is "Keep", also when removing marks ("Tiếng Việt" → "Tieng Viet"). It now converts from TCVN3 to Unicode by default.
- Email addresses and words followed by @ ! # are no longer marked (user@gmail.com used to become ủe@gmail.com). "www." keeps all three w's.
- A few shortcuts come with the first start: ko → không, dc → được, vs → với, mn → mọi người, ntn → như thế nào. Vietnamese can be typed in the shortcut list.
- The interface is in Vietnamese when Windows is set to the Vietnam region or formats, also on English Windows.
- Started from Downloads or a temporary folder, EasyKey offers to move itself to a permanent folder on the first start.
- "Check for updates" shows "Checking…", and says it could not check when offline instead of "up to date".
- Double-clicking the tray icon only opens Settings, without switching Vietnamese/English twice.
- Game mode no longer shows the internal name "TheEasyKeyProject" before any app was used. The startup entry is quoted, so it works with spaces in the user name.
- The switch popup is drawn with Direct2D and DirectWrite: letters stay sharp and unbroken when Windows scales the screen above 100%, and the badge letter is centered.

## 1.0.6 (Windows 11 x64)
- Update notice: EasyKey checks at start and every 6 hours (on by default, can be turned off in Settings › System). When a new version is out, the tray icon gets a green arrow, the tray menu shows "Update to …" and Settings shows an update line at the bottom; nothing is installed without asking.
- Game mode: turn it on per game from the tray menu ("Game mode for …") or Settings › Apps › Game mode. In the game, keys go straight to the game so Q, W, E, R… are never turned into marked letters; press Enter to open the chat and type Vietnamese, Enter (send) or Esc (close) goes back.
- The input method hotkey also works while typing English; the popup names the new input method with "used when typing Vietnamese".
- The README links straight to the latest `EasyKey.exe`.

## 1.0.5 (Windows 11 x64)
- A popup also shows when the character encoding is changed from the tray menu or the Settings window.
- While typing English, changing the input method or character encoding shows the new choice in the popup with "used when typing Vietnamese" (it used to show only "English").
- Removed the "Hold Ctrl to skip spell check" and "Hold Alt to pause EasyKey" options. Pressing Esc right after a word already gives back the keys you typed, and pressing Alt alone on Windows moves to the app's menu bar.
- The English UI, README and changelog now say "character encoding" everywhere instead of "code table".
- A shorter README with screenshots; the build guide moved to [docs/BUILD.en.md](https://github.com/iamcino/EasyKey/blob/master/docs/BUILD.en.md).

## 1.0.4 (Windows 11 x64)
- Fixed a legacy character encoding (VNI Windows, TCVN3, Unicode combining…) or the Vietnamese/English choice going back to the previous one, for example Unicode, after picking it in Settings or the tray menu and returning to the app. EasyKey now remembers the choice for the app you are typing in.
- The input method popup also shows when the input method is changed from the tray menu or the Settings window.
- With "Run as admin" on, EasyKey now starts through Task Scheduler using the Windows API instead of the schtasks command. The task is no longer stopped by Windows after 72 hours, also runs on battery, and if the task can't be created EasyKey still starts with Windows (without admin rights).
- Tidied the source file headers: the OpenKey credit (Mai Vu Tuyen) stays, with an SPDX GPL-3.0-only line; removed the unused macOS key code file.

## 1.0.3 (Windows 11 x64)
- Update checks run in the background, so the mouse and keyboard no longer stall while EasyKey checks for or downloads an update.
- Added "Check for updates…" to the tray menu.
- Advanced settings: every group starts closed, only one group is open at a time, and opening a group scrolls so the whole group shows.
- "Code table" is now called "Character encoding" in English.
- Brought back the desktop shortcut option (Advanced › System).
- Tray icon: the V (typing Vietnamese) takes the accent color; the E stays black and white.
- Input method shortcut (Advanced › Typing, off by default): each press moves Telex → VNI → Simple Telex and shows the popup. It works only while typing Vietnamese.
- The Vietnamese/English popup also shows when switching from the tray menu or the Settings window.
- Color mode: follow Windows, Light or Dark (Advanced › Appearance).
- Redrew the About, Shortcut list and Convert tool windows in the style of the Settings window; they follow the light/dark mode and the accent color.
- Questions and notices (updates, resetting settings, removing EasyKey, the other-keyboard warning…) use the EasyKey style too. Short notices such as "up to date" or "converted" show in the popup.

## 1.0.2 (Windows 11 x64)
- Fixed extra letters in Chrome and Edge on pages that handle keys themselves, such as typing tests ("số" came out as "sốôô"), in Telex and VNI.
- "Quick Telex" is now called "Quick typing": cc → ch, gg → gi, nn → ng… work with every input method, VNI included.

## 1.0.1 (Windows 11 x64)
- Open the shortcut list from the settings window: "Advanced" › "Typing" › "Shortcut list".
- Esc to put back the typed keys now works in web chats such as Facebook (EasyKey also holds back the Esc key release, so the page does not close the chat).
- Capitalizing the first letter of a sentence now also works after "?" and "!", not only after "." and Enter.
- Capitalizing the first letter: the first word typed after switching to another window is capitalized too.
- A tidier "Advanced" section: groups are folded, click a group name to open it, "Typing" starts open. Larger, stronger group headers.
- The autocomplete fix and the Chrome and Edge fix are always on, with no switch. Removed "Open this window when EasyKey starts", "Desktop shortcut" and "Check for updates now" (it is in About). To open Settings, run EasyKey.exe again or pick Settings in the tray menu.

## 1.0.0 (Windows 11 x64)
The first release of EasyKey.

- Windows 11 64-bit only.
- Copies the settings and shortcut list from OpenKey on the first run.
- Fixed Vietnamese typing on PCs where software tags keystrokes, such as Bitdefender.
- A new Windows 11 (2026) style settings window: Mica background, Windows accent color, two large Vietnamese/English buttons, the input method as one row of buttons, and everything else under "Advanced". Changes apply immediately. Light and dark mode follow Windows and change as soon as Windows does.
- A small notice at the bottom of the screen when you switch Vietnamese/English with the hotkey. It can be turned off under "Advanced".
- Unicode by default. The legacy character encodings (Unicode combining, TCVN3, VNI Windows) moved to "Advanced" › "Legacy character encoding". CP 1258 is no longer listed, except for people already using it.
- Set the Vietnamese/English switch key by pressing the key combination itself, for example Ctrl + Shift or Alt + Z.
- Dark mode for the tray menu.
- Per-monitor DPI, the Segoe UI Variable font, and a monochrome tray icon that follows the taskbar color.
- The default switch key on a new install is Alt + Z (it used to be set to a macOS key code by mistake).
- Removed the "type in other languages" option, which had no effect on Windows.
- "Fix Chrome and Edge" is on by default; fixed the character before the cursor being deleted when capitalizing the first letter of a sentence.
- Removed the "Support Metro apps" and "Modern icon" options.
- Shorter tooltips.
- Telex/VNI engine tests run on CI.
- A more compact settings window (440 px wide). The main screen only has the Vietnamese/English switch, the switch key and the input method; every other option is under "Advanced".
- "Smart typing" combines spell check and restoring keys on misspelled words into one switch.
- Press Esc right after a word to get back the keys you typed, for example "việt" becomes "vieejt".
- The first run asks nothing and only shows a small notice on how to switch Vietnamese/English; the settings window no longer opens at start.
- A warning when UniKey, EVKey, OpenKey, GoTiengViet or the Windows Vietnamese keyboard runs at the same time (with "Don't remind me again"). EasyKey never closes other apps.
- A "What's new" page shows once after each update.
- Text-only tray menu: Type Vietnamese, Input method, Character encoding, Shortcut list, Convert text, Settings, Quit.
- English UI: choose Windows, Tiếng Việt or English under "Advanced" › "Appearance". It covers the settings window, tray menu, notices and dialogs.
- Accent color: follow Windows, 7 presets (each with its own light and dark shade), or your own hex code, with an automatic dark mode shade and a contrast check.
- Hooks the keyboard again when Windows drops EasyKey's hook (for example after the PC slows down or wakes from sleep).
- No Vietnamese/English notice while a game or presentation runs fullscreen.
- A one-time notice when typing into a window that runs as administrator, since Windows does not let EasyKey type there.
- "Remove EasyKey from Windows" button: stops starting with Windows, deletes the shortcut and settings, then opens the folder so you can delete the file.
- Updates install from inside EasyKey.exe: the new version is downloaded over HTTPS, checked against its SHA-256, always asks before installing, and restarts EasyKey. EasyKeyUpdate.exe is gone.
- Releases include `EasyKey.exe` and `EasyKey.exe.sha256` next to the zip.
- The README explains how to remove red spell check underlines under Vietnamese in each app, and how to build EasyKey yourself.
