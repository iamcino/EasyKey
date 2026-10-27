/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
//GDI+ needs the full windows.h and objidl.h, include them before stdafx.h trims windows.h down
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <cmath>
#include "FluentWindow.h"
#include "AppDelegate.h"
#include "ThemeHelper.h"

#pragma comment(lib, "comctl32.lib")

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED 0x0320
#endif

#define FLUENT_CLASS _T("EasyKeyFluentWindow")

typedef BOOL(WINAPI *AdjustWindowRectExForDpiFunc)(LPRECT, DWORD, BOOL, DWORD, UINT);

static void adjustWindowRect(RECT& rc, const DWORD& style, const UINT& dpi) {
	static AdjustWindowRectExForDpiFunc adjustForDpi =
		(AdjustWindowRectExForDpiFunc)GetProcAddress(GetModuleHandle(_T("user32.dll")), "AdjustWindowRectExForDpi");
	if (adjustForDpi)
		adjustForDpi(&rc, style, FALSE, 0, dpi);
	else
		AdjustWindowRectEx(&rc, style, FALSE, 0);
}

/*-------------------------------- window --------------------------------*/

FluentWindow::FluentWindow(const HINSTANCE& hInstance) {
	this->hInstance = hInstance;
	ThemeHelper::startGdiplus();
}

FluentWindow::~FluentWindow() {
	destroy();
	freeFonts();
	if (editBrush)
		DeleteObject(editBrush);
}

HWND FluentWindow::getHwnd() const {
	return hWnd;
}

void FluentWindow::create(const wstring& title, const int& widthDips, const HWND& owner) {
	static ATOM windowClass = 0;
	if (!windowClass) {
		WNDCLASSEX wc = { sizeof(wc) };
		wc.style = CS_DBLCLKS;
		wc.lpfnWndProc = windowProc;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
		wc.hIconSm = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = FLUENT_CLASS;
		windowClass = RegisterClassEx(&wc);
	}
	width = widthDips;
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
	if (resizable)
		style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
	CreateWindowEx(0, FLUENT_CLASS, title.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, owner, NULL, hInstance, this);
	if (!hWnd)
		return;
	dpi = ThemeHelper::getDpi(hWnd);
	applyTheme();
	createFonts();
	relayout();
	fit();
	relayout();
	ShowWindow(hWnd, SW_SHOWNORMAL);
	SetForegroundWindow(hWnd);
}

void FluentWindow::bringOnTop() {
	if (!hWnd)
		return;
	if (IsIconic(hWnd))
		ShowWindow(hWnd, SW_RESTORE);
	SetForegroundWindow(hWnd);
}

void FluentWindow::close() {
	if (hWnd)
		PostMessage(hWnd, WM_CLOSE, 0, 0);
}

void FluentWindow::destroy() {
	if (!hWnd)
		return;
	for (auto& edit : edits)
		RemoveWindowSubclass(edit.second, editProc, edit.first);
	edits.clear();
	SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
	DestroyWindow(hWnd);
	hWnd = NULL;
}

void FluentWindow::onClosed() {
	//deletes this window
	AppDelegate::getInstance()->closeWindow(this);
}

void FluentWindow::fillData() {
	if (!hWnd)
		return;
	SetWindowText(hWnd, title().c_str());
	applyTheme();
	relayout();
	fit();
	relayout();
}

int FluentWindow::s(const int& value) const {
	return MulDiv(value, dpi, USER_DEFAULT_SCREEN_DPI);
}

int FluentWindow::clientWidth() const {
	RECT rc;
	GetClientRect(hWnd, &rc);
	return rc.right > 0 ? rc.right : s(width);
}

int FluentWindow::clientHeight() const {
	RECT rc;
	GetClientRect(hWnd, &rc);
	return rc.bottom;
}

void FluentWindow::applyTheme() {
	dark = ThemeHelper::isAppDarkMode();
	COLORREF solid = dark ? RGB(0x20, 0x20, 0x20) : RGB(0xF3, 0xF3, 0xF3);
	translucent = ThemeHelper::applyBackdrop(hWnd, solid);
	fillPalette(pal, dark, translucent, solid);
	//edit controls need a solid background, the field around them is painted the same
	editBack = dark ? RGB(0x2D, 0x2D, 0x2D) : RGB(0xFF, 0xFF, 0xFF);
	editText = dark ? RGB(0xFF, 0xFF, 0xFF) : RGB(0x1B, 0x1B, 0x1B);
	if (editBrush)
		DeleteObject(editBrush);
	editBrush = CreateSolidBrush(editBack);
	for (auto& edit : edits)
		InvalidateRect(edit.second, NULL, TRUE);
	InvalidateRect(hWnd, NULL, FALSE);
}

