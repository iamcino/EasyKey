# Independent QA Report — EasyKey Bug C2 (Central Highlands Names)

- **Reviewer:** Darius (Independent QA)
- **Subject:** `garen/bugc2-highland-names` on GitHub (`https://github.com/iamcino/EasyKey.git`), tip `ddb830c`, base `master` `867fcd6`
- **QA clone & branch:** `C:\projects\easykey-aaa\repo-qa-bugc2`, branch `darius/qa-bugc2` (branched from `ddb830c`)
- **Verdict:** **PASS**

---

## 1. Build & Full Suite Verification (Measured)

All builds and tests were run cleanly from `C:\projects\easykey-aaa\repo-qa-bugc2` (CMake test build in `C:\projects\easykey-aaa\qa-bugc2-tests`).

| Check | Command / Target | Status | Measured Details |
|---|---|---|---|
| MSBuild App (Release x64) | `EasyKey.sln` (`/p:Configuration=Release /p:Platform=x64`) | **confirmed** | Exit 0; `0 Warning(s)`, `0 Error(s)`; `EasyKey.exe` = **680,960 bytes** |
| CMake Test Build (Release) | `Sources/EasyKey/tests` -> `qa-bugc2-tests` | **confirmed** | Exit 0; all 5 test executables built cleanly |
| `ctest` (Release, full suite) | `ctest -C Release --output-on-failure -V` | **confirmed** | **5/5** passed in **6.20 sec** |
| `EngineTests` | Suite #1 | **confirmed** | **3440/3440** passed (1.48 sec) |
| `EngineStateTests` | Suite #2 | **confirmed** | **23/23** passed (0.96 sec) |
| `UpdateSignatureTests` | Suite #3 | **confirmed** | **18/18** passed (1.58 sec) |
| `UpdateCleanupTests` | Suite #4 | **confirmed** | **9/9** passed (0.91 sec) |
| `HttpBodyTests` | Suite #5 | **confirmed** | **19 checks, 0 failures** (0.67 sec) |

---

## 2. Independent Fresh-Session Probe Measurements

Built a standalone probe (`C:\projects\easykey-aaa\qa-bugc2-probe\build\Release\probe.exe`) linked directly against `repo-qa-bugc2/Sources/EasyKey/engine/*.cpp`, plus a `master` (`867fcd6`) reference probe (`qa-bugc2-probe-master`) from a temporary detached worktree. Each case ran in a fresh OS process with mouse reset and `startNewSession()`.

| Item Checked | Status | Measured Results |
|---|---|---|
| Ten shapes in Telex & VNI at screen, Space, `.`, `,` (80 fresh processes) | **confirmed** | All 10 shapes (`Kroong`/`Kro6ng` -> `Krông`, `Drowng`/`Dro7ng` -> `Drơng`, `Ddrowng`/`D9ro7ng` -> `Đrơng`, `Adrowng`/`Adro7ng` -> `Adrơng`, `Addrowng`/`Ad9ro7ng` -> `Ađrơng`, `Blowh`/`Blo7h` -> `Blơh`, `Kpaw`/`Kpa8` -> `Kpă`, `Adraang`/`Adra6ng` -> `Adrâng`, `Proong`/`Pro6ng` -> `Prông`, `Pawh`/`Pa8h` -> `Păh`) produced the expected screen and survived Space, `.`, and `,` in both Telex and VNI. Completing-key `bpc`/`ncc` matched: `5/5` (`Krông`, `Drơng`, `Đrơng`, `Prông`), `6/6` (`Adrơng`, `Adrâng`), `7/6` (`Ađrơng`), `4/4` (`Blơh`), `3/3` (`Kpă`), `0/0` (`Pawh`). |
| Phrases `Chuw Proong` / `Chu7 Pro6ng` and `Chuw Pawh` / `Chu7 Pa8h` at Space, `.`, `,` | **confirmed** | `Chư Prông` and `Chư Păh` survived Space, `.`, and `,` in both Telex and VNI. |
| Tone spot checks (`Kroongr `, `Kro6ng3 `, `Proongr `, `Pro6ng3 `, `Blowhs `, `Pawhs `, `Kpaws `, `Adraangs `) | **confirmed** | Produced `Krổng `, `Krổng `, `Prổng `, `Prổng `, `Blớh `, `Pắh `, `Kpắ `, `Adrấng ` respectively. |
| Floor neighbours vs `master` (`867fcd6`) across Telex, EV, and VNI at screen, Space, `.`, `,` | **confirmed** | **72/72** cells (`proof`, `practice`, `prawn`, `program`, `approach`, `paws` x 3 modes x 4 endings) had **0 mismatches** between `ddb830c` and `867fcd6`. |
| Macro-on case (`Addrowng` in Telex, `Ad9ro7ng` in VNI, `Addrowngs` in Telex) | **confirmed** | `Addrowng` and `Ad9ro7ng` produced `Ađrơng` with `bpc=7`, `ncc=6`, and `macro(6): 00010041 00020044 00000052 0004004F 0000004E 00000047`; `Addrowngs` produced `Ađrớng` with `macro(6): 00010041 00020044 00000052 000C004F 0000004E 00000047`. |
| Backspace-residue case (`Kroong<` and `Kroong< ` in Telex) | **confirmed** | `Kroong<` -> `Krôn`; `Kroong< ` -> `Kroon ` (`bpc=4`, `ncc=5`). |

---

## 3. Diff & Design Conformance Inspection

| Check | Status | Verification Notes |
|---|---|---|
| Diff confined to `Engine.cpp` plus tests; no spelling table edits | **confirmed** | `git diff --stat 867fcd6..ddb830c` lists only `Sources/EasyKey/engine/Engine.cpp` (+204/-2), `Sources/EasyKey/tests/BugC2Tests.inc` (+527), and `Sources/EasyKey/tests/EngineTests.cpp` (+3). `Vietnamese.cpp` and all other engine/app files are untouched. |
| Macro clamp matches specified `start < 0` form | **confirmed** | `Engine.cpp:2114-2123` computes `int start = (int)_index - (int)hBPC;` before any unsigned wrap; when `start < 0`, it loops `i = 0; i < _index; i++` pushing `TypingWord[i]`, and when `start >= 0` it preserves the existing `for (i = _index - hBPC; i < hNCC + (_index - hBPC); i++)` loop. |

---

## 4. Findings

- No regressions or blockers found. All builds, unit tests, independent fresh-process probes, `master` floor comparisons, and diff inspections passed.
