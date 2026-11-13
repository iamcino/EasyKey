/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include "../win32/EasyKey/EasyKey/HttpBody.h"

static int total = 0, failures = 0;
static void check(bool ok, const char* label) {
    ++total;
    if (!ok) {
        ++failures;
        std::printf("FAIL [%s]\n", label);
    }
}

//One scripted read: the value returned, the byte count reported, the bytes
//placed in the buffer. writeBytes=false leaves *bytesRead untouched, the
//way a real reader may behave on its end-of-stream call.
struct Step {
    BOOL result;
    DWORD bytes;
    const char* data;
    bool writeBytes;
    bool fillTail; //fill the whole buffer with 'X' before copying data
};

struct Script {
    Step steps[8];
    int count;
};

static Step makeStep(BOOL result, DWORD bytes, const char* data) {
    Step s;
    s.result = result;
    s.bytes = bytes;
    s.data = data;
    s.writeBytes = true;
    s.fillTail = false;
    return s;
}
static Step chunk(const char* text) {
    return makeStep(TRUE, (DWORD)std::strlen(text), text);
}
static Step eof() {
    return makeStep(TRUE, 0, nullptr);
}

static int scriptPos = 0;
static BOOL WINAPI scriptedRead(void* request, void* buffer, DWORD capacity, DWORD* bytesRead) {
    auto* script = static_cast<Script*>(request);
    if (scriptPos >= script->count) {
        *bytesRead = 0;
        return TRUE;
    }
    const Step& step = script->steps[scriptPos++];
    if (step.fillTail)
        std::memset(buffer, 'X', capacity);
    if (step.data && step.bytes > 0 && step.bytes <= capacity)
        std::memcpy(buffer, step.data, step.bytes);
    if (step.writeBytes)
        *bytesRead = step.bytes;
    return step.result;
}

static bool run(Script& script, size_t maxBytes, bool haveLength, DWORD contentLength, std::string& body) {
    scriptPos = 0;
    return readHttpBody(&script, &scriptedRead, maxBytes, haveLength, contentLength, body);
}

int main() {
    const size_t big = 1024 * 1024;
    std::string body;

    {   //1: one chunk, then end of stream
        Script s = { { chunk("hello"), eof() }, 2 };
        check(run(s, big, false, 0, body) && body == "hello", "single chunk");
    }
    {   //2: several short reads concatenate
        Script s = { { chunk("he"), chunk("llo"), chunk(" wor"), chunk("ld"), eof() }, 5 };
        check(run(s, big, false, 0, body) && body == "hello world", "short reads");
    }
    {   //3: the first read fails at once
        Script s = { { makeStep(FALSE, 0, nullptr), eof() }, 2 };
        check(!run(s, big, false, 0, body) && body.empty(), "immediate failure");
    }
    {   //4: the first read fails with a dirty byte count
        Script s = { { makeStep(FALSE, 7, nullptr), eof() }, 2 };
        check(!run(s, big, false, 0, body) && body.empty(), "immediate failure, dirty count");
    }
    {   //5: a read fails after a good chunk; the partial body is dropped
        Script s = { { chunk("part"), makeStep(FALSE, 0, nullptr), eof() }, 3 };
        check(!run(s, big, false, 0, body) && body.empty(), "failure mid-transfer");
    }
    {   //6: a later failing call reports dirty bytes; still a failure, body empty
        Script s = { { chunk("part"), makeStep(FALSE, 9, nullptr), eof() }, 3 };
        check(!run(s, big, false, 0, body) && body.empty(), "failure mid-transfer, dirty count");
    }
    {   //7: bytes past the reported count are never taken (sentinel tail)
        Step t = chunk("abc");
        t.fillTail = true;
        Script s = { { t, eof() }, 2 };
        check(run(s, big, false, 0, body) && body == "abc", "sentinel tail ignored");
    }
    {   //8: a body exactly at the cap succeeds
        Script s = { { chunk("12345678"), eof() }, 2 };
        check(run(s, 8, false, 0, body) && body == "12345678", "size == cap");
    }
    {   //9: one byte over the cap fails
        Script s = { { chunk("123456789"), eof() }, 2 };
        check(!run(s, 8, false, 0, body) && body.empty(), "size == cap + 1");
    }
    {   //10: Content-Length above the real size fails
        Script s = { { chunk("abc"), eof() }, 2 };
        check(!run(s, big, true, 5, body) && body.empty(), "length above actual");
    }
    {   //11: Content-Length below the real size fails
        Script s = { { chunk("abcde"), eof() }, 2 };
        check(!run(s, big, true, 3, body) && body.empty(), "length below actual");
    }
    {   //12: oversize fails even when Content-Length agrees with it
        Script s = { { chunk("123456789"), eof() }, 2 };
        check(!run(s, 8, true, 9, body) && body.empty(), "oversize with matching length");
    }
    {   //13: Content-Length: 0 with an empty body succeeds
        Script s = { { eof() }, 1 };
        check(run(s, big, true, 0, body) && body.empty(), "content-length 0");
    }
    {   //14: no length: a decoy length value is ignored
        Script s = { { chunk("abc"), eof() }, 2 };
        check(run(s, big, false, 999999, body) && body == "abc", "decoy length ignored");
    }
    {   //15: a pre-seeded body is replaced on success
        body = "SEED";
        Script s = { { chunk("ab"), eof() }, 2 };
        check(run(s, big, false, 0, body) && body == "ab", "pre-seeded replaced");
    }
    {   //16: a pre-seeded body is emptied on failure
        body = "SEED";
        Script s = { { makeStep(FALSE, 0, nullptr), eof() }, 2 };
        check(!run(s, big, false, 0, body) && body.empty(), "pre-seeded emptied");
    }
    {   //17: a byte count above the buffer capacity fails, nothing appended
        Script s = { { makeStep(TRUE, 16385, nullptr), eof() }, 2 };
        check(!run(s, big, false, 0, body) && body.empty(), "count above capacity");
    }
    {   //18: an empty 200 (immediate end of stream, no length) succeeds
        Script s = { { eof() }, 1 };
        check(run(s, big, false, 0, body) && body.empty(), "empty body");
    }
    {   //19: the end-of-stream call leaves the byte count untouched; the
        //helper's own zeroed count is what ends the loop
        Step silent = eof();
        silent.writeBytes = false;
        Script s = { { chunk("ab"), silent }, 2 };
        check(run(s, big, false, 0, body) && body == "ab", "eof leaves count untouched");
    }

    std::printf("HttpBodyTests: %d checks, %d failures\n", total, failures);
    return failures ? 1 : 0;
}
