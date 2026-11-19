//
//  MixedModeDecider.h
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//
//  The mixed English and Vietnamese mode decision: at the end of a word,
//  decide whether it was English or Vietnamese from how common each reading
//  is and from the language of the words before it. This class owns all the
//  mixed-mode state; the engine gathers the inputs and executes the decision
//  (the restore/fix writes on the HookState and the line history stay there).
//

#ifndef MixedModeDecider_h
#define MixedModeDecider_h

#include <string>
#include <vector>
#include "DataType.h"

//the last word, when it could be read both ways and only one space follows it
struct MixedWord {
    vector<Uint32> keys, vietnamese;
    int score, contextBefore;
    bool english;
};

//token classifier for code-like identifiers, camelCase, URLs, and emails in Mixed mode
bool isCodeLikeToken(const vector<Uint32>& rawKeys, int inputType);

//everything the decision reads about the word that just ended; the raw and
//converted forms are borrowed for the duration of the call only
struct MixedWordInput {
    std::string keys;              //the current word as lower-case a-z (empty when it holds another character)
    bool allCaps;
    bool codeLike;                 //the token looks like an identifier
    bool changed;                  //the engine changed the raw keys
    bool spellingOk;               //post-check !tempDisableKey, not the _spellingOK flag
    bool valid;                    //false: the entry guard refused the word (see decide())
    bool spaceFollows;
    bool fixLastEnabled;           //config mixedFixLast
    bool doubleCode;               //IS_DOUBLE_CODE(codeTable): fix-last is forbidden there
    const Uint32* rawKeys; size_t rawCount;             //the current word, as typed
    const Uint32* vietnamese; size_t vietnameseCount;   //the current word, as converted
    size_t shownLength;            //_index before restore, in buffer units
};

//what the engine has to do with the word that just ended
struct MixedDecision {
    int language = 0;              //1 English, -1 Vietnamese, 0 can't tell
    bool clear = false;            //the decision is strong enough to settle the word before
    bool restore = false;          //show the current word as its raw keys
    bool fixLast = false;          //rewrite the word before as lastText
    std::vector<Uint32> lastText;  //the word before in its new form (fixLast only)
    Byte lastShownCount = 0;       //the shown length of the word before that the fix replaces (fixLast only)
};

class MixedModeDecider {
public:
    MixedDecision decide(const MixedWordInput& in);
    //the rejection path for a word the entry guard refused: only the stored
    //candidate loses its validity; phrase memory and context are kept
    void invalidateCandidate();
    void forgetLast();  //word break (keyboard/caret/control), second Space, Delete, successful Esc restore
    void forgetAll();   //forgetLast plus the context: only a Mouse word break
#ifdef EASYKEY_TEST
    //test-only inspection: tests/CMakeLists.txt defines EASYKEY_TEST for the
    //test targets only, so none of this exists in the app build; the three
    //accessors are const and return by value, they cannot mutate anything
    int mixedContextForTest() const { return _context; }
    bool hasMixedLastForTest() const { return _hasLast; }
    int lastRawLenForTest() const { return _lastRawLen; }
    //test fixture: installs a synthetic stored candidate (and clears the
    //phrase memory, exactly what the direct writes it replaces did)
    void setMixedLastForTest(const MixedWord& word);
#endif
private:
    //the language of the last words, from -100 (Vietnamese) to 100 (English)
    int _context = 0;
    MixedWord _last;
    bool _hasLast = false;
    char _lastRaw[16] = {0};
    Byte _lastRawLen = 0;
};

#endif /* MixedModeDecider_h */
