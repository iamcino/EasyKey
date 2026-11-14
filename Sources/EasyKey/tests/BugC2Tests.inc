// Bug C2 — Central Highlands names (design v2, re-gate CONFIRM).
//
// Data-driven rows for the engine suite. A mouse reset precedes every
// cell so mixed-English context starts neutral. Floor rows are master
// behavior and must pass before and after the engine change. Shape rows
// are the design's screen and Space / period / comma results.
//
// Modes: T = Telex, EV = Telex with mixed English, N = VNI.
// A null screen means the typed keys. A null boundary word means the
// screen form is what survives the boundary.

static const unsigned kC2T = 1;
static const unsigned kC2EV = 2;
static const unsigned kC2N = 4;
static const unsigned kC2Screen = 1;
static const unsigned kC2Space = 2;
static const unsigned kC2Dot = 4;
static const unsigned kC2Comma = 8;
static const unsigned kC2AllEnds = 15;
static const unsigned kC2SpellOff = 1;

struct BugC2Row {
    const char* keys;
    const wchar_t* screen;
    const wchar_t* boundary;
    unsigned modes;
    unsigned ends;
    unsigned opt;
};

static wstring bugC2Widen(const char* s) {
    wstring w;
    for (const char* p = s; *p; p++)
        w += (wchar_t)(unsigned char)*p;
    return w;
}

static vKeyConfig bugC2Config(unsigned mode, unsigned opt, int useMacro) {
    vKeyConfig c = testCfg();
    c.language = 1;
    c.inputType = (mode == kC2N) ? vVNI : vTelex;
    c.mixedEnglish = (mode == kC2EV) ? 1 : 0;
    c.mixedFixLast = 0;
    c.checkSpelling = (opt & kC2SpellOff) ? 0 : 1;
    c.restoreIfWrongSpelling = 1;
    c.freeMark = 0;
    c.useMacro = useMacro;
    c.useModernOrthography = 1;
    c.codeTable = 0;
    c.quickTelex = 0;
    c.quickStartConsonant = 0;
    c.quickEndConsonant = 0;
    c.upperCaseFirstChar = 0;
    return c;
}

static wstring typeBugC2(const string& keys, const vKeyConfig& config) {
    vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0, config, 0, false);
    return typeKeysWith(keys, config);
}

static void expectBugC2One(const char* section, const char* modeName, const char* ending,
                           const string& keys, const wstring& expected, const vKeyConfig& config) {
    total++;
    wstring actual = typeBugC2(keys, config);
    if (actual != expected) {
        failures++;
        printf("FAIL [BugC2 %s %s %s] \"%s\": expected \"%s\", got \"%s\"\n",
               section, modeName, ending, keys.c_str(),
               toUtf8(expected).c_str(), toUtf8(actual).c_str());
    }
}

static void runBugC2Rows(const char* section, const BugC2Row* rows, int count) {
    for (int r = 0; r < count; r++) {
        const BugC2Row& row = rows[r];
        wstring screen = row.screen ? wstring(row.screen) : bugC2Widen(row.keys);
        wstring word = row.boundary ? wstring(row.boundary) : screen;
        for (unsigned bit = kC2T; bit <= kC2N; bit <<= 1) {
            if (!(row.modes & bit))
                continue;
            const char* modeName = bit == kC2T ? "T" : bit == kC2EV ? "EV" : "N";
            vKeyConfig cfg = bugC2Config(bit, row.opt, 0);
            if (row.ends & kC2Screen)
                expectBugC2One(section, modeName, "screen", row.keys, screen, cfg);
            if (row.ends & kC2Space)
                expectBugC2One(section, modeName, "space", string(row.keys) + " ", word + L" ", cfg);
            if (row.ends & kC2Dot)
                expectBugC2One(section, modeName, "period", string(row.keys) + ".", word + L".", cfg);
            if (row.ends & kC2Comma)
                expectBugC2One(section, modeName, "comma", string(row.keys) + ",", word + L",", cfg);
        }
    }
}

