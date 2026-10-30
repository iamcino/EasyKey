/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

Based on OpenKey - The Cross platform Open source Vietnamese
Keyboard application.
Copyright (C) 2019 Mai Vu Tuyen
Contact: maivutuyen.91@gmail.com
Github: https://github.com/tuyenvm/OpenKey

SPDX-License-Identifier: GPL-3.0-only

This file is part of EasyKey, a modified version of OpenKey,
released under the GNU General Public License version 3.
See LICENSE and NOTICE.
-----------------------------------------------------------*/
#include "EasyKeyHelper.h"
#include <stdarg.h>
#include <cwctype>
#include <Shlobj.h>
#include <tlhelp32.h>
#include <map>
#include <taskschd.h>
#include <comutil.h>
#define SECURITY_WIN32
#include <security.h>

#pragma comment(lib, "version.lib")
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "secur32.lib")

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
static const char* _noAppYet = "TheEasyKeyProject"; //until the user switches to an app
static string _exeNameUtf8 = _noAppYet; //the last app whose name was read
static string _unknownProgram = "UnknownProgram";
static string* _frontName = &_exeNameUtf8; //what the window of _cacheProcessId counts as

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

//the "EasyKey" task in the root of Task Scheduler starts EasyKey as admin at logon.
//Made through the Task Scheduler API instead of schtasks.exe: schtasks gives no error back and its
//defaults stop the task after 72 hours and skip it on battery
static bool registerAdminTask(const wstring& path) {
	HRESULT comInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	bool ok = false;
	ITaskService* service = NULL;
	ITaskFolder* folder = NULL;
	ITaskDefinition* task = NULL;
	ITriggerCollection* triggers = NULL;
	ITrigger* trigger = NULL;
	ILogonTrigger* logonTrigger = NULL;
	IPrincipal* principal = NULL;
	ITaskSettings* settings = NULL;
	IActionCollection* actions = NULL;
	IAction* action = NULL;
	IExecAction* execAction = NULL;
	IRegisteredTask* registered = NULL;

	//only the logon of the user who turned the option on
	TCHAR user[256 + 1 + 256];
	ULONG userSize = _countof(user);
	if (!GetUserNameEx(NameSamCompatible, user, &userSize))
		goto done;

	if (FAILED(CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&service)))
		goto done;
	if (FAILED(service->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t())))
		goto done;
	if (FAILED(service->GetFolder(_bstr_t(L"\\"), &folder)))
		goto done;
	if (FAILED(service->NewTask(0, &task)))
		goto done;

	if (FAILED(task->get_Triggers(&triggers)) || FAILED(triggers->Create(TASK_TRIGGER_LOGON, &trigger)) ||
		FAILED(trigger->QueryInterface(IID_ILogonTrigger, (void**)&logonTrigger)) ||
		FAILED(logonTrigger->put_UserId(_bstr_t(user))))
		goto done;

	if (FAILED(task->get_Principal(&principal)) ||
		FAILED(principal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN)) ||
		FAILED(principal->put_RunLevel(TASK_RUNLEVEL_HIGHEST)))
		goto done;

	if (FAILED(task->get_Settings(&settings)))
		goto done;
	settings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
	settings->put_StopIfGoingOnBatteries(VARIANT_FALSE);
	settings->put_ExecutionTimeLimit(_bstr_t(L"PT0S")); //no time limit
	settings->put_MultipleInstances(TASK_INSTANCES_IGNORE_NEW);
	settings->put_StartWhenAvailable(VARIANT_FALSE);

	if (FAILED(task->get_Actions(&actions)) || FAILED(actions->Create(TASK_ACTION_EXEC, &action)) ||
		FAILED(action->QueryInterface(IID_IExecAction, (void**)&execAction)) ||
		FAILED(execAction->put_Path(_bstr_t(path.c_str()))))
		goto done;

	ok = SUCCEEDED(folder->RegisterTaskDefinition(_bstr_t(L"EasyKey"), task, TASK_CREATE_OR_UPDATE,
		_variant_t(user), _variant_t(), TASK_LOGON_INTERACTIVE_TOKEN, _variant_t(L""), &registered));

done:
	if (registered) registered->Release();
	if (execAction) execAction->Release();
	if (action) action->Release();
	if (actions) actions->Release();
	if (settings) settings->Release();
	if (principal) principal->Release();
	if (logonTrigger) logonTrigger->Release();
	if (trigger) trigger->Release();
	if (triggers) triggers->Release();
	if (task) task->Release();
	if (folder) folder->Release();
	if (service) service->Release();
	if (SUCCEEDED(comInit))
		CoUninitialize();
	if (!ok) {
		LOG(L"Can't register the EasyKey task\n");
	}
	return ok;
}

