/*----------------------------------------------------------
OpenKey - The Cross platform Open source Vietnamese Keyboard application.

Copyright (C) 2019 Mai Vu Tuyen
Contact: maivutuyen.91@gmail.com
Github: https://github.com/tuyenvm/OpenKey
Fanpage: https://www.facebook.com/OpenKeyVN

EasyKey - Vietnamese input for Windows 11, based on OpenKey.
Copyright (C) 2026 Cino

This file is belong to the OpenKey project, Win32 version
which is released under GPL license.
You can fork, modify, improve this program. If you
redistribute your new version, it MUST be open source.
-----------------------------------------------------------*/
#include "EasyKeyHelper.h"
#include <stdarg.h>
#include <cwctype>
#include <Shlobj.h>
#include <tlhelp32.h>
#include <map>

#pragma comment(lib, "version.lib")

static BYTE* _regData = 0;

static LPCTSTR sk = TEXT("SOFTWARE\\EasyKey");
static LPCTSTR _openKeyRegPath = TEXT("SOFTWARE\\TuyenMai\\OpenKey"); //settings of OpenKey, copied once on first run
static HKEY hKey;
static bool _firstRun = false;
static LPCTSTR _runOnStartupKeyPath = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
static TCHAR _executePath[MAX_PATH];
static bool _hasGetPath = false;

static DWORD _cacheProcessId = 0, _tempProcessId = 0;
static HWND _tempWnd;
static TCHAR _exePath[1024] = { 0 };
static LPCTSTR _exeName = _exePath;
static HANDLE _proc;
static string _exeNameUtf8 = "TheEasyKeyProject";
static string _unknownProgram = "UnknownProgram";

int CF_RTF = RegisterClipboardFormat(_T("Rich Text Format"));
int CF_HTML = RegisterClipboardFormat(_T("HTML Format"));
int CF_EASYKEY = RegisterClipboardFormat(_T("EasyKey Format"));

void EasyKeyHelper::openRegKey() {
	LONG nError = RegOpenKeyEx(HKEY_CURRENT_USER, sk, NULL, KEY_ALL_ACCESS, &hKey);
	if (nError == ERROR_FILE_NOT_FOUND) 	{
		nError = RegCreateKeyEx(HKEY_CURRENT_USER, sk, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
		if (nError == ERROR_SUCCESS) {
			_firstRun = true;
			//first run: keep the settings and macros of an existing OpenKey install
			RegCopyTree(HKEY_CURRENT_USER, _openKeyRegPath, hKey);
		}
	}
	if (nError) {
		LOG(L"result %d\n", nError);
	}
}

void EasyKeyHelper::setRegInt(LPCTSTR key, const int & val) {
	openRegKey();
	RegSetValueEx(hKey, key, 0, REG_DWORD, (LPBYTE)&val, sizeof(val));
	RegCloseKey(hKey);
}

int EasyKeyHelper::getRegInt(LPCTSTR key, const int & defaultValue) {
	openRegKey();
	int val = defaultValue;
	DWORD size = sizeof(val);
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, key, 0, 0, (LPBYTE)&val, &size)) {
		val = defaultValue;
	}
	RegCloseKey(hKey);
	return val;
}

void EasyKeyHelper::setRegBinary(LPCTSTR key, const BYTE * pData, const int & size) {
	openRegKey();
	RegSetValueEx(hKey, key, 0, REG_BINARY, pData, size);
	RegCloseKey(hKey);
}

BYTE * EasyKeyHelper::getRegBinary(LPCTSTR key, DWORD& outSize) {
	openRegKey();
	if (_regData) {
		delete[] _regData;
		_regData = NULL;
	}
	DWORD size = 0;
	RegQueryValueEx(hKey, key, 0, 0, 0, &size);
	_regData = new BYTE[size];
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, key, 0, 0, _regData, &size)) {
		delete[] _regData;
		_regData = NULL;
	}
	outSize = size;
	RegCloseKey(hKey);
	return _regData;
}

