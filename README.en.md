# EasyKey

[Tiếng Việt](README.md) · **English**

A Vietnamese keyboard for Windows 11. It is one `EasyKey.exe` under 1 MB, with nothing to install and a Windows 11 style look.

[**Download the latest EasyKey.exe**](https://github.com/iamcino/EasyKey/releases/latest/download/EasyKey.exe) · [Release page](https://github.com/iamcino/EasyKey/releases/latest) · [Changelog](CHANGELOG.en.md) · [Report a bug](https://github.com/iamcino/EasyKey/issues)

<p align="center">
  <img src="docs/screenshots/settings-light.png" alt="Settings window, light mode" width="400">
  <img src="docs/screenshots/settings-dark.png" alt="Settings window, dark mode" width="400">
</p>
<p align="center">
  <img src="docs/screenshots/flyout.png" alt="Popup when switching between Vietnamese and English" width="300">
</p>

## Contents
- [Requirements](#requirements)
- [Install](#install)
- [Using EasyKey](#using-easykey)
- [Features](#features)
- [Updating and removing](#updating-and-removing)
- [Troubleshooting](#troubleshooting)
- [Developer documentation](#developer-documentation)
- [License](#license)

## Requirements
| | |
|---|---|
| Operating system | Windows 11, 64-bit (x64). |
| Rights | No administrator rights needed. |
| Runtimes | No .NET, Visual C++ Redistributable or other library needed. |
| Other keyboards | Quit UniKey, EVKey, OpenKey... Two Vietnamese keyboards running together add every mark twice. EasyKey tells you when it finds one running. |

## Install
1. Download [`EasyKey.exe`](https://github.com/iamcino/EasyKey/releases/latest/download/EasyKey.exe).
2. Run it. If Windows shows a warning, see [Windows warnings](#windows-warnings).
3. On its first start from the Downloads folder, EasyKey offers to move itself to `%LOCALAPPDATA%\Programs\EasyKey` so cleaning up Downloads doesn't break it. Click **Move** and you're done.

Defaults: Telex input, Unicode encoding, starts with Windows, **Alt + Z** switches between Vietnamese and English.

If OpenKey was used on this PC, EasyKey copies its settings and shortcut list on the first start.

## Using EasyKey
EasyKey lives in the notification area (the right end of the taskbar). Its icon is a small tile with **V** while typing Vietnamese (also in mixed mode) and **E** while typing English.

| To do this | Do this |
|---|---|
| Switch Vietnamese/English | Press **Alt + Z**, or click the tray icon |
| Mix English words into Vietnamese | Right-click the tray icon › Type English and Vietnamese, or pick Mixed in Settings |
| Open Settings | Double-click the tray icon, or right-click › Settings… |
| Change input method or encoding | Right-click the tray icon › Input method / Character encoding, or use Settings |
| Undo the marks EasyKey just added | Press **Esc** right after the word (việt → vieejt) |
| Turn on game mode for a game | While in the game, right-click the tray icon › Game mode for … |
| Change the switch keys | Settings › Switch with › Change |

The Settings window has two parts:
- **Main screen:** Vietnamese, Mixed or English, the switch keys, the input method (Telex, VNI, Simple Telex).
- **Advanced:** the groups Typing, Appearance, Character encoding, Spelling, Quick typing and shortcuts, Apps, System. Every item has a one-line explanation under it.

## Features
- **Input methods:** Telex, VNI, Simple Telex.
- **Character encoding:** Unicode. Legacy encodings (Unicode combining, TCVN3, VNI Windows) are under Advanced › Character encoding, for old fonts only.
- **Smart typing:** adds marks only to real Vietnamese words and leaves English words as typed.
- **Mixed mode:** type Vietnamese as usual, and English words get their keys back when the word ends, even words that are also Vietnamese such as "this" (thí), "test" (tét), "mix" (mĩ). EasyKey weighs how common the word is in each language and the language of the words around it: "tôi sẽ đi" stays Vietnamese, "There are" does not turn into "Thể are". Alt + Z switches between English and the Vietnamese mode used last.
- **Shortcuts** of any length (ko → không), with a few common ones built in. Optional **quick typing** (cc → ch, f → ph...).
- **Per app memory:** each app keeps its own Vietnamese/English mode and encoding.
- **No doubled letters** in the Chrome, Edge and Brave address bars and other autocomplete boxes.
- **Game mode:** skill keys (Q, W, E, R…) go straight to the game; press Enter to open the chat and type Vietnamese.
- **Windows 11 look:** light/dark, your choice of accent color, Vietnamese or English interface, a small popup when switching.
- **No ENG/VIE indicator** on the taskbar, because EasyKey adds no keyboard layout to Windows.

## Updating and removing
**Updating.** EasyKey checks for a new version when it starts and every 6 hours. When one is out, the tray icon gets a green arrow. Right-click › **Update to…** to install it. EasyKey always asks first, downloads only from this project's releases and checks the SHA-256 hash before it replaces the file. Turn automatic checks off under Advanced › System.

**Removing.** Go to Advanced › System › **Remove EasyKey from Windows**. EasyKey deletes its startup entry, desktop shortcut, settings and shortcut list, then quits. Delete `EasyKey.exe` afterwards. EasyKey writes nothing to system folders.

## Troubleshooting

### Windows warnings
EasyKey is **not code signed yet**. SmartScreen may show "Windows protected your PC" (click **More info** › **Run anyway**), and antivirus software may block `EasyKey.exe`. Every keyboard has to read key presses to add marks, so an unsigned one draws suspicion.

The source code is public. Every release is built on GitHub Actions with build provenance attestation. Check it with the [GitHub CLI](https://cli.github.com):

```
gh attestation verify EasyKey<version>-Windows.zip --repo iamcino/EasyKey
```

If you trust it, you can add the EasyKey folder to your antivirus exclusions.

### Vietnamese spell check
Red underlines under Vietnamese come from the app's spell checker reading English, not from EasyKey:

- **Chrome, Edge, Cốc Cốc:** Settings › Languages, add Vietnamese and turn on spell check for it, or turn off English spell check.
- **Word, Outlook, PowerPoint:** select the text, Review › Language › Set Proofing Language › Vietnamese (or tick "Do not check spelling or grammar").
- **Notepad:** Settings (gear) › turn off Spell check.

Adding Vietnamese under Windows Settings › Language also removes the underlines, but it brings back the ENG/VIE indicator on the taskbar, so it isn't recommended.

### Vietnamese doesn't work in one app
- **Apps running as administrator** (Task Manager, installers...): Windows doesn't let normal apps send keys to them. Turn on Advanced › System › **Run as administrator**. EasyKey shows a notice when you open such an app.
- **The app gets wrong text when marks are added** (missing or extra letters): try Advanced › Apps › **Send text through the clipboard**. This replaces the clipboard content each time a mark is added, so turn it on only when needed.
- **Games:** turn on game mode for the game (see [Using EasyKey](#using-easykey)).

### Wrong text
If wrong text keeps showing up (raw keys like "chaats", stray capitals...), you can record a key log to attach to a bug report:

1. Open PowerShell and run `reg add HKCU\Software\EasyKey /v vDebugKeyLog /t REG_DWORD /d 1 /f`.
2. Quit EasyKey (right-click the tray icon › Quit EasyKey) and start it again.
3. Type until the problem happens. The log is `%TEMP%\EasyKeyKeyLog.txt`.
4. Turn it off with `reg delete HKCU\Software\EasyKey /v vDebugKeyLog /f` and start EasyKey again.

The log holds the key codes you press, so turn it on only when needed, don't type passwords while it is on, and look it over before sending it.

## Developer documentation
- [Design](docs/DESIGN.en.md): how EasyKey works, source layout, settings storage, updates, releases.
- [Build it yourself](docs/BUILD.en.md): build with Visual Studio or GitHub Actions, run the engine tests.
- [Changelog](CHANGELOG.en.md).

## License
Copyright (C) 2026 Cino. Contact: duongtheson@proton.me

EasyKey is released under the [GNU GPL version 3](LICENSE) and is based on [OpenKey](https://github.com/tuyenvm/OpenKey) by Mai Vu Tuyen (Copyright (C) 2019, GPLv3). Some files written entirely for EasyKey are also available under the [MIT](LICENSES/MIT.txt) license. The interface font Be Vietnam Pro is under the [OFL 1.1](LICENSES/OFL-1.1.txt). Details in [NOTICE](NOTICE).
