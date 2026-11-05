//
//  PasswordState.h
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//
//  Pure pieces of the password-field bypass state machine, extracted from the
//  Win32 layer (EasyKey.cpp) without behavior changes: the state word codec
//  and the next-state decisions. The Win32 layer keeps the atomic word, the
//  probe worker thread and the CAS loops, and asks this unit what the next
//  word should be.
//
//  State values are the engine's enum PasswordState (engine/Engine.h):
//  0 = PW_NORMAL, 1 = PW_PASSWORD, 2 = PW_PENDING.
//  Source values are the Win32 layer's PasswordStateSource:
//  0 = none, 1 = Win32, 2 = UIA, 3 = MSAA.
//

#ifndef PasswordState_h
#define PasswordState_h

#include <cstdint>

//how long a probe may stay pending before the hook thread falls back to normal
constexpr uint64_t kPasswordPendingTimeoutMs = 60;

//state values (the engine's enum PasswordState) and source values (the Win32
//layer's enum PasswordStateSource), named here so this unit needs neither
//header; EasyKey.cpp static_asserts that these stay equal to the enums
constexpr int kStateNormal = 0;
constexpr int kStatePassword = 1;
constexpr int kStatePending = 2;
constexpr int kSourceNone = 0;
constexpr int kSourceWin32 = 1;

//state word layout: low byte = state, next byte = source, rest zero
uint32_t makePasswordStateWord(int state, int source);
int getPasswordState(uint32_t word);
int getPasswordSource(uint32_t word);

//a decided transition of the state word; fields the decision does not touch
//stay false/zero
struct PasswordTransition {
    bool changesWord = false;   //the caller should store nextState/nextSource
    int nextState = 0;
    int nextSource = 0;
    bool markDirty = false;     //set the dirty flag (a session reset follows)
    bool bumpGeneration = false; //invalidate any probe result still in flight
};

//a probe result arrived for the current focus: from PW_PENDING it settles the
//state (password keeps the probe's source, not-password resets to none and is
//dirty); from PW_NORMAL a password result upgrades (dirty), a not-password
//result changes nothing; PW_PASSWORD is sticky and never downgrades here
PasswordTransition passwordAfterProbeResult(int currentState, bool isPassword, int source);

//the hook thread noticed a pending probe whose deadline has passed: fall back
//to normal, dirty, and bump the generation so the late result is discarded
PasswordTransition passwordAfterPendingTimeout(int currentState, uint64_t now, uint64_t deadline);

struct PasswordSchedule {
    bool schedule = false;      //false only for the sticky-password case below
    uint64_t deadline = 0;      //when the pending state expires
};

//a probe is about to be scheduled for a focus: a sticky PW_PASSWORD on the
//same focus is kept unless forced; anything else goes PW_PENDING with a
//fresh deadline
PasswordSchedule passwordOnProbeScheduled(int currentState, bool force, uint64_t now);

//the UIA source was disabled in settings: pending states and UIA/MSAA-sourced
//password states reset to normal; a Win32-sourced password state survives
bool passwordResetWhenUiaDisabled(int state, int source);

#endif /* PasswordState_h */
