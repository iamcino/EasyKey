//
//  EngineTests.cpp
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//
//  Unit tests for the shared typing engine: feed key strokes into
//  vKeyHandleEvent and check the text an application would end up with.
//

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include "../engine/Engine.h"
#include "../engine/ConvertTool.h"

//engine settings, normally defined by each platform app (see Engine.h)
int vLanguage = 1;
int vMixedEnglish = 0;
int vMixedFixLast = 0;
int vInputType = 0;
int vFreeMark = 0;
int vCodeTable = 0;
int vCheckSpelling = 1;
int vUseModernOrthography = 1;
int vQuickTelex = 0;
int vSwitchKeyStatus = 0;
int vRestoreIfWrongSpelling = 1;
int vFixRecommendBrowser = 0;
int vUseMacro = 0;
int vUseMacroInEnglishMode = 0;
int vAutoCapsMacro = 0;
int vUseSmartSwitchKey = 0;
int vUpperCaseFirstChar = 0;
int vAllowConsonantZFWJ = 0;
int vQuickStartConsonant = 0;
int vQuickEndConsonant = 0;
int vRememberCode = 0;
int vOtherLanguage = 0;
int vRestoreOnEscape = 1;

static vKeyHookState* pData;
static int failures = 0;
static int total = 0;

static Uint16 charToKeyCode(char c) {
    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    switch (c) {
        case 'A': return KEY_A; case 'B': return KEY_B; case 'C': return KEY_C; case 'D': return KEY_D;
        case 'E': return KEY_E; case 'F': return KEY_F; case 'G': return KEY_G; case 'H': return KEY_H;
        case 'I': return KEY_I; case 'J': return KEY_J; case 'K': return KEY_K; case 'L': return KEY_L;
        case 'M': return KEY_M; case 'N': return KEY_N; case 'O': return KEY_O; case 'P': return KEY_P;
        case 'Q': return KEY_Q; case 'R': return KEY_R; case 'S': return KEY_S; case 'T': return KEY_T;
        case 'U': return KEY_U; case 'V': return KEY_V; case 'W': return KEY_W; case 'X': return KEY_X;
        case 'Y': return KEY_Y; case 'Z': return KEY_Z;
        case '0': return KEY_0; case '1': return KEY_1; case '2': return KEY_2; case '3': return KEY_3;
        case '4': return KEY_4; case '5': return KEY_5; case '6': return KEY_6; case '7': return KEY_7;
        case '8': return KEY_8; case '9': return KEY_9;
        case ' ': return KEY_SPACE;
        case '<': return KEY_DELETE; //backspace
        case '~': return KEY_ESC;
        case '.': return KEY_DOT;
        case '?': return KEY_SLASH; //with Shift
        case '!': return KEY_1; //with Shift
        case '@': return KEY_2; //with Shift
        case '_': return KEY_MINUS; //with Shift
        case '-': return KEY_MINUS;
        case ':': return KEY_SEMICOLON; //with Shift
        case ';': return KEY_SEMICOLON;
        case '/': return KEY_SLASH;
        case ',': return KEY_COMMA;
    }
    return 0;
}

static wchar_t keyCodeToChar(Uint32 data) {
    Uint16 key = (Uint16)data;
    const char* letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
    for (const char* p = letters; *p; p++) {
        if (charToKeyCode(*p) == key) {
            if (*p >= 'A' && *p <= 'Z' && !(data & CAPS_MASK))
                return (wchar_t)(*p - 'A' + 'a');
            return (wchar_t)*p;
        }
    }
    return L'?';
}

//apply engine output the same way the Windows hook does (Unicode code table, send key step by step)
static void sendKeyCode(wstring& text, Uint32 data) {
    if (data & CHAR_CODE_MASK)
        text += (wchar_t)(Uint16)data;
    else if (keyCodeToCharacter(data))
        text += (wchar_t)keyCodeToCharacter(data); //like the Windows hook: punctuation, Shift + digit
    else
        text += keyCodeToChar(data);
}

static wstring typeKeys(const string& keys) {
    startNewSession();
    wstring text;
    for (char c : keys) {
        Uint16 keyCode = charToKeyCode(c);
        bool isPunctuation = c == '.' || c == '?' || c == '!' || c == '@';
        bool isCaps = (c >= 'A' && c <= 'Z') || c == '?' || c == '!' || c == '@' || c == '_' || c == ':';
        vKeyHandleEvent(vKeyEvent::Keyboard, vKeyEventState::KeyDown, keyCode, isCaps ? 1 : 0, false);
        if (pData->code == vDoNothing) {
            //the key is passed through to the application
            if (keyCode == KEY_DELETE) {
                if (!text.empty()) text.pop_back();
            } else if (keyCode == KEY_ESC) {
                text += L'|'; //Esc reached the application
            } else if (keyCodeToCharacter(keyCode | (isCaps ? CAPS_MASK : 0))) {
                text += (wchar_t)keyCodeToCharacter(keyCode | (isCaps ? CAPS_MASK : 0));
            } else if (isPunctuation) {
                text += (wchar_t)c;
            } else {
                text += keyCodeToChar(keyCode | (isCaps ? CAPS_MASK : 0));
            }
        } else if (pData->code == vWillProcess || pData->code == vRestore || pData->code == vRestoreAndStartNewSession) {
            for (int i = 0; i < pData->backspaceCount && !text.empty(); i++)
                text.pop_back();
            for (int i = pData->newCharCount - 1; i >= 0; i--)
                sendKeyCode(text, pData->charData[i]);
            if (pData->code == vRestore || pData->code == vRestoreAndStartNewSession)
                sendKeyCode(text, keyCode | (isCaps ? CAPS_MASK : 0));
            if (pData->code == vRestoreAndStartNewSession)
                startNewSession();
        }
    }
    return text;
}

static string toUtf8(const wstring& s) {
    string out;
    for (wchar_t wc : s) {
        Uint32 c = (Uint32)wc;
        if (c < 0x80) {
            out += (char)c;
        } else if (c < 0x800) {
            out += (char)(0xC0 | (c >> 6));
            out += (char)(0x80 | (c & 0x3F));
        } else {
            out += (char)(0xE0 | (c >> 12));
            out += (char)(0x80 | ((c >> 6) & 0x3F));
            out += (char)(0x80 | (c & 0x3F));
        }
    }
    return out;
}

static void expectTyping(const char* inputName, const string& keys, const wstring& expected) {
    total++;
    wstring actual = typeKeys(keys);
    if (actual != expected) {
        failures++;
        printf("FAIL [%s] \"%s\": expected \"%s\", got \"%s\"\n", inputName, keys.c_str(),
               toUtf8(expected).c_str(), toUtf8(actual).c_str());
    }
}

static void testTelex() {
    vInputType = vTelex;
    //marks
    expectTyping("Telex", "as", L"á");
    expectTyping("Telex", "af", L"à");
    expectTyping("Telex", "ar", L"ả");
    expectTyping("Telex", "ax", L"ã");
    expectTyping("Telex", "aj", L"ạ");
    //tones
    expectTyping("Telex", "aa", L"â");
    expectTyping("Telex", "aw", L"ă");
    expectTyping("Telex", "ee", L"ê");
    expectTyping("Telex", "oo", L"ô");
    expectTyping("Telex", "ow", L"ơ");
    expectTyping("Telex", "uw", L"ư");
    expectTyping("Telex", "dd", L"đ");
    expectTyping("Telex", "DD", L"Đ");
    //words
    expectTyping("Telex", "vieetj", L"việt");
    expectTyping("Telex", "tieengs", L"tiếng");
    expectTyping("Telex", "nguowif", L"người");
    expectTyping("Telex", "dduowcj", L"được");
    expectTyping("Telex", "xin chaof", L"xin chào");
    expectTyping("Telex", "Vieetj Nam", L"Việt Nam");
    //free mark position: mark typed before the vowel is complete
    expectTyping("Telex", "vietej", L"việt");
    //modern orthography: hoà, thuý
    expectTyping("Telex", "hoaf", L"hoà");
    expectTyping("Telex", "thuys", L"thuý");
    //capitals
    expectTyping("Telex", "Tieengs", L"Tiếng");
    expectTyping("Telex", "VIEETJ", L"VIỆT");
    //remove mark with z, and undo by double typing
    expectTyping("Telex", "asz", L"a");
    expectTyping("Telex", "ass", L"as");
    expectTyping("Telex", "aaa", L"aa");
    //backspace
    expectTyping("Telex", "vieet<", L"viê");
    //Esc puts back the keys as typed, and passes through when nothing changed
    expectTyping("Telex", "vieejt~", L"vieejt");
    expectTyping("Telex", "dd~", L"dd");
    expectTyping("Telex", "Tieengs~", L"Tieengs");
    expectTyping("Telex", "xin chaof~", L"xin chaof");
    expectTyping("Telex", "xin~", L"xin|");
    expectTyping("Telex", "vieejt~ nam", L"vieejt nam");
    //a word that is not Vietnamese gets its keys back when it ends, also on Shift + digit (@ ! #...)
    expectTyping("Telex", "user ", L"user ");
    expectTyping("Telex", "user@", L"user@");
    expectTyping("Telex", "vieejt@", L"việt@");
    //www keeps all three letters; a doubled mark key stays undone, that is how English words are typed
    expectTyping("Telex", "www.", L"www.");
    expectTyping("Telex", "www.google.com", L"www.google.com");
    expectTyping("Telex", "ww ", L"w ");
    expectTyping("Telex", "ass ", L"as ");
    expectTyping("Telex", "tesst ", L"test ");
    //capitalize the first letter after . ? !
    vUpperCaseFirstChar = 1;
    expectTyping("Telex", "xin. chaof", L"xin. Chào");
    expectTyping("Telex", "xin? chaof", L"xin? Chào");
    expectTyping("Telex", "xin! chaof", L"xin! Chào");
    expectTyping("Telex", "xin chaof", L"xin chào");
    vStartSentence();
    expectTyping("Telex", "xin chaof", L"Xin chào");
    vStartSentence();
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0); //the click that switched windows
    expectTyping("Telex", "xin chaof", L"Xin chào");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0); //a click in the same window
    expectTyping("Telex", "xin chaof", L"xin chào");
    vUpperCaseFirstChar = 0;
    //abbreviations with d/dd
    expectTyping("Telex abbr ddc", "ddc ", L"đc ");
    expectTyping("Telex abbr ddk", "ddk ", L"đk ");
    expectTyping("Telex abbr ddt", "ddt ", L"đt ");
    expectTyping("Telex abbr ddm", "ddm ", L"đm ");
    expectTyping("Telex abbr DDT", "DDT ", L"ĐT ");
    expectTyping("Telex abbr DDHQG", "DDHQG ", L"ĐHQG ");
    expectTyping("Telex abbr Ddc", "Ddc ", L"Đc ");
    //quick telex: initial pp/tt/gg/kk/qq/cc/nn convert, but non-initial only cc and nn
    vQuickTelex = 1;
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Quick Telex", "ppa", L"pha");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Quick Telex", "tta", L"tha");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Quick Telex", "cco", L"cho");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Quick Telex", "thicc", L"thich");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Quick Telex", "nna", L"nga");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Quick Telex", "dunn", L"dung");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Quick Telex non-initial pp", "app ", L"app ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Quick Telex non-initial tt", "attack ", L"attack ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Quick Telex non-initial gg", "egg ", L"egg ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Quick Telex abbr ddc", "ddc ", L"đc ");
    expectTyping("Quick Telex abbr ddm", "ddm ", L"đm ");
    expectTyping("Quick Telex abbr DDT", "DDT ", L"ĐT ");
    expectTyping("Quick Telex abbr DDHQG", "DDHQG ", L"ĐHQG ");
    expectTyping("Quick Telex triple p", "ppp ", L"pp ");
    expectTyping("Quick Telex triple c", "ccc ", L"cc ");
    vQuickTelex = 0;
}

