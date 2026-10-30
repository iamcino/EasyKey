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
#pragma once
#include "stdafx.h"

class SystemTrayHelper {
private:
	static void CALLBACK WaitToCreateFakeWindow(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime);
	static HWND createFakeWindow(const HINSTANCE& hIns);
	static void createPopupMenu();

	static void _createSystemTrayIcon(const HINSTANCE& hIns);
public:
	static void createSystemTrayIcon(const HINSTANCE& hIns);
	static void updateData();
	//rebuild the menu after the display language changed
	static void rebuildMenu();
	static void removeSystemTray();
};