void FluentWindow::createFonts() {
	freeFonts();
	fontBody = createFluentFont(false, (float)s(14));
	fontCaption = createFluentFont(false, (float)s(12));
	fontSmall = createFluentFont(false, (float)s(13));
	fontStrong = createFluentFont(true, (float)s(13));
	fontTitle = createFluentFont(true, (float)s(15));
	fontDisplay = createFluentFont(true, (float)s(28));
	fontBadge = createFluentFont(false, (float)s(22), true);

	editFont = CreateFont(-s(14), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, ThemeHelper::getFontFace());
	for (auto& edit : edits)
		SendMessage(edit.second, WM_SETFONT, (WPARAM)editFont, TRUE);
}

void FluentWindow::freeFonts() {
	FluentFont** fonts[] = { &fontBody, &fontCaption, &fontSmall, &fontStrong, &fontTitle, &fontDisplay, &fontBadge };
	for (FluentFont** font : fonts)
		freeFluentFont(*font);
	if (editFont) {
		//the edits still hold it until they get the new one
		HFONT old = editFont;
		editFont = NULL;
		DeleteObject(old);
	}
}

void FluentWindow::fit() {
	DWORD style = (DWORD)GetWindowLongPtr(hWnd, GWL_STYLE);
	RECT current;
	GetWindowRect(hWnd, &current);
	if (resizable && placed)
		return;
	RECT frame = { 0, 0, s(width), contentHeight };
	adjustWindowRect(frame, style, dpi);
	int w = frame.right - frame.left, h = frame.bottom - frame.top;

	MONITORINFO mi = { sizeof(mi) };
	HWND owner = GetWindow(hWnd, GW_OWNER);
	if (!placed) {
		//over the window that opened it, otherwise on the monitor with the mouse
		POINT cursor;
		GetCursorPos(&cursor);
		HMONITOR monitor = owner && IsWindowVisible(owner) ? MonitorFromWindow(owner, MONITOR_DEFAULTTONEAREST) : MonitorFromPoint(cursor, MONITOR_DEFAULTTOPRIMARY);
		GetMonitorInfo(monitor, &mi);
		int x = mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - w) / 2;
		int y = mi.rcWork.top + (mi.rcWork.bottom - mi.rcWork.top - h) / 2;
		if (owner && IsWindowVisible(owner)) {
			RECT ownerRect;
			GetWindowRect(owner, &ownerRect);
			x = (ownerRect.left + ownerRect.right - w) / 2;
			y = (ownerRect.top + ownerRect.bottom - h) / 2;
		}
		x = max((int)mi.rcWork.left, min(x, (int)mi.rcWork.right - w));
		y = max((int)mi.rcWork.top, min(y, (int)mi.rcWork.bottom - h));
		placed = true;
		SetWindowPos(hWnd, NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
		return;
	}
	GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &mi);
	int y = current.top;
	if (y + h > mi.rcWork.bottom)
		y = max((int)mi.rcWork.top, (int)mi.rcWork.bottom - h);
	SetWindowPos(hWnd, NULL, current.left, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

void FluentWindow::relayout() {
	if (!hWnd)
		return;
	controls.clear();
	layout();
	//edits the layout no longer lists are hidden
	for (auto& edit : edits) {
		bool listed = indexOf(F_EDIT, edit.first) >= 0;
		ShowWindow(edit.second, listed ? SW_SHOWNA : SW_HIDE);
	}
	if (hot >= (int)controls.size())
		hot = -1;
	if (pressed >= (int)controls.size())
		pressed = -1;
	if (focus >= (int)controls.size())
		focus = -1;
	InvalidateRect(hWnd, NULL, FALSE);
}

/*-------------------------------- building --------------------------------*/

int FluentWindow::add(const Control& c) {
	controls.push_back(c);
	return (int)controls.size() - 1;
}

int FluentWindow::addText(const wstring& text, const RECT& rc, FluentFont* font, const DWORD& color, const int& align, const bool& wrap) {
	Control c;
	c.kind = F_TEXT;
	c.text = text;
	c.rc = rc;
	c.font = font;
	c.color = color;
	c.align = align;
	c.wrap = wrap;
	return add(c);
}

int FluentWindow::addCard(const RECT& rc) {
	Control c;
	c.kind = F_CARD;
	c.rc = rc;
	return add(c);
}

int FluentWindow::addBadge(const RECT& rc) {
	Control c;
	c.kind = F_BADGE;
	c.rc = rc;
	return add(c);
}

int FluentWindow::addDivider(const int& left, const int& right, const int& y) {
	Control c;
	c.kind = F_DIVIDER;
	c.rc = makeRect(left, y, right - left, 1);
	return add(c);
}

int FluentWindow::buttonWidth(const wstring& text) const {
	return max(s(96), textWidth(fontSmall, text) + s(32));
}

int FluentWindow::addButton(const int& id, const wstring& text, const RECT& rc, const bool& accent, const bool& enabled) {
	Control c;
	c.kind = accent ? F_ACCENT : F_BUTTON;
	c.id = id;
	c.text = text;
	c.rc = rc;
	c.enabled = enabled;
	return add(c);
}

int FluentWindow::addLink(const int& id, const wstring& text, const int& x, const int& y, const int& height) {
	Control c;
	c.kind = F_LINK;
	c.id = id;
	c.text = text;
	c.rc = makeRect(x, y, textWidth(fontSmall, text), height);
	return add(c);
}

int FluentWindow::addSwitchRow(const int& id, const wstring& label, const wstring& hint, const RECT& rc, const bool& on, const bool& divider) {
	Control c;
	c.kind = F_SWITCH;
	c.id = id;
	c.text = label;
	c.hint = hint;
	c.rc = rc;
	c.on = on;
	c.divider = divider;
	c.ctl = makeRect(rc.right - s(14) - s(40), centerY(rc) - s(10), s(40), s(20));
	return add(c);
}

int FluentWindow::addComboRow(const int& id, const wstring& label, const wstring& value, const RECT& rc, const int& fieldWidth, const bool& divider) {
	Control c;
	c.kind = F_COMBO;
	c.id = id;
	c.text = label;
	c.value = value;
	c.rc = rc;
	c.divider = divider;
	c.ctl = makeRect(rc.right - s(14) - fieldWidth, centerY(rc) - s(15), fieldWidth, s(30));
	return add(c);
}

int FluentWindow::addHotKeyRow(const int& id, const wstring& label, const int& hotKey, const RECT& rc, const bool& divider) {
	Control c;
	c.kind = F_HOTKEY;
	c.id = id;
	c.text = label;
	c.hotKey = hotKey;
	c.rc = rc;
	c.divider = divider;
	wstring link = capturing == id ? TR("Huỷ", "Cancel") : TR("Đổi phím", "Change");
	int linkWidth = textWidth(fontSmall, link) + s(16);
	c.ctl = makeRect(rc.right - s(8) - linkWidth, centerY(rc) - s(13), linkWidth, s(26));
	return add(c);
}

int FluentWindow::addEdit(const int& id, const RECT& rc, const wstring& placeholder) {
	Control c;
	c.kind = F_EDIT;
	c.id = id;
	c.rc = rc;
	int index = add(c);

	HWND edit = getEdit(id);
	if (!edit) {
		edit = CreateWindowEx(0, WC_EDIT, _T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
			0, 0, 10, 10, hWnd, (HMENU)(INT_PTR)id, hInstance, NULL);
		SetWindowSubclass(edit, editProc, id, (DWORD_PTR)this);
		SendMessage(edit, WM_SETFONT, (WPARAM)editFont, FALSE);
		SendMessage(edit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, 0);
		edits.push_back(make_pair(id, edit));
	}
	SendMessage(edit, EM_SETCUEBANNER, TRUE, (LPARAM)placeholder.c_str());
	//one line of text, centred in the field
	HDC hdc = GetDC(edit);
	HGDIOBJ old = SelectObject(hdc, editFont);
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	SelectObject(hdc, old);
	ReleaseDC(edit, hdc);
	int lineHeight = tm.tmHeight + s(2);
	MoveWindow(edit, rc.left + s(11), centerY(rc) - lineHeight / 2, rc.right - rc.left - s(22), lineHeight, TRUE);
	return index;
}

HWND FluentWindow::getEdit(const int& id) const {
	for (auto& edit : edits) {
		if (edit.first == id)
			return edit.second;
	}
	return NULL;
}

wstring FluentWindow::getEditText(const int& id) const {
	HWND edit = getEdit(id);
	if (!edit)
		return L"";
	int length = GetWindowTextLength(edit);
	wstring text(length + 1, L'\0');
	GetWindowText(edit, &text[0], length + 1);
	text.resize(length);
	return text;
}

void FluentWindow::setEditText(const int& id, const wstring& text) {
	HWND edit = getEdit(id);
	if (edit)
		SetWindowText(edit, text.c_str());
}

void FluentWindow::focusEdit(const int& id) {
	HWND edit = getEdit(id);
	if (edit) {
		SetFocus(edit);
		SendMessage(edit, EM_SETSEL, 0, -1);
	}
}

int FluentWindow::indexOf(const int& kind, const int& id) const {
	for (int i = 0; i < (int)controls.size(); i++) {
		if (controls[i].kind == kind && controls[i].id == id)
			return i;
	}
	return -1;
}

int FluentWindow::showMenu(const RECT& anchor, const vector<wstring>& items, const int& checked) {
	HMENU menu = CreatePopupMenu();
	for (int i = 0; i < (int)items.size(); i++)
		AppendMenu(menu, MF_STRING | (i == checked ? MF_CHECKED : MF_UNCHECKED), i + 1, items[i].c_str());
	RECT field = anchor;
	MapWindowPoints(hWnd, NULL, (LPPOINT)&field, 2);
	TPMPARAMS params = { sizeof(params) };
	params.rcExclude = field;
	int command = (int)TrackPopupMenuEx(menu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_VERTICAL,
		field.left, field.bottom, hWnd, &params);
	DestroyMenu(menu);
	return command - 1;
}

/*-------------------------------- painting --------------------------------*/

//GDI+ clip of the moment, so the text stays inside a scrolled list like the shapes do
static RECT clipOf(Gdiplus::Graphics& g) {
	Gdiplus::RectF bounds;
	g.GetClipBounds(&bounds);
	RECT clip = { (LONG)floor(max(bounds.X, -100000.0f)), (LONG)floor(max(bounds.Y, -100000.0f)),
		(LONG)ceil(min(bounds.GetRight(), 100000.0f)), (LONG)ceil(min(bounds.GetBottom(), 100000.0f)) };
	return clip;
}

int FluentWindow::textWidth(FluentFont* font, const wstring& text) const {
	return fluentTextWidth(font, text);
}

int FluentWindow::textHeight(FluentFont* font, const wstring& text, const int& width) const {
	return fluentTextHeight(font, text, width);
}

void FluentWindow::drawText(Gdiplus::Graphics& g, const wstring& text, const RECT& rc, FluentFont* font, const DWORD& color,
							const int& hAlign, const int& vAlign, const bool& wrap) {
	textBatch.add(text, rc, font, color, hAlign, vAlign, wrap, clipOf(g));
}

void FluentWindow::paintSwitch(Gdiplus::Graphics& g, const RECT& rc, const bool& on, const bool& isHot, const bool& enabled) {
	float height = (float)(rc.bottom - rc.top);
	float knob = (float)s(isHot && enabled ? 14 : 12);
	float cy = rc.top + height / 2;
	if (on) {
		fillRound(g, rc, height / 2, !enabled ? pal.disabled : isHot ? pal.accentHover : pal.accent);
		float cx = rc.right - height / 2;
		Gdiplus::SolidBrush knobBrush((Gdiplus::ARGB)(enabled ? pal.onAccent : pal.field));
		g.FillEllipse(&knobBrush, cx - knob / 2, cy - knob / 2, knob, knob);
	} else {
		DWORD color = enabled ? pal.sub : pal.disabled;
		fillRound(g, rc, height / 2, isHot && enabled ? pal.fieldHover : 0, color);
		float cx = rc.left + height / 2;
		Gdiplus::SolidBrush knobBrush((Gdiplus::ARGB)color);
		g.FillEllipse(&knobBrush, cx - knob / 2, cy - knob / 2, knob, knob);
	}
}

//keycaps with a thicker bottom edge, like the keys they stand for
int FluentWindow::drawKeys(Gdiplus::Graphics& g, const wstring& hotKey, int x, const int& cy) {
	vector<wstring> keys = splitHotKey(hotKey);
	for (size_t k = 0; k < keys.size(); k++) {
		if (k > 0) {
			int plus = textWidth(fontCaption, _T("+"));
			drawText(g, _T("+"), makeRect(x, cy - s(10), plus, s(20)), fontCaption, pal.sub, 0, 1);
			x += plus + s(5);
		}
		int w = textWidth(fontCaption, keys[k]) + s(14);
		RECT cap = makeRect(x, cy - s(10), w, s(20));
		RECT edge = cap;
		OffsetRect(&edge, 0, 1);
		fillRound(g, edge, (float)s(5), pal.stroke);
		fillRound(g, cap, (float)s(5), pal.field, pal.stroke);
		drawText(g, keys[k], cap, fontCaption, pal.text, 1, 1);
		x = cap.right + s(5);
	}
	return x;
}

void FluentWindow::paint(HDC hdc) {
	RECT client;
	GetClientRect(hWnd, &client);
	if (client.right <= 0 || client.bottom <= 0)
		return;

	//double buffered in a premultiplied 32 bit bitmap, its transparent pixels show the Mica backdrop
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = client.right;
	bmi.bmiHeader.biHeight = -client.bottom;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	void* bits = NULL;
	HBITMAP bitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (!bitmap)
		return;
	HDC mem = CreateCompatibleDC(hdc);
	HGDIOBJ oldBitmap = SelectObject(mem, bitmap);
	{
		Gdiplus::Bitmap canvas(client.right, client.bottom, client.right * 4, PixelFormat32bppPARGB, (BYTE*)bits);
		Gdiplus::Graphics g(&canvas);
		g.Clear(Gdiplus::Color((Gdiplus::ARGB)pal.bg));
		g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
		for (int i = 0; i < (int)controls.size(); i++)
			paintControl(g, controls[i], i);

		//keyboard focus ring
		if (focusVisible && focus >= 0 && GetFocus() == hWnd) {
			const Control& c = controls[focus];
			RECT ring = c.kind == F_HOTKEY ? c.ctl : c.rc;
			InflateRect(&ring, s(2), s(2));
			RECT inner = ring;
			InflateRect(&inner, -1, -1);
			Gdiplus::GraphicsPath path;
			addRoundRect(path, inner, (float)s(8));
			Gdiplus::Pen pen((Gdiplus::ARGB)pal.text, 2);
			g.DrawPath(&pen, &path);
		}
	}
	textBatch.draw(mem, client.right, client.bottom);
	BitBlt(hdc, 0, 0, client.right, client.bottom, mem, 0, 0, SRCCOPY);
	SelectObject(mem, oldBitmap);
	DeleteObject(bitmap);
	DeleteDC(mem);
}

void FluentWindow::paintControl(Gdiplus::Graphics& g, const Control& c, const int& index) {
	bool isHot = index == hot;
	bool isPressed = isHot && index == pressed;
	const RECT& rc = c.rc;
	int cy = centerY(rc);
	DWORD textColor = c.color ? c.color : pal.text;

	//label of a row, with an optional hint below it
	auto drawLabel = [&](const int& right) {
		RECT label = { rc.left + s(14), rc.top, right, rc.bottom };
		DWORD color = c.enabled ? pal.text : pal.disabled;
		if (!c.hint.empty()) {
			RECT top = label, bottom = label;
			top.bottom = cy + s(1);
			bottom.top = cy + s(1);
			drawText(g, c.text, top, fontBody, color, 0, 2);
			drawText(g, c.hint, bottom, fontCaption, c.enabled ? pal.sub : pal.disabled, 0, 0);
		} else {
			drawText(g, c.text, label, fontBody, color, 0, 1);
		}
	};
	auto rowBackground = [&](const bool& hover) {
		if (c.divider)
			drawLine(g, pal.divider, rc.left + 1, rc.right - 1, rc.top);
		if (hover && c.enabled) {
			RECT area = { rc.left + 1, rc.top + 1, rc.right - 1, rc.bottom };
			fillRect(g, isPressed ? pal.cardPressed : pal.cardHover, area);
		}
	};

	switch (c.kind) {
	case F_TEXT:
		drawText(g, c.text, rc, c.font ? c.font : fontBody, textColor, c.align, c.wrap ? 0 : 1, c.wrap);
		break;
	case F_CARD:
		fillRound(g, rc, (float)s(8), pal.card, pal.stroke);
		break;
	case F_DIVIDER:
		drawLine(g, pal.divider, rc.left, rc.right, rc.top);
		break;
	case F_BADGE: {
		float radius = (rc.right - rc.left) * 0.22f;
		fillRound(g, rc, radius, pal.accent);
		drawText(g, _T("V"), rc, fontBadge, pal.onAccent, 1, 1);
		break;
	}
	case F_BUTTON: {
		DWORD fill = !c.enabled ? pal.field : isPressed ? pal.cardPressed : isHot ? pal.fieldHover : pal.field;
		fillRound(g, rc, (float)s(6), fill, pal.stroke);
		drawText(g, c.text, rc, fontSmall, c.enabled ? pal.text : pal.disabled, 1, 1);
		break;
	}
	case F_ACCENT: {
		DWORD fill = !c.enabled ? pal.disabled : isHot ? pal.accentHover : pal.accent;
		fillRound(g, rc, (float)s(6), isPressed ? fade(fill, 85) : fill);
		drawText(g, c.text, rc, fontSmall, pal.onAccent, 1, 1);
		break;
	}
	case F_LINK:
		drawText(g, c.text, rc, c.font ? c.font : fontSmall, isHot ? pal.linkHover : pal.link, 0, 1);
		break;
	case F_SWITCH:
		rowBackground(isHot);
		drawLabel(c.ctl.left - s(12));
		paintSwitch(g, c.ctl, c.on, isHot, c.enabled);
		break;
	case F_COMBO: {
		rowBackground(isHot);
		drawLabel(c.ctl.left - s(12));
		fillRound(g, c.ctl, (float)s(6), isHot ? pal.fieldHover : pal.field, pal.stroke);
		RECT field = { c.ctl.left + s(10), c.ctl.top, c.ctl.right - s(30), c.ctl.bottom };
		drawText(g, c.value, field, fontSmall, pal.text, 0, 1);
		drawChevron(g, pal.sub, (float)(c.ctl.right - s(16)), (float)centerY(c.ctl), (float)s(10), false);
		break;
	}
	case F_HOTKEY: {
		rowBackground(false);
		int labelWidth = textWidth(fontBody, c.text);
		drawText(g, c.text, makeRect(rc.left + s(14), rc.top, labelWidth, rc.bottom - rc.top), fontBody, pal.text, 0, 1);
		int x = rc.left + s(14) + labelWidth + s(12);
		RECT rest = { x, rc.top, c.ctl.left - s(4), rc.bottom };
		if (capturing == c.id) {
			if (captureModifiers) {
				x = drawKeys(g, EasyKeyHelper::getHotKeyString(captureModifiers | 0xFE), x, cy);
				drawText(g, _T("+ …"), makeRect(x, rc.top, rest.right - x, rc.bottom - rc.top), fontCaption, pal.sub, 0, 1);
			} else {
				drawText(g, TR("Nhấn tổ hợp phím mới…", "Press the new keys…"), rest, fontCaption, pal.text, 0, 1);
			}
		} else {
			wstring keys = EasyKeyHelper::getHotKeyString(c.hotKey);
			if (keys.empty())
				drawText(g, TR("Chưa đặt phím", "No key set"), rest, fontCaption, pal.sub, 0, 1);
			else
				drawKeys(g, keys, x, cy);
		}
		bool linkHot = isHot;
		if (linkHot)
			fillRound(g, c.ctl, (float)s(6), pal.cardHover);
		drawText(g, capturing == c.id ? TR("Huỷ", "Cancel") : TR("Đổi phím", "Change"), c.ctl, fontSmall,
			linkHot ? pal.linkHover : pal.link, 1, 1);
		break;
	}
	case F_EDIT: {
		HWND edit = getEdit(c.id);
		bool focused = edit && GetFocus() == edit;
		fillRound(g, rc, (float)s(6), argb(255, editBack), pal.stroke);
		//Fluent text boxes: a quiet line at the bottom, the accent while typing
		RECT line = { rc.left + s(6), rc.bottom - (focused ? s(2) : 1), rc.right - s(6), rc.bottom };
		fillRect(g, focused ? pal.accent : fade(pal.sub, 70), line);
		break;
	}
	case F_CUSTOM:
		paintCustom(g, c);
		break;
	}
}

/*-------------------------------- input --------------------------------*/

int FluentWindow::hitTest(const POINT& pt) const {
	for (int i = (int)controls.size() - 1; i >= 0; i--) {
		const Control& c = controls[i];
		if (c.kind == F_TEXT || c.kind == F_CARD || c.kind == F_DIVIDER || c.kind == F_BADGE)
			continue;
		if ((c.kind == F_BUTTON || c.kind == F_ACCENT || c.kind == F_SWITCH || c.kind == F_COMBO) && !c.enabled)
			continue;
		const RECT& area = c.kind == F_HOTKEY ? c.ctl : c.rc;
		if (PtInRect(&area, pt))
			return i;
	}
	return -1;
}

bool FluentWindow::isFocusable(const Control& c) const {
	switch (c.kind) {
	case F_TEXT:
	case F_CARD:
	case F_DIVIDER:
	case F_BADGE:
		return false;
	case F_BUTTON:
	case F_ACCENT:
	case F_SWITCH:
	case F_COMBO:
		return c.enabled;
	}
	return true;
}

void FluentWindow::setFocusTo(const int& index) {
	if (index < 0 || index >= (int)controls.size())
		return;
	focus = index;
	focusVisible = true;
	if (controls[index].kind == F_EDIT)
		focusEdit(controls[index].id);
	else
		SetFocus(hWnd);
	InvalidateRect(hWnd, NULL, FALSE);
}

void FluentWindow::moveFocus(const int& direction) {
	int count = (int)controls.size();
	if (count == 0)
		return;
	int index = focus;
	for (int step = 0; step < count; step++) {
		index = index < 0 ? (direction > 0 ? 0 : count - 1) : (index + direction + count) % count;
		if (isFocusable(controls[index])) {
			setFocusTo(index);
			return;
		}
	}
}

void FluentWindow::activate(const int& index) {
	if (index < 0 || index >= (int)controls.size())
		return;
	//copy, the handlers below can rebuild the controls
	Control c = controls[index];
	switch (c.kind) {
	case F_SWITCH:
		onToggle(c.id);
		break;
	case F_HOTKEY:
		if (capturing == c.id)
			endCapture();
		else
			startCapture(c.id);
		break;
	case F_EDIT:
		focusEdit(c.id);
		break;
	case F_BUTTON:
	case F_ACCENT:
	case F_LINK:
	case F_COMBO:
		onActivate(c.id);
		break;
	}
	if (hWnd)
		InvalidateRect(hWnd, NULL, FALSE);
}

void FluentWindow::startCapture(const int& id) {
	endCapture();
	capturing = id;
	captureModifiers = 0;
	onCaptureChange(id, true);
	SetFocus(hWnd);
	relayout();
}

void FluentWindow::endCapture() {
	if (capturing < 0)
		return;
	int id = capturing;
	capturing = -1;
	onCaptureChange(id, false);
	relayout();
}

void FluentWindow::onCaptureKey(const UINT& uMsg, const WPARAM& vk) {
	bool down = uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN;
	int mask = modifierMask(vk);
	auto offer = [&](const int& hotKey) {
		if (onHotKey(capturing, hotKey)) {
			endCapture();
		} else {
			MessageBeep(MB_OK);
			captureModifiers = 0;
			InvalidateRect(hWnd, NULL, FALSE);
		}
	};
	if (down) {
		if (mask) {
			captureModifiers |= mask;
			InvalidateRect(hWnd, NULL, FALSE);
			return;
		}
		int mods = pressedModifiers();
		if (vk == VK_ESCAPE && mods == 0) {
			endCapture();
		} else if ((vk == VK_BACK || vk == VK_DELETE) && mods == 0) {
			offer(EMPTY_HOTKEY_DATA);
		} else if (mods != 0 && (vk == VK_SPACE || keyCodeToCharacter((Uint32)vk) != 0)) {
			offer(mods | (int)vk | ((int)vk << 24));
		} else {
			MessageBeep(MB_OK); //a hotkey needs at least one modifier and a typing key
		}
		return;
	}
	//modifiers only, like Ctrl + Shift: taken when the first of them is released
	if (mask && (captureModifiers & mask)) {
		if (countModifiers(captureModifiers) >= 2) {
			offer(captureModifiers | 0xFE | (0xFE << 24));
		} else {
			captureModifiers = 0;
			InvalidateRect(hWnd, NULL, FALSE);
		}
	}
}

/*-------------------------------- messages --------------------------------*/

LRESULT CALLBACK FluentWindow::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_NCCREATE) {
		FluentWindow* window = (FluentWindow*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		window->hWnd = hWnd;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
	}
	FluentWindow* window = (FluentWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (window)
		return window->handleMessage(uMsg, wParam, lParam);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK FluentWindow::editProc(HWND hEdit, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR id, DWORD_PTR data) {
	FluentWindow* window = (FluentWindow*)data;
	switch (uMsg) {
	case WM_KEYDOWN:
		if (wParam == VK_TAB) {
			window->moveFocus(GetKeyState(VK_SHIFT) < 0 ? -1 : 1);
			return 0;
		}
		if (wParam == VK_RETURN) {
			window->onEditEnter((int)id);
			return 0;
		}
		if (wParam == VK_ESCAPE) {
			window->close();
			return 0;
		}
		break;
	case WM_CHAR:
		if (wParam == '\t' || wParam == '\r' || wParam == 27)
			return 0; //handled on key down, no beep
		break;
	case WM_SETFOCUS:
		window->focus = window->indexOf(F_EDIT, (int)id);
		window->focusVisible = false;
		InvalidateRect(window->hWnd, NULL, FALSE);
		break;
	case WM_KILLFOCUS:
		InvalidateRect(window->hWnd, NULL, FALSE);
		break;
	}
	return DefSubclassProc(hEdit, uMsg, wParam, lParam);
}

LRESULT FluentWindow::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
	switch (uMsg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		paint(hdc);
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND:
		return 1;
	case WM_SIZE:
		if (resizable && placed)
			relayout();
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_GETMINMAXINFO:
		if (resizable) {
			RECT frame = { 0, 0, s(minWidth), s(minHeight) };
			adjustWindowRect(frame, (DWORD)GetWindowLongPtr(hWnd, GWL_STYLE), dpi);
			MINMAXINFO* info = (MINMAXINFO*)lParam;
			info->ptMinTrackSize.x = frame.right - frame.left;
			info->ptMinTrackSize.y = frame.bottom - frame.top;
			return 0;
		}
		break;
	case WM_DPICHANGED: {
		dpi = HIWORD(wParam);
		createFonts();
		RECT* suggested = (RECT*)lParam;
		SetWindowPos(hWnd, NULL, suggested->left, suggested->top,
			suggested->right - suggested->left, suggested->bottom - suggested->top, SWP_NOZORDER | SWP_NOACTIVATE);
		relayout();
		if (!resizable) {
			fit();
			relayout();
		}
		return 0;
	}
	case WM_SETTINGCHANGE:
	case WM_THEMECHANGED:
		if (ThemeHelper::isThemeChangedMessage(uMsg, lParam))
			applyTheme();
		break;
	case WM_DWMCOLORIZATIONCOLORCHANGED:
		applyTheme(); //the accent color changed
		break;
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam, editText);
		SetBkColor((HDC)wParam, editBack);
		return (LRESULT)editBrush;
	case WM_COMMAND:
		if (HIWORD(wParam) == EN_CHANGE)
			onEditChange(LOWORD(wParam));
		return 0;
	case WM_MOUSEMOVE: {
		if (!trackingMouse) {
			TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hWnd, 0 };
			TrackMouseEvent(&tme);
			trackingMouse = true;
		}
		if (customDrag && focus >= 0 && controls[focus].kind == F_CUSTOM) {
			onCustomMouse(controls[focus], uMsg, pt);
			return 0;
		}
		int index = hitTest(pt);
		if (index >= 0 && controls[index].kind == F_CUSTOM)
			onCustomMouse(controls[index], uMsg, pt);
		if (index != hot) {
			if (hot >= 0 && hot < (int)controls.size() && controls[hot].kind == F_CUSTOM)
				onCustomMouse(controls[hot], WM_MOUSELEAVE, pt);
			hot = index;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		return 0;
	}
	case WM_MOUSELEAVE:
		trackingMouse = false;
		if (hot >= 0 && hot < (int)controls.size() && controls[hot].kind == F_CUSTOM)
			onCustomMouse(controls[hot], WM_MOUSELEAVE, pt);
		hot = -1;
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT) {
			bool onLink = hot >= 0 && hot < (int)controls.size() && (controls[hot].kind == F_LINK || controls[hot].kind == F_HOTKEY);
			bool onField = hot >= 0 && hot < (int)controls.size() && controls[hot].kind == F_EDIT;
			SetCursor(LoadCursor(NULL, onLink ? IDC_HAND : onField ? IDC_IBEAM : IDC_ARROW));
			return TRUE;
		}
		break;
	case WM_MOUSEWHEEL: {
		POINT screen = pt;
		ScreenToClient(hWnd, &screen);
		onWheel(screen, GET_WHEEL_DELTA_WPARAM(wParam));
		return 0;
	}
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK: {
		int index = hitTest(pt);
		if (capturing >= 0 && (index < 0 || controls[index].kind != F_HOTKEY || controls[index].id != capturing))
			endCapture();
		index = hitTest(pt);
		focusVisible = false;
		if (index >= 0 && controls[index].kind == F_EDIT) {
			focusEdit(controls[index].id);
			return 0;
		}
		SetFocus(hWnd);
		if (index >= 0 && controls[index].kind == F_CUSTOM) {
			focus = index;
			customDrag = onCustomMouse(controls[index], uMsg, pt);
			if (customDrag)
				SetCapture(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if (index >= 0) {
			pressed = index;
			SetCapture(hWnd);
		}
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	}
	case WM_LBUTTONUP: {
		if (customDrag) {
			customDrag = false;
			ReleaseCapture();
			if (focus >= 0 && controls[focus].kind == F_CUSTOM)
				onCustomMouse(controls[focus], uMsg, pt);
			return 0;
		}
		if (pressed < 0)
			return 0;
		int was = pressed;
		pressed = -1;
		ReleaseCapture();
		if (hitTest(pt) == was) {
			focus = was;
			activate(was);
		}
		if (hWnd)
			InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	}
	case WM_CAPTURECHANGED:
		customDrag = false;
		pressed = -1;
		return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (capturing >= 0) {
			onCaptureKey(uMsg, wParam);
			return 0;
		}
		if (uMsg != WM_KEYDOWN)
			break;
		if (focus >= 0 && focus < (int)controls.size() && controls[focus].kind == F_CUSTOM && onCustomKey(controls[focus], wParam))
			return 0;
		switch (wParam) {
		case VK_TAB:
			moveFocus(GetKeyState(VK_SHIFT) < 0 ? -1 : 1);
			return 0;
		case VK_DOWN:
		case VK_RIGHT:
			moveFocus(1);
			return 0;
		case VK_UP:
		case VK_LEFT:
			moveFocus(-1);
			return 0;
		case VK_SPACE:
		case VK_RETURN:
			if (!(lParam & 0x40000000) && focus >= 0) { //ignore auto-repeat
				focusVisible = true;
				activate(focus);
			}
			return 0;
		case VK_ESCAPE:
			close();
			return 0;
		}
		break;
	case WM_CHAR:
	case WM_SYSCHAR:
		if (capturing >= 0)
			return 0;
		break;
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		if (uMsg == WM_KILLFOCUS)
			endCapture();
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_CLOSE:
		endCapture();
		//may delete this window, do not touch members afterwards
		onClosed();
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
