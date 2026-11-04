//
//  Macro.cpp
//  OpenKey
//
//  Created by Tuyen on 8/4/19.
//  Copyright © 2019 Tuyen Mai. All rights reserved.
//
//  Part of EasyKey, a modified version of OpenKey (https://github.com/tuyenvm/OpenKey).
//  SPDX-License-Identifier: GPL-3.0-only
//

#include "Macro.h"
#include "Vietnamese.h"
#include "Engine.h"

#include <memory.h>
#include <stdio.h>

using namespace std;

//main data
map<vector<Uint32>, MacroData> macroMap;

extern int vCodeTable;
//local variable
static int c = 0;
static bool _macroFlag = false;
static Uint16 _kChar = 0;
static Uint32 _charBuff;
static int _kMacro;

static void convert(const string& str, vector<Uint32>& outData) {
    outData.clear();
    wstring data = utf8ToWideString(str);
    Uint32 t = 0;
    int kSign = -1;
    int k = 0;
    for (int i = 0; i < data.size(); i++) {
        t = (Uint32)data[i];
        
        //find normal character fist
        if (_characterMap.find(t) != _characterMap.end()) {
            outData.push_back(_characterMap[t]);
            continue;
        }
        
        //find character which has tone/mark
        for (map<Uint32, vector<Uint16>>::iterator it = _codeTable[0].begin(); it != _codeTable[0].end(); ++it) {
            kSign = -1;
            k = 0;
            for (int j = 0; j < it->second.size(); j++) {
                if ((Uint16)t == it->second[j]) {
                    kSign = 0;
                    outData.push_back(_codeTable[vCodeTable][it->first][k] | CHAR_CODE_MASK);
                    break;
                }//end if
                k++;
            }
            if (kSign != -1)
                break;
        }
        if (kSign != -1)
            continue;
        
        //find other character
        outData.push_back(t | PURE_CHARACTER_MASK); //mark it as pure character
    }
}

/**
 * data structure:
 * byte 0 and 1: macro count
 *
 * byte n: macroText size (macroTextSize)
 * byte n + macroTextSize: macroText data
 *
 * byte m, m+1: macroContentSize
 * byte m+1 + macroContentSize: macroContent data
 *
 * ...
 * next macro
 */
void initMacroMap(const Byte* pData, const int& size) {
    macroMap.clear();
    Uint16 macroCount = 0;
    Uint32 cursor = 0;
    if (size >= 2) {
        memcpy(&macroCount, pData + cursor, 2);
        cursor+=2;
    }
    Uint8 macroTextSize;
    Uint16 macroContentSize;
    for (int i = 0; i < macroCount; i++) {
        //corrupt or truncated data: keep the macros that are still whole,
        //drop the cut one and stop (found by the step 3.4 sanitizer sweep)
        if ((int)cursor + 1 > size)
            break;
        macroTextSize = pData[cursor++];
        if ((int)cursor + macroTextSize + 2 > size)
            break;
        string macroText((char*)pData + cursor, macroTextSize);
        cursor += macroTextSize;

        memcpy(&macroContentSize, pData + cursor, 2);
        cursor+=2;
        if ((int)cursor + macroContentSize > size)
            break;
        string macroContent((char*)pData + cursor, macroContentSize);
        cursor += macroContentSize;
        
        MacroData data;
        data.macroText = macroText;
        data.macroContent = macroContent;
        
        vector<Uint32> key;
        convert(macroText, key);
        convert(macroContent, data.macroContentCode);
        
        macroMap[key] = data;
    }
}

void getMacroSaveData(vector<Byte>& outData) {
    //same rule as the smart switch save data: an entry the format cannot
    //represent (name over the one-byte length, content over the two-byte
    //length) is skipped instead of truncating its length and corrupting the
    //entries after it (finding F7); the count covers the written entries
    vector<Byte> body;
    Uint16 totalMacro = 0;
    for (std::map<vector<Uint32>, MacroData>::iterator it = macroMap.begin(); it != macroMap.end() && totalMacro < 0xFFFF; ++it) {
        if (it->second.macroText.size() > 0xFF || it->second.macroContent.size() > 0xFFFF)
            continue;
        body.push_back((Byte)it->second.macroText.size());
        for (int j = 0; j < it->second.macroText.size(); j++) {
            body.push_back(it->second.macroText[j]);
        }
        
        Uint16 macroContentSize = (Uint16)it->second.macroContent.size();
        body.push_back((Byte)macroContentSize);
        body.push_back(macroContentSize>>8);
        for (int j = 0; j < macroContentSize; j++) {
            body.push_back(it->second.macroContent[j]);
        }
        totalMacro++;
    }
    outData.push_back((Byte)totalMacro);
    outData.push_back((Byte)(totalMacro>>8));
    outData.insert(outData.end(), body.begin(), body.end());
}

static bool modifyCaseUnicode(Uint32& code, const bool& isUpperCase=true) {
    _charBuff = code;
    if (!(code & CHAR_CODE_MASK)) { //for normal char
        code &= isUpperCase ? CAPS_MASK :  ~CAPS_MASK;
        return code != _charBuff;
    }
    
    //for unicode character
    for (map<Uint32, vector<Uint16>>::iterator it = _codeTable[vCodeTable].begin(); it != _codeTable[vCodeTable].end(); ++it) {
        for (_kMacro = 0; _kMacro < it->second.size(); _kMacro++) {
            if ((Uint16)code == it->second[_kMacro]) {
                if (_kMacro % 2 == 0 && !isUpperCase)
                    _kMacro++;
                else if (_kMacro % 2 != 0 && isUpperCase)
                    _kMacro--;
                code = _codeTable[vCodeTable][it->first][_kMacro] | CHAR_CODE_MASK;
                return code != _charBuff;;
            }//end if
        }
    }
    return false;
}

