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
#include "FluentUi.h"

namespace Gdiplus { class Graphics; class Font; class StringFormat; }

/*
 * The settings window. It paints its own Windows 11 style controls with GDI+
 * on a Mica backdrop (a solid color when transparency is off), follows the
 * app light/dark theme and the Windows accent color, and applies every change
 * immediately, so there are no OK or Cancel buttons.
 */
class SettingsWindow {
private:
	struct Item {
		int kind;
		int id;          //option, action or choice
		int linkAction;  //action of the link inside the row, 0 if none
		LPCTSTR label;
		LPCTSTR hint;
		RECT rc;         //whole row, in content coordinates
		RECT ctl;        //switch, field or button
		RECT link;
		int textRight;   //label and hint end here
		bool afterLabel; //first row of a group, no divider above it
	};

	HINSTANCE hInstance;
	HWND hWnd = NULL;
	UINT dpi = USER_DEFAULT_SCREEN_DPI;
	FluentPalette pal;
	bool translucent = false;
	Gdiplus::Font *fontBody = NULL, *fontCaption = NULL, *fontSmall = NULL, *fontStrong = NULL, *fontTitle = NULL, *fontBadge = NULL;
	Gdiplus::StringFormat* textFormat = NULL;

	vector<Item> items;
	RECT heroArea, expandArea;
	int contentHeight = 0, scrollY = 0;
	bool expanded = false;
	int openGroups = 0; //bit per group of Nâng cao, all start closed

	int hotItem = -1, hotPart = 0;
	int pressedItem = -1, pressedPart = 0;
	int focusItem = -1, focusPart = 0;
	bool focusVisible = false, trackingMouse = false;
	bool hotScrollBar = false, draggingThumb = false;
	int dragStartY = 0, dragStartScroll = 0;

	bool capturing = false;
	int captureTarget = 0; //0 the switch key, 1 the input method key
	int savedSwitchKey = 0, savedInputKey = 0, captureModifiers = 0;
	int hexEditing = -1; //custom accent field being typed in: 0 light, 1 dark
	wstring hexBuffer;
private:
	static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	int s(const int& value) const;
	void applyTheme();
	void createFonts();
	void freeFonts();
	int textWidth(Gdiplus::Font* font, LPCTSTR text) const;

	void layout();
	void fitWindow(const bool& center);
	int viewHeight() const;
	int maxScroll() const;
	void scrollTo(int y);
	void ensureVisible(const int& item);
	void revealGroup(const int& group);
	bool getThumb(RECT& track, RECT& thumb) const;

	void paint(HDC hdc);
	void paintItem(Gdiplus::Graphics& g, const Item& item, const int& index);
	void paintSwitch(Gdiplus::Graphics& g, const RECT& rc, const bool& on, const bool& hot, const bool& enabled);
	void drawText(Gdiplus::Graphics& g, LPCTSTR text, const RECT& rc, Gdiplus::Font* font, const DWORD& color, const int& hAlign, const int& vAlign);
	RECT toView(const RECT& rc) const;

	bool hitTest(const POINT& pt, int& item, int& part) const;
	bool isInteractive(const int& item, const int& part) const;
	void setHot(const int& item, const int& part);
	void moveFocus(const int& direction);
	void activate(const int& item, const int& part);
	void doAction(const int& action);

	bool getOption(const int& id) const;
	bool isOptionEnabled(const int& id) const;
	void setOption(const int& id, const bool& on);
	void showDropDown(const Item& item);
	void showGameApps(const Item& item);
	void requestRestartAsAdmin();

	void startHexEdit(const int& field);
	void endHexEdit();
	bool onHexKey(const UINT& uMsg, const WPARAM& key);
	void applyHex();

	void startCapture(const int& target);
	void endCapture(const bool& save, const int& hotKey);
	bool isHotKeyTaken(const int& hotKey) const;
	bool onCaptureKey(const UINT& uMsg, const WPARAM& vk);
public:
	SettingsWindow(const HINSTANCE& hInstance);
	~SettingsWindow();
	void show();
	void bringOnTop();
	void fillData();
	HWND getHwnd();
	void destroy();
};