void EasyKeyHelper::registerRunOnStartup(const int& val) {
	if (val) {
		if (vRunAsAdmin) {
			string path = wideStringToUtf8(getFullPath());
			char buff[MAX_PATH];
			sprintf_s(buff, "schtasks /create /sc onlogon /tn EasyKey /rl highest /tr \"%s\" /f", path.c_str());
			WinExec(buff, SW_HIDE);
		} else {
			RegOpenKeyEx(HKEY_CURRENT_USER, _runOnStartupKeyPath, NULL, KEY_ALL_ACCESS, &hKey);
			wstring path = getFullPath();
			RegSetValueEx(hKey, _T("EasyKey"), 0, REG_SZ, (byte*)path.c_str(), ((DWORD)path.size() + 1) * sizeof(TCHAR));
			RegCloseKey(hKey);
		}
	} else {
		RegOpenKeyEx(HKEY_CURRENT_USER, _runOnStartupKeyPath, NULL, KEY_ALL_ACCESS, &hKey);
		RegDeleteValue(hKey, _T("EasyKey"));
		RegCloseKey(hKey);
		WinExec("schtasks /delete  /tn EasyKey /f", SW_HIDE);
	}
}

LPTSTR EasyKeyHelper::getExecutePath() {
	if (!_hasGetPath) {
		HMODULE hModule = GetModuleHandleW(NULL);
		GetModuleFileNameW(hModule, _executePath, MAX_PATH);
		_hasGetPath = true;
	}
	return _executePath;
}

string& EasyKeyHelper::getFrontMostAppExecuteName() {
	_tempWnd = GetForegroundWindow();
	GetWindowThreadProcessId(_tempWnd, &_tempProcessId);
	if (_tempProcessId == _cacheProcessId) {
		return _exeNameUtf8;
	}
	_cacheProcessId = _tempProcessId;
	_proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, _tempProcessId);
	GetProcessImageFileName((HMODULE)_proc, _exePath, 1024);
	CloseHandle(_proc);
	
	if (wcscmp(_exePath, _T("")) == 0) {
		return _unknownProgram;
	}
	_exeName = _tcsrchr(_exePath, '\\') + 1;
	if (wcscmp(_exeName, _T("EasyKey64.exe")) == 0 ||
		wcscmp(_exeName, _T("EasyKey32.exe")) == 0 || 
		wcscmp(_exeName, _T("explorer.exe")) == 0) {
		return _exeNameUtf8;
	}
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, _exeName, (int)lstrlen(_exeName), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, _exeName, (int)lstrlen(_exeName), &strTo[0], size_needed, NULL, NULL);
	_exeNameUtf8 = strTo;
	//LOG(L"%s\n", utf8ToWideString(_exeNameUtf8).c_str());
	return _exeNameUtf8;
}

string & EasyKeyHelper::getLastAppExecuteName() {
	if (!vUseSmartSwitchKey)
		return getFrontMostAppExecuteName();
	return _exeNameUtf8;
}

wstring EasyKeyHelper::getFullPath() {
	HMODULE hModule = GetModuleHandle(NULL);
	TCHAR path[MAX_PATH];
	GetModuleFileName(hModule, path, MAX_PATH);
	wstring rs(path);
	return rs;
}

wstring EasyKeyHelper::getClipboardText(const int& type) {
	// Try opening the clipboard
	if (!OpenClipboard(nullptr)) {
		return _T("");
	}

	// Get handle of clipboard object for ANSI text
	HANDLE hData = GetClipboardData(type);
	if (hData == nullptr) {
		return _T("");
	}

	// Lock the handle to get the actual text pointer
	wchar_t * pszText = static_cast<wchar_t*>(GlobalLock(hData));
	if (pszText == nullptr) {
		return _T("");
	}

	// Save text in a string class instance
	wstring text(pszText);
	
	// Release the lock
	GlobalUnlock(hData);

	// Release the clipboard
	CloseClipboard();
	
	return text;
}

void EasyKeyHelper::setClipboardText(LPCTSTR data, const int & len, const int& type) {
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(WCHAR));
	memcpy(GlobalLock(hMem), data, len * sizeof(WCHAR));
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(type, hMem);
	CloseClipboard();
}

