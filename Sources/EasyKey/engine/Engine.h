//
//  Engine.h
//  OpenKey
//
//  Created by Tuyen on 1/18/19.
//  Copyright © 2019 Tuyen Mai. All rights reserved.
//
//  Part of EasyKey, a modified version of OpenKey (https://github.com/tuyenvm/OpenKey).
//  SPDX-License-Identifier: GPL-3.0-only
//

#ifndef Engine_h
#define Engine_h




#include "DataType.h"
#include "Vietnamese.h"
#include "Macro.h"
#include "SmartSwitchKey.h"
#include "ConvertTool.h"

#define IS_DEBUG 1

#ifndef LOBYTE
#define LOBYTE(data) (data & 0xFF)
#endif // !LOBYTE
#ifndef HIBYTE
#define HIBYTE(data) ((data>>8) & 0xFF)
#endif // !HIBYTE

#define GET_SWITCH_KEY(data) (data & 0xFF)
#define HAS_CONTROL(data) ((data & 0x100) ? 1 : 0)
#define HAS_OPTION(data) ((data & 0x200) ? 1 : 0)
#define HAS_COMMAND(data) ((data & 0x400) ? 1 : 0)
#define HAS_SHIFT(data) ((data & 0x800) ? 1 : 0)
#define GET_BOOL(data) (data ? 1 : 0)
#define HAS_BEEP(data) (data & 0x8000)
#define SET_SWITCH_KEY(data, key) data = (data & 0xFF) | key
#define SET_CONTROL_KEY(data, val) data|=val<<8;
#define SET_OPTION_KEY(data, val) data|=val<<9;
#define SET_COMMAND_KEY(data, val) data|=val<<10;

//The application's settings for the engine, handed in on every vKeyHandleEvent
//call (plan point 2). The defaults below are the application's own defaults.
//Secondary entry points (startNewSession, vEnglishMode, ...) have no config
//parameter: they read the config of the most recent vKeyHandleEvent call
//through vKeyActiveConfig() - that pointer is the single mechanism.
struct vKeyConfig {
/*
 * 0: English
 * 1: Vietnamese
 */
int language = 1;

/*
 * Mixed English and Vietnamese, works while vLanguage is 1 (EasyKey)
 * 0: No
 * 1: Yes, a word that reads as English gets its keys back when it ends: "this" stays "this", not "thí"
 */
int mixedEnglish = 0;

/*
 * Fix the previous word if the current word is clearly another language (EasyKey)
 * 0: No (default in 1.0.13: off to avoid erratic backspaces in async/rich-text apps)
 * 1: Yes (experimental retroactive correction)
 */
int mixedFixLast = 0;

/*
 * 0: Telex
 * 1: VNI
 */
int inputType = 0;

/**
 * 0: No
 * 1: Yes
 */
int freeMark = 0;

/*
 * 0: Unicode
 * 1: TCVN3 (ABC)
 * 2: VNI-Windows
 */
int codeTable = 0;

/*
 * first 8 bit: keycode
 * bit 8: Control on/off
 * bit 9: Option on/off
 * bit 10: Command on/off
 */
int switchKeyStatus = 0x5A00025A;

/**
 * 0: No
 * 1: Yes
 */
int checkSpelling = 1;

/*
 * 0: òa, úy
 * 1: oà uý
*/
int useModernOrthography = 1;

/**
 * 0: No
 * 1: Yes
 * (cc=ch, gg=gi, kk=kh, nn=ng, qq=qu, pp=ph, tt=th, uu=ươ)
 */
int quickTelex = 0;

/**
 * Work together with vCheckSpelling
 * 0: No
 * 1: Yes
 *
 */
int restoreIfWrongSpelling = 1;

/*
 * Fix recommend browser's address, excel,...
 * 0: No
 * 1: Yes
 */
int fixRecommendBrowser = 0;

/**
 * Macro on or off
 */
int useMacro = 1;

/**
 * Still use macro if you are in english mode
 */
int useMacroInEnglishMode = 1;

/**
 * Ex: define: btw -> by the way
 * Type: `Btw` -> `By the way`
 * Type: `BTW` -> `BY THE WAY`
 */
int autoCapsMacro = 0;

/**
 * auto switch language when switch app
 * 0: No
 * 1: Yes
 */
int useSmartSwitchKey = 1;

/**
 * Auto write upper case character for first letter.
 * 0: No
 * 1: Yes
 */
int upperCaseFirstChar = 0;

/**
 * Allow write word with consonant Z, F, W, J
 * 0: No
 * 1: Yes
 */
int allowConsonantZFWJ = 0;

/**
 * 0: No; 1: Yes
 * f -> ph: fanh -> phanh,...
 * j ->gi: jang -> giang,...
 * w ->qu: wen -> quen,...
 */
int quickStartConsonant = 0;

/**
 * 0: No; 1: Yes
 * g -> ng: hag -> hang,...
 * h -> nh: vih -> vinh,...
 * k -> ch: bak -> bach,...
 */
int quickEndConsonant = 0;

/**
 * 0: No; 1: Yes
 * Auto remember table code for each application
 */
int rememberCode = 1;

/**
 * 0: No; 1: Yes
 * Turn off Vietnamese when typing in another language.
 */
int otherLanguage = 1;

/*
 * Esc while typing a word puts back the keys as typed (EasyKey)
 * 0: No
 * 1: Yes
 */
int restoreOnEscape = 1;

/*
 * Password field raw bypass (EasyKey)
 * 0: No
 * 1: Yes, pass keys raw when focused on a password field
 */
int passwordFieldBypass = 1;
};

//the configuration of the most recent vKeyHandleEvent call
//(a default-constructed vKeyConfig before the first call)
const vKeyConfig& vKeyActiveConfig();

enum PasswordState {
    PW_NORMAL = 0,
    PW_PASSWORD = 1,
    PW_PENDING = 2
};

/**
 * Call this function first to receive data pointer
 */
void* vKeyInit();

/**
 * Convert engine character to real character
 */
Uint32 getCharacterCode(const Uint32& data);

/**
 * MAIN entry point for each key
 * event: mouse or keyboard event
 * state: additional state for event
 * data: key code
 * isCaps: caplock is on or shift key is pressing
 * otherControlKey: ctrl, option,... is pressing
 */
void vKeyHandleEvent(const vKeyEvent& event,
                     const vKeyEventState& state,
                     const Uint16& data,
                     const vKeyConfig& config,
                     const Uint8& capsStatus=0,
                     const bool& otherControlKey=false);

/**
 * Start a new word
 */
void startNewSession();

/**
 * do some task in english mode (use for macro)
 */
void vEnglishMode(const vKeyEventState& state, const Uint16& data, const bool& isCaps, const bool& otherControlKey);

/**
 * the next word starts a sentence, e.g. after switching to another window
 */
void vStartSentence();

/**
 * some utils function
 */
wstring utf8ToWideString(const string& str);
string wideStringToUtf8(const wstring& str);

#endif /* Engine_h */
