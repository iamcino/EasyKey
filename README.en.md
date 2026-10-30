<div align="center">

<img src="docs/images/easykey.png" width="96" alt="EasyKey">

# EasyKey

**A light Vietnamese keyboard for Windows 11: one file under 1 MB, English words typed right in your Vietnamese, and a true Windows 11 look.**

[Tiếng Việt](README.md) · **English**

[![Version](https://img.shields.io/github/v/release/iamcino/EasyKey?label=version&color=1f6fe5)](https://github.com/iamcino/EasyKey/releases/latest)
![Platform](https://img.shields.io/badge/platform-Windows%2011%20x64-1f6fe5)
![C++](https://img.shields.io/badge/C%2B%2B-Win32-00599c?logo=cplusplus)
[![License: GPL v3](https://img.shields.io/badge/license-GPL%20v3-green)](LICENSE)

[**Download EasyKey.exe**](https://github.com/iamcino/EasyKey/releases/latest/download/EasyKey.exe) · [Release page](https://github.com/iamcino/EasyKey/releases/latest) · [Changelog](CHANGELOG.en.md) · [Report a bug](https://github.com/iamcino/EasyKey/issues)

</div>

---

## Contents

- [What is EasyKey?](#what-is-easykey)
  - [Screenshots](#screenshots)
- [Why EasyKey](#why-easykey)
- [Features](#features)
- [System requirements](#system-requirements)
- [Installation](#installation)
- [Getting started](#getting-started)
- [How it works](#how-it-works)
- [Privacy](#privacy)
- [Uninstalling](#uninstalling)
- [Building from source](#building-from-source)
- [FAQ and troubleshooting](#faq-and-troubleshooting)
- [Disclaimer](#disclaimer)
- [License and credits](#license-and-credits)

## What is EasyKey?

EasyKey is a Vietnamese keyboard for Windows 11. It is built on the
[OpenKey](https://github.com/tuyenvm/OpenKey) typing engine, with the Windows
app rewritten to fit a modern Windows. Download one `EasyKey.exe`, run it and
start typing: Telex, VNI or Simple Telex, Unicode encoding, **Alt + Z** to
switch between Vietnamese and English.

EasyKey adds no keyboard layout to Windows, so the taskbar shows no ENG/VIE
indicator. It lives in the notification area, needs no administrator rights,
no .NET and no extra libraries, and removes itself cleanly with one button.

### Screenshots

| Settings window (light mode) | Settings window (dark mode) |
|:---:|:---:|
| <img src="docs/screenshots/settings-light.png" width="390" alt="Settings window, light mode"> | <img src="docs/screenshots/settings-dark.png" width="390" alt="Settings window, dark mode"> |

<p align="center">
  <strong>Popup when switching between Vietnamese and English</strong><br><br>
  <img src="docs/screenshots/flyout.png" width="280" alt="Popup when switching between Vietnamese and English">
</p>

## Why EasyKey

| Strength | What it means for you |
|----------|-----------------------|
| **One file, under 1 MB** | No installer, no DLLs, no runtimes. Copy it anywhere, even a USB stick, and it runs. |
| **English and Vietnamese in one sentence** | Mixed mode spots English words and gives you back the keys you typed when the word ends, even words that are also Vietnamese, such as "this" (thí) or "test" (tét). No switching in the middle of a sentence. |
| **Reliable where keyboards often fail** | No doubled letters in the Chrome, Edge and Brave address bars; the hook comes back by itself when Windows drops it (the "chaats" bug in Discord); game mode sends skill keys straight to the game. |
| **Remembers each app** | Every app keeps its own Vietnamese/English mode and encoding, Microsoft Store apps included. Word on TCVN3, Chrome still on Unicode. |
| **A real Windows 11 look** | Mica, light and dark following Windows, your own accent color, DirectWrite text that stays sharp at any scale, the Be Vietnam Pro font built in. Vietnamese or English interface. |
| **No ENG/VIE indicator on the taskbar** | Windows keeps a single keyboard, so there is no language button taking space or jumping around. |
| **Safe updates** | Downloads only over HTTPS from this project's releases, checks the SHA-256 hash, always asks first. Every release is built in public on GitHub Actions with build provenance. |
| **Private** | No account, no data collection, no record of what you type. Its only connection is the update check on GitHub. |
| **Open source** | GPL v3. The typing engine has a test suite that runs on Windows and Linux on every change. |

## Features

### Vietnamese typing

- **Input methods:** Telex, VNI, Simple Telex. Change it from the tray menu, in
  Settings, or with a key combination (optional).
- **Smart typing:** adds marks only to real Vietnamese words and leaves
  English words as typed. Press **Esc** right after a word to get the keys back
  (việt → vieejt).
- **Modern marks** (hoà, thuý) or the classic style (hòa, thúy).
- **Shortcuts** of any length, with a few common ones built in (ko → không,
  dc → được, vs → với, mn → mọi người, ntn → như thế nào). They follow the
  capitalization you type and can work in English mode too.
- **Quick typing** (optional): cc → ch, gg → gi, nn → ng; first consonants
  f → ph, j → gi, w → qu; last consonants g → ng, h → nh, k → ch.
- **Capitalize the first letter of sentences**, also after ? and ! and after
  switching windows.
- **Character encoding:** Unicode. Legacy encodings (Unicode combining,
  TCVN3, VNI Windows) are under Advanced › Character encoding, for old fonts
  only.
- **Convert tool** between encodings, upper/lower case and removing marks,
  with a key combination that converts the clipboard on the spot.

### Mixed English and Vietnamese mode

A third mode next to Vietnamese and English, for people who mix both
languages (coding, work, study):

- Type Vietnamese as usual. When a word ends (space, punctuation), EasyKey
  decides whether it was English or Vietnamese. If it was English, the marked
  word is replaced with exactly the keys you typed.
- It also catches English words that Telex turns into Vietnamese: "this"
  (thí), "test" (tét), "mix" (mĩ).
- It reads the context: "tôi sẽ đi" stays Vietnamese, and "There are" does not
  become "Thể are". A clearly English word also fixes the word right before it.
- Turn it on with the **Mixed** button (EV badge) in Settings or **Type English
  and Vietnamese** in the tray menu. **Alt + Z** or a click on the tray icon goes
  through the three modes in turn: Vietnamese, Mixed, English.

### Per app

- **Vietnamese/English mode and encoding are remembered per app.** Switch
  windows and EasyKey switches with you; the tray menu always ticks the
  encoding of the app in front. Microsoft Store apps are remembered under
  their own name; apps whose name can't be read keep the current choice
  instead of saving it for the wrong app.
- **Game mode:** turn it on per game from the tray menu while playing. Q, W, E,
  R… go straight to the game; press Enter to open the chat and type
  Vietnamese, Enter or Esc to go back.
- **No doubled letters** in the Chrome, Edge and Brave address bars and other
  autocomplete boxes.
- **Administrator apps:** EasyKey tells you when you open one, and can run as
  administrator itself (through Task Scheduler, with no UAC prompt at every
  sign-in).
- **Send text through the clipboard** for apps that get marks wrong.

### Interface

- A compact Settings window: the main screen picks the mode, switch keys and
  input method; **Advanced** holds the groups Typing, Appearance, Character
  encoding, Spelling, Quick typing and shortcuts, Apps and System, each item
  with a one-line explanation.
- Light, dark or following Windows; the Windows accent, 7 preset colors or a
  custom one (with an automatic dark mode shade and a contrast check).
- Vietnamese or English interface, picked from your Windows region.
- A small popup when you switch language, input method or encoding (hidden
  while a full-screen game runs). An optional beep on switching.
- A tray icon drawn at the exact pixel size for every scale: a key in the accent
  color with **V** for Vietnamese and **EV** for Mixed, a neutral tile with **E**
  for English, a glowing green arrow when an update is out.

### App

- **Updates itself** from within the app, always asking first, with a SHA-256
  check.
- **Puts its keyboard hook back** as soon as Windows drops it, at the next
  key.
- Warns when UniKey, EVKey, OpenKey… run at the same time.
- On the first start from Downloads, offers to move itself to a permanent
  folder.
- Used OpenKey before? On the first start, EasyKey copies its settings and
  shortcut list.

## System requirements

| | |
|---|---|
| Operating system | Windows 11, 64-bit (x64). |
| Rights | No administrator rights needed. |
| Runtimes | No .NET, Visual C++ Redistributable or other library needed. |
| Other keyboards | Quit UniKey, EVKey, OpenKey… Two Vietnamese keyboards running together add every mark twice. |

## Installation

1. Download [`EasyKey.exe`](https://github.com/iamcino/EasyKey/releases/latest/download/EasyKey.exe)
   (or the zip from the [release page](https://github.com/iamcino/EasyKey/releases/latest)).
2. Run it. If Windows shows a warning, see [Windows warnings](#windows-warnings).
3. On its first start from the Downloads folder, EasyKey offers to move itself
   to `%LOCALAPPDATA%\Programs\EasyKey` so cleaning up Downloads doesn't break
   it. Click **Move** and you're done.

Defaults: Telex input, Unicode encoding, starts with Windows, **Alt + Z**
switches between Vietnamese and English.

### Updating

EasyKey checks for a new version when it starts and every 6 hours (turn it off
under Advanced › System). When one is out, the tray icon gets a green arrow,
the tray menu shows **Update to …** and the bottom of Settings says so. Nothing
is installed until you agree. Then EasyKey downloads the new `EasyKey.exe` from
the release page, checks its SHA-256 hash, replaces the running file and
restarts. Right-click › **Check for updates…** to check right away.

## Getting started

1. Run EasyKey. Its icon appears in the notification area (the right end of
   the taskbar).
2. Type in any app. Press **Alt + Z** to switch between Vietnamese and English.
3. Mix in English often? Right-click the icon › **Type English and
   Vietnamese**.
4. Double-click the icon to open Settings and pick your input method, colors
   and keys.

| To do this | Do this |
|---|---|
| Switch Vietnamese → Mixed → English | Press **Alt + Z**, or click the tray icon |
| Mix English words into Vietnamese | Right-click the tray icon › Type English and Vietnamese, or pick Mixed in Settings |
| Open Settings | Double-click the tray icon, or right-click › Settings… |
| Change input method or encoding | Right-click the tray icon › Input method / Character encoding, or use Settings |
| Undo the marks EasyKey just added | Press **Esc** right after the word (việt → vieejt) |
| Turn on game mode for a game | While in the game, right-click the tray icon › Game mode for … |
| Change the switch keys | Settings › Switch with › Change |
| Edit your shortcuts | Settings › Advanced › Typing › Shortcut list |

## How it works

```
 Keyboard
    │
    ▼
 Low-level keyboard hook ──► Engine (Telex / VNI / Simple Telex, spelling, mixed mode)
    │                              │
    │                              ▼
    │                     characters to delete + new text
    │                              │
    ▼                              ▼
 App you type in ◄────── Backspace + Unicode characters (or through the clipboard)
```

EasyKey lets keys reach the app as usual. When a mark is needed, it deletes
the old letters and sends the new ones. It registers no keyboard with Windows,
so there is no ENG/VIE indicator. When you switch windows, EasyKey loads the
mode and encoding it remembers for that app. Details are in the
[design document](docs/DESIGN.en.md).

| Folder | Role |
|--------|------|
| `Sources/EasyKey/engine` | Vietnamese typing engine, plain C++ with no Windows API calls |
| `Sources/EasyKey/tests` | Engine tests (CMake, run on Windows and Linux) |
| `Sources/EasyKey/win32` | Windows app: hooks, interface, settings, updates |
| `tools` | Scripts that make the icons, the font and the mixed mode word list |

## Privacy

EasyKey has no account, collects no usage data, shows no ads and does not
record what you type. Settings live in `HKCU\Software\EasyKey` on your PC. The
only connection EasyKey makes on its own is to GitHub, to check for and
download updates (you can turn it off). See [PRIVACY.md](PRIVACY.md).

## Uninstalling

Go to Settings › Advanced › System › **Remove EasyKey from Windows**. EasyKey
deletes its startup entry, its Task Scheduler task (if any), its desktop
shortcut, its settings and shortcut list, then quits. Delete `EasyKey.exe`
afterwards. EasyKey writes nothing to system folders.

## Building from source

You need Visual Studio 2022 with the **Desktop development with C++**
workload:

```powershell
msbuild Sources\EasyKey\win32\EasyKey\EasyKey.sln -p:Configuration=Release -p:Platform=x64
# Sources\EasyKey\win32\EasyKey\x64\Release\EasyKey.exe
```

The engine tests run on Windows and Linux:

```bash
cmake -S Sources/EasyKey/tests -B build
cmake --build build --config Release
ctest --test-dir build -C Release
```

The full guide, including building on GitHub with nothing installed, is in
[docs/BUILD.en.md](docs/BUILD.en.md). Contributions are welcome; please read
[CONTRIBUTING.md](CONTRIBUTING.md).

## FAQ and troubleshooting

**How is EasyKey different from OpenKey?**
EasyKey keeps OpenKey's typing engine and rewrites almost all of the Windows
app: a new interface, mixed mode, game mode, built-in updates, hook recovery,
browser fixes and many other fixes. See the [changelog](CHANGELOG.en.md).

**Do I have to uninstall UniKey or EVKey?**
No, just quit them. Two keyboards running together add every mark twice.
EasyKey tells you when it finds another one running.

**Does EasyKey send what I type anywhere?**
No. Keys are handled on your PC only. EasyKey writes no keys to a file unless
you turn on the key log for a bug report yourself (see
[Wrong text](#wrong-text)).

**Is there a macOS, Linux or ARM64 version?**
Not yet. EasyKey focuses on Windows 11 x64 until it is truly stable.

### Windows warnings

EasyKey is **not code signed yet**. SmartScreen may show "Windows protected
your PC" (click **More info** › **Run anyway**), and antivirus software may
block `EasyKey.exe`. Every keyboard has to read key presses to add marks, so
an unsigned one draws suspicion.

The source code is public. Every release is built on GitHub Actions with build
provenance attestation. Check it with the [GitHub CLI](https://cli.github.com):

```
gh attestation verify EasyKey<version>-Windows.zip --repo iamcino/EasyKey
```

If you trust it, you can add the EasyKey folder to your antivirus exclusions.

### Vietnamese spell check

Red underlines under Vietnamese come from the app's spell checker reading
English, not from EasyKey:

- **Chrome, Edge, Cốc Cốc:** Settings › Languages, add Vietnamese and turn on
  spell check for it, or turn off English spell check.
- **Word, Outlook, PowerPoint:** select the text, Review › Language › Set
  Proofing Language › Vietnamese (or tick "Do not check spelling or grammar").
- **Notepad:** Settings (gear) › turn off Spell check.

Adding Vietnamese under Windows Settings › Language also removes the
underlines, but it brings back the ENG/VIE indicator on the taskbar, so it
isn't recommended.

### Vietnamese doesn't work in one app

- **Apps running as administrator** (Task Manager, installers…): Windows
  doesn't let normal apps send keys to them. Turn on Advanced › System › **Run
  as administrator**.
- **The app gets wrong text when marks are added** (missing or extra letters):
  try Advanced › Apps › **Send text through the clipboard**. This replaces the
  clipboard content each time a mark is added, so turn it on only when needed.
- **Games:** turn on game mode for the game (see
  [Getting started](#getting-started)).
- **A Vietnamese word comes back as English keys** (or the other way round) in
  mixed mode: press **Esc** right after the word, or switch to Vietnamese mode
  for text that is all Vietnamese. Please
  [report it](https://github.com/iamcino/EasyKey/issues) with the sentence you
  typed so the word list can be tuned.

### Wrong text

If wrong text keeps showing up (raw keys like "chaats", stray capitals…), you
can record a key log to attach to a bug report:

1. Open PowerShell and run
   `reg add HKCU\Software\EasyKey /v vDebugKeyLog /t REG_DWORD /d 1 /f`.
2. Quit EasyKey (right-click the tray icon › Quit EasyKey) and start it again.
3. Type until the problem happens. The log is `%TEMP%\EasyKeyKeyLog.txt`.
4. Turn it off with `reg delete HKCU\Software\EasyKey /v vDebugKeyLog /f` and
   start EasyKey again.

The log holds the key codes you press, so turn it on only when needed, don't
type passwords while it is on, and look it over before sending it.

## Disclaimer

EasyKey is free software provided **"as is", without warranty of any kind**,
under the GPL v3. In short:

- EasyKey reads key presses through a Windows keyboard hook to add marks; keys
  are handled on your computer only.
- You are responsible for using EasyKey in line with the law and the rules of
  your workplace, school or the software you use (for example a game's rules).
- The authors are not liable for lost data, wrong text or any other damage
  from using EasyKey.
- `EasyKey.exe` is not code signed. Download it only from the
  [official release page](https://github.com/iamcino/EasyKey/releases).
- EasyKey is an independent project, not affiliated with Microsoft or other
  keyboards, and not published by the OpenKey author.

The full text is in [DISCLAIMER.md](DISCLAIMER.md). If you do not agree, do
not use EasyKey.

## License and credits

Copyright (C) 2026 Cino. Contact: duongtheson@proton.me

EasyKey is released under the [GNU GPL version 3](LICENSE) and is based on
[OpenKey](https://github.com/tuyenvm/OpenKey) by Mai Vu Tuyen (Copyright (C)
2019, GPLv3). Some files written entirely for EasyKey are also available under
the [MIT](LICENSES/MIT.txt) license. Third-party parts:

- The interface font **Be Vietnam Pro**: [SIL OFL 1.1](LICENSES/OFL-1.1.txt).
- The mixed mode word list, made from
  [wordfreq](https://github.com/rspeer/wordfreq) data: CC BY-SA 4.0.

Details are in [NOTICE](NOTICE). Report security problems as described in [SECURITY.md](SECURITY.md). Changes are listed in the
[changelog](CHANGELOG.en.md).
