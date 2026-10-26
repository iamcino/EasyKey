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
#include "FluentWindow.h"

//converts the text in the clipboard between code tables and letter cases
class ConvertToolWindow : public FluentWindow {
private:
	int savedHotKey = 0; //the hotkey while new keys are being typed
public:
	ConvertToolWindow(const HINSTANCE& hInstance);
	void show();
protected:
	void layout() override;
	wstring title() const override;
	void onActivate(const int& id) override;
	void onToggle(const int& id) override;
	bool onHotKey(const int& id, const int& hotKey) override;
	void onCaptureChange(const int& id, const bool& started) override;
};