bool EasyKeyHelper::quickConvert() {
	//read data from clipboard
	//support Unicode raw string, Rich Text Format and HTML

	if (!OpenClipboard(nullptr)) {
		return false;
	}

	string dataHTML, dataRTF;
	wstring dataUnicode;

	char* pHTML = 0, pRTF = 0;
	wchar_t* pUnicode = 0;

	//HTML
	HANDLE hData = GetClipboardData(CF_HTML);
	if (hData) {
		pHTML = static_cast<char*>(GlobalLock(hData));
		GlobalUnlock(hData);
	}
	if (pHTML) {
		dataHTML = pHTML;
		dataHTML = convertUtil(dataHTML);
	}

	//UNICODE
	hData = GetClipboardData(CF_UNICODETEXT);
	if (hData) {
		pUnicode = static_cast<wchar_t*>(GlobalLock(hData));
		GlobalUnlock(hData);
	}
	if (pUnicode) {
		dataUnicode = pUnicode;
		dataUnicode = utf8ToWideString(convertUtil(wideStringToUtf8(dataUnicode)));
	}

	OpenClipboard(0);
	EmptyClipboard();

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (int)(dataHTML.size() + 1) * sizeof(char));
	memcpy(GlobalLock(hMem), dataHTML.c_str(), (int)(dataHTML.size() + 1) * sizeof(char));
	GlobalUnlock(hMem);
	SetClipboardData(CF_HTML, hMem);

	hMem = GlobalAlloc(GMEM_MOVEABLE, (int)(dataUnicode.size() + 1) * sizeof(wchar_t));
	memcpy(GlobalLock(hMem), dataUnicode.c_str(), (int)(dataUnicode.size() + 1) * sizeof(wchar_t));
	GlobalUnlock(hMem);
	SetClipboardData(CF_UNICODETEXT, hMem);

	CloseClipboard();
	return true;
}

DWORD EasyKeyHelper::getVersionNumber() {
	// get the filename of the executable containing the version resource
	TCHAR szFilename[MAX_PATH + 1] = { 0 };
	if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0) {
		return 0;
	}

	// allocate a block of memory for the version info
	DWORD dummy;
	UINT dwSize = GetFileVersionInfoSize(szFilename, &dummy);
	if (dwSize == 0) {
		return 0;
	}
	std::vector<BYTE> data(dwSize);

	// load the version info
	if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0])) {
		return 0;
	}

	LPBYTE lpBuffer = NULL;

	if (VerQueryValue(&data[0], _T("\\"), (VOID FAR * FAR*) & lpBuffer, &dwSize)) {
		if (dwSize) {
			VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
			if (verInfo->dwSignature == 0xfeef04bd) {
				return ((verInfo->dwFileVersionMS >> 16) & 0xffff) |
					(((verInfo->dwFileVersionMS >> 0) & 0xffff) << 8) |
					(((verInfo->dwFileVersionLS >> 16) & 0xffff) << 16);
			}
		}
	}

	return 0;
}

wstring EasyKeyHelper::getVersionString() {
	TCHAR versionBuffer[MAX_PATH];
	DWORD ver = getVersionNumber();
	wsprintfW(versionBuffer, _T("%d.%d.%d"), ver & 0xFF, (ver>>8) & 0xFF, (ver >> 16) & 0xFF);
	return wstring(versionBuffer);

	// get the filename of the executable containing the version resource
	TCHAR szFilename[MAX_PATH + 1] = { 0 };
	if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0) { 
		return _T("");
	}
}

wstring EasyKeyHelper::getHotKeyString(const int& hotKey) {
	wstring text;
	static const struct { int mask; LPCTSTR name; } modifiers[] = {
		{ 0x100, _T("Ctrl") }, { 0x200, _T("Alt") }, { 0x400, _T("Win") }, { 0x800, _T("Shift") }
	};
	for (int i = 0; i < 4; i++) {
		if (hotKey & modifiers[i].mask) {
			if (!text.empty())
				text += _T(" + ");
			text += modifiers[i].name;
		}
	}
	unsigned short key = GET_SWITCH_KEY(hotKey);
	if (key != 0xFE && key != 0) {
		if (!text.empty())
			text += _T(" + ");
		if (key == VK_SPACE) {
			text += _T("Space");
		} else if (wchar_t c = (wchar_t)keyCodeToCharacter(key)) {
			text += (wchar_t)towupper(c);
		} else {
			TCHAR name[32] = { 0 };
			GetKeyNameText(MapVirtualKey(key, MAPVK_VK_TO_VSC) << 16, name, 32);
			text += name[0] ? name : _T("?");
		}
	}
	return text;
}

