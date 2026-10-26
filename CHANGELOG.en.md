# Changelog

[Tiếng Việt](CHANGELOG.md) · **English**

## Next version
- Open the shortcut list from the settings window: "Advanced" › "Typing" › "Shortcut list".
- Esc to put back the typed keys now works in web chats such as Facebook (EasyKey also holds back the Esc key release, so the page does not close the chat).
- Capitalizing the first letter of a sentence now also works after "?" and "!", not only after "." and Enter.

## 1.0.0 (Windows 11 x64)
The first release of EasyKey.

- Windows 11 64-bit only.
- Copies the settings and shortcut list from OpenKey on the first run.
- Fixed Vietnamese typing on PCs where software tags keystrokes, such as Bitdefender.
- A new Windows 11 (2026) style settings window: Mica background, Windows accent color, two large Vietnamese/English buttons, the input method as one row of buttons, and everything else under "Advanced". Changes apply immediately. Light and dark mode follow Windows and change as soon as Windows does.
- A small notice at the bottom of the screen when you switch Vietnamese/English with the hotkey. It can be turned off under "Advanced".
- Unicode by default. The legacy code tables (Unicode combining, TCVN3, VNI Windows) moved to "Advanced" › "Legacy code table". CP 1258 is no longer listed, except for people already using it.
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
- Text-only tray menu: Type Vietnamese, Input method, Code table, Shortcut list, Convert text, Settings, Quit.
- English UI: choose Windows, Tiếng Việt or English under "Advanced" › "Appearance". It covers the settings window, tray menu, notices and dialogs.
- Accent color: follow Windows, 7 presets (each with its own light and dark shade), or your own hex code, with an automatic dark mode shade and a contrast check.
- Hooks the keyboard again when Windows drops EasyKey's hook (for example after the PC slows down or wakes from sleep).
- No Vietnamese/English notice while a game or presentation runs fullscreen.
- A one-time notice when typing into a window that runs as administrator, since Windows does not let EasyKey type there.
- "Remove EasyKey from Windows" button: stops starting with Windows, deletes the shortcut and settings, then opens the folder so you can delete the file.
- Updates install from inside EasyKey.exe: the new version is downloaded over HTTPS, checked against its SHA-256, always asks before installing, and restarts EasyKey. EasyKeyUpdate.exe is gone.
- Releases include `EasyKey.exe` and `EasyKey.exe.sha256` next to the zip.
- The README explains how to remove red spell check underlines under Vietnamese in each app, and how to build EasyKey yourself.
