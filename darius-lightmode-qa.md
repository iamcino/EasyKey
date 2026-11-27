# Independent QA Report — EasyKey PR #94: Light-Mode Edit Surface

- **Reviewer:** Darius (Independent QA)
- **PR:** https://github.com/iamcino/EasyKey/pull/94
- **Subject branch & commit:** `garen/lightmode-edit-surface` at `a3f84b52a03149b5372d6085c81a65500bfa631b`
- **QA clone & branch:** `C:\projects\easykey-aaa\darius-qa\lightmode-qa`, branch `darius/qa-lightmode` (branched from `a3f84b52a03149b5372d6085c81a65500bfa631b`)
- **Files changed in PR:**
  - `Sources/EasyKey/win32/EasyKey/EasyKey/FluentWindow.cpp` (+28/−1)
  - `Sources/EasyKey/win32/EasyKey/EasyKey/Style.manifest` (+8)
- **Verdict:** **PASS**

---

## 1. Clean Build Verification — Check (a)

- **Command:** `MSBuild.exe EasyKey.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m /nologo`
- **Result:** **PASS** — Exit code `0`, `0 Warning(s)`, `0 Error(s)` (50.23 s).
- **Output binary:** `C:\projects\easykey-aaa\darius-qa\lightmode-qa\Sources\EasyKey\win32\EasyKey\x64\Release\EasyKey.exe`
  - **Size:** `681,984` bytes
  - **Timestamp:** `2026-11-27 17:15:04 +07:00`

---

## 2. Live UI & Pixel Measurements — Checks (b), (c), (d), (e)

All UI checks were executed in the interactive desktop session (Session 2) against the freshly built `EasyKey.exe`, capturing real DWM-composed screen pixels via `CopyFromScreen` (`DWMWA_EXTENDED_FRAME_BOUNDS`) and measuring RGB values inside both `WC_EDIT` controls (`ID_NAME` = Shortcut, `ID_CONTENT` = Text) and their surrounding field padding.

Artifacts directory: `C:\projects\easykey-aaa\darius-qa\lightmode-qa-work\shots\`

| Check | Mode / Config | Edit 1 (Shortcut) Measured Pixels | Edit 2 (Text) Measured Pixels | Surrounding Field | Bleach `[243..254]` | Screenshot | Status |
|---|---|---|---|---|---|---|---|
| **(b1)** Light (`vThemeMode=0`) | `Apps=1, Sys=1, vThemeMode=0, Trans=1` | `vn`: darkest `(27,27,27)`, mode `(255,255,255)` | `test`: darkest `(0,0,0)` (caret) / `(27,27,27)`, mode `(255,255,255)` | `(255,255,255)` (no seam) | `0` | `b1_light_vtheme0.png` | **PASS** |
| **(b2)** Light (`vThemeMode=1`) | `Apps=1, Sys=1, vThemeMode=1, Trans=1` | `vn`: darkest `(27,27,27)`, mode `(255,255,255)` | `test`: darkest `(0,0,0)` (caret) / `(27,27,27)`, mode `(255,255,255)` | `(255,255,255)` (no seam) | `0` | `b2_light_vtheme1.png` | **PASS** |
| **(c1)** Dark (`vThemeMode=0`) | `Apps=0, Sys=0, vThemeMode=0, Trans=1` | `vn`: brightest `(255,255,255)`, mode `(45,45,45)` | `test`: brightest `(255,255,255)`, mode `(45,45,45)` | `(45,45,45)` (no `(77,77,77)` box) | `0` | `c1_dark_vtheme0.png` | **PASS** |
| **(c2)** Dark (`vThemeMode=2`) | `Apps=0, Sys=0, vThemeMode=2, Trans=1` | `vn`: brightest `(255,255,255)`, mode `(45,45,45)` | `test`: brightest `(255,255,255)`, mode `(45,45,45)` | `(45,45,45)` (no `(77,77,77)` box) | `0` | `c2_dark_vtheme2.png` | **PASS** |
| **(d)** Placeholder (Light) | `Apps=1, Sys=1, vThemeMode=0, Trans=1`, empty | `"e.g. vn"`: darkest `(87,87,87)`, mode `(255,255,255)` | `"e.g. Việt Nam"`: darkest `(87,87,87)`, mode `(255,255,255)` | `(255,255,255)` | `0` | `d_placeholder_light.png` | **PASS** |
| **(e1)** Mica off (Light) | `Apps=1, Sys=1, vThemeMode=0, Trans=0` | `vn`: darkest `(27,27,27)`, mode `(255,255,255)` | `test`: darkest `(0,0,0)`, mode `(255,255,255)` | `(255,255,255)` | `0` | `e1_micaoff_light.png` | **PASS** |
| **(e2)** Mica off (Dark) | `Apps=0, Sys=0, vThemeMode=0, Trans=0` | `vn`: brightest `(255,255,255)`, mode `(45,45,45)` | `test`: brightest `(255,255,255)`, mode `(45,45,45)` | `(45,45,45)` | `0` | `e2_micaoff_dark.png` | **PASS** |

---

## 3. Smoke Checks — Check (f)

| Smoke Check | Measured Result | Status |
|---|---|---|
| App launches | `EasyKey.exe` launched cleanly in Session 2 | **PASS** |
| Tray icon present | UIA found tray icon `"EasyKey EasyKey: typing English and Vietnamese"` at `(982,655,1022,695)` | **PASS** |
| Telex typing in Notepad | Typed `vieejt nam ` in fresh Notepad tab (`vLanguage=1`); read back `việt nam ` | **PASS** |
| Settings opens | Second-instance launch opened Settings (`title="EasyKey"`, `442x268`, `f_settings.png`) | **PASS** |
| Live light/dark toggle while window open | Toggled `AppsUseLightTheme`/`SystemUsesLightTheme` `1 -> 0 -> 1` (`vThemeMode=0, Trans=1`) with Shortcut list open; process remained alive (`pid=18380`), `f_live_toggle_light.png` measured darkest `(27,27,27)` on `(255,255,255)`, `bleach=0` | **PASS** |

---

## 4. Verdict & Restore

- **Verdict:** **PASS** — All checks (a)–(f) passed with zero bleach-gray pixels, exact expected edit backgrounds (`(255,255,255)` light, `(45,45,45)` dark), readable placeholders (`(87,87,87)`), clean Mica-off behavior, and clean smoke execution.
- **System restore:** Restored `AppsUseLightTheme=0`, `SystemUsesLightTheme=0`, `vThemeMode=2`, `EnableTransparency=1`, `vLanguage=0`, stopped all `EasyKey.exe` processes, and removed all temporary scheduled tasks.