static void deleteAdminTask() {
	HRESULT comInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	ITaskService* service = NULL;
	ITaskFolder* folder = NULL;
	if (SUCCEEDED(CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&service)) &&
		SUCCEEDED(service->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t())) &&
		SUCCEEDED(service->GetFolder(_bstr_t(L"\\"), &folder))) {
		folder->DeleteTask(_bstr_t(L"EasyKey"), 0); //fails when there is no task, that's fine
	}
	if (folder) folder->Release();
	if (service) service->Release();
	if (SUCCEEDED(comInit))
		CoUninitialize();
}

static void setRunValue(const bool& on) {
	HKEY runKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _runOnStartupKeyPath, NULL, KEY_ALL_ACCESS, &runKey) != ERROR_SUCCESS)
		return;
	if (on) {
		//quoted, or a path with spaces (C:\Users\Nguyen Van A\...) makes Windows try C:\Users\Nguyen.exe first
		wstring path = L"\"" + EasyKeyHelper::getFullPath() + L"\"";
		RegSetValueEx(runKey, _T("EasyKey"), 0, REG_SZ, (byte*)path.c_str(), ((DWORD)path.size() + 1) * sizeof(TCHAR));
	} else {
		RegDeleteValue(runKey, _T("EasyKey"));
	}
	RegCloseKey(runKey);
}

void EasyKeyHelper::registerRunOnStartup(const int& val) {
	if (val) {
		//the task needs EasyKey running as admin; when that fails, start without admin rather than not at all
		if (vRunAsAdmin && registerAdminTask(getFullPath())) {
			setRunValue(false);
		} else {
			setRunValue(true);
		}
	} else {
		setRunValue(false);
		deleteAdminTask();
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

//a Store app (Calculator, Settings, Photos...) runs inside a frame of ApplicationFrameHost.exe,
//the app itself is the process of the core window in that frame
static DWORD processOfWindow(const HWND& hWnd) {
	DWORD processId = 0;
	GetWindowThreadProcessId(hWnd, &processId);
	TCHAR className[64];
	if (GetClassName(hWnd, className, 64) && lstrcmp(className, _T("ApplicationFrameWindow")) == 0) {
		HWND core = FindWindowEx(hWnd, NULL, _T("Windows.UI.Core.CoreWindow"), NULL);
		DWORD coreProcessId = 0;
		if (core && GetWindowThreadProcessId(core, &coreProcessId) && coreProcessId)
			processId = coreProcessId;
	}
	return processId;
}

string& EasyKeyHelper::getFrontMostAppExecuteName() {
	_tempWnd = GetForegroundWindow();
	_tempProcessId = processOfWindow(_tempWnd);
	//EasyKey's own windows (Settings, the tray menu) keep the app the user came from,
	//so a language or code table picked there is remembered for that app
	if (_tempProcessId == _cacheProcessId || _tempProcessId == GetCurrentProcessId()) {
		return *_frontName;
	}
	_cacheProcessId = _tempProcessId;
	//the limited right works for every app of the user, PROCESS_VM_READ is refused by some
	//(and by apps run as admin), which left the name of the previous app in place
	string name;
	HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, _tempProcessId);
	if (process) {
		TCHAR path[MAX_PATH];
		DWORD size = MAX_PATH;
		if (QueryFullProcessImageName(process, 0, path, &size)) {
			LPCTSTR file = _tcsrchr(path, '\\');
			name = wideStringToUtf8(file ? file + 1 : path);
		}
		CloseHandle(process);
	}
	if (name.empty()) {
		_frontName = &_unknownProgram;
	} else {
		if (name.compare("explorer.exe") != 0) //the desktop and the taskbar keep the app the user came from
			_exeNameUtf8 = name;
		_frontName = &_exeNameUtf8;
	}
	return *_frontName;
}

bool EasyKeyHelper::isRealApp(const string& exe) {
	return !exe.empty() && exe != _unknownProgram && exe != _noAppYet;
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
	//Vietnamese Windows, or English Windows set to Vietnam (region or formats) as most PCs in Vietnam are
	static int windowsVietnamese = -1;
	if (windowsVietnamese < 0)
		windowsVietnamese = PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_VIETNAMESE ||
			GetUserGeoID(GEOCLASS_NATION) == 251 || //Vietnam
			PRIMARYLANGID(LANGIDFROMLCID(GetUserDefaultLCID())) == LANG_VIETNAMESE;
	return !windowsVietnamese;
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