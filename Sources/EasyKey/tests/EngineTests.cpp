//
//  EngineTests.cpp
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//
//  Unit tests for the shared typing engine: feed key strokes into
//  vKeyHandleEvent and check the text an application would end up with.
//

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <tuple>
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
}

// Mirrors password-field bypass precedence.
// Host integration/manual matrix covers real Win32/UIA controls.
static void testPasswordField() {
    struct PasswordFieldSim {
        bool isPasswordField = false;
        bool inGame = false;
        bool gameChat = false;
        bool bypassSetting = true;
        uint64_t lastHookTick = 0;
        void* lastFocusHwnd = nullptr;
        uint32_t lastFocusPid = 0;
        int evaluateCount = 0;

        void invalidateCache() {
            lastFocusHwnd = nullptr;
            lastFocusPid = 0;
            isPasswordField = false;
        }

        void updateFocus(void* hwnd, uint32_t pid, bool isPasswordControl) {
            if (!bypassSetting) {
                isPasswordField = false;
                lastFocusHwnd = nullptr;
                lastFocusPid = 0;
                return;
            }
            if (hwnd == lastFocusHwnd && pid == lastFocusPid) {
                return; // Cached, skip re-evaluating
            }
            evaluateCount++;
            lastFocusHwnd = hwnd;
            lastFocusPid = pid;
            bool wasPass = isPasswordField;
            isPasswordField = isPasswordControl;

            if (isPasswordField) {
                gameChat = false;
                startNewSession();
                vStartSentence();
            } else if (wasPass) {
                startNewSession();
                vStartSentence();
            }
        }

        void setBypass(bool enabled, void* currentHwnd, uint32_t currentPid, bool isCurrentControlPassword) {
            bypassSetting = enabled;
            invalidateCache();
            updateFocus(currentHwnd, currentPid, isCurrentControlPassword);
            startNewSession();
            vStartSentence();
        }

        // Returns true if passed-through raw to target app, false if intercepted by engine
        bool handleKey(uint16_t keycode, uint64_t currentTick) {
            lastHookTick = currentTick; // Bookkeeping must always update
            if (isPasswordField) {
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
            if (isPasswordField) {
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
