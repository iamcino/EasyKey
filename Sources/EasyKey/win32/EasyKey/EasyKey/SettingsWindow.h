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
#pragma once
#include "stdafx.h"

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
	struct Palette {
		DWORD bg, card, cardHover, cardPressed, stroke, divider, track;
		DWORD text, sub, disabled, accent, accentHover, onAccent;
		DWORD field, fieldHover, link, linkHover;
	};

	HINSTANCE hInstance;
	HWND hWnd = NULL;
	UINT dpi = USER_DEFAULT_SCREEN_DPI;
	Palette pal;
	bool translucent = false;
	Gdiplus::Font *fontBody = NULL, *fontCaption = NULL, *fontSmall = NULL, *fontStrong = NULL, *fontTitle = NULL, *fontBadge = NULL;
	Gdiplus::StringFormat* textFormat = NULL;

	vector<Item> items;
	RECT heroArea, expandArea;
	int contentHeight = 0, scrollY = 0;
	bool expanded = false;

	int hotItem = -1, hotPart = 0;
	int pressedItem = -1, pressedPart = 0;
	int focusItem = -1, focusPart = 0;
	bool focusVisible = false, trackingMouse = false;
	bool hotScrollBar = false, draggingThumb = false;
	int dragStartY = 0, dragStartScroll = 0;

	bool capturing = false;
	int savedSwitchKey = 0, captureModifiers = 0;
	bool checkingUpdate = false;
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
	void requestRestartAsAdmin();
	void checkUpdate();

	void startCapture();
	void endCapture(const bool& save, const int& hotKey);
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