bool EasyKeyHelper::isEnglishUi() {
	if (vUiLanguage == 1)
		return false;
	if (vUiLanguage == 2)
		return true;
	return PRIMARYLANGID(GetUserDefaultUILanguage()) != LANG_VIETNAMESE;
}

bool EasyKeyHelper::isWindowElevated(const HWND& hWnd) {
	static DWORD cachedProcess = 0;
	static bool cachedResult = false;
	DWORD processId = 0;
	GetWindowThreadProcessId(hWnd, &processId);
	if (processId == 0 || processId == GetCurrentProcessId())
		return false;
	if (processId == cachedProcess)
		return cachedResult;

	auto integrity = [](const HANDLE& process, DWORD& level) {
		HANDLE token = NULL;
		if (!OpenProcessToken(process, TOKEN_QUERY, &token))
			return false;
		BYTE buffer[64];
		DWORD size = 0;
		bool ok = GetTokenInformation(token, TokenIntegrityLevel, buffer, sizeof(buffer), &size) != 0;
		if (ok) {
			PSID sid = ((TOKEN_MANDATORY_LABEL*)buffer)->Label.Sid;
			level = *GetSidSubAuthority(sid, *GetSidSubAuthorityCount(sid) - 1);
		}
		CloseHandle(token);
		return ok;
	};
	DWORD own = 0, other = 0;
	integrity(GetCurrentProcess(), own);
	bool elevated = false;
	HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
	if (!process) {
		elevated = GetLastError() == ERROR_ACCESS_DENIED;
	} else {
		//a token we may not even read belongs to a more privileged process
		elevated = integrity(process, other) ? other > own : GetLastError() == ERROR_ACCESS_DENIED;
		CloseHandle(process);
	}
	cachedProcess = processId;
	cachedResult = elevated;
	return elevated;
}

void EasyKeyHelper::removeFromSystem() {
	registerRunOnStartup(false);
	TCHAR desktop[MAX_PATH], shortcut[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktop))) {
		wsprintf(shortcut, _T("%s\\EasyKey.lnk"), desktop);
		DeleteFile(shortcut);
	}
	RegDeleteTree(HKEY_CURRENT_USER, sk);
}

bool EasyKeyHelper::isFirstRun() {
	return _firstRun;
}

wstring EasyKeyHelper::findOtherKeyboard() {
	//exe names start with these: UniKeyNT.exe, EVKey64.exe, OpenKey64.exe...
	static const struct { LPCTSTR prefix, name; } keyboards[] = {
		{ _T("unikey"), _T("UniKey") }, { _T("evkey"), _T("EVKey") }, { _T("openkey"), _T("OpenKey") },
		{ _T("gotiengviet"), _T("GoTiengViet") }, { _T("vietkey"), _T("Vietkey") },
	};
	wstring found;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 entry = { sizeof(entry) };
		for (BOOL ok = Process32First(snapshot, &entry); ok && found.empty(); ok = Process32Next(snapshot, &entry)) {
			wstring exe = entry.szExeFile;
			for (wchar_t& c : exe)
				c = towlower(c);
			for (auto& keyboard : keyboards) {
				if (exe.compare(0, lstrlen(keyboard.prefix), keyboard.prefix) == 0)
					found = keyboard.name;
			}
		}
		CloseHandle(snapshot);
	}
	if (found.empty()) {
		//a Vietnamese keyboard of Windows (a TSF input method) also adds marks to what is typed
		HWND foreground = GetForegroundWindow();
		HKL layout = GetKeyboardLayout(foreground ? GetWindowThreadProcessId(foreground, NULL) : 0);
		if (LOWORD((UINT_PTR)layout) == MAKELANGID(LANG_VIETNAMESE, SUBLANG_VIETNAMESE_VIETNAM))
			found = TR("Bàn phím tiếng Việt của Windows", "The Windows Vietnamese keyboard");
	}
	return found;
}