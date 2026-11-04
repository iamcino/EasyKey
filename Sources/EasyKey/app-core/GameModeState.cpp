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

GameModeState::BindingCheck GameModeState::onKeyboardBindingCheck(uint64_t now, bool boundWindowAlive, uint64_t foregroundId, bool foregroundPidMatches) {
    BindingCheck result;
    if (windowId == 0)
        return result;
    if (!boundWindowAlive) {
        result.effects = clearBinding();
        return result;
    }
    if (armedExpired(now)) {
        result.effects = clearBinding();
        return result;
    }
    result.useForeground = true;
    if (foregroundId == windowId) {
        if (!confirmed || !inGame) {
            if (foregroundPidMatches) {
                confirmed = true;
                if (!inGame)
                    result.effects = enterGame();
                lastForegroundTick = now;
            } else {
                result.effects = clearBinding();
                result.useForeground = false;
            }
        } else {
            lastForegroundTick = now;
        }
    } else {
        if (inGame)
            result.effects = leaveGame();
    }
    return result;
}

GameModeEffects GameModeState::onMouseBindingCheck(uint64_t now, bool boundWindowAlive, uint64_t foregroundId, bool foregroundIsBoundProcess) {
    GameModeEffects fx;
    if (windowId != 0 && !boundWindowAlive)
        return clearBinding();
    if (!confirmed && lastForegroundTick != 0 && armedExpired(now))
        return clearBinding();
    if (foregroundId == windowId && foregroundId != 0) {
        if (foregroundIsBoundProcess) {
            confirmed = true;
            lastForegroundTick = now;
            fx = enterGame();
        } else {
            fx = clearBinding();
        }
    } else if (foregroundId != 0) {
        if (foregroundIsBoundProcess) {
            windowId = foregroundId;
            confirmed = true;
            lastForegroundTick = now;
            fx = enterGame();
        }
    }
    return fx;
}

GameModeEffects GameModeState::onForegroundChangedAway(uint64_t now, bool boundWindowAlive) {
    GameModeEffects fx = leaveGame();
    if (windowId != 0 && (!boundWindowAlive || armedExpired(now)))
        fx.absorb(clearBinding());
    return fx;
}

GameModeEffects GameModeState::onGameKeyDown(const KeyInput& input) {
    GameModeEffects fx;
    if ((input.vkCode == kVkReturn || input.vkCode == kVkEscape) && !input.altOrCtrl) {
        if (input.vkCode == kVkReturn) {
            if (chatOpen) {
                fx = closeChat();
            } else {
                chatOpen = true;
                chatRawCommand = false;
                chatTokenStart = true;
                fx.chatOpened = true;
            }
        } else {
            fx = closeChat();
        }
        return fx;
    }
    if (!chatOpen)
        return fx;
    if (chatRawCommand) {
        if (input.vkCode == kVkSpace) {
            chatRawCommand = false;
            chatTokenStart = true;
        }
        return fx;
    }
    if (input.vkCode == kVkSlash && chatTokenStart && !input.altOrCtrl) {
        chatRawCommand = true;
        chatTokenStart = false;
        return fx;
    }
    if (input.vkCode == kVkSpace) {
        chatTokenStart = true;
    } else if (input.vkCode != kVkShift && input.vkCode != kVkCapital && input.vkCode != kVkLShift && input.vkCode != kVkRShift) {
        chatTokenStart = false;
    }
    return fx;
}