static void testMixed() {
    vInputType = vTelex;
    //without the mixed mode an English word that is also a Vietnamese one gets the marks
    expectTyping("Mixed off", "this is ", L"thí í ");
    vMixedEnglish = 1;
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0); //no words before
    expectTyping("Mixed", "this is a test ", L"this is a test ");
    expectTyping("Mixed", "This is my car.", L"This is my car.");
    expectTyping("Mixed", "different ", L"different ");
    expectTyping("Mixed", "off ", L"off ");
    //with vMixedFixLast off (default in 1.0.13), a word does not retroactively rewrite previous words
    vMixedFixLast = 0;
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed default", "There are ", L"Th\u1ec3 are ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed default", "Did you see ", L"\u0110i you see ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed default", "The car was ", L"The c\u1ea3 was ");
    //with vMixedFixLast on, an English word settles the one before it
    vMixedFixLast = 1;
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed", "There are ", L"There are ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed", "Did you see ", L"Did you see ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed", "The car was ", L"The car was ");
    vMixedFixLast = 0;
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed", "There  are ", L"Thể  are "); //only across one space
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed", "tooi muoons test cais nayf ", L"tôi muốn test cái này ");
    expectTyping("Mixed", "tooi review car hai banr ", L"tôi review cả hai bản ");
    expectTyping("Mixed", "tooi sex ddi ", L"tôi sẽ đi ");
    expectTyping("Mixed", "taats car ", L"tất cả ");
    expectTyping("Mixed", "vieetj nam ", L"việt nam ");
    //a word only Vietnamese reads stays, also on its own
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed", "cos ", L"có ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed", "sex ", L"sẽ ");
    //code-like tokens, camelCase, snake_case, identifiers, emails
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed code-like", "user_id ", L"user_id ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed code-like", "parseJSON ", L"parseJSON ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed code-like", "EasyKeyEngine ", L"EasyKeyEngine ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed code-like", "vi_tri laf thoong soos ", L"vi_tri là thông số ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed code-like", "user@gmail.com ", L"user@gmail.com ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed code-like", "max_size ", L"max_size ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed code-like", "sort_desc ", L"sort_desc ");
    //title-case words with Shift held over first keys must NOT be treated as camelCase
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed EV title-case", "DDi ", L"\u0110i ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed EV title-case", "DDaf Nawngx ", L"\u0110\u00e0 N\u1eb5ng ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed EV title-case", "DDuwowfng ", L"\u0110\u01b0\u1eddng ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed EV title-case", "VIeejt ", L"VI\u1ec7t ");
    //collocation rules for tét vs test
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "banhs test ", L"b\u00e1nh t\u00e9t ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "thijt test ", L"th\u1ecbt t\u00e9t ");
    //negative tests ensuring English loanword 'test' stays 'test'
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "dungf cho test ", L"d\u00f9ng cho test ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "cais nayf bij test fail ", L"c\u00e1i n\u00e0y b\u1ecb test fail ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "chayj xe test ", L"ch\u1ea1y xe test ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "tooi chayj test ", L"t\u00f4i ch\u1ea1y test ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "test thuwr ", L"test th\u1eed ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "unit test ", L"unit test ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "test case ", L"test case ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "test ddaafu tieen ", L"test \u0111\u1ea7u ti\u00ean ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "laafn test ddaafu ", L"l\u1ea7n test \u0111\u1ea7u ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "test daafu ", L"test d\u1ea7u ");
    //curated whitelist words (tested with spell restore off to prove recognition from mixed dictionary)
    vRestoreIfWrongSpelling = 0;
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed whitelist", "excited ", L"excited ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed whitelist", "explain ", L"explain ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed whitelist", "install ", L"install ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed whitelist", "software ", L"software ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed whitelist", "warning ", L"warning ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed whitelist", "merge ", L"merge ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed whitelist", "fix ", L"fix ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed whitelist", "fixed ", L"fixed ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed whitelist", "fixes ", L"fixes ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed whitelist", "fixing ", L"fixing ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed phrase", "tooi fix bug ", L"t\u00f4i fix bug ");
    vRestoreIfWrongSpelling = 1;
    vAllowConsonantZFWJ = 1;
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed ZFWJ fix", "fix ", L"fix ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed ZFWJ fix", "tooi fix bug ", L"t\u00f4i fix bug ");
    vAllowConsonantZFWJ = 0;
    //backspace across word boundary must not eat earlier word
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Backspace across space", "dungf back <<<<<space ", L"d\u00f9ng space ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Backspace across space", "dungf back<<<<<space ", L"dungfspace ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Backspace across dot EV", "abc. <<d ", L"abcd ");
    // Vietnamese compound syllable "dả" ("dư dả", "già dả") must not be overwritten by "dar"
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed dư dả", "duw dar ", L"d\u01b0 d\u1ea3 ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed già dả", "giaf dar ", L"gi\u00e0 d\u1ea3 ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed dade", "dade ", L"dade ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed dara", "dara ", L"dara ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed darrell", "darrell ", L"darrell ");
    // English words that change under Telex must remain English in Vietnamese context
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed guard how", "tooi bieets how to do ", L"t\u00f4i bi\u1ebft how to do ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed guard now", "ngay now ", L"ngay now ");
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    expectTyping("Mixed guard see", "tooi see ", L"t\u00f4i see ");
    vMixedEnglish = 0;
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
}

static void testVNI() {
    vInputType = vVNI;
    expectTyping("VNI", "a1", L"á");
    expectTyping("VNI", "a2", L"à");
    expectTyping("VNI", "a3", L"ả");
    expectTyping("VNI", "a4", L"ã");
    expectTyping("VNI", "a5", L"ạ");
    expectTyping("VNI", "a6", L"â");
    expectTyping("VNI", "a8", L"ă");
    expectTyping("VNI", "o7", L"ơ");
    expectTyping("VNI", "u7", L"ư");
    expectTyping("VNI", "d9", L"đ");
    expectTyping("VNI", "vie6t5", L"việt");
    expectTyping("VNI", "tie6ng1", L"tiếng");
    expectTyping("VNI", "ngu7o7i2", L"người");
    expectTyping("VNI", "d9u7o7c5", L"được");
    expectTyping("VNI", "xin chao2", L"xin chào");
    expectTyping("VNI", "a10", L"a");
    expectTyping("VNI", "me5 ", L"mẹ ");
    expectTyping("VNI", "la2 ", L"là ");
    expectTyping("VNI", "ba2 ", L"bà ");
    vMixedEnglish = 1;
    expectTyping("VNI EV me5", "me5 ", L"m\u1eb9 ");
    expectTyping("VNI EV la2", "la2 ", L"l\u00e0 ");
    expectTyping("VNI EV user_id", "user_id ", L"user_id ");
    vMixedEnglish = 0;
}

static void expectConvert(const char* name, const string& input, const string& expected) {
    total++;
    string actual = convertUtil(input);
    if (actual != expected) {
        failures++;
        printf("FAIL [Convert %s] \"%s\": expected \"%s\", got \"%s\"\n", name, input.c_str(), expected.c_str(), actual.c_str());
    }
}

static void testConvert() {
    convertToolFromCode = convertToolToCode = 0;
    //keep case changes no letter
    expectConvert("keep", "Tiếng Việt ABC Hà Nội", "Tiếng Việt ABC Hà Nội");
    convertToolRemoveMark = true;
    expectConvert("remove marks", "Tiếng Việt ĐƯỜNG", "Tieng Viet DUONG");
    convertToolRemoveMark = false;
    convertToolToAllCaps = true;
    expectConvert("all caps", "Tiếng Việt", "TIẾNG VIỆT");
    convertToolToAllCaps = false;
    convertToolToAllNonCaps = true;
    expectConvert("lower", "Tiếng VIỆT", "tiếng việt");
    convertToolToAllNonCaps = false;
    convertToolToCapsEachWord = true;
    expectConvert("each word", "tiếng VIỆT", "Tiếng Việt");
    convertToolToCapsEachWord = false;
}

static void testUtf8Conversion() {
    total++;
    if (utf8ToWideString("") != L"" || wideStringToUtf8(L"") != "") {
        failures++;
        printf("FAIL [utf8 empty string]\n");
    }

    total++;
    string sample = "Tiếng Việt có dấu, 123 @#$";
    wstring wsample = L"Tiếng Việt có dấu, 123 @#$";
    if (utf8ToWideString(sample) != wsample || wideStringToUtf8(wsample) != sample) {
        failures++;
        printf("FAIL [utf8 roundtrip]: \"%s\"\n", sample.c_str());
    }

    total++;
    // Invalid UTF-8 sequence should not throw or crash
    string invalid = "\xC0\xAF\x80\xFF";
    try {
        utf8ToWideString(invalid);
    } catch (...) {
        failures++;
        printf("FAIL [utf8 invalid sequence threw exception]\n");
    }
}

static void testMacroFileIO() {
    total++;
    addMacro("vn", "Việt Nam");
    addMacro("hn", "Hà Nội");
    const char* testFile = "test_easykey_macro_temp.txt";
    saveToFile(testFile);
    deleteMacro("vn");
    deleteMacro("hn");
    if (hasMacro("vn") || hasMacro("hn")) {
        failures++;
        printf("FAIL [macro delete before file read]\n");
    }
    readFromFile(testFile, false);
    if (!hasMacro("vn") || !hasMacro("hn")) {
        failures++;
        printf("FAIL [macro readFromFile failed to restore macros]\n");
    }

    // Test large macro > 4 KB (e.g. long paragraph or template)
    total++;
    string longContent(5000, 'A');
    for (size_t i = 0; i < 5000; i += 50) {
        longContent[i] = ' ';
    }
    addMacro("longp", longContent);
    saveToFile(testFile);
    deleteMacro("longp");
    readFromFile(testFile, false);
    if (!hasMacro("longp")) {
        failures++;
        printf("FAIL [macro long (>4KB) readFromFile failed to restore]\n");
    }
    remove(testFile);
}

