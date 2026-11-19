# Independent QA Report — EasyKey MixedModeDecider (Package 0 + 1)

- **Reviewer:** Darius (Independent QA)
- **Subject:** `garen/mixed-mode-decider` (PR #86), head `2cc530ffa04d3b5da76552ed39281722e063cd65`, Package 0 `7195deb`, base `master` `6dc66e73` (Release 1.1.6)
- **QA clone & branch:** `C:\projects\easykey-aaa\darius-qa\mixed-decider`, branch `darius/qa-mixed-decider` (branched from `2cc530f`)
- **Reference baseline clone:** `C:\projects\easykey-aaa\darius-qa\baseline-116` at pristine `6dc66e73`
- **Verdict:** **PASS**

---

## 1. Build & Full Suite Verification (Measured)

All builds and tests were executed cleanly in an isolated QA workspace (`C:\projects\easykey-aaa\darius-qa\mixed-decider`).

| Check | Command / Target | Status | Measured Details |
|---|---|---|---|
| MSBuild App (Release x64) | `EasyKey.vcxproj` (`/p:Configuration=Release /p:Platform=x64`) | **PASS** | Exit 0; `0 Warning(s)`, `0 Error(s)`; compiles `engine/MixedModeDecider.cpp`; produced `EasyKey.exe` = **680,960 bytes** |
| CMake Test Build (Release) | `Sources/EasyKey/tests` -> `build-tests` (MSVC 19.44.35229, VS 2022 x64) | **PASS** | Exit 0; all test targets compiled cleanly with no warnings treated as failures |
| `ctest` (Release, full suite) | `ctest -C Release --output-on-failure -V` | **PASS** | **6/6 passed** in **6.69 sec** (100% pass rate) |
| `EngineTests` | Suite #1 | **PASS** | **3536/3536 passed** (1.37 sec) — matches Package 0 characterization count |
| `EngineStateTests` | Suite #2 | **PASS** | **62/62 passed** (1.05 sec) — matches Package 0 characterization count |
| `PacketDeliveryTests` | Suite #3 | **PASS** | **15/15 passed** (0.77 sec) — matches Package 0 characterization count |
| `UpdateSignatureTests` | Suite #4 | **PASS** | **18/18 passed** (1.21 sec) |
| `UpdateCleanupTests` | Suite #5 | **PASS** | **9/9 passed** (0.71 sec) |
| `HttpBodyTests` | Suite #6 | **PASS** | **19 checks, 0 failures** (0.82 sec) |

---

## 2. Differential Behavior Battery vs Pristine Master (Measured)

To independently guarantee that the extraction of `MixedModeDecider` from `Engine.cpp` is 100% behavior-preserving, two standalone fresh-process test probes were constructed:
1. `probe-head`: linked against `Sources/EasyKey/engine/*.cpp` from the branch head (`2cc530f`), including `MixedModeDecider.cpp`.
2. `probe-base`: linked against `Sources/EasyKey/engine/*.cpp` from pristine master baseline `6dc66e73` (Release 1.1.6).

Both probes share identical harness drivers (`probe.cpp`), emulating Windows hook keystroke injection and screen buffer tracking. Each test invocation ran in a **fresh OS process** with initial Mouse break event and `startNewSession()`. The harness logged the full keystroke sequence, final screen text (UTF-8), and the complete packet stream (`code`, `extCode`, `backspaceCount`, `newCharCount`, `charData`) at every boundary and key event.

A comprehensive differential battery of **79 test cases** across **158 fresh OS processes** was executed. The probe outputs were verified for strict byte-for-byte identity.

### Battery Results Summary

| Battery Group | Test Count | Process Count | Mismatches | Status | Measured Behavior & Output |
|---|---:|---:|---:|---|---|
| **1. Phrases** | 13 | 26 | 0 | **PASS** | Byte-identical across both trees: `There are ` -> `"There are "` (fix-last=1) vs `"Thể are "` (fix-last=0); `this car test ` -> `"this car test "` (fix-last=1) vs `"this cả test "` (fix-last=0); `Did you see ` -> `"Did you see "` (fix-last=1) vs `"Đi you see "` (fix-last=0); `The car was ` -> `"The car was "` (fix-last=1) vs `"The cả was "` (fix-last=0); `xe bus ` -> `"xe bus "` (Telex EV). Pure mode controls also matched byte-for-byte. |
| **2. Context chains** | 5 | 10 | 0 | **PASS** | `this this this ` -> `"this this this "` (EV fl=0 & fl=1); `tooi muoons ` -> `"tôi muốn "` across EV fl=0, fl=1, and pure Telex. |
| **3. Tie case** | 3 | 6 | 0 | **PASS** | `tooi as ` -> `"tôi á "` across EV fl=0, fl=1, and pure Telex. |
| **4. Code-like tokens** | 9 | 18 | 0 | **PASS** | Identifiers do not pull context: `this user_id as ` -> `"this user_id as "`; `tooi user_id as ` -> `"tôi user_id á "`; `test a1 ` -> `"test a1 "` in Telex EV; `test a1 ` -> `"test á "` in VNI EV & pure VNI. |
| **5. Double-space after fixable pair** | 6 | 12 | 0 | **PASS** | Second Space suppresses/kills pending fix across both trees identically: `this car  are ` -> `"this cả  are "`; `There  are ` -> `"Thể  are "`; `this car  test ` -> `"this cả  test "`. |
| **6. Fix followed by Delete / Backspace** | 9 | 18 | 0 | **PASS** | Post-fix deletions through space, rewritten word, and previous context matched byte-for-byte: `this car are < ` -> `"this car are "`; `this car are <<<<<` -> `"this car"`; `this car are <<<<<<<<<` -> `"this"`; `this car are <<<<<<<<<<<<new ` -> `"tnew "`; `There are < ` -> `"There are "`; `There are <<<<<` -> `"There"`; `There are <<<<<<<<<` -> `"T"`; `this car test < ` -> `"this car test "`; `this car test <<<<<<<` -> `"this ca"`. |
| **7. Pinned known bugs & Telex controls** | 12 | 24 | 0 | **PASS** | Pinned current behaviors reproduced identically: mixed `doo ` -> `"doo "` (restores raw at boundary Space; packet code 3, ext 4, bs 2, nc 3); pure Telex `doo ` -> `"dô "`; mid-word `doo` -> `"dô"`; `cauw ` -> `"cauw "` (raw in EV and pure); mid-word `cauw` -> `"cauw"`; `caau ` -> `"câu "` in EV and pure; mid-word `caau` -> `"câu"`. |
| **8. VNI controls** | 6 | 12 | 0 | **PASS** | `do6 ` -> `"dô "` across VNI pure and VNI mixed (fl=0 and fl=1); `ca6u ` -> `"câu "` across VNI pure and VNI mixed. |
| **9. Additional boundary & code table cases** | 16 | 32 | 0 | **PASS** | `as tooi ` -> `"á tôi "` (fix to Vietnamese); `this car are ` -> `"this car are "` (fix to English); `this car tooi ` -> `"this cả tôi "` (no rewrite); `banhs test ` -> `"bánh tét "` (phrase after space); `banhs. test ` -> `"bánh. test "` (phrase starved by dot); `There are. ` -> `"There are. "` (fix at dot fl=1) vs `"Thể are. "` (fl=0); `There are, ` -> `"There are, "`; double-code table checks: TCVN3 (fix allowed, bs 6 nc 9), VNI code & Unicode compound (fix blocked, bs 2 nc 3 restore only); spelling on `qaf ` -> `"qaf "`. |
| **Total Battery** | **79** | **158** | **0** | **PASS** | **0 mismatches.** Screen text and all event packets are 100% byte-identical. |

---

## 3. Registration & Symbol Inspection

1. **Application Project Registration:**
   - `Sources/EasyKey/win32/EasyKey/EasyKey/EasyKey.vcxproj`: registers `MixedModeDecider.h` at line 107 and `MixedModeDecider.cpp` at line 146.
   - `Sources/EasyKey/win32/EasyKey/EasyKey/EasyKey.vcxproj.filters`: registers `MixedModeDecider.h` at line 66 and `MixedModeDecider.cpp` at line 176 under the `engine` filter.

2. **Test Target Registration:**
   - `Sources/EasyKey/tests/CMakeLists.txt`: compiles `MixedModeDecider.cpp` into all three engine-bearing test targets:
     - `EngineTests` (line 20)
     - `EngineStateTests` (line 45)
     - `PacketDeliveryTests` (line 64)

3. **Symbol Cleanliness:**
   - Full symbol inspection of `Sources/EasyKey/engine/Engine.cpp` confirms that all previous file-static mixed-mode state variables and constants have been completely removed:
     - `_mixedContext`, `_mixedLast`, `_hasMixedLast`, `_lastRaw`, `_lastRawLen`, `mixedBias`, `MIXED_CLEAR`, `MIXED_VIETNAMESE_LEAN`, `MIXED_CONTEXT_WEIGHT` are absent from `Engine.cpp`.
   - `Engine.cpp` interacts with mixed mode solely through `static MixedModeDecider _mixedDecider;` and its clean public methods (`decide()`, `forgetLast()`, `forgetAll()`).

4. **Test-Only Accessor Encapsulation:**
   - Test inspection accessors (`mixedContextForTest()`, `hasMixedLastForTest()`, `lastRawLenForTest()`, `setMixedLastForTest()`) in `MixedModeDecider.h` / `MixedModeDecider.cpp` are strictly guarded by `#ifdef EASYKEY_TEST`.
   - `EASYKEY_TEST` is defined only in `Sources/EasyKey/tests/CMakeLists.txt` (lines 78–80) with `PRIVATE` scope for the three test targets, and is never defined in the application project. Production code has zero mutation or leak surface.

---

## 4. Observations & Findings

- **No blocking findings, no regressions.**
- The separation between decision (`MixedModeDecider`) and execution (`Engine.cpp`) strictly preserves all semantics and timings of baseline 1.1.6.
- The two pinned known bugs (`doo ` boundary restore and `cauw ` raw trailing-w placement) behave identically to baseline 1.1.6 across all modes and controls, exactly fulfilling section 4c of the confirmed plan.
- Visual Studio project build and CMake test suite run cleanly with 0 errors and 0 warnings.

**Verdict: PASS.** Branch `garen/mixed-mode-decider` (PR #86) satisfies all independent QA gate requirements.