bool findMacro(vector<Uint32>& key, vector<Uint32>& macroContentCode) {
    for (c = 0; c < key.size(); c++) {
        key[c] = getCharacterCode(key[c]);
    }
    if (macroMap.find(key) != macroMap.end()) {
        macroContentCode.clear();
        MacroData data = macroMap[key];
        macroContentCode = data.macroContentCode;
        return true;
    }
    if (vAutoCapsMacro) {
        _macroFlag = false;
        if (key.size() > 1 && modifyCaseUnicode(key[1], false)) {
            _macroFlag = true;
            for (c = 2; c < key.size(); c++) {
                modifyCaseUnicode(key[c], false);
            }
        }
        
        if (key.size() > 0 && modifyCaseUnicode(key[0], false)) {
            if (macroMap.find(key) != macroMap.end()) {
                macroContentCode.clear();
                MacroData data = macroMap[key];
                macroContentCode = data.macroContentCode;
                for (c = 0; c < macroContentCode.size(); c++) {
                    if (c == 0 || _macroFlag) {
                        _kChar = keyCodeToCharacter(macroContentCode[c]);
                        if (_kChar != 0) {
                            _kChar = toupper(_kChar);
                            macroContentCode[c] = _characterMap[_kChar];
                            continue;
                        }
                        if (macroContentCode[c] & CHAR_CODE_MASK) {
                            modifyCaseUnicode(macroContentCode[c]);
                        }
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool hasMacro(const string& macroName) {
    vector<Uint32> key;
    convert(macroName, key);
    return (macroMap.find(key) != macroMap.end());
}

void getAllMacro(vector<vector<Uint32>>& keys, vector<string>& macroTexts, vector<string>& macroContents) {
    keys.clear();
    macroTexts.clear();
    macroContents.clear();
    for (std::map<vector<Uint32>, MacroData>::iterator it = macroMap.begin(); it != macroMap.end(); ++it) {
        keys.push_back(it->first);
        macroTexts.push_back(it->second.macroText);
        macroContents.push_back(it->second.macroContent);
    }
}

bool addMacro(const string& macroText, const string& macroContent) {
    vector<Uint32> key;
    convert(macroText, key);
    if (macroMap.find(key) == macroMap.end()) { //add new macro
        MacroData data;
        data.macroText = macroText;
        data.macroContent = macroContent;
        convert(macroContent, data.macroContentCode);
        macroMap[key] = data;
    } else { //edit this macro
        macroMap[key].macroContent = macroContent;
        convert(macroContent, macroMap[key].macroContentCode);
    }
    return true;
}

bool deleteMacro(const string& macroText) {
    vector<Uint32> key;
    convert(macroText, key);
    if (macroMap.find(key) != macroMap.end()) {
        macroMap.erase(key);
        return true;
    }
    return false;
}

void onTableCodeChange() {
    for (std::map<vector<Uint32>, MacroData>::iterator it = macroMap.begin(); it != macroMap.end(); ++it) {
        convert(it->second.macroContent, it->second.macroContentCode);
    }
}

void saveToFile(const string& path) {
#ifdef _WIN32
    FILE* myfile = NULL;
    if (_wfopen_s(&myfile, utf8ToWideString(path).c_str(), L"w") != 0 || !myfile)
        return;
#else
    FILE* myfile = fopen(path.c_str(), "w");
    if (!myfile)
        return;
#endif

    fputs(";Compatible OpenKey Macro Data file for UniKey*** version=1 ***\n", myfile);
    for (std::map<vector<Uint32>, MacroData>::iterator it = macroMap.begin(); it != macroMap.end(); ++it) {
        fputs(it->second.macroText.c_str(), myfile);
        fputc(':', myfile);
        fputs(it->second.macroContent.c_str(), myfile);
        fputc('\n', myfile);
    }
    fclose(myfile);
}

void readFromFile(const string& path, const bool& append) {
#ifdef _WIN32
    FILE* myfile = NULL;
    if (_wfopen_s(&myfile, utf8ToWideString(path).c_str(), L"r") != 0 || !myfile)
        return;
#else
    FILE* myfile = fopen(path.c_str(), "r");
    if (!myfile)
        return;
#endif

    if (!append) {
        macroMap.clear();
    }

    auto parseLine = [](int lineNum, const string& line) {
        if (lineNum <= 1) return;
        size_t pos = line.find(":");
        if (string::npos != pos) {
            string name = line.substr(0, pos);
            string content = line.substr(pos + 1);
            while (name.empty() && !content.empty()) {
                pos = content.find(":");
                if (string::npos != pos) {
                    name += ":";
                    name += content.substr(0, pos);
                    content = content.substr(pos + 1);
                } else {
                    break;
                }
            }

            if (!name.empty() && !hasMacro(name)) {
                addMacro(name, content);
            }
        }
    };

    string line;
    int k = 0;
    int ch;
    while ((ch = fgetc(myfile)) != EOF) {
        if (ch == '\n') {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            k++;
            parseLine(k, line);
            line.clear();
        } else {
            line.push_back((char)ch);
        }
    }
    if (!line.empty()) {
        if (line.back() == '\r') {
            line.pop_back();
        }
        k++;
        parseLine(k, line);
    }
    fclose(myfile);
}