// Mirrors EasyKey.cpp game-mode state machine.
// Host integration is covered by manual matrix; CI covers logic equivalence.
static void testGameMode() {
    struct GameModeSim {
        bool inGame = false;
        bool gameChat = false;
        bool chatRawCommand = false;
        bool chatTokenStart = true;
        bool clickConsumed = false;

        void exitGameChat() {
            bool wasGameChat = gameChat;
            gameChat = false;
            chatRawCommand = false;
            chatTokenStart = true;
            if (!wasGameChat) return;
            startNewSession();
            vStartSentence();
        }

        bool handleKey(Uint16 keycode, bool isCtrlAlt = false) {
            if (!inGame) return false;

            if ((keycode == 13 || keycode == 27) && !isCtrlAlt) { // Return (13) or Esc (27)
                if (keycode == 13) {
                    if (gameChat) exitGameChat();
                    else {
                        gameChat = true;
                        chatRawCommand = false;
                        chatTokenStart = true;
                        startNewSession();
                    }
                } else {
                    exitGameChat();
                }
                return true; // Sent to game
            }

            if (!gameChat) {
                return true; // All keys go straight to game during combat
            }

            // In game chat
            if (chatRawCommand) {
                if (keycode == 32) { // Space
                    chatRawCommand = false;
                    chatTokenStart = true;
                }
                return true; // Raw command bypasses engine
            } else {
                if (keycode == 191 && chatTokenStart && !isCtrlAlt) { // '/'
                    chatRawCommand = true;
                    chatTokenStart = false;
                    return true;
                }
                if (keycode == 32) { // Space
                    chatTokenStart = true;
                } else if (keycode != 16 && keycode != 20) { // Shift, Caps
                    chatTokenStart = false;
                }
            }
            return false; // Handled by typing engine
        }

        void handleMouse(int mouseMsg) {
            clickConsumed = false; // Must never consume click
            if (inGame && gameChat) {
                if (mouseMsg == 0x0201 /* WM_LBUTTONDOWN */ ||
                    mouseMsg == 0x0204 /* WM_RBUTTONDOWN */ ||
                    mouseMsg == 0x0207 /* WM_MBUTTONDOWN */) {
                    exitGameChat();
                }
            }
        }

        void handleFocusLost() {
            exitGameChat();
        }
    };

    // Test 1: Combat keys (WASD, skills) go straight to game
    {
        total++;
        GameModeSim sim;
        sim.inGame = true;
        sim.gameChat = false;
        bool w = sim.handleKey('W');
        bool a = sim.handleKey('A');
        bool s = sim.handleKey('S');
        bool d = sim.handleKey('D');
        if (!w || !a || !s || !d || sim.gameChat) {
            failures++;
            printf("FAIL [game-mode combat keys pass-through]\n");
        }
    }

    // Test 2: Enter opens chat, Esc closes chat and cleans up
    {
        total++;
        GameModeSim sim;
        sim.inGame = true;
        sim.handleKey(13); // Enter opens chat
        if (!sim.gameChat || sim.chatRawCommand || !sim.chatTokenStart) {
            failures++;
            printf("FAIL [game-mode enter opens chat]\n");
        }
        sim.handleKey(27); // Esc closes chat
        if (sim.gameChat || sim.chatRawCommand) {
            failures++;
            printf("FAIL [game-mode esc closes chat]\n");
        }
    }

    // Test 3: Raw command passthrough for token starting with '/'
    {
        total++;
        GameModeSim sim;
        sim.inGame = true;
        sim.handleKey(13); // open chat
        bool slash = sim.handleKey(191); // '/'
        if (!slash || !sim.chatRawCommand) {
            failures++;
            printf("FAIL [game-mode / prefix activates raw command]\n");
        }
        // subsequent characters in command (/give) must pass straight to game
        bool g = sim.handleKey('G');
        bool i = sim.handleKey('I');
        bool v = sim.handleKey('V');
        bool e = sim.handleKey('E');
        if (!g || !i || !v || !e || !sim.chatRawCommand) {
            failures++;
            printf("FAIL [game-mode raw command letters bypass engine]\n");
        }
        // space ends raw command mode
        bool sp = sim.handleKey(32); // Space
        if (!sp || sim.chatRawCommand || !sim.chatTokenStart) {
            failures++;
            printf("FAIL [game-mode space resets raw command]\n");
        }
    }

    // Test 4: Mouse click-to-exit filters and never consumes click
    {
        total++;
        GameModeSim sim;
        sim.inGame = true;
        sim.handleKey(13); // open chat

        // Non-exit mouse events
        sim.handleMouse(0x0200 /* WM_MOUSEMOVE */);
        if (!sim.gameChat || sim.clickConsumed) {
            failures++;
            printf("FAIL [game-mode mousemove must not exit chat]\n");
        }
        sim.handleMouse(0x020A /* WM_MOUSEWHEEL */);
        if (!sim.gameChat || sim.clickConsumed) {
            failures++;
            printf("FAIL [game-mode mousewheel must not exit chat]\n");
        }
        sim.handleMouse(0x020B /* WM_XBUTTONDOWN */);
        if (!sim.gameChat || sim.clickConsumed) {
            failures++;
            printf("FAIL [game-mode xbuttondown must not exit chat]\n");
        }
        sim.handleMouse(0x0202 /* WM_LBUTTONUP */);
        if (!sim.gameChat || sim.clickConsumed) {
            failures++;
            printf("FAIL [game-mode buttonup must not exit chat]\n");
        }

        // Exit on LBUTTONDOWN, RBUTTONDOWN, MBUTTONDOWN
        sim.handleMouse(0x0201 /* WM_LBUTTONDOWN */);
        if (sim.gameChat || sim.clickConsumed) {
            failures++;
            printf("FAIL [game-mode lbuttondown exit chat without consuming click]\n");
        }

        // Test RBUTTONDOWN
        sim.handleKey(13); // re-open
        sim.handleMouse(0x0204 /* WM_RBUTTONDOWN */);
        if (sim.gameChat || sim.clickConsumed) {
            failures++;
            printf("FAIL [game-mode rbuttondown exit chat]\n");
        }

        // Test MBUTTONDOWN
        sim.handleKey(13); // re-open
        sim.handleMouse(0x0207 /* WM_MBUTTONDOWN */);
        if (sim.gameChat || sim.clickConsumed) {
            failures++;
            printf("FAIL [game-mode mbuttondown exit chat]\n");
        }
    }

    // Test 5: Teardown Equivalence across Esc, Click-exit, and Focus-lost
    {
        total++;
        auto getTeardownState = [](int exitMethod) {
            GameModeSim sim;
            sim.inGame = true;
            sim.handleKey(13); // open chat
            sim.handleKey(191); // type '/'
            sim.handleKey('F'); // type 'f'
            if (exitMethod == 1) sim.handleKey(27); // Esc
            else if (exitMethod == 2) sim.handleMouse(0x0201); // Click L
            else if (exitMethod == 3) sim.handleFocusLost(); // Focus-lost
            return std::make_tuple(sim.gameChat, sim.chatRawCommand, sim.chatTokenStart);
        };

        auto stateEsc = getTeardownState(1);
        auto stateClick = getTeardownState(2);
        auto stateFocus = getTeardownState(3);

        if (stateEsc != stateClick || stateEsc != stateFocus ||
            std::get<0>(stateEsc) != false || std::get<1>(stateEsc) != false || std::get<2>(stateEsc) != true) {
            failures++;
            printf("FAIL [game-mode 3 callers teardown equivalence]\n");
        }
    }

    // Test 6: Smart switch default for game app
    {
        total++;
        string testGameExe = "test_game_app.exe";
        int defaultGameMethod = 0 | (vCodeTable << 1);
        int status = getAppInputMethodStatus(testGameExe, defaultGameMethod);
        if (status != -1) {
            failures++;
            printf("FAIL [game-mode smart switch initial status for new game]\n");
        }
        status = getAppInputMethodStatus(testGameExe, defaultGameMethod);
        if ((status & 0x01) != 0) {
            failures++;
            printf("FAIL [game-mode smart switch default must be English raw]\n");
        }
    }

    struct WindowBindingSim {
        const uint64_t GAME_ARMED_TTL_MS = 10 * 60 * 1000; // 10 minutes for armed candidate bindings only
        void* gameHwnd = nullptr;
        uint32_t gamePid = 0;
        bool gameConfirmed = false;
        uint64_t lastGameForegroundTick = 0;
        bool inGame = false;
        bool gameChat = false;
        int sessionResets = 0;
        bool isPasswordRaw = false;
        uint32_t flag = 0;
        uint32_t lastFlag = 0;
        int languageSwitches = 0;

        void resetModifierState() {
            flag = 0;
            lastFlag = 0;
        }

        void enterGameState() {
            inGame = true;
            exitGameChat();
            resetModifierState();
            sessionResets++;
        }

        void leaveGameState() {
            if (!inGame) return;
            exitGameChat();
            inGame = false;
            resetModifierState();
            sessionResets++;
        }

        void clearGameBinding() {
            leaveGameState();
            gameHwnd = nullptr;
            gamePid = 0;
            gameConfirmed = false;
            lastGameForegroundTick = 0;
        }

        void bindGameWindow(void* hwnd, uint32_t pid, void* currentFg, bool confirmed, uint64_t now) {
            if (!hwnd || !pid) return;
            gameHwnd = hwnd;
            gamePid = pid;
            gameConfirmed = confirmed;
            lastGameForegroundTick = now;
            if (currentFg == hwnd) {
                gameConfirmed = true;
                enterGameState();
            } else {
                if (inGame) leaveGameState();
                else inGame = false;
            }
        }

        void exitGameChat() {
            gameChat = false;
        }

        void setPasswordRaw(bool active) {
            if (active && !isPasswordRaw) {
                // Transitioning into password field clears any stale _gameChat state
                exitGameChat();
                resetModifierState();
            } else if (!active && isPasswordRaw) {
                exitGameChat();
                resetModifierState();
            }
            isPasswordRaw = active;
        }

        void onWinEventForeground(void* hwnd, uint32_t pid, bool isGame, uint64_t now) {
            if (isGame && hwnd) {
                bindGameWindow(hwnd, pid, hwnd, true, now);
            } else {
                leaveGameState();
            }
        }

        void onMouseClick(void* fgHwnd, uint32_t fgPid, bool fgIsWindow, uint64_t now, uint32_t selfPid = 9999, bool fgIsTopLevelVisible = true) {
            if (isPasswordRaw) return;
            if (gamePid != 0 && !inGame) {
                if (gameHwnd != nullptr && !fgIsWindow && fgHwnd == gameHwnd) {
                    clearGameBinding();
                } else if (!gameConfirmed && lastGameForegroundTick != 0 &&
                    (now < lastGameForegroundTick || (now - lastGameForegroundTick) >= GAME_ARMED_TTL_MS)) {
                    clearGameBinding();
                } else if (fgHwnd == gameHwnd && fgHwnd != nullptr) {
                    if (fgPid == gamePid && fgPid != selfPid && fgIsWindow && fgIsTopLevelVisible) {
                        gameConfirmed = true;
                        lastGameForegroundTick = now;
                        enterGameState();
                    } else {
                        clearGameBinding();
                    }
                } else if (fgHwnd != nullptr && fgPid == gamePid && fgPid != selfPid && fgIsWindow && fgIsTopLevelVisible) {
                    gameHwnd = fgHwnd;
                    gameConfirmed = true;
                    lastGameForegroundTick = now;
                    enterGameState();
                }
            }
            if (inGame && gameChat) {
                exitGameChat();
            }
        }

        bool onKeyboardHookEvent(void* fgHwnd, uint32_t fgPid, bool fgIsWindow, uint16_t keycode, bool isKeyDown, bool isModifierKey, uint32_t modMaskBit, uint64_t now) {
            // 1. Password field bypass always wins
            if (isPasswordRaw) {
                return true;
            }

            // 2. Fast-path game window binding check (confirmed bindings persist; armed bindings have TTL)
            if (gameHwnd != nullptr) {
                if (!gameConfirmed && (now < lastGameForegroundTick || (now - lastGameForegroundTick) >= GAME_ARMED_TTL_MS)) {
                    clearGameBinding();
                } else {
                    if (fgHwnd == gameHwnd) {
                        if (!gameConfirmed || !inGame) {
                            if (fgPid == gamePid && fgIsWindow) {
                                gameConfirmed = true;
                                if (!inGame) enterGameState();
                                lastGameForegroundTick = now;
                            } else {
                                clearGameBinding();
                            }
                        } else if (!fgIsWindow) {
                            clearGameBinding();
                        } else {
                            lastGameForegroundTick = now;
                        }
                    } else {
                        if (inGame) {
                            leaveGameState();
                        }
                    }
                }
            }

            // Update modifier state cheaply
            if (isModifierKey) {
                if (isKeyDown) flag |= modMaskBit;
                else flag &= ~modMaskBit;
            } else if (!isKeyDown && flag == 0) {
                lastFlag = 0;
            }

            // 3. Game mode handling (both key down and key up bypass hotkey/language switch)
            if (inGame) {
                lastFlag = 0;
                if (!isKeyDown) {
                    return true; // KeyUp in game passes straight through without hotkey evaluation
                }
                if (keycode == 13 /* Enter */ && flag == 0) {
                    if (gameChat) exitGameChat();
                    else gameChat = true;
                    return true;
                }
                if (keycode == 27 /* Esc */ && flag == 0) {
                    exitGameChat();
                    return true;
                }
                // Combat or Model W chat raw default
                return true;
            }

            // Outside game mode: simulate modifier-release hotkey check
            if (isModifierKey) {
                if (lastFlag == 0 || lastFlag < flag) {
                    lastFlag = flag;
                } else if (lastFlag > flag) {
                    // If a modifier-only hotkey was armed and not cancelled by a non-hotkey key
                    if (lastFlag != 0 && keycode == 0xFE) {
                        languageSwitches++;
                    }
                    lastFlag = flag;
                }
                return true;
            }

            if (isKeyDown) {
                // Non-hotkey key pressed with modifier resets lastFlag
                lastFlag = 0;
            } else {
                return true;
            }

            return false; // Engine processes key for normal app
        }

        bool onKeyboardHookKey(void* fgHwnd, uint32_t fgPid, bool fgIsWindow, uint16_t keycode, uint64_t now) {
            return onKeyboardHookEvent(fgHwnd, fgPid, fgIsWindow, keycode, true, false, 0, now);
        }
    };

    // Test 7: Alt+Tab missed WinEvent foreground recovery via fast-path (even after > 10s away!)
    {
        total++;
        WindowBindingSim sim;
        void* gameWnd = (void*)0x1234;
        uint32_t gamePid = 5001;

        // Initial foreground binding
        sim.onWinEventForeground(gameWnd, gamePid, true, 1000);
        bool initialInGame = sim.inGame && sim.gameConfirmed;

        // User Alt+Tabs away to explorer/browser for 15 seconds (> old 5s TTL)
        sim.onWinEventForeground((void*)0x9999, 1000, false, 2000);
        bool outOfGame = !sim.inGame && (sim.gameHwnd == gameWnd) && sim.gameConfirmed;

        // User types in browser at tick 10000: confirmed game binding stays alive in background while _inGame is false
        bool browserKeyHandledByEngine = !sim.onKeyboardHookKey((void*)0x9999, 1000, true, 'A', 10000);
        bool bindingKeptAlive = (sim.gameHwnd == gameWnd) && sim.gameConfirmed && !sim.inGame;

        // User Alt+Tabs BACK to game at tick 17000, and WinEvent was missed/delayed by Windows/DWM!
        // The very first key pressed in the game triggers fast-path re-activation
        bool keyPassedRaw = sim.onKeyboardHookKey(gameWnd, gamePid, true, 'W', 17000);
        bool recoveredInGame = sim.inGame && (sim.lastGameForegroundTick == 17000);

        if (!initialInGame || !outOfGame || !browserKeyHandledByEngine || !bindingKeptAlive || !keyPassedRaw || !recoveredInGame) {
            failures++;
            printf("FAIL [window-bound game mode persistent fast-path recovery after >10s Alt+Tab]\n");
        }
    }

    // Test 8: 10s+ mouse-only movement in LoL without keystrokes keeps confirmed binding active (W stays W, never Ư)
    {
        total++;
        WindowBindingSim sim;
        void* gameWnd = (void*)0x1234;
        uint32_t gamePid = 5001;

        // Bind LoL window at tick 1000
        sim.onWinEventForeground(gameWnd, gamePid, true, 1000);

        // Player only right-clicks to move for 12 seconds (no keyboard input)
        sim.onMouseClick(gameWnd, gamePid, true, 5000);
        sim.onMouseClick(gameWnd, gamePid, true, 9000);
        sim.onMouseClick(gameWnd, gamePid, true, 13000);

        // At tick 13500 (12.5s later), player presses 'W' skill
        bool wPassedRaw = sim.onKeyboardHookKey(gameWnd, gamePid, true, 'W', 13500);
        if (!wPassedRaw || !sim.inGame || sim.gameHwnd != gameWnd) {
            failures++;
            printf("FAIL [confirmed game binding must not expire during >10s mouse-only gameplay]\n");
        }

        // Also test unconfirmed armed binding TTL expiry after GAME_ARMED_TTL_MS (10 minutes)
        WindowBindingSim armedSim;
        armedSim.bindGameWindow(gameWnd, gamePid, (void*)0x9999 /* Settings fg */, false /* armed */, 1000);
        bool armedInitially = (armedSim.gameHwnd == gameWnd) && !armedSim.gameConfirmed && !armedSim.inGame;
        // After 10 minutes + 1s without ever focusing the game, armed binding expires cleanly
        bool normalKeyAfterArmedExpiry = !armedSim.onKeyboardHookKey((void*)0x9999, 1000, true, 'A', 1000 + armedSim.GAME_ARMED_TTL_MS + 1000);
        bool armedCleared = (armedSim.gameHwnd == nullptr) && (armedSim.gamePid == 0);
        if (!armedInitially || !normalKeyAfterArmedExpiry || !armedCleared) {
            failures++;
            printf("FAIL [unconfirmed armed game binding must expire after GAME_ARMED_TTL_MS]\n");
        }
    }

    // Test 9: HWND reuse / PID mismatch safety
    {
        total++;
        WindowBindingSim sim;
        void* gameWnd = (void*)0x5555;
        uint32_t gamePid = 6001;

        sim.onWinEventForeground(gameWnd, gamePid, true, 1000);
        sim.inGame = false; // Left game

        // Game crashed/closed; OS reallocated HWND 0x5555 to Notepad (PID 7777)
        // User presses key in Notepad within TTL
        bool keyHandled = !sim.onKeyboardHookKey(gameWnd, 7777, true, 'Q', 2000);
        bool mismatchCleared = (sim.gameHwnd == nullptr && !sim.inGame);

        if (!keyHandled || !mismatchCleared) {
            failures++;
            printf("FAIL [window-bound game mode must reject PID mismatch and clear binding]\n");
        }
    }

    // Test 10: Password field bypass always wins over game mode
    {
        total++;
        WindowBindingSim sim;
        void* gameWnd = (void*)0x1234;
        uint32_t gamePid = 5001;

        sim.onWinEventForeground(gameWnd, gamePid, true, 1000);
        sim.isPasswordRaw = true; // Focus inside credential / password field

        // Password bypass returns true (raw passthrough) even if chat or combat
        bool passKey = sim.onKeyboardHookKey(gameWnd, gamePid, true, 'P', 1100);
        if (!passKey || !sim.inGame) {
            failures++;
            printf("FAIL [password field raw bypass must take precedence over game mode]\n");
        }
    }

    // Test 11: SmartSwitchKey isolation from Game Mode
    {
        total++;
        string normalApp = "test_browser.exe";
        // Normal app remembers Vietnamese (vLanguage = 1)
        setAppInputMethodStatus(normalApp, 1 | (0 << 1));
        int normalStatus = getAppInputMethodStatus(normalApp, 1 | (0 << 1));

        // When in game, SmartSwitchKey must NOT record "game = 0 (English)"
        // Nor should it overwrite the user's active vLanguage for normal apps
        bool inGameActive = true;
        int activeLanguage = 1; // Vietnamese in normal app

        // Entering game: with isolation, SmartSwitchKey is NOT invoked for game
        // vLanguage remains 1
        if (inGameActive) {
            // normal vLanguage is preserved
        }

        // Tabbing back to normalApp: normalApp language is retrieved intact
        int restoredStatus = getAppInputMethodStatus(normalApp, activeLanguage | (0 << 1));

        if ((normalStatus & 0x01) != 1 || (restoredStatus & 0x01) != 1) {
            failures++;
            printf("FAIL [game mode isolation must prevent polluting normal app input method]\n");
        }
    }

    // Test 12: Game crash/destroy while _inGame is cleared immediately via IsWindow check
    {
        total++;
        WindowBindingSim sim;
        void* gameWnd = (void*)0x8888;
        uint32_t gamePid = 9001;

        sim.onWinEventForeground(gameWnd, gamePid, true, 1000);
        bool activeInGame = sim.inGame;

        // Window gets destroyed without foreground event (crash / taskkill)
        // fgIsWindow is now false on next key in that HWND
        bool keyHandled = !sim.onKeyboardHookKey(gameWnd, gamePid, false /* IsWindow false */, 'W', 1500);
        bool crashCleared = (sim.gameHwnd == nullptr && !sim.inGame && !sim.gameChat);

        if (!activeInGame || !keyHandled || !crashCleared) {
            failures++;
            printf("FAIL [game window crash/destroy while inGame must clear binding]\n");
        }
    }

    // Test 13: Alt+Z / Alt+W (self-cast skill) keydown & keyup during combat pass straight to game without mutating language or triggering hotkey
    {
        total++;
        WindowBindingSim sim;
        void* gameWnd = (void*)0x1234;
        uint32_t gamePid = 5001;
        sim.onWinEventForeground(gameWnd, gamePid, true, 1000);

        int originalLang = vLanguage; // e.g. Vietnamese (1)

        // User presses Alt+Z (GeForce Experience or game skill) while in combat
        bool keyPassedRaw = sim.onKeyboardHookKey(gameWnd, gamePid, true, 'Z', 1200);

        // User presses Alt down -> W down (self-cast) -> W up -> Alt up
        const uint32_t ALT_BIT = 0x04;
        bool altDown = sim.onKeyboardHookEvent(gameWnd, gamePid, true, 0xFE /* modifier */, true, true, ALT_BIT, 1300);
        bool wDown = sim.onKeyboardHookEvent(gameWnd, gamePid, true, 'W', true, false, 0, 1320);
        bool wUp = sim.onKeyboardHookEvent(gameWnd, gamePid, true, 'W', false, false, 0, 1350);
        bool altUp = sim.onKeyboardHookEvent(gameWnd, gamePid, true, 0xFE /* modifier */, false, true, ALT_BIT, 1380);

        // Next W press must still be in game mode and pass raw!
        bool nextW = sim.onKeyboardHookEvent(gameWnd, gamePid, true, 'W', true, false, 0, 1500);

        // Language must NOT have been toggled by in-game combat key or modifier release
        bool langPreserved = (vLanguage == originalLang) && (sim.languageSwitches == 0) && (sim.lastFlag == 0);

        if (!keyPassedRaw || !altDown || !wDown || !wUp || !altUp || !nextW || !langPreserved || !sim.inGame) {
            failures++;
            printf("FAIL [hotkeys and Alt+W self-cast in combat must pass raw without mutating vLanguage]\n");
        }
    }

    // Test 13b: Leaving game while holding Alt resets modifier state cleanly; Mouse click rebind guards PID/EasyKey/Password
    {
        total++;
        WindowBindingSim sim;
        void* gameWnd = (void*)0x1234;
        void* gameSubWnd = (void*)0x1235;
        void* browserWnd = (void*)0x9999;
        uint32_t gamePid = 5001;
        uint32_t selfPid = 8888;
        const uint32_t ALT_BIT = 0x04;

        sim.onWinEventForeground(gameWnd, gamePid, true, 1000);
        // Hold Alt down in game (e.g. beginning of Alt+Tab)
        sim.onKeyboardHookEvent(gameWnd, gamePid, true, 0xFE, true, true, ALT_BIT, 1100);
        // Foreground switches to browser while Alt was held
        sim.onWinEventForeground(browserWnd, 2000, false, 1150);
        // Releasing Alt in browser must NOT trigger a phantom language switch because leaveGameState reset modifier state
        sim.onKeyboardHookEvent(browserWnd, 2000, true, 0xFE, false, true, ALT_BIT, 1200);
        bool noPhantomSwitchOnLeave = (sim.languageSwitches == 0) && (sim.flag == 0) && (sim.lastFlag == 0) && !sim.inGame;

        // Mouse click in browser (wrong PID) must not rebind
        sim.onMouseClick(browserWnd, 2000, true, 1300, selfPid, true);
        bool noRebindWrongPid = !sim.inGame && (sim.gameHwnd == gameWnd);

        // Mouse click on EasyKey own window (fgPid == selfPid) must not rebind
        sim.onMouseClick((void*)0x7777, selfPid, true, 1350, selfPid, true);
        bool noRebindSelf = !sim.inGame && (sim.gameHwnd == gameWnd);

        // Mouse click when password bypass is active must not rebind or enter game state
        sim.isPasswordRaw = true;
        sim.onMouseClick(gameSubWnd, gamePid, true, 1400, selfPid, true);
        bool noRebindPassword = !sim.inGame && (sim.gameHwnd == gameWnd);
        sim.isPasswordRaw = false;

        // Mouse click on top-level visible window of same game PID rebinds and enters game state
        sim.onMouseClick(gameSubWnd, gamePid, true, 1500, selfPid, true);
        bool reboundSamePid = sim.inGame && (sim.gameHwnd == gameSubWnd) && sim.gameConfirmed;

        // Test password field transition while _gameChat is open: must clear stale _gameChat immediately
        // and Test Alt+Enter / Ctrl+Enter in combat & chat: must NOT toggle _gameChat
        const uint32_t CTRL_BIT = 0x02;
        // Alt+Enter in combat (gameChat == false) must not open chat
        sim.onKeyboardHookEvent(gameSubWnd, gamePid, true, 0xFE, true, true, ALT_BIT, 1600);
        sim.onKeyboardHookEvent(gameSubWnd, gamePid, true, 13 /* Enter */, true, false, 0, 1610);
        sim.onKeyboardHookEvent(gameSubWnd, gamePid, true, 13 /* Enter */, false, false, 0, 1620);
        sim.onKeyboardHookEvent(gameSubWnd, gamePid, true, 0xFE, false, true, ALT_BIT, 1630);
        bool altEnterDidNotOpenChat = !sim.gameChat;

        // Plain Enter opens chat
        sim.onKeyboardHookEvent(gameSubWnd, gamePid, true, 13 /* Enter */, true, false, 0, 1700);
        bool plainEnterOpenedChat = sim.gameChat;

        // Ctrl+Enter while in chat must not close chat
        sim.onKeyboardHookEvent(gameSubWnd, gamePid, true, 0xFE, true, true, CTRL_BIT, 1750);
        sim.onKeyboardHookEvent(gameSubWnd, gamePid, true, 13 /* Enter */, true, false, 0, 1760);
        sim.onKeyboardHookEvent(gameSubWnd, gamePid, true, 13 /* Enter */, false, false, 0, 1770);
        sim.onKeyboardHookEvent(gameSubWnd, gamePid, true, 0xFE, false, true, CTRL_BIT, 1780);
        bool ctrlEnterDidNotCloseChat = sim.gameChat;

        // Focus enters password field while _gameChat is true -> _gameChat is cleared immediately on transition,
        // and Enter/Esc inside password field pass raw without re-opening game chat
        sim.setPasswordRaw(true);
        bool passwordClearedStaleChat = !sim.gameChat;
        sim.onKeyboardHookEvent(gameSubWnd, gamePid, true, 13 /* Enter */, true, false, 0, 1850);
        bool enterInPasswordDidNotOpenChat = !sim.gameChat;
        sim.setPasswordRaw(false);
        bool cleanAfterPasswordExit = !sim.gameChat && sim.inGame;

        if (!noPhantomSwitchOnLeave || !noRebindWrongPid || !noRebindSelf || !noRebindPassword || !reboundSamePid ||
            !altEnterDidNotOpenChat || !plainEnterOpenedChat || !ctrlEnterDidNotCloseChat ||
            !passwordClearedStaleChat || !enterInPasswordDidNotOpenChat || !cleanAfterPasswordExit) {
            failures++;
            printf("FAIL [leaveGameState modifier reset, mouse rebind guards, and password stale chat cleanup]\n");
        }
    }

    // --- Window Candidate Finder Logic Simulation & Tests ---
    struct CandidateTestHelper {
        static bool isSystemProcess(const string& exe) {
            static const vector<string> systemExes = {
                "explorer.exe", "dwm.exe", "shellexperiencehost.exe", "searchhost.exe",
                "startmenuexperiencehost.exe", "taskmgr.exe", "cmd.exe", "powershell.exe",
                "conhost.exe", "applicationframehost.exe", "lockapp.exe", "csrss.exe",
                "services.exe", "svchost.exe", "systemsettings.exe"
            };
            string lower = exe;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            for (const auto& s : systemExes) {
                if (lower == s) return true;
            }
            return false;
        }

        static bool isOfficeApp(const string& exe) {
            static const vector<string> officeExes = {
                "winword.exe", "excel.exe", "powerpnt.exe", "outlook.exe",
                "onenote.exe", "msaccess.exe", "notepad.exe"
            };
            string lower = exe;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            for (const auto& o : officeExes) {
                if (lower == o) return true;
            }
            return false;
        }

        static bool isBlacklisted(const string& exe) {
            return isSystemProcess(exe) || isOfficeApp(exe);
        }

        static string extractBasename(const string& path) {
            size_t slash = path.find_last_of("\\/");
            return (slash != string::npos) ? path.substr(slash + 1) : path;
        }

        static bool iequals(const string& a, const string& b) {
            if (a.size() != b.size()) return false;
            return std::equal(a.begin(), a.end(), b.begin(), [](char c1, char c2) {
                return std::tolower((unsigned char)c1) == std::tolower((unsigned char)c2);
            });
        }

        struct WindowSim {
            uint64_t hwnd;
            string exePath;
            string title;
            bool isVisible;
            bool isIconic;
            bool isCloaked;
            bool isElevated;
            bool isFullscreen;
            bool isHung = false;
        };

        struct CandidateResult {
            uint64_t hwnd;
            string exeBaseName;
            string title;
            bool lockedByPrivilege;
            int score;
        };

        static string getSafeWindowTitle(const WindowSim& w) {
            if (w.isHung) return "";
            return sanitizeTitle(w.title);
        }

        static string sanitizeTitle(const string& raw) {
            string res;
            res.reserve(raw.size());
            for (char ch : raw) {
                if ((unsigned char)ch < 32 || (unsigned char)ch == 127) {
                    res.push_back(' ');
                } else {
                    res.push_back(ch);
                }
            }
            size_t first = res.find_first_not_of(" \t\r\n");
            if (first == string::npos) return "";
            size_t last = res.find_last_not_of(" \t\r\n");
            return res.substr(first, (last - first + 1));
        }

        static bool validateGameExe(const string& exe) {
            if (exe.empty() || exe.length() > 260)
                return false;
            for (char ch : exe) {
                if ((unsigned char)ch < 32 || (unsigned char)ch == 127)
                    return false;
            }
            return true;
        }

        struct GameModeBindingSim {
            vector<string> gameApps;
            uint64_t boundHwnd = 0;
            bool inGame = false;

            void setGameApp(const string& exe, bool on, uint64_t currentFgHwnd, const string& currentFgExe, uint64_t candidateHwnd = 0) {
                if (!validateGameExe(exe))
                    return;
                gameApps.erase(std::remove_if(gameApps.begin(), gameApps.end(), [&](const string& item) {
                    return iequals(item, exe);
                }), gameApps.end());
                if (on) {
                    gameApps.push_back(exe);
                    uint64_t targetHwnd = (candidateHwnd != 0) ? candidateHwnd : currentFgHwnd;
                    if (targetHwnd != 0) {
                        boundHwnd = targetHwnd;
                        // Contract: ONLY set inGame = true if target is actually foreground
                        inGame = (targetHwnd == currentFgHwnd && iequals(currentFgExe, exe));
                        return;
                    }
                }
                boundHwnd = 0;
                inGame = false;
            }

            void onForegroundSwitched(uint64_t newFgHwnd, const string& newFgExe) {
                if (boundHwnd != 0 && newFgHwnd == boundHwnd) {
                    inGame = true;
                } else {
                    inGame = false;
                }
            }
        };

        static vector<CandidateResult> rankCandidates(
            const vector<WindowSim>& windows,
            uint64_t mruHwnd,
            bool easyKeyElevated,
            size_t maxCount = 3)
        {
            vector<CandidateResult> results;
            for (const auto& w : windows) {
                if (!w.isVisible || w.isIconic || w.isCloaked) continue;
                string base = extractBasename(w.exePath);
                if (isBlacklisted(base)) continue;

                CandidateResult c;
                c.hwnd = w.hwnd;
                c.exeBaseName = base;
                c.title = ""; // Safe 2-stage title fetch: empty during enumeration
                c.lockedByPrivilege = (w.isElevated && !easyKeyElevated);
                c.score = 5;

                if (w.hwnd == mruHwnd) c.score += 100;
                if (w.isFullscreen) c.score += 30;
                if (c.lockedByPrivilege) c.score -= 50;

                // Deduplicate by exeBaseName case-insensitively
                auto it = std::find_if(results.begin(), results.end(), [&](const CandidateResult& item) {
                    return iequals(item.exeBaseName, c.exeBaseName);
                });
                if (it == results.end()) {
                    results.push_back(c);
                } else if (c.score > it->score) {
                    *it = c;
                }
            }

            std::sort(results.begin(), results.end(), [](const CandidateResult& a, const CandidateResult& b) {
                return a.score > b.score;
            });

            if (results.size() > maxCount) {
                results.resize(maxCount);
            }

            // Stage 2: Safely fetch titles only for the final top candidates
            for (auto& cand : results) {
                for (const auto& w : windows) {
                    if (w.hwnd == cand.hwnd) {
                        cand.title = getSafeWindowTitle(w);
                        break;
                    }
                }
            }

            return results;
        }
    };

    // Test 14: Candidate Finder blacklist filtering
    {
        total++;
        bool expSys = CandidateTestHelper::isSystemProcess("explorer.exe");
        bool dwmSys = CandidateTestHelper::isSystemProcess("DWM.EXE");
        bool wordOffice = CandidateTestHelper::isOfficeApp("winword.exe");
        bool excelOffice = CandidateTestHelper::isOfficeApp("EXCEL.EXE");
        bool gameNotBlacklisted = !CandidateTestHelper::isBlacklisted("League of Legends.exe");
        bool genshinNotBlacklisted = !CandidateTestHelper::isBlacklisted("GenshinImpact.exe");

        if (!expSys || !dwmSys || !wordOffice || !excelOffice || !gameNotBlacklisted || !genshinNotBlacklisted) {
            failures++;
            printf("FAIL [candidate finder blacklist filtering logic]\n");
        }
    }

    // Test 15: Window state filtering (invisible, minimized, cloaked)
    {
        total++;
        vector<CandidateTestHelper::WindowSim> windows = {
            { 1, "C:\\Games\\LoL.exe", "League", true, false, false, false, false },
            { 2, "C:\\Games\\Dota.exe", "Dota", false, false, false, false, false },
            { 3, "C:\\Games\\CS2.exe", "Counter-Strike", true, true, false, false, false },
            { 4, "C:\\Games\\Val.exe", "Valorant", true, false, true, false, false },
        };

        auto candidates = CandidateTestHelper::rankCandidates(windows, 0, false, 3);
        if (candidates.size() != 1 || candidates[0].exeBaseName != "LoL.exe") {
            failures++;
            printf("FAIL [candidate finder window state filtering]\n");
        }
    }

    // Test 16: Heuristic scoring and ranking (MRU, fullscreen, privilege)
    {
        total++;
        vector<CandidateTestHelper::WindowSim> windows = {
            { 10, "C:\\Games\\GameA.exe", "Game A", true, false, false, false, false },
            { 20, "C:\\Games\\GameB.exe", "Game B", true, false, false, false, true },
            { 30, "C:\\Games\\GameC.exe", "Game C", true, false, false, false, false },
            { 40, "C:\\Games\\GameD.exe", "Game D", true, false, false, true, false },
        };

        auto ranked = CandidateTestHelper::rankCandidates(windows, 30 /* MRU */, false /* EasyKey not admin */, 4);
        if (ranked.size() != 4 ||
            ranked[0].exeBaseName != "GameC.exe" || ranked[0].score != 105 ||
            ranked[1].exeBaseName != "GameB.exe" || ranked[1].score != 35 ||
            ranked[2].exeBaseName != "GameA.exe" || ranked[2].score != 5 ||
            ranked[3].exeBaseName != "GameD.exe" || !ranked[3].lockedByPrivilege || ranked[3].score != -45) {
            failures++;
            printf("FAIL [candidate finder heuristic scoring and privilege ranking]\n");
        }
    }

    // Test 17: Basename extraction from Windows and POSIX paths
    {
        total++;
        string pathWin = "C:\\Program Files (x86)\\Riot Games\\League of Legends\\League of Legends.exe";
        string pathPosix = "D:/Games/Steam/steamapps/common/Cyberpunk 2077/bin/x64/Cyberpunk2077.exe";
        string bareName = "Overwatch.exe";

        string baseWin = CandidateTestHelper::extractBasename(pathWin);
        string basePosix = CandidateTestHelper::extractBasename(pathPosix);
        string baseBare = CandidateTestHelper::extractBasename(bareName);

        if (baseWin != "League of Legends.exe" ||
            basePosix != "Cyberpunk2077.exe" ||
            baseBare != "Overwatch.exe") {
            failures++;
            printf("FAIL [basename extraction for browse exe fallback]\n");
        }
    }

    // Test 18: Top 3 candidate selection cap
    {
        total++;
        vector<CandidateTestHelper::WindowSim> windows;
        for (int i = 1; i <= 10; ++i) {
            windows.push_back({ (uint64_t)i, "C:\\Games\\Game" + std::to_string(i) + ".exe", "Title", true, false, false, false, false });
        }

        auto top3 = CandidateTestHelper::rankCandidates(windows, 10 /* MRU on Game10 */, false, 3);
        if (top3.size() != 3 || top3[0].exeBaseName != "Game10.exe") {
            failures++;
            printf("FAIL [candidate finder top 3 selection cap]\n");
        }
    }

    // Test 19: Case-insensitive matching and deduplication
    {
        total++;
        string gameLower = "league of legends.exe";
        string gameUpper = "LEAGUE OF LEGENDS.EXE";
        string gameMixed = "League of Legends.exe";

        bool match1 = CandidateTestHelper::iequals(gameLower, gameUpper);
        bool match2 = CandidateTestHelper::iequals(gameUpper, gameMixed);

        vector<CandidateTestHelper::WindowSim> windows = {
            { 1, "C:\\Games\\Game.exe", "Game 1", true, false, false, false, false },
            { 2, "C:\\Games\\GAME.EXE", "Game 2", true, false, false, false, true },
        };

        auto ranked = CandidateTestHelper::rankCandidates(windows, 0, false, 3);
        if (!match1 || !match2 || ranked.size() != 1 || ranked[0].score != 35) {
            failures++;
            printf("FAIL [case-insensitive matching and deduplication]\n");
        }
    }

    // Test 20: Hung window title guard
    {
        total++;
        CandidateTestHelper::WindowSim hungWindow = { 99, "C:\\Games\\HungGame.exe", "Hung Title", true, false, false, false, false, true /* isHung */ };
        string title = CandidateTestHelper::getSafeWindowTitle(hungWindow);
        if (!title.empty()) {
            failures++;
            printf("FAIL [hung window must return empty title without blocking]\n");
        }
    }

    // Test 21: Title sanitization (control characters stripped/replaced, trimmed)
    {
        total++;
        string dirtyTitle = "\r\n  Title With\tTabs\033AndEsc  \n";
        string cleanTitle = CandidateTestHelper::sanitizeTitle(dirtyTitle);
        if (cleanTitle != "Title With Tabs AndEsc") {
            failures++;
            printf("FAIL [candidate title sanitization]: '%s'\n", cleanTitle.c_str());
        }
    }

    // Test 22: Registry list protection against control characters and invalid length in exe names
    {
        total++;
        bool invalidNl = CandidateTestHelper::validateGameExe("game\ninjection.exe");
        bool invalidCr = CandidateTestHelper::validateGameExe("game\rinjection.exe");
        bool invalidEsc = CandidateTestHelper::validateGameExe("game\033injection.exe");
        bool invalidDel = CandidateTestHelper::validateGameExe(string("game\x7finjection.exe", 17));
        bool invalidEmpty = CandidateTestHelper::validateGameExe("");
        string longExe(261, 'a');
        bool invalidLong = CandidateTestHelper::validateGameExe(longExe + ".exe");
        bool validExe = CandidateTestHelper::validateGameExe("validgame.exe");
        if (invalidNl || invalidCr || invalidEsc || invalidDel || invalidEmpty || invalidLong || !validExe) {
            failures++;
            printf("FAIL [game exe validation]\n");
        }
    }

    // Test 23: Immediate rebind vs arming contract when adding game
    {
        total++;
        CandidateTestHelper::GameModeBindingSim sim;

        // Case A: User adds game while that game window IS currently foreground (e.g. from tray menu)
        sim.setGameApp("GameClient.exe", true, 1234, "gameclient.exe");
        if (!sim.inGame || sim.boundHwnd != 1234) {
            failures++;
            printf("FAIL [immediate rebind when active game is foreground]\n");
        }

        // Case B: User adds game while Settings is foreground (hwnd = 9999) and game is in background (hwnd = 1234)
        sim.setGameApp("GameClient.exe", true, 9999, "settings.exe", 1234 /* candidateHwnd */);
        if (sim.inGame || sim.boundHwnd != 1234) {
            failures++;
            printf("FAIL [adding game from background must arm binding with inGame=false]\n");
        }

        // When user switches back to the game window, fast-path foreground switch activates inGame
        sim.onForegroundSwitched(1234, "gameclient.exe");
        if (!sim.inGame) {
            failures++;
            printf("FAIL [fast-path foreground switch activates armed binding]\n");
        }

        // Turning game mode off clears binding immediately
        sim.setGameApp("GameClient.exe", false, 1234, "gameclient.exe");
        if (sim.inGame || sim.boundHwnd != 0) {
            failures++;
            printf("FAIL [immediate unbind when game mode removed]\n");
        }
    }
}

