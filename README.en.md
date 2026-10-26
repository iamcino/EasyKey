# EasyKey

[Tiếng Việt](README.md) · **English**

A Vietnamese keyboard for Windows 11 (64-bit).

[Download the latest version](https://github.com/iamcino/EasyKey/releases/latest) · [Changelog](CHANGELOG.en.md) · [Report a bug](https://github.com/iamcino/EasyKey/issues)

## Requirements
Windows 11 x64.

## Install
1. Download `EasyKey<version>-Windows.zip` from the [Releases](https://github.com/iamcino/EasyKey/releases) page.
2. Unzip it into a folder you will keep and run `EasyKey.exe`. Nothing to set up: the defaults are Telex, Unicode, start with Windows, and Alt + Z to switch between Vietnamese and English.
3. Quit other Vietnamese keyboards (UniKey, EVKey, OpenKey...) to avoid conflicts. EasyKey tells you if it finds one running.

If OpenKey is installed, EasyKey copies its settings and shortcut list on the first run. Settings are stored in `HKEY_CURRENT_USER\Software\EasyKey`.

## Windows warnings when you run it
EasyKey is **not code signed** yet. Because of that, when you download or run it:
- Windows SmartScreen may show "Windows protected your PC". Click **More info** › **Run anyway** to run it.
- Windows Defender or another antivirus may block, quarantine or delete `EasyKey.exe`.

This is not a bug, and not a security problem in EasyKey. Every keyboard app has to read keystrokes and send back the marked text, so antivirus software easily suspects unsigned programs that do this. The source code is public, and every release is built on GitHub Actions with build provenance. You can check the zip or `EasyKey.exe` you downloaded with the [GitHub CLI](https://cli.github.com):

```
gh attestation verify EasyKey<version>-Windows.zip --repo iamcino/EasyKey
```

Please weigh this before using it. If you trust it, you can add the EasyKey folder to your antivirus exclusions. Later versions will be signed once a certificate is available.

## Features
- Telex, VNI and Simple Telex input methods.
- Unicode code table. The legacy tables Unicode combining, TCVN3 (ABC) and VNI Windows are under Advanced.
- Modern mark placement (hoà, thuý) or the classic one (hòa, thúy).
- Smart typing: marks are added only to real Vietnamese words, and misspelled words get their keys back. Press Esc right after a word to get back the keys you typed (việt → vieejt).
- Quick Telex, shorthand for first and last consonants, and shortcuts (macros) of any length.
- Remembers Vietnamese/English mode and the code table per app.
- Fixes repeated letters in fields with autocomplete, including the Chrome and Edge address bar.
- Text converter and shortcut list, opened from the tray menu.
- A compact one-page Windows 11 settings window in Vietnamese or English, light and dark mode following Windows, your choice of accent color or your own hex color, high DPI support, and a tray icon that matches the taskbar.
- Checks for and installs updates from inside EasyKey (always asks first, checks the SHA-256).
- Removes itself cleanly from Windows with one button in Settings.
- No language indicator on the taskbar.

## Vietnamese spell check
Red underlines under Vietnamese words come from each app's own spell checker, which checks against English. They are not an EasyKey bug. The quickest fix is to add Vietnamese to that app's spell check:

- **Chrome, Edge, Cốc Cốc:** open Settings › Languages, add Vietnamese to your preferred languages, then turn on spell check for Vietnamese. If the browser has no Vietnamese dictionary, turn off spell check for English or turn spell check off entirely.
- **Word, Outlook, PowerPoint:** select the text, go to Review › Language › Set Proofing Language and pick Vietnamese. If Office has no Vietnamese proofing tools, tick "Do not check spelling or grammar" for that text.
- **Notepad:** open Settings (the gear icon) and turn off Spell check.

Adding Vietnamese in Windows Settings › Language also removes the underlines in many apps, but it brings back the language indicator (ENG/VIE) on the taskbar, so EasyKey does not recommend it.

## Build it yourself
You can build EasyKey yourself instead of downloading a release.

**Option 1: build on your PC**
1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) (the free Community edition) with the **Desktop development with C++** workload (it includes MSVC v143 and the Windows SDK).
2. Get the source: `git clone https://github.com/iamcino/EasyKey.git`, or Code › Download ZIP on GitHub.
3. Open `Sources/EasyKey/win32/EasyKey/EasyKey.sln`, choose the `Release` configuration and the `x64` platform, then Build › Build Solution.
4. The exe is at `Sources/EasyKey/win32/EasyKey/x64/Release/EasyKey.exe`.

Or build from the command line in "Developer PowerShell for VS 2022":

```
msbuild Sources\EasyKey\win32\EasyKey\EasyKey.sln -p:Configuration=Release -p:Platform=x64
```

**Option 2: build on GitHub, nothing to install**
Fork this repository. Every push or pull request to `master` makes GitHub Actions run the engine tests and build `EasyKey.exe`. Open the Actions tab, open the latest run and download `EasyKey` under Artifacts. Pushing a tag that matches `versionName` in `version.json` (for example `1.0.0`), or running the MSBuild workflow by hand on `master` (Actions › MSBuild › Run workflow), publishes a release.

**Run the engine tests** (needs [CMake](https://cmake.org) and a C++ compiler, works on Windows and Linux):

```
cmake -S Sources/EasyKey/tests -B build
cmake --build build --config Release
ctest --test-dir build -C Release
```

A build of your own is not signed either, so you may see the warnings described in [Windows warnings when you run it](#windows-warnings-when-you-run-it).

## License
Copyright (C) 2026 Cino. Contact: duongtheson@proton.me

EasyKey is free software, released under the [GNU GPL version 3](LICENSE). EasyKey is based on [OpenKey](https://github.com/tuyenvm/OpenKey), Copyright (C) 2019 Mai Vũ Tuyên, also under GPLv3. See [NOTICE](NOTICE).
