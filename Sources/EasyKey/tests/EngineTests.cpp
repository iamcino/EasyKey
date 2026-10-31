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
    printf("%d/%d engine tests passed\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