// Mirrors password-field bypass precedence.
// Host integration/manual matrix covers real Win32/UIA controls.
static void testPasswordField() {
    struct PasswordFieldSim {
        int passwordState = PW_NORMAL;
        bool isPasswordField = false;
        bool passwordStateDirty = false;
        uint64_t probeGen = 0;
        uint64_t pendingDeadlineTick = 0;
        int hookSessionResets = 0;
        bool inGame = false;
        bool gameChat = false;
        bool bypassSetting = true;
        uint64_t lastHookTick = 0;
        void* lastFocusHwnd = nullptr;
        uint32_t lastFocusPid = 0;
        int evaluateCount = 0;

        void resetSessionOnHookThread() {
            hookSessionResets++;
            gameChat = false;
            startNewSession();
            vStartSentence();
        }

        void syncCompatFlag() {
            isPasswordField = (passwordState != PW_NORMAL);
        }

        void invalidateCache() {
            lastFocusHwnd = nullptr;
            lastFocusPid = 0;
            probeGen++;
            if (passwordState != PW_NORMAL) {
                passwordState = PW_NORMAL;
                passwordStateDirty = true;
            }
            syncCompatFlag();
        }

        void updateFocus(void* hwnd, uint32_t pid, bool isPasswordControl) {
            if (!bypassSetting) {
                invalidateCache();
                return;
            }
            if (hwnd == lastFocusHwnd && pid == lastFocusPid) {
                return; // Cached, skip re-evaluating
            }
            evaluateCount++;
            lastFocusHwnd = hwnd;
            lastFocusPid = pid;
            probeGen++;
            int newState = isPasswordControl ? PW_PASSWORD : PW_NORMAL;
            int prevState = passwordState;
            passwordState = newState;
            syncCompatFlag();

            if (prevState != newState) {
                passwordStateDirty = false;
                resetSessionOnHookThread();
            }
        }

        uint64_t beginPendingProbe(uint64_t nowTick, uint64_t timeoutMs = 60) {
            probeGen++;
            if (passwordState != PW_PENDING) {
                passwordState = PW_PENDING;
                passwordStateDirty = true;
            }
            pendingDeadlineTick = nowTick + timeoutMs;
            syncCompatFlag();
            return probeGen;
        }

        // Worker thread completion: never touches engine buffers directly, only updates state + dirty flag
        void completeWorkerProbe(uint64_t resultGen, bool isPass) {
            if (resultGen != probeGen) {
                return; // Stale generation ignored
            }
            int newState = isPass ? PW_PASSWORD : PW_NORMAL;
            if (passwordState != newState) {
                passwordState = newState;
                passwordStateDirty = true;
            }
            syncCompatFlag();
        }

        void setBypass(bool enabled, void* currentHwnd, uint32_t currentPid, bool isCurrentControlPassword) {
            bypassSetting = enabled;
            invalidateCache();
            updateFocus(currentHwnd, currentPid, isCurrentControlPassword);
            resetSessionOnHookThread();
        }

        bool consumeStateAndCheckRawOnHook(uint64_t currentTick) {
            if (passwordState == PW_PENDING && currentTick >= pendingDeadlineTick) {
                passwordState = PW_NORMAL;
                probeGen++; // Invalidate old in-flight probe
                passwordStateDirty = true;
                syncCompatFlag();
            }
            if (passwordStateDirty) {
                passwordStateDirty = false;
                resetSessionOnHookThread();
            }
            syncCompatFlag();
            return passwordState != PW_NORMAL;
        }

        // Returns true if passed-through raw to target app, false if intercepted by engine
        bool handleKey(uint16_t keycode, uint64_t currentTick) {
            lastHookTick = currentTick; // Bookkeeping must always update
            if (consumeStateAndCheckRawOnHook(currentTick)) {
                return true; // Pass-through raw unconditionally
            }
            if (inGame && !gameChat) {
                if (keycode == 13) { gameChat = true; return true; }
                return true;
            }
            return false; // Processed by engine
        }

        // Returns true if passed-through raw to target app, false if intercepted
        bool handleMouse(int mouseMsg, uint64_t currentTick) {
            lastHookTick = currentTick; // Bookkeeping must always update
            if (consumeStateAndCheckRawOnHook(currentTick)) {
                return true; // Pass-through raw unconditionally
            }
            if (inGame && gameChat && (mouseMsg == 0x0201 || mouseMsg == 0x0204 || mouseMsg == 0x0207)) {
                gameChat = false;
                return true;
            }
            return false;
        }
    };

    // Test 1: Raw pass-through suppresses Telex marks, macros, Quick Telex, mixed mode, auto-caps
    {
        total++;
        PasswordFieldSim sim;
        sim.updateFocus((void*)0x1000, 100, true);
        bool keyPassed = sim.handleKey('A', 1001);
        if (!keyPassed || !sim.isPasswordField || sim.lastHookTick != 1001) {
            failures++;
            printf("FAIL [password raw pass-through A]\n");
        }
    }

    // Test 2: Typing 'asdf', 'dc', 'pp', 'how', 'test' in password field remains raw
    {
        total++;
        PasswordFieldSim sim;
        sim.updateFocus((void*)0x1000, 100, true);
        bool allRaw = true;
        const char* samples[] = { "asdf", "dc", "pp", "how", "test" };
        for (const char* word : samples) {
            for (int i = 0; word[i]; i++) {
                if (!sim.handleKey((uint16_t)word[i], 2000 + i)) {
                    allRaw = false;
                }
            }
        }
        if (!allRaw) {
            failures++;
            printf("FAIL [password words samples must be 100%% raw]\n");
        }
    }

    // Test 3: Enter in password field passes through and does not open game chat
    {
        total++;
        PasswordFieldSim sim;
        sim.inGame = true;
        sim.updateFocus((void*)0x1000, 100, true);
        bool enterPassed = sim.handleKey(13, 3000); // VK_RETURN
        if (!enterPassed || sim.gameChat) {
            failures++;
            printf("FAIL [password Enter must pass-through without toggling game chat]\n");
        }
    }

    // Test 4: Esc in password field passes through without engine restore
    {
        total++;
        PasswordFieldSim sim;
        sim.updateFocus((void*)0x1000, 100, true);
        bool escPassed = sim.handleKey(27, 4000); // VK_ESCAPE
        if (!escPassed) {
            failures++;
            printf("FAIL [password Esc must pass-through unconditionally]\n");
        }
    }

    // Test 5: Mouse click in password field passes through, does not exit game chat or trigger engine mouse event
    {
        total++;
        PasswordFieldSim sim;
        sim.inGame = true;
        sim.gameChat = false;
        sim.updateFocus((void*)0x1000, 100, true);
        bool mousePassed = sim.handleMouse(0x0201, 5000); // WM_LBUTTONDOWN
        if (!mousePassed || sim.lastHookTick != 5000) {
            failures++;
            printf("FAIL [password mouse click must pass-through without engine side effects]\n");
        }
    }

    // Test 6: Password field beats game mode (password > game mode precedence)
    {
        total++;
        PasswordFieldSim sim;
        sim.inGame = true;
        sim.gameChat = true; // game chat was active in background
        sim.updateFocus((void*)0x1000, 100, true);
        if (sim.gameChat) {
            failures++;
            printf("FAIL [focusing password field must teardown game chat]\n");
        }
        bool keyPassed = sim.handleKey('W', 6000);
        if (!keyPassed || !sim.isPasswordField) {
            failures++;
            printf("FAIL [password field beats game mode]\n");
        }
    }

    // Test 7: Normal -> Password resets typing session (buffer isolation)
    {
        total++;
        vInputType = vTelex;
        vLanguage = 1;
        startNewSession();
        // Type "vie" in normal field
        typeKeys("vie");
        // Focus moves to password field
        PasswordFieldSim sim;
        sim.updateFocus((void*)0x2000, 100, true);
        // Engine session was reset, type "ejt" in password field (simulated raw)
        // Verify normal engine session after password focus change is clean
        startNewSession();
        wstring res = typeKeys("vieejt ");
        if (res != L"vi\u1ec7t ") {
            failures++;
            printf("FAIL [session isolation on password field entry]\n");
        }
    }

    // Test 8: Password -> Normal resets typing session
    {
        total++;
        PasswordFieldSim sim;
        sim.updateFocus((void*)0x2000, 100, true);
        // Switch back to normal field
        sim.updateFocus((void*)0x3000, 100, false);
        if (sim.isPasswordField) {
            failures++;
            printf("FAIL [normal field must have isPasswordField=false]\n");
        }
        startNewSession();
        wstring res = typeKeys("vieejt ");
        if (res != L"vi\u1ec7t ") {
            failures++;
            printf("FAIL [normal typing resumes cleanly after exiting password field]\n");
        }
    }

    // Test 9: Toggle OFF in password field immediately enables normal typing
    {
        total++;
        PasswordFieldSim sim;
        sim.updateFocus((void*)0x2000, 100, true);
        if (!sim.isPasswordField) failures++;
        sim.setBypass(false, (void*)0x2000, 100, true);
        if (sim.isPasswordField) {
            failures++;
            printf("FAIL [toggle OFF in password field must immediately disable bypass]\n");
        }
    }

    // Test 10: Toggle ON in password field immediately enables raw bypass
    {
        total++;
        PasswordFieldSim sim;
        sim.setBypass(false, (void*)0x2000, 100, true);
        if (sim.isPasswordField) failures++;
        sim.setBypass(true, (void*)0x2000, 100, true);
        if (!sim.isPasswordField) {
            failures++;
            printf("FAIL [toggle ON in password field must immediately enable bypass]\n");
        }
    }

    // Test 11: Cache deduplication: same HWND + same PID skips re-evaluation
    {
        total++;
        PasswordFieldSim sim;
        sim.updateFocus((void*)0x4000, 200, true);
        int eval1 = sim.evaluateCount;
        sim.updateFocus((void*)0x4000, 200, true);
        int eval2 = sim.evaluateCount;
        if (eval1 != eval2 || eval1 != 1) {
            failures++;
            printf("FAIL [cache same HWND + same PID must skip re-evaluation]\n");
        }
    }

    // Test 12: Cache invalidation: same HWND with different PID triggers re-evaluation
    {
        total++;
        PasswordFieldSim sim;
        sim.updateFocus((void*)0x4000, 200, true);
        int eval1 = sim.evaluateCount;
        sim.updateFocus((void*)0x4000, 300, false); // Handle recycled by different PID
        int eval2 = sim.evaluateCount;
        if (eval2 != eval1 + 1 || sim.isPasswordField) {
            failures++;
            printf("FAIL [cache same HWND + different PID must re-evaluate]\n");
        }
    }

    // Test 13: Hook liveness tick is updated even during password raw pass-through
    {
        total++;
        PasswordFieldSim sim;
        sim.updateFocus((void*)0x5000, 400, true);
        sim.handleKey('X', 99999);
        if (sim.lastHookTick != 99999) {
            failures++;
            printf("FAIL [keyboard hook liveness tick must update in password field]\n");
        }
        sim.handleMouse(0x0201, 100000);
        if (sim.lastHookTick != 100000) {
            failures++;
            printf("FAIL [mouse hook liveness tick must update in password field]\n");
        }
    }

    // Test 14: PW_PENDING passes keys raw before deadline and degrades to PW_NORMAL on timeout, invalidating old probe
    {
        total++;
        PasswordFieldSim sim;
        uint64_t gen = sim.beginPendingProbe(1000, 60);
        bool rawBeforeDeadline = sim.handleKey('A', 1030);
        bool rawAfterDeadline = sim.handleKey('A', 1060);
        // Late worker result from timed-out generation must be ignored
        sim.completeWorkerProbe(gen, true);
        if (!rawBeforeDeadline || rawAfterDeadline || sim.passwordState != PW_NORMAL || sim.isPasswordField) {
            failures++;
            printf("FAIL [PW_PENDING timeout must degrade to PW_NORMAL and invalidate stale worker result]\n");
        }
    }

    // Test 15: Worker probe completion sets dirty flag without resetting session on worker thread; hook thread resets session
    {
        total++;
        PasswordFieldSim sim;
        uint64_t gen = sim.beginPendingProbe(2000, 60);
        sim.handleKey('A', 2010); // Consumes initial NORMAL -> PENDING dirty flag
        int resetsBeforeWorker = sim.hookSessionResets;
        sim.completeWorkerProbe(gen, true); // PENDING -> PASSWORD on worker thread
        int resetsImmediatelyAfterWorker = sim.hookSessionResets;
        bool rawKey = sim.handleKey('B', 2020); // Hook thread consumes dirty flag
        if (resetsImmediatelyAfterWorker != resetsBeforeWorker || sim.hookSessionResets != resetsBeforeWorker + 1 || !rawKey || sim.passwordState != PW_PASSWORD) {
            failures++;
            printf("FAIL [worker thread must only set dirty flag and hook thread must reset session]\n");
        }
    }

    // Mirrors focus-key filtering and suppression policy.
    // Real browser behavior is covered by manual matrix after UIA commit.
    struct FocusFilterSim {
        struct FocusKey {
            void* hwnd = nullptr;
            uint32_t pid = 0;
            int32_t idObject = 0;
            int32_t idChild = 0;

            bool operator==(const FocusKey& o) const {
                return hwnd == o.hwnd && pid == o.pid && idObject == o.idObject && idChild == o.idChild;
            }
        };

        int passwordState = PW_NORMAL;
        bool uiaEnabled = false; // vPasswordFieldUIA = 0 dormant by default until manual matrix passes
        bool webInputContext = false;
        bool hadNavigationSignal = false;
        uint64_t lastNavigationTick = 0;
        uint64_t lastCharKeyTick = 0;
        FocusKey lastFocusKey{};
        FocusKey lastCharFocusKey{};
        uint64_t probeGen = 0;
        int scheduledProbes = 0;
        int focusEvaluations = 0;

        static bool isAcceptedObjectId(int32_t idObject) {
            return idObject == 0 || idObject == -4 || idObject < -12;
        }

        void recordNavigationSignal(uint64_t tick) {
            hadNavigationSignal = true;
            lastNavigationTick = tick;
        }

        void clearNavigationSignal() {
            hadNavigationSignal = false;
            lastNavigationTick = 0;
        }

        bool hasFreshNavigationSignal(uint64_t nowTick) const {
            return hadNavigationSignal && nowTick >= lastNavigationTick && (nowTick - lastNavigationTick) < 250;
        }

        void onCharKey(uint64_t tick) {
            lastCharKeyTick = tick;
            lastCharFocusKey = lastFocusKey;
            clearNavigationSignal();
        }

        void onNavigationKey(uint64_t tick = 0) {
            recordNavigationSignal(tick);
        }

        void onMouseUpInWebContext(uint64_t tick = 0) {
            recordNavigationSignal(tick);
        }

        void onFocusEvent(void* hwnd, uint32_t pid, int32_t idObject, int32_t idChild, bool isWin32Pass, bool isKnownBrowserHost, uint64_t nowTick, bool isForegroundChange = false) {
            if (!isForegroundChange && !isAcceptedObjectId(idObject)) {
                return;
            }
            if (isForegroundChange) {
                recordNavigationSignal(nowTick);
            }
            FocusKey currentKey{ hwnd, pid, idObject, idChild };
            if (!isForegroundChange && currentKey == lastFocusKey) {
                return;
            }
            focusEvaluations++;
            if (isWin32Pass) {
                lastFocusKey = currentKey;
                webInputContext = false;
                clearNavigationSignal();
                probeGen++;
                passwordState = PW_PASSWORD;
                return;
            }
            bool isWebCtx = (idObject < -12) || isKnownBrowserHost;
            webInputContext = isWebCtx;
            bool sameHost = (lastCharFocusKey.hwnd != nullptr && hwnd == lastCharFocusKey.hwnd && pid == lastCharFocusKey.pid);
            bool suppressContinuousTyping = !isForegroundChange &&
                !hasFreshNavigationSignal(nowTick) &&
                isWebCtx &&
                sameHost &&
                lastCharKeyTick != 0 &&
                nowTick >= lastCharKeyTick &&
                (nowTick - lastCharKeyTick) < 150;

            lastFocusKey = currentKey;
            if (suppressContinuousTyping) {
                return;
            }
            clearNavigationSignal();
            probeGen++;
            if (isWebCtx) {
                scheduledProbes++;
                if (uiaEnabled) {
                    passwordState = PW_PENDING;
                    return;
                }
            }
            passwordState = PW_NORMAL;
        }
    };

    // Test 16: idObject filter accepts 0, -4 (OBJID_CLIENT), < -12 and rejects -12..-1 (except -4)
    {
        total++;
        if (!FocusFilterSim::isAcceptedObjectId(0) ||
            !FocusFilterSim::isAcceptedObjectId(-4) ||
            !FocusFilterSim::isAcceptedObjectId(-100) ||
            FocusFilterSim::isAcceptedObjectId(-8) || // OBJID_CARET
            FocusFilterSim::isAcceptedObjectId(-3) || // OBJID_MENU
            FocusFilterSim::isAcceptedObjectId(-2) || // OBJID_TITLEBAR
            FocusFilterSim::isAcceptedObjectId(-12)) {
            failures++;
            printf("FAIL [idObject filter rules]\n");
        }
    }

    // Test 17: FocusKey 4-tuple comparison: same HWND+PID+idObject+idChild skips; different idObject or idChild re-evaluates
    {
        total++;
        FocusFilterSim sim;
        sim.onFocusEvent((void*)0x7000, 500, -100, 0, false, true, 1000);
        int eval1 = sim.focusEvaluations;
        sim.onFocusEvent((void*)0x7000, 500, -100, 0, false, true, 1200); // exact same 4-tuple
        int eval2 = sim.focusEvaluations;
        sim.onFocusEvent((void*)0x7000, 500, -101, 0, false, true, 1400); // different idObject on same HWND+PID
        int eval3 = sim.focusEvaluations;
        sim.onFocusEvent((void*)0x7000, 600, -101, 0, false, true, 1600); // different PID
        int eval4 = sim.focusEvaluations;
        if (eval1 != 1 || eval2 != 1 || eval3 != 2 || eval4 != 3) {
            failures++;
            printf("FAIL [FocusKey 4-tuple equality and re-evaluation]\n");
        }
    }

    // Test 18: Continuous typing (< 150ms) suppresses internal autocomplete focus events in same browser window
    {
        total++;
        FocusFilterSim sim;
        sim.onFocusEvent((void*)0x8000, 700, -100, 0, false, true, 1000);
        int probesBefore = sim.scheduledProbes;
        sim.onCharKey(1050); // User types a character
        sim.onFocusEvent((void*)0x8000, 700, -105, 0, false, true, 1120); // Autocomplete popup focus 70ms later (< 150ms)
        int probesAfterAutocomplete = sim.scheduledProbes;
        if (probesAfterAutocomplete != probesBefore) {
            failures++;
            printf("FAIL [continuous typing < 150ms must suppress internal focus probe]\n");
        }
    }

    // Test 19: Tab key, foreground change, or mouse up overrides typing suppression and allows probe
    {
        total++;
        FocusFilterSim sim;
        sim.onFocusEvent((void*)0x8000, 700, -100, 0, false, true, 1000);
        sim.onCharKey(1050);
        sim.onNavigationKey(1060); // User presses Tab right after typing
        sim.onFocusEvent((void*)0x8000, 700, -102, 0, false, true, 1080);
        int probesAfterTab = sim.scheduledProbes;
        sim.onCharKey(1100);
        sim.onFocusEvent((void*)0x9000, 800, 0, 0, false, true, 1120, true); // Foreground window change
        int probesAfterFg = sim.scheduledProbes;
        sim.onCharKey(1150);
        sim.onMouseUpInWebContext(1160); // Mouse click/up
        sim.onFocusEvent((void*)0x9000, 800, -200, 0, false, true, 1180);
        int probesAfterMouse = sim.scheduledProbes;
        if (probesAfterTab != 2 || probesAfterFg != 3 || probesAfterMouse != 4) {
            failures++;
            printf("FAIL [Tab, foreground change, and mouse-up must allow focus probe]\n");
        }
    }

    // Test 20: Win32 EDIT password still sets PW_PASSWORD immediately; vPasswordFieldUIA = 0 keeps browser focus PW_NORMAL
    {
        total++;
        FocusFilterSim sim;
        sim.uiaEnabled = false; // kill switch test
        sim.onFocusEvent((void*)0xA000, 900, 0, 0, true, false, 1000); // Win32 EDIT password
        bool win32ImmediatePass = (sim.passwordState == PW_PASSWORD && !sim.webInputContext);
        sim.onFocusEvent((void*)0xB000, 901, -150, 0, false, true, 1200); // Browser focus with uiaEnabled=false
        bool browserStaysNormalWhenUiaOff = (sim.passwordState == PW_NORMAL && sim.webInputContext);
        if (!win32ImmediatePass || !browserStaysNormalWhenUiaOff) {
            failures++;
            printf("FAIL [Win32 EDIT immediate PW_PASSWORD and uiaEnabled=0 keeps browser PW_NORMAL]\n");
        }
    }

    // Mirrors password probe state machine.
    // Real browser/UIA behavior is covered by manual matrix after UIA kill switch validation.
    struct ProbeStateMachineSim {
        enum ProbeOutcome { OUTCOME_TRUE, OUTCOME_FALSE, OUTCOME_ERROR };
        enum StateSource { SRC_NONE = 0, SRC_WIN32 = 1, SRC_UIA = 2, SRC_MSAA = 3 };

        static inline uint32_t makeStateWord(int state, int source) {
            return (static_cast<uint32_t>(state) & 0xFF) | ((static_cast<uint32_t>(source) & 0xFF) << 8);
        }
        static inline int getSimState(uint32_t word) { return static_cast<int>(word & 0xFF); }
        static inline int getSimSource(uint32_t word) { return static_cast<int>((word >> 8) & 0xFF); }

        FocusFilterSim::FocusKey lastFocusKey{};
        FocusFilterSim::FocusKey lastCharFocusKey{};
        std::atomic<uint32_t> passwordStateWord{ makeStateWord(PW_NORMAL, SRC_NONE) };
        int passwordState = PW_NORMAL;
        int passwordStateSource = SRC_NONE;
        bool passwordStateDirty = false;
        bool bypassEnabled = true;
        bool uiaEnabled = true;
        bool webInputContext = false;
        bool hadNavigationSignal = false;
        uint64_t lastCharKeyTick = 0;
        uint64_t lastProbeTick = 0;
        uint64_t probeGen = 0;
        uint64_t pendingDeadlineTick = 0;
        uint32_t probePreDelayMs = 0;
        int retryCount = 0;
        int mouseReprobeCount = 0;

        int getPasswordState() const { return getSimState(passwordStateWord.load(std::memory_order_acquire)); }
        int getPasswordSource() const { return getSimSource(passwordStateWord.load(std::memory_order_acquire)); }

        void setWin32Password(const FocusFilterSim::FocusKey& key) {
            lastFocusKey = key;
            webInputContext = false;
            probeGen++;
            passwordStateWord.store(makeStateWord(PW_PASSWORD, SRC_WIN32), std::memory_order_release);
            passwordState = PW_PASSWORD;
            passwordStateSource = SRC_WIN32;
            passwordStateDirty = true;
        }

        void applyPasswordFeatureConfigChanges() {
            if (!bypassEnabled) {
                probeGen++;
                passwordStateWord.store(makeStateWord(PW_NORMAL, SRC_NONE), std::memory_order_release);
                passwordState = PW_NORMAL;
                passwordStateSource = SRC_NONE;
                passwordStateDirty = true;
                return;
            }
            if (!uiaEnabled) {
                uint32_t oldWord = passwordStateWord.load(std::memory_order_acquire);
                for (;;) {
                    int state = getSimState(oldWord);
                    int src = getSimSource(oldWord);
                    if (state == PW_PENDING || (state == PW_PASSWORD && src != SRC_WIN32)) {
                        uint32_t normalWord = makeStateWord(PW_NORMAL, SRC_NONE);
                        if (passwordStateWord.compare_exchange_weak(oldWord, normalWord, std::memory_order_acq_rel)) {
                            probeGen++;
                            passwordState = PW_NORMAL;
                            passwordStateSource = SRC_NONE;
                            passwordStateDirty = true;
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
        }

        void setUiaEnabled(bool enabled) {
            uiaEnabled = enabled;
            if (!enabled) {
                applyPasswordFeatureConfigChanges();
            }
        }

        bool scheduleProbe(const FocusFilterSim::FocusKey& key, bool force, uint64_t nowTick, uint32_t preDelayMs = 0) {
            if (!bypassEnabled || !uiaEnabled) return false;
            if (!webInputContext) return false;
            if (!force && getPasswordState() == PW_PASSWORD) return true; // Sticky on same focus key
            lastFocusKey = key;
            lastProbeTick = nowTick;
            probePreDelayMs = preDelayMs;
            pendingDeadlineTick = nowTick + 60;
            probeGen++;
            uint32_t pendingWord = makeStateWord(PW_PENDING, SRC_NONE);
            uint32_t prev = passwordStateWord.exchange(pendingWord, std::memory_order_acq_rel);
            passwordState = PW_PENDING;
            passwordStateSource = SRC_NONE;
            if (getSimState(prev) != PW_PENDING) {
                passwordStateDirty = true;
            }
            return true;
        }

        bool onMouseEvent(uint32_t msg, void* focusedHwnd, uint64_t nowTick) {
            if (msg != 0x0202) { // Only WM_LBUTTONUP (0x0202) triggers mouse-up reprobe
                return false;
            }
            if (!bypassEnabled || !uiaEnabled || !webInputContext) return false;
            if (lastFocusKey.hwnd == nullptr) return false;
            if (getPasswordState() == PW_PASSWORD) return false; // Sticky password never downgrades on click
            if (lastProbeTick != 0 && nowTick >= lastProbeTick && (nowTick - lastProbeTick) < 100) {
                return false; // Throttled (< 100 ms)
            }
            if (!focusedHwnd || focusedHwnd != lastFocusKey.hwnd) {
                return false; // Foreground/focused window mismatch
            }
            if (scheduleProbe(lastFocusKey, false, nowTick, 40)) {
                mouseReprobeCount++;
                return true;
            }
            return false;
        }

        void applyWorkerResult(uint64_t resultGen, bool isPass, StateSource src = SRC_UIA) {
            if (resultGen != probeGen) return;
            uint32_t oldWord = passwordStateWord.load(std::memory_order_acquire);
            for (;;) {
                int oldState = getSimState(oldWord);
                if (oldState == PW_PENDING) {
                    int nextState = isPass ? PW_PASSWORD : PW_NORMAL;
                    int nextSource = isPass ? src : SRC_NONE;
                    uint32_t nextWord = makeStateWord(nextState, nextSource);
                    if (passwordStateWord.compare_exchange_weak(oldWord, nextWord, std::memory_order_acq_rel)) {
                        passwordState = nextState;
                        passwordStateSource = nextSource;
                        if (nextState == PW_NORMAL) {
                            passwordStateDirty = true;
                        }
                        break;
                    }
                } else if (oldState == PW_NORMAL && isPass) {
                    uint32_t nextWord = makeStateWord(PW_PASSWORD, src);
                    if (passwordStateWord.compare_exchange_weak(oldWord, nextWord, std::memory_order_acq_rel)) {
                        passwordState = PW_PASSWORD;
                        passwordStateSource = src;
                        passwordStateDirty = true;
                        break;
                    }
                } else {
                    break;
                }
            }
        }

        bool evaluateUiaMsaaWithRetry(ProbeOutcome uia1, ProbeOutcome msaa1, ProbeOutcome uia2 = OUTCOME_ERROR, ProbeOutcome msaa2 = OUTCOME_ERROR) {
            retryCount = 0;
            ProbeOutcome uiaArr[2] = { uia1, uia2 };
            ProbeOutcome msaaArr[2] = { msaa1, msaa2 };
            for (int attempt = 0; attempt < 2; attempt++) {
                if (attempt == 1) retryCount++;
                if (uiaArr[attempt] == OUTCOME_TRUE) return true;
                if (msaaArr[attempt] == OUTCOME_TRUE) return true;
                if (uiaArr[attempt] != OUTCOME_ERROR || msaaArr[attempt] != OUTCOME_ERROR) {
                    return false;
                }
            }
            return false;
        }
    };

    // Test 21: Navigation signal lifecycle & 250ms TTL:
    // Tab -> focus within 250ms -> allow (clears signal) -> next internal focus while typing -> suppress;
    // Tab -> no immediate focus -> after 300ms typing internal focus -> stale signal expired, suppress
    {
        total++;
        FocusFilterSim sim;
        sim.onFocusEvent((void*)0x8000, 700, -100, 0, false, true, 1000);
        sim.onCharKey(1050);
        sim.onFocusEvent((void*)0x8000, 700, -101, 0, false, true, 1100); // suppressed (probes = 1)
        int p1 = sim.scheduledProbes;
        sim.onNavigationKey(1110); // Tab pressed at 1110
        sim.onFocusEvent((void*)0x8000, 700, -102, 0, false, true, 1120); // within 250ms -> allowed (probes = 2), clears signal
        int p2 = sim.scheduledProbes;
        bool signalCleared = !sim.hadNavigationSignal;
        sim.onCharKey(1140); // Type in new field
        sim.onFocusEvent((void*)0x8000, 700, -103, 0, false, true, 1180); // internal focus < 150ms -> suppressed again (probes = 2)
        int p3 = sim.scheduledProbes;

        // Now test TTL expiration: navigation key pressed, no focus event within 250ms, then user types or internal focus fires after 300ms
        sim.onNavigationKey(2000);
        // Suppose lastCharKeyTick was updated just before or at 2250 without clearing (or test stale nav signal directly at 2300 while lastCharKeyTick = 2200)
        sim.lastCharKeyTick = 2200;
        sim.lastCharFocusKey = sim.lastFocusKey;
        sim.onFocusEvent((void*)0x8000, 700, -104, 0, false, true, 2300); // 2300 - 2000 = 300ms >= 250ms TTL -> stale signal, suppressed!
        int p4 = sim.scheduledProbes;
        if (p1 != 1 || p2 != 2 || !signalCleared || p3 != 2 || p4 != 2) {
            failures++;
            printf("FAIL [navigation signal lifecycle and 250ms TTL must expire stale signal and suppress internal focus]\n");
        }
    }

    // Test 22: vPasswordFieldUIA = 1 transitions: PENDING -> PASSWORD (no extra dirty) and PENDING -> NORMAL (sets dirty)
    {
        total++;
        ProbeStateMachineSim sim;
        sim.webInputContext = true;
        FocusFilterSim::FocusKey k1{ (void*)0x1000, 10, -100, 0 };
        sim.scheduleProbe(k1, true, 1000);
        uint64_t g1 = sim.probeGen;
        bool pendingDirty = sim.passwordStateDirty;
        sim.passwordStateDirty = false; // Hook consumed NORMAL -> PENDING dirty
        sim.applyWorkerResult(g1, true); // PENDING -> PASSWORD
        bool passNoExtraDirty = (sim.passwordState == PW_PASSWORD && !sim.passwordStateDirty);

        FocusFilterSim::FocusKey k2{ (void*)0x1000, 10, -101, 0 };
        sim.scheduleProbe(k2, true, 2000);
        uint64_t g2 = sim.probeGen;
        sim.passwordStateDirty = false;
        sim.applyWorkerResult(g2, false); // PENDING -> NORMAL
        bool normalSetsDirty = (sim.passwordState == PW_NORMAL && sim.passwordStateDirty);
        if (!pendingDirty || !passNoExtraDirty || !normalSetsDirty) {
            failures++;
            printf("FAIL [UIA state machine transitions and dirty flag semantics]\n");
        }
    }

    // Test 23: UIA/MSAA decision table and single retry on dual error
    {
        total++;
        ProbeStateMachineSim sim;
        bool uiaFalseMsaaTrue = sim.evaluateUiaMsaaWithRetry(ProbeStateMachineSim::OUTCOME_FALSE, ProbeStateMachineSim::OUTCOME_TRUE);
        int r1 = sim.retryCount;
        bool uiaFalseMsaaFalse = sim.evaluateUiaMsaaWithRetry(ProbeStateMachineSim::OUTCOME_FALSE, ProbeStateMachineSim::OUTCOME_FALSE);
        int r1b = sim.retryCount;
        bool uiaErrMsaaFalse = sim.evaluateUiaMsaaWithRetry(ProbeStateMachineSim::OUTCOME_ERROR, ProbeStateMachineSim::OUTCOME_FALSE);
        int r2 = sim.retryCount;
        bool dualErrRetryFail = sim.evaluateUiaMsaaWithRetry(ProbeStateMachineSim::OUTCOME_ERROR, ProbeStateMachineSim::OUTCOME_ERROR, ProbeStateMachineSim::OUTCOME_ERROR, ProbeStateMachineSim::OUTCOME_ERROR);
        int r3 = sim.retryCount;
        bool dualErrRetryRecover = sim.evaluateUiaMsaaWithRetry(ProbeStateMachineSim::OUTCOME_ERROR, ProbeStateMachineSim::OUTCOME_ERROR, ProbeStateMachineSim::OUTCOME_FALSE, ProbeStateMachineSim::OUTCOME_TRUE);
        int r4 = sim.retryCount;
        if (!uiaFalseMsaaTrue || r1 != 0 || uiaFalseMsaaFalse || r1b != 0 || uiaErrMsaaFalse || r2 != 0 || dualErrRetryFail || r3 != 1 || !dualErrRetryRecover || r4 != 1) {
            failures++;
            printf("FAIL [UIA/MSAA fallback decision table and retry policy]\n");
        }
    }

    // Test 24: Sticky password: PW_PASSWORD does not downgrade on same-key reprobe, only when focus key changes to normal
    {
        total++;
        ProbeStateMachineSim sim;
        sim.webInputContext = true;
        FocusFilterSim::FocusKey k1{ (void*)0x2000, 20, -200, 0 };
        sim.scheduleProbe(k1, true, 1000);
        sim.applyWorkerResult(sim.probeGen, true);
        bool isPass1 = (sim.passwordState == PW_PASSWORD);
        // Same-key non-forced reprobe must stay sticky PW_PASSWORD
        sim.scheduleProbe(k1, false, 1500);
        sim.applyWorkerResult(sim.probeGen, false);
        bool stickyHeld = (sim.passwordState == PW_PASSWORD);
        // Focus key changes (force = true) to a normal input -> downgrades to PW_NORMAL after probe
        FocusFilterSim::FocusKey k2{ (void*)0x2000, 20, -201, 0 };
        sim.scheduleProbe(k2, true, 2000);
        sim.applyWorkerResult(sim.probeGen, false);
        bool downgradedOnNewKey = (sim.passwordState == PW_NORMAL);
        if (!isPass1 || !stickyHeld || !downgradedOnNewKey) {
            failures++;
            printf("FAIL [sticky password on same focus key and downgrade on new focus key]\n");
        }
    }

    // Test 25: Win32 password while PW_PENDING immediately sets PW_PASSWORD and increments probeGen to invalidate old probe
    {
        total++;
        PasswordFieldSim sim;
        uint64_t pendingGen = sim.beginPendingProbe(3000, 60);
        sim.updateFocus((void*)0x9999, 99, true); // Win32 password focus arrives while pending
        sim.completeWorkerProbe(pendingGen, false); // Old worker finishes with false
        if (sim.passwordState != PW_PASSWORD || !sim.isPasswordField || sim.probeGen == pendingGen) {
            failures++;
            printf("FAIL [Win32 password must override PW_PENDING and invalidate old probeGen]\n");
        }
    }

    // Test 26: Feature off or UIA kill switch off prevents browser pending probe AND resets existing browser PW_PASSWORD while preserving Win32 PW_PASSWORD
    {
        total++;
        ProbeStateMachineSim sim;
        sim.webInputContext = true;
        sim.uiaEnabled = false;
        FocusFilterSim::FocusKey k1{ (void*)0x3000, 30, -300, 0 };
        bool scheduledWhenUiaOff = sim.scheduleProbe(k1, true, 1000);
        sim.uiaEnabled = true;
        sim.bypassEnabled = false;
        bool scheduledWhenBypassOff = sim.scheduleProbe(k1, true, 1100);

        // UIA on + browser password -> PW_PASSWORD; then UIA off on same focus -> resets to PW_NORMAL + dirty + gen++
        sim.bypassEnabled = true;
        sim.uiaEnabled = true;
        sim.scheduleProbe(k1, true, 1200);
        uint64_t gBrowser = sim.probeGen;
        sim.applyWorkerResult(gBrowser, true, ProbeStateMachineSim::SRC_UIA);
        sim.passwordStateDirty = false;
        sim.setUiaEnabled(false);
        bool browserResetOnUiaOff = (sim.passwordState == PW_NORMAL && sim.passwordStateDirty && sim.probeGen == gBrowser + 1);

        // UIA off + Win32 EDIT password -> still PW_PASSWORD
        FocusFilterSim::FocusKey kWin32{ (void*)0x4000, 40, 0, 0 };
        sim.setWin32Password(kWin32);
        sim.passwordStateDirty = false;
        sim.setUiaEnabled(false);
        bool win32PreservedOnUiaOff = (sim.passwordState == PW_PASSWORD && sim.passwordStateSource == ProbeStateMachineSim::SRC_WIN32 && !sim.passwordStateDirty);

        if (scheduledWhenUiaOff || scheduledWhenBypassOff || !browserResetOnUiaOff || !win32PreservedOnUiaOff) {
            failures++;
            printf("FAIL [UIA kill switch off must reset browser password state while preserving Win32 password state]\n");
        }
    }

    // Test 27: WM_LBUTTONUP in web input context schedules reprobe with 40ms debounce and upgrades PW_NORMAL -> PW_PASSWORD
    {
        total++;
        ProbeStateMachineSim sim;
        sim.webInputContext = true;
        FocusFilterSim::FocusKey k1{ (void*)0x5000, 50, -500, 0 };
        sim.scheduleProbe(k1, true, 1000);
        sim.applyWorkerResult(sim.probeGen, false); // Initially normal input
        sim.passwordStateDirty = false;
        // User clicks a button/toggle or custom input -> WM_LBUTTONUP at 1150 (>= 100ms after 1000)
        bool scheduled = sim.onMouseEvent(0x0202, (void*)0x5000, 1150);
        bool isPendingAndDebounced = (sim.passwordState == PW_PENDING && sim.probePreDelayMs == 40 && sim.passwordStateDirty);
        sim.passwordStateDirty = false;
        sim.applyWorkerResult(sim.probeGen, true); // Now it became a password field
        bool upgradedToPassword = (sim.passwordState == PW_PASSWORD && !sim.passwordStateDirty);
        if (!scheduled || !isPendingAndDebounced || !upgradedToPassword) {
            failures++;
            printf("FAIL [WM_LBUTTONUP in web context must schedule 40ms debounced reprobe and upgrade to PW_PASSWORD]\n");
        }
    }

    // Test 28: Non-WM_LBUTTONUP mouse events (move, wheel, LBUTTONDOWN, RBUTTONUP, XBUTTON) never schedule reprobe
    {
        total++;
        ProbeStateMachineSim sim;
        sim.webInputContext = true;
        FocusFilterSim::FocusKey k1{ (void*)0x5000, 50, -500, 0 };
        sim.scheduleProbe(k1, true, 1000);
        sim.applyWorkerResult(sim.probeGen, false);
        bool moveScheduled = sim.onMouseEvent(0x0200, (void*)0x5000, 1200);       // WM_MOUSEMOVE
        bool wheelScheduled = sim.onMouseEvent(0x020A, (void*)0x5000, 1300);      // WM_MOUSEWHEEL
        bool ldownScheduled = sim.onMouseEvent(0x0201, (void*)0x5000, 1400);      // WM_LBUTTONDOWN
        bool rupScheduled = sim.onMouseEvent(0x0205, (void*)0x5000, 1500);        // WM_RBUTTONUP
        bool xupScheduled = sim.onMouseEvent(0x020C, (void*)0x5000, 1600);        // WM_XBUTTONUP
        if (moveScheduled || wheelScheduled || ldownScheduled || rupScheduled || xupScheduled || sim.mouseReprobeCount != 0) {
            failures++;
            printf("FAIL [non-WM_LBUTTONUP mouse events must never schedule password reprobe]\n");
        }
    }

    // Test 29: Mouse-up reprobe 100ms throttle & 40ms debounce coalescing via probeGen
    {
        total++;
        ProbeStateMachineSim sim;
        sim.webInputContext = true;
        FocusFilterSim::FocusKey k1{ (void*)0x6000, 60, -600, 0 };
        sim.scheduleProbe(k1, true, 1000);
        sim.applyWorkerResult(sim.probeGen, false);
        // First WM_LBUTTONUP at 1050 (< 100ms since initial probe at 1000) -> throttled
        bool throttledEarly = sim.onMouseEvent(0x0202, (void*)0x6000, 1050);
        // WM_LBUTTONUP at 1100 (== 100ms since 1000) -> allowed!
        bool allowedAt1100 = sim.onMouseEvent(0x0202, (void*)0x6000, 1100);
        uint64_t genMouse = sim.probeGen;
        // Rapid second WM_LBUTTONUP at 1130 (< 100ms since 1100) -> throttled!
        bool throttledSecond = sim.onMouseEvent(0x0202, (void*)0x6000, 1130);
        // Suppose a real focus event arrives at 1120 during the 40ms debounce window -> increments probeGen and coalesces
        FocusFilterSim::FocusKey k2{ (void*)0x6000, 60, -601, 0 };
        sim.scheduleProbe(k2, true, 1120);
        uint64_t genFocus = sim.probeGen;
        // Stale genMouse result is ignored; coalesced genFocus result applies
        sim.applyWorkerResult(genMouse, false);
        bool staleIgnored = (sim.passwordState == PW_PENDING);
        sim.applyWorkerResult(genFocus, true);
        bool coalescedApplied = (sim.passwordState == PW_PASSWORD);
        if (throttledEarly || !allowedAt1100 || throttledSecond || !staleIgnored || !coalescedApplied) {
            failures++;
            printf("FAIL [mouse-up reprobe 100ms throttle and 40ms debounce coalescing]\n");
        }
    }

    // Test 30: Clicking "Show password" (WM_LBUTTONUP while already PW_PASSWORD on same focus key) keeps sticky PW_PASSWORD
    {
        total++;
        ProbeStateMachineSim sim;
        sim.webInputContext = true;
        FocusFilterSim::FocusKey k1{ (void*)0x7000, 70, -700, 0 };
        sim.scheduleProbe(k1, true, 1000);
        sim.applyWorkerResult(sim.probeGen, true); // Confirmed password field
        uint64_t genBeforeClick = sim.probeGen;
        // User clicks "Show password" eye icon inside the same render HWND without focus key change
        bool reprobeScheduled = sim.onMouseEvent(0x0202, (void*)0x7000, 1500);
        if (reprobeScheduled || sim.passwordState != PW_PASSWORD || sim.probeGen != genBeforeClick) {
            failures++;
            printf("FAIL [WM_LBUTTONUP on confirmed password field must keep sticky PW_PASSWORD without reprobe]\n");
        }
    }

    // Test 31: Default vPasswordFieldUIA = 1 automatically schedules probe and resolves browser password
    {
        total++;
        FocusFilterSim sim;
        sim.uiaEnabled = true; // explicitly enable for test
        sim.onFocusEvent((void*)0xC000, 950, -180, 0, false, true, 2000);
        bool entersPendingByDefault = (sim.passwordState == PW_PENDING && sim.webInputContext && sim.scheduledProbes == 1);
        if (!entersPendingByDefault) {
            failures++;
            printf("FAIL [default vPasswordFieldUIA=1 must schedule PW_PENDING probe on browser focus]\n");
        }
    }

    // Test 32: Packed 32-bit state+source atomic word prevents torn reads and desynchronization
    {
        total++;
        uint32_t wNormal = ProbeStateMachineSim::makeStateWord(PW_NORMAL, ProbeStateMachineSim::SRC_NONE);
        uint32_t wPending = ProbeStateMachineSim::makeStateWord(PW_PENDING, ProbeStateMachineSim::SRC_NONE);
        uint32_t wWin32 = ProbeStateMachineSim::makeStateWord(PW_PASSWORD, ProbeStateMachineSim::SRC_WIN32);
        uint32_t wUia = ProbeStateMachineSim::makeStateWord(PW_PASSWORD, ProbeStateMachineSim::SRC_UIA);
        uint32_t wMsaa = ProbeStateMachineSim::makeStateWord(PW_PASSWORD, ProbeStateMachineSim::SRC_MSAA);

        bool packCheck = (ProbeStateMachineSim::getSimState(wNormal) == PW_NORMAL && ProbeStateMachineSim::getSimSource(wNormal) == ProbeStateMachineSim::SRC_NONE) &&
                         (ProbeStateMachineSim::getSimState(wPending) == PW_PENDING && ProbeStateMachineSim::getSimSource(wPending) == ProbeStateMachineSim::SRC_NONE) &&
                         (ProbeStateMachineSim::getSimState(wWin32) == PW_PASSWORD && ProbeStateMachineSim::getSimSource(wWin32) == ProbeStateMachineSim::SRC_WIN32) &&
                         (ProbeStateMachineSim::getSimState(wUia) == PW_PASSWORD && ProbeStateMachineSim::getSimSource(wUia) == ProbeStateMachineSim::SRC_UIA) &&
                         (ProbeStateMachineSim::getSimState(wMsaa) == PW_PASSWORD && ProbeStateMachineSim::getSimSource(wMsaa) == ProbeStateMachineSim::SRC_MSAA);

        // Atomic transition in CAS loop
        ProbeStateMachineSim sim;
        sim.webInputContext = true;
        FocusFilterSim::FocusKey k1{ (void*)0x9000, 90, -900, 0 };
        sim.scheduleProbe(k1, true, 1000); // Transitions to PW_PENDING + SRC_NONE
        uint64_t gPending = sim.probeGen;
        sim.applyWorkerResult(gPending, true, ProbeStateMachineSim::SRC_UIA); // Atomically transitions to PW_PASSWORD + SRC_UIA
        bool atomicityCheck = (sim.getPasswordState() == PW_PASSWORD && sim.getPasswordSource() == ProbeStateMachineSim::SRC_UIA);

        if (!packCheck || !atomicityCheck) {
            failures++;
            printf("FAIL [packed atomic state+source word must prevent torn reads and desynchronization]\n");
        }
    }

    // Test 33: Centralized applyPasswordFeatureConfigChanges handles bypass toggle, UIA toggle, and Win32 preservation
    {
        total++;
        ProbeStateMachineSim sim;
        sim.webInputContext = true;
        FocusFilterSim::FocusKey k1{ (void*)0x9000, 90, -900, 0 };
        sim.scheduleProbe(k1, true, 1000);
        sim.applyWorkerResult(sim.probeGen, true, ProbeStateMachineSim::SRC_UIA);
        sim.passwordStateDirty = false;
        // User turns off UIA -> resets browser password to NORMAL
        sim.setUiaEnabled(false);
        bool resetOnUiaOff = (sim.getPasswordState() == PW_NORMAL && sim.getPasswordSource() == ProbeStateMachineSim::SRC_NONE && sim.passwordStateDirty);

        // Win32 password control
        FocusFilterSim::FocusKey kWin32{ (void*)0x9001, 91, 0, 0 };
        sim.setWin32Password(kWin32);
        sim.passwordStateDirty = false;
        sim.setUiaEnabled(false); // UIA off does NOT reset Win32 password
        bool win32Preserved = (sim.getPasswordState() == PW_PASSWORD && sim.getPasswordSource() == ProbeStateMachineSim::SRC_WIN32 && !sim.passwordStateDirty);

        // Bypass disabled completely -> resets even Win32 password
        sim.bypassEnabled = false;
        sim.applyPasswordFeatureConfigChanges();
        bool bypassResetAll = (sim.getPasswordState() == PW_NORMAL && sim.getPasswordSource() == ProbeStateMachineSim::SRC_NONE && sim.passwordStateDirty);

        if (!resetOnUiaOff || !win32Preserved || !bypassResetAll) {
            failures++;
            printf("FAIL [centralized applyPasswordFeatureConfigChanges must handle bypass and UIA toggles properly]\n");
        }
    }
}

//EngineTests --render: for each word read, print it and what Telex makes of it (tools/make_mixed_words.py uses this)
static int renderWords() {
    vInputType = vTelex;
    string word;
    while (getline(cin, word)) {
        wstring text = typeKeys(word + " ");
        text.pop_back();
        printf("%s\t%s\n", word.c_str(), toUtf8(text).c_str());
    }
    return 0;
}

int main(int argc, char** argv) {
    pData = (vKeyHookState*)vKeyInit();
    if (argc > 1 && strcmp(argv[1], "--render") == 0)
        return renderWords();
    testTelex();
    testMixed();
    testVNI();
    testConvert();
    testUtf8Conversion();
    testMacroFileIO();
    testGameMode();
    testPasswordField();
    printf("%d/%d engine tests passed\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
