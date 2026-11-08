//
//  GameModeState.cpp
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//

#include "GameModeState.h"

//virtual key codes the chat logic looks at (kept local so this file needs no Windows headers)
static constexpr uint16_t kVkReturn = 0x0D;
static constexpr uint16_t kVkEscape = 0x1B;
static constexpr uint16_t kVkSpace = 0x20;
static constexpr uint16_t kVkShift = 0x10;
static constexpr uint16_t kVkCapital = 0x14;
static constexpr uint16_t kVkLShift = 0xA0;
static constexpr uint16_t kVkRShift = 0xA1;
static constexpr uint16_t kVkSlash = 0xBF; //VK_OEM_2, the / key on the layouts this targets

GameModeEffects GameModeState::closeChat() {
    GameModeEffects fx;
    fx.chatClosed = chatOpen;
    chatOpen = false;
    chatRawCommand = false;
    chatTokenStart = true;
    return fx;
}

GameModeEffects GameModeState::enterGame() {
    GameModeEffects fx = closeChat();
    inGame = true;
    fx.sessionReset = true;
    return fx;
}

GameModeEffects GameModeState::leaveGame() {
    GameModeEffects fx;
    if (!inGame)
        return fx;
    fx = closeChat();
    inGame = false;
    fx.sessionReset = true;
    return fx;
}

GameModeEffects GameModeState::clearBinding() {
    GameModeEffects fx = leaveGame();
    windowId = 0;
    processId = 0;
    confirmed = false;
    lastForegroundTick = 0;
    fx.bindingCleared = true;
    return fx;
}

GameModeEffects GameModeState::bind(uint64_t window, uint32_t pid, bool confirmedFlag, uint64_t now, bool isForeground) {
    GameModeEffects fx;
    windowId = window;
    processId = pid;
    confirmed = confirmedFlag;
    lastForegroundTick = now;
    if (isForeground) {
        confirmed = true;
        fx = enterGame();
    } else if (inGame) {
        fx = leaveGame();
    }
    return fx;
}

GameModeState::BindingCheck GameModeState::onKeyboardBindingCheck(
    uint64_t now,
    bool processAlive,
    bool boundWindowAlive,
    uint64_t foregroundId,
    uint32_t foregroundPid,
    bool candidateEligible,
    bool candidateExeIsGame
) {
    BindingCheck result;

    if (windowId == 0 && processId == 0 && !candidateExeIsGame) {
        return result;
    }

    // 1. Process exit check:
    // If we have a bound process and that process died, clear binding completely
    if (processId != 0 && !processAlive) {
        result.effects = clearBinding();
        return result;
    }

    // 2. Armed binding expiry check:
    if (armedExpired(now)) {
        result.effects = clearBinding();
        return result;
    }

    // 3. Bound window destruction:
    // If the bound window was destroyed, drop windowId (leave game if inGame)
    // but keep processId intact while the process is still alive.
    if (windowId != 0 && !boundWindowAlive) {
        windowId = 0;
        if (inGame) {
            result.effects.absorb(leaveGame());
        }
    }

    bool pidMatches = (processId != 0 && foregroundPid != 0 && foregroundPid == processId);
    bool matchesGame = pidMatches || candidateExeIsGame;

    // Case A: Foreground is the currently bound window
    if (foregroundId != 0 && foregroundId == windowId) {
        result.useForeground = true;
        if (!confirmed || !inGame) {
            // Unconfirmed armed binding or entering from inactive state
            if (pidMatches || candidateExeIsGame) {
                confirmed = true;
                if (foregroundPid != 0) {
                    processId = foregroundPid;
                }
                if (!inGame) {
                    result.effects.absorb(enterGame());
                }
                lastForegroundTick = now;
            } else {
                // PID mismatch on the bound window: clear binding
                result.effects.absorb(clearBinding());
                result.useForeground = false;
            }
        } else {
            // Steady state: already confirmed and in game
            if (foregroundPid != 0 && !matchesGame) {
                result.effects.absorb(clearBinding());
                result.useForeground = false;
            } else {
                lastForegroundTick = now;
            }
        }
        return result;
    }

    // Case B: Foreground is a new eligible window belonging to the game process or game exe
    if (foregroundId != 0 && candidateEligible && matchesGame) {
        result.useForeground = true;
        windowId = foregroundId;
        if (foregroundPid != 0) {
            processId = foregroundPid;
        }
        confirmed = true;
        lastForegroundTick = now;
        result.effects.absorb(enterGame());
        result.rebindOccurred = true;
        return result;
    }

    // Case C: Foreground is not a game window (tabbed away or ineligible window)
    result.useForeground = true;
    if (inGame) {
        result.effects.absorb(leaveGame());
    }
    return result;
}