static void runBugC2Literals(const char* section, const char** words, int count, unsigned modes) {
    for (int i = 0; i < count; i++) {
        BugC2Row row = {words[i], nullptr, nullptr, modes, kC2AllEnds, 0};
        runBugC2Rows(section, &row, 1);
    }
}

// The six cells of Ađrơng, caps from the surviving raw keys. A copy that
// started at TypingWord[-1] cannot produce this sequence.
static const Uint32 kC2AdRong[] = {
    KEY_A | CAPS_MASK, KEY_D | TONE_MASK, KEY_R, KEY_O | TONEW_MASK, KEY_N, KEY_G
};
static const Uint32 kC2AdRongSac[] = {
    KEY_A | CAPS_MASK, KEY_D | TONE_MASK, KEY_R, KEY_O | TONEW_MASK | MARK1_MASK, KEY_N, KEY_G
};
static const Uint32 kC2Krong[] = {
    KEY_K | CAPS_MASK, KEY_R, KEY_O | TONE_MASK, KEY_N, KEY_G
};
static const Uint32 kC2Soong[] = {KEY_S, KEY_O | TONE_MASK, KEY_N, KEY_G};
static const Uint32 kC2Blows[] = {KEY_B, KEY_L, KEY_O, KEY_W, KEY_S};
static const Uint32 kC2Pawh[] = {KEY_P | CAPS_MASK, KEY_A | TONEW_MASK, KEY_H};

struct BugC2Macro {
    const char* keys;
    unsigned mode;
    const wchar_t* screen;
    const wchar_t* boundary;
    const Uint32* cells;
    int ncells;
    int bpc;
    int ncc;
};

static void runBugC2Macros(const char* section, const BugC2Macro* rows, int count) {
    for (int r = 0; r < count; r++) {
        const BugC2Macro& row = rows[r];
        const char* modeName = row.mode == kC2T ? "T" : row.mode == kC2EV ? "EV" : "N";
        vKeyConfig cfg = bugC2Config(row.mode, 0, 1);
        wstring screen = typeBugC2(row.keys, cfg);
        total++;
        bool cellsOk = (int)pData->macroKey.size() == row.ncells;
        if (cellsOk) {
            for (int i = 0; i < row.ncells; i++) {
                if (pData->macroKey[i] != row.cells[i])
                    cellsOk = false;
            }
        }
        bool packetOk = (int)pData->backspaceCount == row.bpc && (int)pData->newCharCount == row.ncc;
        if (screen != row.screen || !cellsOk || !packetOk) {
            failures++;
            printf("FAIL [BugC2 %s %s macro] \"%s\": screen expected \"%s\" got \"%s\"; bpc/ncc expected %d/%d got %d/%d; macro",
                   section, modeName, row.keys, toUtf8(wstring(row.screen)).c_str(), toUtf8(screen).c_str(),
                   row.bpc, row.ncc, (int)pData->backspaceCount, (int)pData->newCharCount);
            for (size_t i = 0; i < pData->macroKey.size(); i++)
                printf(" %08X", pData->macroKey[i]);
            printf("\n");
        }
        wstring word = row.boundary ? wstring(row.boundary) : wstring(row.screen);
        expectBugC2One(section, modeName, "space", string(row.keys) + " ", word + L" ", cfg);
    }
}

