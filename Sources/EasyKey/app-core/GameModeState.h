//
//  GameModeState.h
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//
//  Pure state machine for Game Mode: the window binding (armed/confirmed), the
//  in-game flag and the game chat state. Extracted from the Win32 layer
//  (EasyKey.cpp) without behavior changes: window identity is an abstract
//  uint64_t id (the Win32 layer maps its HWND), time is a uint64_t millisecond
//  tick passed in by the caller (the Win32 layer passes GetTickCount64()), and
//  everything the machine cannot decide alone (IsWindow, the foreground
//  window, pid checks) arrives as event inputs. Side effects the Win32 layer
//  must perform (session resets) are reported back as GameModeEffects flags.
//

#ifndef GameModeState_h
#define GameModeState_h

#include <cstdint>

//Side effects for the Win32 layer to run, in this order, after a transition.
struct GameModeEffects {
    //the chat was open and closed: startNewSession + vStartSentence (+ clear the sync key on double codes)
    bool chatClosed = false;
    //the game was entered or left: resetModifierState + the same session reset as above
    bool sessionReset = false;
    //the chat was opened with Enter: startNewSession only
    bool chatOpened = false;
    //the game binding was dropped entirely
    bool bindingCleared = false;

    void absorb(const GameModeEffects& other) {
        chatClosed = chatClosed || other.chatClosed;
        sessionReset = sessionReset || other.sessionReset;
        chatOpened = chatOpened || other.chatOpened;
        bindingCleared = bindingCleared || other.bindingCleared;
    }
    bool any() const { return chatClosed || sessionReset || chatOpened || bindingCleared; }
};

struct GameModeState {
    //how long an armed (never foregrounded) candidate binding lives
    static constexpr uint64_t kArmedTtlMs = 10 * 60 * 1000;

    uint64_t windowId = 0;          //0 = no binding
    uint32_t processId = 0;
    bool confirmed = false;         //the bound window was seen in the foreground with a matching pid
    uint64_t lastForegroundTick = 0;
    bool inGame = false;            //the foreground app is in game mode
    bool chatOpen = false;          //Enter opened the chat
    bool chatRawCommand = false;    //a token starting with / is passed raw
    bool chatTokenStart = true;     //the next chat key starts a new token

    //an armed binding expires when the TTL has elapsed; a tick that went
    //backwards (now < last) expires it too, as in the original checks
    bool armedExpired(uint64_t now) const {
        return !confirmed && (now < lastForegroundTick || (now - lastForegroundTick) >= kArmedTtlMs);
    }

    //exitGameChat: always clears the chat flags; the session reset only runs when the chat was open
    GameModeEffects closeChat();
    //enterGameState: closes the chat, sets inGame, resets the session
    GameModeEffects enterGame();
    //leaveGameState: no-op when not in game
    GameModeEffects leaveGame();
    //clearGameBinding: leaves the game and drops the binding
    GameModeEffects clearBinding();
    //bindGameWindow without the Win32 guards (the caller checked the window and
    //its pid): isForeground says the bound window is the foreground window now
    GameModeEffects bind(uint64_t window, uint32_t pid, bool confirmedFlag, uint64_t now, bool isForeground);
    //the foreground-changed handler assigns inGame directly after binding a game window
    void forceInGame() { inGame = true; }

    struct BindingCheck {
        GameModeEffects effects;
        //the hook may use the foreground window it found (false after a clear
        //on pid mismatch/destruction/expiry, where the original drops it)
        bool useForeground = false;
        bool rebindOccurred = false;
    };
    //the keyboard hook's fast-path binding check on every key event
    BindingCheck onKeyboardBindingCheck(
        uint64_t now,
        bool processAlive,
        bool boundWindowAlive,
        uint64_t foregroundId,
        uint32_t foregroundPid,
        bool candidateEligible,
        bool candidateExeIsGame
    );
    //backward-compatible overload for legacy callers/tests
    inline BindingCheck onKeyboardBindingCheck(uint64_t now, bool boundWindowAlive, uint64_t foregroundId, bool foregroundPidMatches) {
        bool processAlive = boundWindowAlive;
        uint32_t fgPid = foregroundPidMatches ? (processId != 0 ? processId : 1) : 0;
        return onKeyboardBindingCheck(now, processAlive, boundWindowAlive, foregroundId, fgPid, foregroundPidMatches, false);
    }

    //the mouse hook's binding check on button-down while not in game
    GameModeEffects onMouseBindingCheck(
        uint64_t now,
        bool processAlive,
        bool boundWindowAlive,
        uint64_t foregroundId,
        uint32_t foregroundPid,
        bool candidateEligible,
        bool candidateExeIsGame
    );
    //backward-compatible overload for legacy callers/tests
    inline GameModeEffects onMouseBindingCheck(uint64_t now, bool boundWindowAlive, uint64_t foregroundId, bool foregroundIsBoundProcess) {
        bool processAlive = boundWindowAlive;
        uint32_t fgPid = foregroundIsBoundProcess ? (processId != 0 ? processId : 1) : 0;
        return onMouseBindingCheck(now, processAlive, boundWindowAlive, foregroundId, fgPid, foregroundIsBoundProcess, false);
    }

    //the foreground-changed handler when the new foreground is not a game app
    GameModeEffects onForegroundChangedAway(uint64_t now, bool processAlive, bool boundWindowAlive);
    //backward-compatible overload for legacy callers/tests
    inline GameModeEffects onForegroundChangedAway(uint64_t now, bool boundWindowAlive) {
        return onForegroundChangedAway(now, boundWindowAlive, boundWindowAlive);
    }

    struct KeyInput {
        uint16_t vkCode = 0;    //the resolved key code the hook works with
        bool altOrCtrl = false; //(the hook's MASK_ALT | MASK_CONTROL state)
    };
    //a key-down while in game (key-up events never reach the state machine).
    //Every in-game key is passed through to the game; only the state changes
    //and the effects matter.
    GameModeEffects onGameKeyDown(const KeyInput& input);
};

#endif /* GameModeState_h */