GameModeEffects GameModeState::onMouseBindingCheck(
    uint64_t now,
    bool processAlive,
    bool boundWindowAlive,
    uint64_t foregroundId,
    uint32_t foregroundPid,
    bool candidateEligible,
    bool candidateExeIsGame
) {
    GameModeEffects fx;

    if (processId != 0 && !processAlive) {
        return clearBinding();
    }

    if (armedExpired(now)) {
        return clearBinding();
    }

    if (windowId != 0 && !boundWindowAlive) {
        windowId = 0;
        if (inGame) {
            fx.absorb(leaveGame());
        }
    }

    bool pidMatches = (processId != 0 && foregroundPid != 0 && foregroundPid == processId);
    bool matchesGame = pidMatches || candidateExeIsGame;

    if (foregroundId != 0 && foregroundId == windowId) {
        if (!confirmed || !inGame) {
            if (pidMatches || candidateExeIsGame) {
                confirmed = true;
                if (foregroundPid != 0) {
                    processId = foregroundPid;
                }
                lastForegroundTick = now;
                fx.absorb(enterGame());
            } else {
                fx.absorb(clearBinding());
            }
        } else {
            if (foregroundPid != 0 && !matchesGame) {
                fx.absorb(clearBinding());
            }
        }
        return fx;
    }

    if (foregroundId != 0 && candidateEligible && matchesGame) {
        windowId = foregroundId;
        if (foregroundPid != 0) {
            processId = foregroundPid;
        }
        confirmed = true;
        lastForegroundTick = now;
        fx.absorb(enterGame());
    }

    return fx;
}

GameModeEffects GameModeState::onForegroundChangedAway(uint64_t now, bool processAlive, bool boundWindowAlive) {
    GameModeEffects fx = leaveGame();
    if (processId != 0 && !processAlive) {
        fx.absorb(clearBinding());
    } else if (windowId != 0 && !boundWindowAlive) {
        windowId = 0;
    } else if (armedExpired(now)) {
        fx.absorb(clearBinding());
    }
    return fx;
}

GameModeEffects GameModeState::onGameKeyDown(const KeyInput& input) {
    GameModeEffects fx;
    if ((input.vkCode == kVkReturn || input.vkCode == kVkEscape) && !input.altOrCtrl) {
        if (input.vkCode == kVkReturn) {
            if (chatOpen) {
                //Enter sends the typed text and closes the chat
                fx = closeChat();
            } else {
                //Enter opens the chat
                chatOpen = true;
                chatRawCommand = false;
                chatTokenStart = true;
                fx.chatOpened = true;
            }
        } else {
            //Escape always closes the chat (canceling input)
            fx = closeChat();
        }
        return fx;
    }
    if (!chatOpen)
        return fx;

    //the rest runs only while chatting
    if (chatTokenStart && input.vkCode == kVkSlash && !input.altOrCtrl) {
        chatRawCommand = true;
        chatTokenStart = false;
        return fx;
    }
    if (input.vkCode == kVkSpace) {
        chatRawCommand = false;
        chatTokenStart = true;
        return fx;
    }
    //Shift and CapsLock do not advance token positions
    if (input.vkCode == kVkShift || input.vkCode == kVkCapital ||
        input.vkCode == kVkLShift || input.vkCode == kVkRShift) {
        return fx;
    }
    //any other typed character ends the start-of-token window
    chatTokenStart = false;
    return fx;
}