static void testBugC2() {
    initMacroMap(nullptr, 0);

    // --- floor: master behavior, named neighbours and the measured literal set ---
    static const char* floorTEV[] = {
        "proof", "proofs", "practice", "prawn", "program", "approach",
        "prove", "proven", "proper", "protect", "problem", "product", "project",
        "promise", "proud", "print", "prince", "pray", "press", "price", "prime",
        "probe", "prong", "prongs", "pro", "promo", "spring", "spray", "spread",
        "spoon", "floor", "brood", "appropriate", "kpop", "kph", "kpa", "kappa",
        "backpack", "skip", "skype", "packed", "ok", "Kpow", "Draang", "Drang",
        "Andraang", "Addraang", "pah", "pahs", "pahf", "pahr", "pahx", "pahj",
        "path", "paths", "bath", "math", "bah", "hah", "nah", "ah", "spawn",
        "blah", "phone", "address", "Address", "range", "songw", "songws",
        "strong", "strongs", "bloom", "blows", "blown", "blood", "Blohs"
    };
    runBugC2Literals("floor", floorTEV, (int)(sizeof(floorTEV) / sizeof(floorTEV[0])), kC2T | kC2EV);

    static const char* floorN[] = {
        "practice", "prawn", "program", "approach", "proof", "prong", "prongs",
        "pah", "pahs", "path", "paws", "mawh", "Dra6ng", "Addra6ng"
    };
    runBugC2Literals("floor", floorN, (int)(sizeof(floorN) / sizeof(floorN[0])), kC2N);

    static const BugC2Row floorRows[] = {
        {"paws", L"p\u1eaf", nullptr, kC2T, kC2AllEnds, 0},
        {"paws", L"p\u1eaf", L"paws", kC2EV, kC2AllEnds, 0},
        {"Pawh<", L"P\u0103", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Pawh< ", L"P\u0103 ", nullptr, kC2T, kC2Screen, 0},
        {"Pawh< ", L"Paw ", nullptr, kC2EV, kC2Screen, 0},
        {"Pawhh", L"P\u0103hh", L"Pawhh", kC2T | kC2EV, kC2AllEnds, 0},
        {"apron", L"\u1ea3pon", L"apron", kC2T | kC2EV, kC2AllEnds, 0},
        {"april", L"\u1ea3pil", L"april", kC2T | kC2EV, kC2AllEnds, 0},
        {"troong", L"tr\u00f4ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"keep", L"k\u00eap", nullptr, kC2T, kC2AllEnds, 0},
        {"keep", L"k\u00eap", L"keep", kC2EV, kC2AllEnds, 0},
        {"aardvark", L"\u1ea9dvark", L"aardvark", kC2T | kC2EV, kC2AllEnds, 0},
        {"aaron", L"\u1ea9on", L"aaron", kC2T | kC2EV, kC2AllEnds, 0},
        {"extra", L"\u1ebdtra", L"extra", kC2T | kC2EV, kC2AllEnds, 0},
        {"mawh", L"m\u0103h", L"mawh", kC2T | kC2EV, kC2AllEnds, 0},
        {"lawh", L"l\u0103h", L"lawh", kC2T | kC2EV, kC2AllEnds, 0},
        {"bawh", L"b\u0103h", L"bawh", kC2T | kC2EV, kC2AllEnds, 0},
        {"kawh", L"k\u0103h", L"kawh", kC2T | kC2EV, kC2AllEnds, 0},
        {"tawh", L"t\u0103h", L"tawh", kC2T | kC2EV, kC2AllEnds, 0},
        {"Paah", L"P\u00e2h", L"Paah", kC2T | kC2EV, kC2AllEnds, 0},
        {"Puwh", L"P\u01b0h", L"Puwh", kC2T | kC2EV, kC2AllEnds, 0},
        {"Chuw Puwh", L"Ch\u01b0 P\u01b0h", L"Ch\u01b0 Puwh", kC2T | kC2EV, kC2AllEnds, 0},
        {"Paw", L"P\u0103", nullptr, kC2T, kC2AllEnds, 0},
        {"Paw", L"P\u0103", L"Paw", kC2EV, kC2AllEnds, 0},
        {"paw", L"p\u0103", nullptr, kC2T, kC2AllEnds, 0},
        {"paw", L"p\u0103", L"paw", kC2EV, kC2AllEnds, 0},
        {"Paws", L"P\u1eaf", nullptr, kC2T, kC2AllEnds, 0},
        {"Paws", L"P\u1eaf", L"Paws", kC2EV, kC2AllEnds, 0},
        {"pawn", L"p\u0103n", nullptr, kC2T, kC2AllEnds, 0},
        {"pawn", L"p\u0103n", L"pawn", kC2EV, kC2AllEnds, 0},
        {"Pawns", L"P\u1eafn", nullptr, kC2T, kC2AllEnds, 0},
        {"Pawns", L"P\u1eafn", L"Pawns", kC2EV, kC2AllEnds, 0},
        {"Pawf", L"P\u1eb1", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pawr", L"P\u1eb3", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pawx", L"P\u1eb5", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pawj", L"P\u1eb7", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Paww", L"Paw", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"phase", L"pha\u00e9", L"phase", kC2T | kC2EV, kC2AllEnds, 0},
        {"photo", L"ph\u00f4t", nullptr, kC2T, kC2AllEnds, 0},
        {"photo", L"ph\u00f4t", L"photo", kC2EV, kC2AllEnds, 0},
        {"dash", L"d\u00e1h", L"dash", kC2T | kC2EV, kC2AllEnds, 0},
        {"Bowh", L"B\u01a1h", L"Bowh", kC2T | kC2EV, kC2AllEnds, 0},
        {"Bowhs", L"B\u01a1hs", L"Bowhs", kC2T | kC2EV, kC2AllEnds, 0},
        {"Downgs", L"D\u1edbng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Do7ng1", L"D\u1edbng", nullptr, kC2N, kC2AllEnds, 0},
        {"Powng", L"P\u01a1ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Po7ng", L"P\u01a1ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Powngs", L"P\u1edbng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"soong", L"s\u00f4ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"so6ng", L"s\u00f4ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Kpaw", L"Kp\u0103", nullptr, kC2T, kC2AllEnds, kC2SpellOff},
        {"Kpaws", L"Kp\u1eaf", nullptr, kC2T, kC2AllEnds, kC2SpellOff},
        {"Adraang", L"Adr\u00e2ng", nullptr, kC2T, kC2AllEnds, kC2SpellOff},
        {"Adraangs", L"Adr\u1ea5ng", nullptr, kC2T, kC2AllEnds, kC2SpellOff},
        {"Proong", L"Pr\u00f4ng", nullptr, kC2T, kC2AllEnds, kC2SpellOff},
        {"Proongs", L"Pr\u1ed1ng", nullptr, kC2T, kC2AllEnds, kC2SpellOff},
        {"Pawh", L"P\u0103h", nullptr, kC2T, kC2AllEnds, kC2SpellOff},
    };
    runBugC2Rows("floor", floorRows, (int)(sizeof(floorRows) / sizeof(floorRows[0])));

    // Incomplete tokens stay raw (M2). These bytes are master and the design.
    static const BugC2Row earlyRows[] = {
        {"Kroon", nullptr, nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kroosng", nullptr, nullptr, kC2T, kC2AllEnds, 0},
        {"Drow", nullptr, nullptr, kC2T, kC2AllEnds, 0},
        {"Drown", nullptr, nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Adrow", nullptr, nullptr, kC2T, kC2AllEnds, 0},
        {"Adrown", nullptr, nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Blow", nullptr, nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Blowsh", nullptr, nullptr, kC2T, kC2AllEnds, 0},
        {"Kpa", nullptr, nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kpas", nullptr, nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Adraan", nullptr, nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Adraasng", nullptr, nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Proo", nullptr, nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Proon", nullptr, nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Proosng", nullptr, nullptr, kC2T | kC2EV, kC2AllEnds, 0},
    };
    runBugC2Rows("floor", earlyRows, (int)(sizeof(earlyRows) / sizeof(earlyRows[0])));

    // Macro controls that master already stores. soong and blows reject a
    // change to the non-negative copy. Pawh's three cells are the buffer
    // insertW already wrote; the space result is the design (admission).
    static const BugC2Macro macroFloor[] = {
        {"soong", kC2T, L"s\u00f4ng", nullptr, kC2Soong, 4, 0, 0},
        {"blows", kC2T, L"blows", nullptr, kC2Blows, 5, 0, 0},
        {"Pawh", kC2T, L"P\u0103h", L"P\u0103h", kC2Pawh, 3, 0, 0},
    };
    runBugC2Macros("macro-floor", macroFloor, 2);
    runBugC2Macros("macro", macroFloor + 2, 1);

    // --- shapes: design expectations (M1) ---
    static const BugC2Row shapes[] = {
        {"Kroong", L"Kr\u00f4ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"kroong", L"kr\u00f4ng", nullptr, kC2T, kC2AllEnds, 0},
        {"KROONG", L"KR\u00d4NG", nullptr, kC2T, kC2AllEnds, 0},
        {"Kro6ng", L"Kr\u00f4ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Kroongs", L"Kr\u1ed1ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kro6ng1", L"Kr\u1ed1ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Kroongf", L"Kr\u1ed3ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kro6ng2", L"Kr\u1ed3ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Kroongr", L"Kr\u1ed5ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kro6ng3", L"Kr\u1ed5ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Kroongx", L"Kr\u1ed7ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kro6ng4", L"Kr\u1ed7ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Kroongj", L"Kr\u1ed9ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kro6ng5", L"Kr\u1ed9ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Kroong Boong", L"Kr\u00f4ng B\u00f4ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kro6ng Bo6ng", L"Kr\u00f4ng B\u00f4ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Drowng", L"Dr\u01a1ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Dro7ng", L"Dr\u01a1ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Drowngs", L"Dr\u1edbng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Dro7ng1", L"Dr\u1edbng", nullptr, kC2N, kC2AllEnds, 0},
        {"Drowngf", L"Dr\u1eddng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Dro7ng2", L"Dr\u1eddng", nullptr, kC2N, kC2AllEnds, 0},
        {"Ddrowng", L"\u0110r\u01a1ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"ddrowng", L"\u0111r\u01a1ng", nullptr, kC2T, kC2AllEnds, 0},
        {"Ddrowngs", L"\u0110r\u1edbng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"D9ro7ng", L"\u0110r\u01a1ng", nullptr, kC2N, kC2AllEnds, 0},
        {"D9ro7ng1", L"\u0110r\u1edbng", nullptr, kC2N, kC2AllEnds, 0},
        {"Adrowng", L"Adr\u01a1ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Adro7ng", L"Adr\u01a1ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Adrowngs", L"Adr\u1edbng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Adro7ng1", L"Adr\u1edbng", nullptr, kC2N, kC2AllEnds, 0},
        {"adrowngs", L"adr\u1edbng", nullptr, kC2T, kC2AllEnds, 0},
        {"Addrowng", L"A\u0111r\u01a1ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Addrowngs", L"A\u0111r\u1edbng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Ad9ro7ng", L"A\u0111r\u01a1ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Ad9ro7ng1", L"A\u0111r\u1edbng", nullptr, kC2N, kC2AllEnds, 0},
        {"Blowh", L"Bl\u01a1h", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Blo7h", L"Bl\u01a1h", nullptr, kC2N, kC2AllEnds, 0},
        {"BLOWH", L"BL\u01a0H", nullptr, kC2T, kC2AllEnds, 0},
        {"Y Blowh", L"Y Bl\u01a1h", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Y Blo7h", L"Y Bl\u01a1h", nullptr, kC2N, kC2AllEnds, 0},
        {"y blowh", L"y bl\u01a1h", nullptr, kC2T, kC2AllEnds, 0},
        {"Blowhs", L"Bl\u1edbh", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Blo7h1", L"Bl\u1edbh", nullptr, kC2N, kC2AllEnds, 0},
        {"Blowhf", L"Bl\u1eddh", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Blo7h2", L"Bl\u1eddh", nullptr, kC2N, kC2AllEnds, 0},
        {"Blowhr", L"Bl\u1edfh", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Blo7h3", L"Bl\u1edfh", nullptr, kC2N, kC2AllEnds, 0},
        {"Blowhx", L"Bl\u1ee1h", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Blo7h4", L"Bl\u1ee1h", nullptr, kC2N, kC2AllEnds, 0},
        {"Blowhj", L"Bl\u1ee3h", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Blo7h5", L"Bl\u1ee3h", nullptr, kC2N, kC2AllEnds, 0},
        {"Kpaw", L"Kp\u0103", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"kpaw", L"kp\u0103", nullptr, kC2T, kC2AllEnds, 0},
        {"KPAW", L"KP\u0102", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kpa8", L"Kp\u0103", nullptr, kC2N, kC2AllEnds, 0},
        {"Kpaws", L"Kp\u1eaf", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kpa81", L"Kp\u1eaf", nullptr, kC2N, kC2AllEnds, 0},
        {"Kpawf", L"Kp\u1eb1", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kpa82", L"Kp\u1eb1", nullptr, kC2N, kC2AllEnds, 0},
        {"Kpawr", L"Kp\u1eb3", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kpa83", L"Kp\u1eb3", nullptr, kC2N, kC2AllEnds, 0},
        {"Kpawx", L"Kp\u1eb5", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kpa84", L"Kp\u1eb5", nullptr, kC2N, kC2AllEnds, 0},
        {"Kpawj", L"Kp\u1eb7", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kpa85", L"Kp\u1eb7", nullptr, kC2N, kC2AllEnds, 0},
        {"Adraang", L"Adr\u00e2ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"adraang", L"adr\u00e2ng", nullptr, kC2T, kC2AllEnds, 0},
        {"ADRAANG", L"ADR\u00c2NG", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Adra6ng", L"Adr\u00e2ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Adraangs", L"Adr\u1ea5ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Adra6ng1", L"Adr\u1ea5ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Adraangf", L"Adr\u1ea7ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Adra6ng2", L"Adr\u1ea7ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Adraangr", L"Adr\u1ea9ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Adra6ng3", L"Adr\u1ea9ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Adraangx", L"Adr\u1eabng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Adra6ng4", L"Adr\u1eabng", nullptr, kC2N, kC2AllEnds, 0},
        {"Adraangj", L"Adr\u1eadng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Adra6ng5", L"Adr\u1eadng", nullptr, kC2N, kC2AllEnds, 0},
        {"Proong", L"Pr\u00f4ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"proong", L"pr\u00f4ng", nullptr, kC2T, kC2AllEnds, 0},
        {"PROONG", L"PR\u00d4NG", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pro6ng", L"Pr\u00f4ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Proongs", L"Pr\u1ed1ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pro6ng1", L"Pr\u1ed1ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Proongf", L"Pr\u1ed3ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pro6ng2", L"Pr\u1ed3ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Proongr", L"Pr\u1ed5ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pro6ng3", L"Pr\u1ed5ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Proongx", L"Pr\u1ed7ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pro6ng4", L"Pr\u1ed7ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Proongj", L"Pr\u1ed9ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pro6ng5", L"Pr\u1ed9ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Chuw Proong", L"Ch\u01b0 Pr\u00f4ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Chu7 Pro6ng", L"Ch\u01b0 Pr\u00f4ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Chuw Proongs", L"Ch\u01b0 Pr\u1ed1ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Chu7 Pro6ng1", L"Ch\u01b0 Pr\u1ed1ng", nullptr, kC2N, kC2AllEnds, 0},
        {"Pawh", L"P\u0103h", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"pawh", L"p\u0103h", nullptr, kC2T, kC2AllEnds, 0},
        {"PAWH", L"P\u0102H", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pa8h", L"P\u0103h", nullptr, kC2N, kC2AllEnds, 0},
        {"Pawhs", L"P\u1eafh", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pa8h1", L"P\u1eafh", nullptr, kC2N, kC2AllEnds, 0},
        {"Pawhf", L"P\u1eb1h", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pa8h2", L"P\u1eb1h", nullptr, kC2N, kC2AllEnds, 0},
        {"Pawhr", L"P\u1eb3h", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pa8h3", L"P\u1eb3h", nullptr, kC2N, kC2AllEnds, 0},
        {"Pawhx", L"P\u1eb5h", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pa8h4", L"P\u1eb5h", nullptr, kC2N, kC2AllEnds, 0},
        {"Pawhj", L"P\u1eb7h", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Pa8h5", L"P\u1eb7h", nullptr, kC2N, kC2AllEnds, 0},
        {"Chuw Pawh", L"Ch\u01b0 P\u0103h", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Chu7 Pa8h", L"Ch\u01b0 P\u0103h", nullptr, kC2N, kC2AllEnds, 0},
        {"Chuw Pawhs", L"Ch\u01b0 P\u1eafh", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Chu7 Pa8h1", L"Ch\u01b0 P\u1eafh", nullptr, kC2N, kC2AllEnds, 0},
        {"Pawsh", L"P\u1eafh", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kroong blows", L"Kr\u00f4ng blows", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kroong Kroong", L"Kr\u00f4ng Kr\u00f4ng", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Blowh address", L"Bl\u01a1h address", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"Kroong rooms", L"Kr\u00f4ng r\u1ed1m", nullptr, kC2T, kC2AllEnds, 0},
        {"Kroong rooms", L"Kr\u00f4ng r\u1ed1m", L"Kr\u00f4ng rooms", kC2EV, kC2AllEnds, 0},
        {"Kroong wrong", L"Kr\u00f4ng \u01b0\u1edfng", nullptr, kC2T, kC2AllEnds, 0},
        {"Kroong wrong", L"Kr\u00f4ng \u01b0\u1edfng", L"Kr\u00f4ng wrong", kC2EV, kC2AllEnds, 0},
    };
    runBugC2Rows("shape", shapes, (int)(sizeof(shapes) / sizeof(shapes[0])));

    // M3 residues. Pawh's remainder is floor and is covered above.
    static const BugC2Row residues[] = {
        {"Kroong<", L"Kr\u00f4n", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Kroong< ", L"Kroon ", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Kroongs< ", L"Kroong ", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Blowh< ", L"Blow ", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Addrowng< ", L"Addrown ", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Ad9ro7ng< ", L"Ad9ro7n ", nullptr, kC2N, kC2Screen, 0},
        {"Kpaw<", L"Kp", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Kpaw< ", L"Kp ", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Adraang<", L"Adr\u00e2n", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Adraang< ", L"Adraan ", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Proong<", L"Pr\u00f4n", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Proong< ", L"Proon ", nullptr, kC2T | kC2EV, kC2Screen, 0},
    };
    runBugC2Rows("residue", residues, (int)(sizeof(residues) / sizeof(residues[0])));

    // M4 longer-token flicker. Pawhh is floor and is covered above.
    static const BugC2Row flicker[] = {
        {"Kroongn", L"Kr\u00f4ngn", L"Kroongn", kC2T | kC2EV, kC2AllEnds, 0},
        {"Kroongg", L"Kr\u00f4ngg", L"Kroongg", kC2T | kC2EV, kC2AllEnds, 0},
        {"Blowhh", L"Bl\u01a1hh", L"Blowhh", kC2T | kC2EV, kC2AllEnds, 0},
        {"Kpawg", L"Kp\u0103g", L"Kpawg", kC2T | kC2EV, kC2AllEnds, 0},
        {"Adraangg", L"Adr\u00e2ngg", L"Adraangg", kC2T | kC2EV, kC2AllEnds, 0},
        {"Proongg", L"Pr\u00f4ngg", L"Proongg", kC2T | kC2EV, kC2AllEnds, 0},
    };
    runBugC2Rows("flicker", flicker, (int)(sizeof(flicker) / sizeof(flicker[0])));

    // M5 mixed-case. Screen converts; a lower-to-upper raw transition
    // restores at an EV boundary. All-caps keeps the name.
    static const BugC2Row mixed[] = {
        {"kRoongS", L"kR\u1ed1ng", nullptr, kC2T, kC2AllEnds, 0},
        {"kRoongS", L"kR\u1ed1ng", L"kRoongS", kC2EV, kC2AllEnds, 0},
        {"KroongS", L"Kr\u1ed1ng", nullptr, kC2T, kC2AllEnds, 0},
        {"KroongS", L"Kr\u1ed1ng", L"KroongS", kC2EV, kC2AllEnds, 0},
        {"KROONGS", L"KR\u1ed0NG", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
        {"ProongS", L"Pr\u1ed1ng", nullptr, kC2T, kC2AllEnds, 0},
        {"ProongS", L"Pr\u1ed1ng", L"ProongS", kC2EV, kC2AllEnds, 0},
        {"pRoongS", L"pR\u1ed1ng", nullptr, kC2T, kC2AllEnds, 0},
        {"pRoongS", L"pR\u1ed1ng", L"pRoongS", kC2EV, kC2AllEnds, 0},
        {"PROONGS", L"PR\u1ed0NG", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
    };
    runBugC2Rows("mixed", mixed, (int)(sizeof(mixed) / sizeof(mixed[0])));

    // M6. The 7/6 rows must read a six-cell buffer whose last cell is G.
    static const BugC2Macro macros[] = {
        {"Addrowng", kC2T, L"A\u0111r\u01a1ng", nullptr, kC2AdRong, 6, 7, 6},
        {"Addrowng", kC2EV, L"A\u0111r\u01a1ng", nullptr, kC2AdRong, 6, 7, 6},
        {"Ad9ro7ng", kC2N, L"A\u0111r\u01a1ng", nullptr, kC2AdRong, 6, 7, 6},
        {"Addrowngs", kC2T, L"A\u0111r\u1edbng", nullptr, kC2AdRongSac, 6, -1, -1},
        {"Addrowngs", kC2EV, L"A\u0111r\u1edbng", nullptr, kC2AdRongSac, 6, -1, -1},
        {"Kroong", kC2T, L"Kr\u00f4ng", nullptr, kC2Krong, 5, 5, 5},
    };
    // Addrowngs's last event is the tone, not the 7/6 rewrite. Skip that packet.
    runBugC2Macros("macro", macros, 3);
    for (int r = 3; r <= 4; r++) {
        BugC2Macro tone = macros[r];
        const char* modeName = tone.mode == kC2T ? "T" : "EV";
        vKeyConfig cfg = bugC2Config(tone.mode, 0, 1);
        wstring screen = typeBugC2(tone.keys, cfg);
        total++;
        bool cellsOk = (int)pData->macroKey.size() == tone.ncells;
        if (cellsOk) {
            for (int i = 0; i < tone.ncells; i++) {
                if (pData->macroKey[i] != tone.cells[i])
                    cellsOk = false;
            }
        }
        if (screen != tone.screen || !cellsOk) {
            failures++;
            printf("FAIL [BugC2 macro %s tone] \"%s\": screen expected \"%s\" got \"%s\"; macro",
                   modeName, tone.keys, toUtf8(wstring(tone.screen)).c_str(), toUtf8(screen).c_str());
            for (size_t i = 0; i < pData->macroKey.size(); i++)
                printf(" %08X", pData->macroKey[i]);
            printf("\n");
        }
        expectBugC2One("macro", modeName, "space", string(tone.keys) + " ", wstring(tone.screen) + L" ", cfg);
    }
    runBugC2Macros("macro", macros + 5, 1);

    // M7 spelling-off guards, and the restore-then-tone sequences.
    // Kpaw / Adraang / Proong / Pawh spelling-off are floor (above).
    static const BugC2Row spell[] = {
        {"Blowhs", L"Bl\u1edbh", nullptr, kC2T, kC2AllEnds, kC2SpellOff},
        {"Pawhs", L"P\u1eafh", nullptr, kC2T, kC2AllEnds, kC2SpellOff},
        {"Blowh <s", L"Bl\u1edbh", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Kroong <s", L"Kr\u1ed1ng", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Pawh <s", L"P\u1eafh", nullptr, kC2T | kC2EV, kC2Screen, 0},
        {"Kpaww", L"Kpaw", nullptr, kC2T | kC2EV, kC2AllEnds, 0},
    };
    runBugC2Rows("spell", spell, (int)(sizeof(spell) / sizeof(spell[0])));

    testCfg().mixedEnglish = 0;
    testCfg().inputType = vTelex;
    testCfg().checkSpelling = 1;
    testCfg().useMacro = 0;
    initMacroMap(nullptr, 0);
}
