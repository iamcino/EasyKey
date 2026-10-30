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
#include "EasyKeyManager.h"
#include <shlobj.h>

static vector<LPCTSTR> _inputType = {
	_T("Telex"),
	_T("VNI"),
	_T("Simple Telex"),
};

static vector<LPCTSTR> _tableCode = {
	_T("Unicode"),
	_T("TCVN3 (ABC)"),
	_T("VNI Windows"),
	_T("Unicode Tổ hợp"),
	_T("Vietnamese Locale CP 1258")
};

/*-----------------------------------------------------------------------*/

extern void EasyKeyInit();
extern void EasyKeyFree();

unsigned short  EasyKeyManager::_lastKeyCode = 0;

vector<LPCTSTR>& EasyKeyManager::getInputType() {
	return _inputType;
}

vector<LPCTSTR>& EasyKeyManager::getTableCode() {
	return _tableCode;
}

void EasyKeyManager::initEngine() {
	EasyKeyInit();
}

void EasyKeyManager::freeEngine() {
	EasyKeyFree();
}

static wstring desktopShortcutPath() {
	TCHAR desktop[MAX_PATH];
	if (FAILED(SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktop)))
		return L"";
	return wstring(desktop) + L"\\EasyKey.lnk";
}

bool EasyKeyManager::hasDesktopShortcut() {
	wstring path = desktopShortcutPath();
	return !path.empty() && GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

void EasyKeyManager::removeDesktopShortcut() {
	wstring path = desktopShortcutPath();
	if (!path.empty())
		DeleteFile(path.c_str());
}

void EasyKeyManager::createDesktopShortcut() {
	CoInitialize(NULL);
	IShellLink* pShellLink = NULL;
	HRESULT hres;
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_ALL,
							IID_IShellLink, (void**)&pShellLink);
	if (SUCCEEDED(hres)) {
		wstring path = EasyKeyHelper::getFullPath();
		pShellLink->SetPath(path.c_str());
		pShellLink->SetDescription(_T("EasyKey - Bộ gõ Tiếng Việt"));
		pShellLink->SetIconLocation(path.c_str(), 0);

		IPersistFile* pPersistFile;
		hres = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);

		if (SUCCEEDED(hres)) {
			hres = pPersistFile->Save(desktopShortcutPath().c_str(), TRUE);
			pPersistFile->Release();
			pShellLink->Release();
		}
	}
}
