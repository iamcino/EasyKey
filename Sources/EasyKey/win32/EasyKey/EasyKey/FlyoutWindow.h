/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11, based on OpenKey.
Copyright (C) 2026 Cino

This file is belong to the OpenKey project, Win32 version
which is released under GPL license.
You can fork, modify, improve this program. If you
redistribute your new version, it MUST be open source.
-----------------------------------------------------------*/
#pragma once
#include "stdafx.h"

/*
 * The small pill near the bottom of the screen that says Vietnamese or English
 * after the switch key is pressed. It never takes focus or mouse clicks and
 * fades out by itself.
 */
class FlyoutWindow {
private:
	HINSTANCE hInstance;
	HWND hWnd = NULL;
	HDC memDC = NULL;
	HBITMAP bitmap = NULL;
	SIZE size = { 0, 0 };
	POINT position = { 0, 0 };
	int alpha = 0;
private:
	static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void showNow();
	void render(const UINT& dpi, const RECT& workArea);
	void present();
	void freeBitmap();
public:
	FlyoutWindow(const HINSTANCE& hInstance);
	~FlyoutWindow();
	//safe to call from the keyboard hook, the flyout is drawn later by the message loop
	void notify();
};
