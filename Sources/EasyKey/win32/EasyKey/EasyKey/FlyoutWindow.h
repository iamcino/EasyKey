/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
#pragma once
#include "stdafx.h"

/*
 * The small pill near the bottom of the screen that says Vietnamese or English
 * after the switch key is pressed. It never takes focus or mouse clicks and
 * fades out by itself.
 */
//what the flyout names
enum { FLYOUT_LANGUAGE, FLYOUT_INPUT_TYPE, FLYOUT_CODE_TABLE };

class FlyoutWindow {
private:
	HINSTANCE hInstance;
	HWND hWnd = NULL;
	HDC memDC = NULL;
	HBITMAP bitmap = NULL;
	SIZE size = { 0, 0 };
	POINT position = { 0, 0 };
	int alpha = 0;
	wstring messageTitle, messageDetail; //a notice instead of the language, see showMessage
	bool messageWarning = true;        //"!" badge, otherwise the V/E badge
	int shown = FLYOUT_LANGUAGE;       //what was switched, see notify
private:
	static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void showNow(const bool& message);
	void render(const UINT& dpi, const RECT& workArea, const bool& message);
	void present();
	void freeBitmap();
public:
	FlyoutWindow(const HINSTANCE& hInstance);
	~FlyoutWindow();
	//safe to call from the keyboard hook, the flyout is drawn later by the message loop.
	//It names the language, the input method or the code table, see FLYOUT_*
	void notify(const int& what = FLYOUT_LANGUAGE);
	//a notice in the same pill, shown a little longer
	void showMessage(const wstring& title, const wstring& detail, const bool& warning = true);
};
