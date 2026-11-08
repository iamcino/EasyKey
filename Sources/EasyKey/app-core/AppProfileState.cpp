//
//  AppProfileState.cpp
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//

#include "AppProfileState.h"

bool AppProfileState::isShellTransitionExe(const std::string& exe) {
    if (exe.empty()) return false;
    static const char* const kShellExes[] = {
        "explorer.exe",
        "shellexperiencehost.exe",
        "startmenuexperiencehost.exe",
        "searchhost.exe"
    };
    for (const char* shell : kShellExes) {
        if (isSameExe(exe, shell)) {
            return true;
        }
    }
    return false;
}

AppProfileState::Decision AppProfileState::evaluateCandidate(
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
    uint64_t committedRoot,
    const std::string& committedExe
) {
    // Rule 2: If current foreground root does not match candidate root, the event is stale
    if (currentFgRoot != 0 && currentFgRoot != candidateRoot) {
        return Decision::IgnoreStale;
    }

    // Rule 3: Shell windows, hidden, minimized, cloaked or EasyKey own windows are skipped
    if (!isWindow || !isVisible || isIconic || isCloaked || isEasyKeyPid) {
        return Decision::IgnoreShellTransition;
    }

    // Rule 4: ApplicationFrameWindow without an active CoreWindow child
    if (isUwpFrameWithoutCore) {
        return Decision::IgnoreUwpWithoutCore;
    }

    // Rule 5: Cannot read exe (unknown)
    if (exe.empty()) {
        return Decision::IgnoreUnknownExe;
    }

    // Rule 3 (shell processes)
    if (isShellTransitionExe(exe)) {
        return Decision::IgnoreShellTransition;
    }

    // Rule 1b: Already committed window (avoids double commit and wiping active typing sessions)
    if (candidateRoot != 0 && candidateRoot == committedRoot && !committedExe.empty() && isSameExe(exe, committedExe)) {
        return Decision::AlreadyCommitted;
    }

    // Rule 10: Game app vs normal app
    if (isGame) {
        return Decision::CommitGame;
    }

    return Decision::CommitApp;
}

bool AppProfileState::shouldSaveProfile(const std::string& targetExe, bool isGameApp) {
    if (targetExe.empty()) return false;
    if (isGameApp) return false;
    if (isShellTransitionExe(targetExe)) return false;
    return true;
}

void AppProfileState::reset() {
    _committedRoot = 0;
    _committedExe.clear();
    _lastStableExe.clear();
    _lastStableIsGame = false;
}

AppProfileState::Decision AppProfileState::onForegroundEvent(
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
) {
    Decision d = evaluateCandidate(
        candidateRoot, currentFgRoot, isWindow, isVisible, isIconic,
        isCloaked, isEasyKeyPid, isUwpFrameWithoutCore, exe, isGame,
        _committedRoot, _committedExe
    );

    if (d == Decision::CommitGame || d == Decision::CommitApp) {
        _committedRoot = candidateRoot;
        _committedExe = exe;
        _lastStableExe = exe;
        _lastStableIsGame = isGame;
    }

    return d;
}
