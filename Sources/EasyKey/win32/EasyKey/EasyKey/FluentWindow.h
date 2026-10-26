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
#include "FluentUi.h"

namespace Gdiplus { class Graphics; class Font; class StringFormat; }

/*
 * Base of the small windows EasyKey paints itself, in the style of the
 * settings window: Mica backdrop, light or dark colors from the color mode,
 * the accent color, rounded cards, buttons, switches and keycaps. Text fields
 * are real edit controls inside a painted field, so typing, IME and EasyKey
 * itself work in them as usual. A subclass lists its controls in layout()
 * and answers the on...() calls.
 */
class FluentWindow {
protected:
	enum Kind {
		F_TEXT,   //plain text, wraps when wrap is set
		F_CARD,   //rounded panel behind a group of rows
		F_DIVIDER,//line between rows of a card, at the top of rc
		F_BADGE,  //the V of EasyKey in the accent color
		F_BUTTON,
		F_ACCENT, //the main button, filled with the accent color
		F_LINK,
		F_SWITCH, //row with a label, an optional hint and a switch; the whole row toggles
		F_COMBO,  //row with a label and a drop-down field showing value
		F_HOTKEY, //row with a label, keycaps and a Change link in ctl
		F_EDIT,   //rounded field around a child edit control
		F_CUSTOM  //painted and handled by the subclass
	};
	struct Control {
		int kind = F_TEXT;
		int id = 0;
		wstring text, hint, value;
		RECT rc = {};            //the whole control or row
		RECT ctl = {};           //switch, field or link inside a row
		Gdiplus::Font* font = NULL;
		DWORD color = 0;         //text color, 0 for the theme's
		int align = 0;           //0 left, 1 center, 2 right
		bool wrap = false;
		bool on = false;         //switch state
		bool enabled = true;
		bool divider = false;    //line above a row inside a card
		int hotKey = 0;          //F_HOTKEY keys, vSwitchKeyStatus format
	};

	HINSTANCE hInstance;
	HWND hWnd = NULL;
	UINT dpi = USER_DEFAULT_SCREEN_DPI;
	FluentPalette pal;
	bool dark = false, translucent = false;
	Gdiplus::Font *fontBody = NULL, *fontCaption = NULL, *fontSmall = NULL, *fontStrong = NULL;
	Gdiplus::Font *fontTitle = NULL, *fontDisplay = NULL, *fontBadge = NULL;
	Gdiplus::StringFormat *textFormat = NULL, *wrapFormat = NULL;
	HFONT editFont = NULL;
	HBRUSH editBrush = NULL;
	COLORREF editBack = 0, editText = 0;

	vector<Control> controls;
	int contentHeight = 0;
	bool resizable = false;
	int minWidth = 0, minHeight = 0; //DIPs, resizable windows only
	int capturing = -1;              //id of the F_HOTKEY row taking new keys
	int captureModifiers = 0;
	int hot = -1, pressed = -1, focus = -1; //control indexes
	bool focusVisible = false;
private:
	int width = 0;                   //DIPs
	vector<pair<int, HWND>> edits;
	bool trackingMouse = false, customDrag = false;
	bool placed = false;
private:
	static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK editProc(HWND hEdit, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR id, DWORD_PTR data);
	LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void applyTheme();
	void createFonts();
	void freeFonts();
	void fit();
	void paint(HDC hdc);
	void paintControl(Gdiplus::Graphics& g, const Control& c, const int& index);
	int hitTest(const POINT& pt) const;
	bool isFocusable(const Control& c) const;
	void moveFocus(const int& direction);
	void setFocusTo(const int& index);
	void activate(const int& index);
	void startCapture(const int& id);
	void endCapture();
	void onCaptureKey(const UINT& uMsg, const WPARAM& vk);
	int indexOf(const int& kind, const int& id) const;
protected:
	int s(const int& value) const;
	int clientWidth() const;
	int clientHeight() const;
	int textWidth(Gdiplus::Font* font, const wstring& text) const;
	int textHeight(Gdiplus::Font* font, const wstring& text, const int& width) const;
	void drawText(Gdiplus::Graphics& g, const wstring& text, const RECT& rc, Gdiplus::Font* font, const DWORD& color,
		const int& hAlign, const int& vAlign, const bool& wrap = false);
	void paintSwitch(Gdiplus::Graphics& g, const RECT& rc, const bool& on, const bool& hot, const bool& enabled);
	int drawKeys(Gdiplus::Graphics& g, const wstring& hotKey, int x, const int& cy);

	//building blocks for layout(), each returns the control's index
	int add(const Control& c);
	int addText(const wstring& text, const RECT& rc, Gdiplus::Font* font, const DWORD& color = 0, const int& align = 0, const bool& wrap = false);
	int addDivider(const int& left, const int& right, const int& y);
	int addCard(const RECT& rc);
	int addBadge(const RECT& rc);
	int addButton(const int& id, const wstring& text, const RECT& rc, const bool& accent = false, const bool& enabled = true);
	int addLink(const int& id, const wstring& text, const int& x, const int& y, const int& height);
	//a row of a card: label on the left; the row is rc, its control is placed at the right
	int addSwitchRow(const int& id, const wstring& label, const wstring& hint, const RECT& rc, const bool& on, const bool& divider);
	int addComboRow(const int& id, const wstring& label, const wstring& value, const RECT& rc, const int& fieldWidth, const bool& divider);
	int addHotKeyRow(const int& id, const wstring& label, const int& hotKey, const RECT& rc, const bool& divider);
	int addEdit(const int& id, const RECT& rc, const wstring& placeholder);
	//a button wide enough for its text
	int buttonWidth(const wstring& text) const;

	HWND getEdit(const int& id) const;
	wstring getEditText(const int& id) const;
	void setEditText(const int& id, const wstring& text);
	void focusEdit(const int& id);

	//a menu under anchor (client coordinates); returns the chosen index or -1
	int showMenu(const RECT& anchor, const vector<wstring>& items, const int& checked);

	void create(const wstring& title, const int& widthDips, const HWND& owner);
	void relayout();

	//the window lists its controls for the current client width; fixed size windows set contentHeight
	virtual void layout() = 0;
	virtual wstring title() const = 0;
	virtual void onActivate(const int& id) {}       //button, link or combo row
	virtual void onToggle(const int& id) {}         //switch row
	virtual void onEditChange(const int& id) {}
	virtual void onEditEnter(const int& id) {}
	//new keys for an F_HOTKEY row, EMPTY_HOTKEY when cleared; false refuses them
	virtual bool onHotKey(const int& id, const int& hotKey) { return true; }
	virtual void onCaptureChange(const int& id, const bool& started) {}
	virtual void paintCustom(Gdiplus::Graphics& g, const Control& c) {}
	//mouse messages inside an F_CUSTOM control; true keeps the following moves coming until the button is up
	virtual bool onCustomMouse(const Control& c, const UINT& uMsg, const POINT& pt) { return false; }
	virtual bool onCustomKey(const Control& c, const WPARAM& key) { return false; }
	virtual bool onWheel(const POINT& pt, const int& delta) { return false; }
	//after WM_CLOSE: by default AppDelegate deletes the window
	virtual void onClosed();
public:
	FluentWindow(const HINSTANCE& hInstance);
	virtual ~FluentWindow();
	HWND getHwnd() const;
	void bringOnTop();
	void close();
	void destroy();
	//theme, accent, language or data changed
	virtual void fillData();
};

#define EMPTY_HOTKEY_DATA 0xFE0000FE
