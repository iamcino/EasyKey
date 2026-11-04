//
//  PasswordState.cpp
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//

#include "PasswordState.h"

//state values (engine's enum PasswordState) and the Win32 source value,
//named locally so this file needs neither header
static constexpr int kStateNormal = 0;
static constexpr int kStatePassword = 1;
static constexpr int kStatePending = 2;
static constexpr int kSourceNone = 0;
static constexpr int kSourceWin32 = 1;

uint32_t makePasswordStateWord(int state, int source) {
    return (static_cast<uint32_t>(state) & 0xFF) | ((static_cast<uint32_t>(source) & 0xFF) << 8);
}

int getPasswordState(uint32_t word) {
    return static_cast<int>(word & 0xFF);
}

int getPasswordSource(uint32_t word) {
    return static_cast<int>((word >> 8) & 0xFF);
}

PasswordTransition passwordAfterProbeResult(int currentState, bool isPassword, int source) {
    PasswordTransition t;
    if (currentState == kStatePending) {
        t.changesWord = true;
        t.nextState = isPassword ? kStatePassword : kStateNormal;
        t.nextSource = isPassword ? source : kSourceNone;
        t.markDirty = !isPassword;
    } else if (currentState == kStateNormal && isPassword) {
        t.changesWord = true;
        t.nextState = kStatePassword;
        t.nextSource = source;
        t.markDirty = true;
    }
    return t;
}

PasswordTransition passwordAfterPendingTimeout(int currentState, uint64_t now, uint64_t deadline) {
    PasswordTransition t;
    if (currentState == kStatePending && now >= deadline) {
        t.changesWord = true;
        t.nextState = kStateNormal;
        t.nextSource = kSourceNone;
        t.markDirty = true;
        t.bumpGeneration = true;
    }
    return t;
}

PasswordSchedule passwordOnProbeScheduled(int currentState, bool force, uint64_t now) {
    PasswordSchedule s;
    if (!force && currentState == kStatePassword)
        return s; //sticky password on the same focus: do not downgrade
    s.schedule = true;
    s.deadline = now + kPasswordPendingTimeoutMs;
    return s;
}

bool passwordResetWhenUiaDisabled(int state, int source) {
    return state == kStatePending || (state == kStatePassword && source != kSourceWin32);
}
