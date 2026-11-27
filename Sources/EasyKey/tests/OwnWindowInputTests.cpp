/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only

The Shortcut list edits show raw Telex and VNI letters. The engine is not
the cause: the low-level hook is not called while that window is in front,
because this process is registered for raw keyboard input. These tests lock
the watchdog's current decision, which leaves that registration on.
-----------------------------------------------------------*/
#include <cstdio>
#include "../app-core/OwnWindowInput.h"

static int total = 0, failures = 0;
static void check(bool ok, const char* label) {
    ++total;
    if (!ok) {
        ++failures;
        std::printf("FAIL [%s]\n", label);
    }
}

int main() {
    check(rawWatchShouldRun(false), "another app in front: raw watch stays on");
    check(rawWatchShouldRun(true), "shortcut list in front: raw watch stays on");

    std::printf("%d/%d own-window input tests passed\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
