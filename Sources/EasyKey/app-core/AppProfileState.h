//
//  AppProfileState.h
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//
//  Pure state machine and codec for per-app profile switching, shell window
//  filtering, and foreground commit tracking. Extracted from the Win32 layer
//  so decisions can be tested cross-platform without Win32 API calls.
//

#ifndef AppProfileState_h
#define AppProfileState_h

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include "ExeName.h"

namespace AppProfileCodec {
    constexpr int8_t kMaskLanguage = 0x01;
    constexpr int8_t kMaskCodeTable = 0x0F; // bits 1..4
    constexpr int8_t kBitHasMixed = (1 << 5); // 0x20
    constexpr int8_t kBitMixedOn  = (1 << 6); // 0x40

    struct UnpackedProfile {
        int language = 1;        // 0: English, 1: Vietnamese
        int codeTable = 0;       // 0..4
        bool hasMixedInfo = false;
        int mixedEnglish = 0;    // 0: off, 1: on
    };

    // Unpacks an Int8 profile value read from smartSwitchKey.
    // raw: the stored value, or -1 if no profile exists.
    // currentMixed: default mixedEnglish to use if hasMixedInfo is false (legacy entry).
    // twoModesOnly: if true, mixedEnglish is always forced to 0 on apply.
    inline UnpackedProfile unpack(int8_t raw, int currentMixed, bool twoModesOnly) {
        UnpackedProfile p;
        if (raw == -1) {
            return p;
        }
        p.language = raw & kMaskLanguage;
        p.codeTable = (raw >> 1) & kMaskCodeTable;
        p.hasMixedInfo = (raw & kBitHasMixed) != 0;
        if (p.hasMixedInfo) {
            p.mixedEnglish = twoModesOnly ? 0 : ((raw & kBitMixedOn) ? 1 : 0);
        } else {
            p.mixedEnglish = twoModesOnly ? 0 : currentMixed;
        }
        return p;
    }

    // Packs a profile into an Int8 for saving.
    // language: 0 or 1.
    // codeTable: 0..4.
    // currentMixed: 0 or 1.
    // twoModesOnly: if true, preserve existing bit 5 and bit 6 from existingRaw.
    // existingRaw: the previous Int8 profile, or -1 if none.
    inline int8_t pack(int language, int codeTable, int currentMixed, bool twoModesOnly, int8_t existingRaw) {
        int8_t val = (int8_t)((language & kMaskLanguage) | ((codeTable & kMaskCodeTable) << 1));
        if (twoModesOnly) {
            if (existingRaw != -1 && (existingRaw & kBitHasMixed)) {
                val |= (existingRaw & (kBitHasMixed | kBitMixedOn));
            }
        } else {
            val |= kBitHasMixed;
            if (currentMixed) {
                val |= kBitMixedOn;
            }
        }
        return (int8_t)(val & 0x7F); // ensure bit 7 is strictly 0
    }
}

class AppProfileState {
public:
    enum class Decision {
        AlreadyCommitted,         // candidate root == _committedRoot && exe == _committedExe
        IgnoreStale,              // event root != current foreground root
        IgnoreShellTransition,    // cloaked, minimized, invisible, easykey own, or shell process
        IgnoreUwpWithoutCore,     // ApplicationFrameWindow without CoreWindow child
        IgnoreUnknownExe,         // exe cannot be queried
        CommitGame,               // valid game foreground
        CommitApp                 // valid normal application foreground
    };

    static bool isShellTransitionExe(const std::string& exe);

    static Decision evaluateCandidate(
        uint64_t candidateRoot,
        uint64_t currentFgRoot,
        bool isWindow,
        bool isVisible,
        bool isIconic,
        bool isCloaked,
        bool isEasyKeyPid,
        bool isUwpFrameWithoutCore,
        const std::string& exe,
        bool isGame,
        uint64_t committedRoot = 0,
        const std::string& committedExe = ""
    );

    // Determines if profile save should be performed for the given target exe.
    // Never saves for game exes, shell transitions, empty exes, or when in game.
    static bool shouldSaveProfile(const std::string& targetExe, bool isGameApp);

    void reset();

    Decision onForegroundEvent(
        uint64_t candidateRoot,
        uint64_t currentFgRoot,
        bool isWindow,
        bool isVisible,
        bool isIconic,
        bool isCloaked,
        bool isEasyKeyPid,
        bool isUwpFrameWithoutCore,
        const std::string& exe,
        bool isGame
    );

    uint64_t getCommittedRoot() const { return _committedRoot; }
    const std::string& getCommittedExe() const { return _committedExe; }
    const std::string& getLastStableExe() const { return _lastStableExe; }
    bool isLastStableGame() const { return _lastStableIsGame; }

    void setLastStable(const std::string& exe, bool isGame) {
        if (!exe.empty() && !isShellTransitionExe(exe)) {
            _lastStableExe = exe;
            _lastStableIsGame = isGame;
        }
    }

private:
    uint64_t _committedRoot = 0;
    std::string _committedExe;
    std::string _lastStableExe;
    bool _lastStableIsGame = false;
};

#endif /* AppProfileState_h */
