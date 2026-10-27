# Changelog

[Tiếng Việt](CHANGELOG.md) · **English**

## Next version
- The mouse and keyboard no longer stall while updating: EasyKey writes the new file on a separate thread and removes its hooks before it starts the new version (that is when antivirus scans the new file).

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
