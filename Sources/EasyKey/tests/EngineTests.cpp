//
//  EngineTests.cpp
//  OpenKey
//
//  Unit tests for the shared typing engine: feed key strokes into
//  vKeyHandleEvent and check the text an application would end up with.
//

#include <cstdio>
#include <string>
#include "../engine/Engine.h"

//engine settings, normally defined by each platform app (see Engine.h)
int vLanguage = 1;
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
int vTempOffSpelling = 0;
int vAllowConsonantZFWJ = 0;
int vQuickStartConsonant = 0;
int vQuickEndConsonant = 0;
int vRememberCode = 0;
int vOtherLanguage = 0;
int vTempOffOpenKey = 0;
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
    else
        text += keyCodeToChar(data);
}

static wstring typeKeys(const string& keys) {
    startNewSession();
    wstring text;
    for (char c : keys) {
        Uint16 keyCode = charToKeyCode(c);
        bool isPunctuation = c == '.' || c == '?' || c == '!';
        bool isCaps = (c >= 'A' && c <= 'Z') || c == '?' || c == '!';
        vKeyHandleEvent(vKeyEvent::Keyboard, vKeyEventState::KeyDown, keyCode, isCaps ? 1 : 0, false);
        if (pData->code == vDoNothing) {
            //the key is passed through to the application
            if (keyCode == KEY_DELETE) {
                if (!text.empty()) text.pop_back();
            } else if (keyCode == KEY_ESC) {
                text += L'|'; //Esc reached the application
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
    //capitalize the first letter after . ? !
    vUpperCaseFirstChar = 1;
    expectTyping("Telex", "xin. chaof", L"xin. Chào");
    expectTyping("Telex", "xin? chaof", L"xin? Chào");
    expectTyping("Telex", "xin! chaof", L"xin! Chào");
    expectTyping("Telex", "xin chaof", L"xin chào");
    vUpperCaseFirstChar = 0;
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
}

int main() {
    pData = (vKeyHookState*)vKeyInit();
    testTelex();
    testVNI();
    printf("%d/%d engine tests passed\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
