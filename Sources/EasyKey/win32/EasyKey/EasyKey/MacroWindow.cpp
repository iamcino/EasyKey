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
//GDI+ needs the full windows.h and objidl.h, include them before stdafx.h trims windows.h down
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <commdlg.h>
#include "MacroWindow.h"
#include "AppDelegate.h"
#include "MessageWindow.h"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 540
#define NAME_WIDTH 150

enum {
	ID_NAME = 1, ID_CONTENT, ID_ADD, ID_DELETE, ID_LIST, ID_AUTO_CAPS, ID_IMPORT, ID_EXPORT
};

MacroWindow::MacroWindow(const HINSTANCE& hInstance) : FluentWindow(hInstance) {
	resizable = true;
	minWidth = 480;
	minHeight = 380;
}

void MacroWindow::show() {
	load();
	create(title(), WINDOW_WIDTH, NULL);
	focusEdit(ID_NAME);
}

void MacroWindow::fillData() {
	load();
	FluentWindow::fillData();
}

wstring MacroWindow::title() const {
	return TR("Bảng gõ tắt", "Shortcut list");
}

void MacroWindow::load() {
	vector<vector<Uint32>> keys;
	vector<string> names, contents;
	getAllMacro(keys, names, contents);
	rows.clear();
	for (size_t i = names.size(); i-- > 0;)
		rows.push_back(make_pair(utf8ToWideString(names[i]), utf8ToWideString(contents[i])));
	if (selected >= (int)rows.size())
		selected = -1;
}

void MacroWindow::save() {
	vector<Byte> data;
	getMacroSaveData(data);
	EasyKeyHelper::setRegBinary(_T("macroData"), data.data(), (int)data.size());
	load();
}

/*-------------------------------- layout --------------------------------*/

int MacroWindow::rowHeight() const {
	return s(36);
}

void MacroWindow::layout() {
	contentHeight = s(WINDOW_HEIGHT); //first size; the window can be resized
	int width = clientWidth(), height = clientHeight();
	if (height <= 0)
		height = contentHeight;
	int left = s(16), right = width - s(16);

	//fields for a shortcut and its text, with the buttons after them
	wstring name = getEditText(ID_NAME);
	bool exists = !name.empty() && hasMacro(wideStringToUtf8(name));
	wstring addText_ = exists ? TR("Lưu", "Save") : TR("Thêm", "Add");
	wstring deleteText = TR("Xoá", "Delete");
	int buttonW = max(buttonWidth(TR("Thêm", "Add")), buttonWidth(TR("Lưu", "Save")));
	int deleteW = buttonWidth(deleteText);
	int y = s(14);
	int nameW = s(NAME_WIDTH);
	int contentLeft = left + nameW + s(8);
	int contentRight = right - buttonW - deleteW - s(16);
	addText(TR("Từ gõ tắt", "Shortcut"), makeRect(left + s(2), y, nameW, s(18)), fontCaption, pal.sub);
	addText(TR("Nội dung", "Text"), makeRect(contentLeft + s(2), y, contentRight - contentLeft, s(18)), fontCaption, pal.sub);
	y += s(22);
	addEdit(ID_NAME, makeRect(left, y, nameW, s(34)), TR("vd: vn", "e.g. vn"));
	addEdit(ID_CONTENT, makeRect(contentLeft, y, contentRight - contentLeft, s(34)), TR("vd: Việt Nam", "e.g. Việt Nam"));
	bool ready = !name.empty() && !getEditText(ID_CONTENT).empty();
	addButton(ID_ADD, addText_, makeRect(contentRight + s(8), y + s(1), buttonW, s(32)), true, ready);
	addButton(ID_DELETE, deleteText, makeRect(right - deleteW, y + s(1), deleteW, s(32)), false, exists);
	y += s(34) + s(14);

	//the list in a card: a heading row, then the shortcuts
	int footerH = s(56);
	int cardBottom = height - footerH;
	addCard(makeRect(left, y, right - left, max(s(80), cardBottom - y)));
	int headH = s(32);
	addText(TR("Từ gõ tắt", "Shortcut"), makeRect(left + s(14), y, nameW - s(14), headH), fontStrong);
	addText(TR("Nội dung", "Text"), makeRect(left + nameW + s(8), y, s(200), headH), fontStrong);
	TCHAR count[64];
	wsprintf(count, TR("%d từ", "%d shortcuts"), (int)rows.size());
	addText(count, makeRect(left, y, right - left - s(14), headH), fontCaption, pal.sub, 2);
	y += headH;
	addDivider(left + 1, right - 1, y);
	listArea = makeRect(left + 1, y + 1, right - left - 2, max(s(40), cardBottom - y - s(2)));
	Control list;
	list.kind = F_CUSTOM;
	list.id = ID_LIST;
	list.rc = listArea;
	add(list);
	scrollTo(scroll);

	//footer: auto caps on the left, files on the right
	int footerY = height - footerH + s(12);
	wstring importText = TR("Nạp từ file…", "Import…"), exportText = TR("Xuất ra file…", "Export…");
	int importW = buttonWidth(importText), exportW = buttonWidth(exportText);
	addButton(ID_EXPORT, exportText, makeRect(right - exportW, footerY, exportW, s(32)));
	addButton(ID_IMPORT, importText, makeRect(right - exportW - s(8) - importW, footerY, importW, s(32)));
	addSwitchRow(ID_AUTO_CAPS, TR("Tự viết hoa theo từ gõ tắt", "Match the capitalization"), L"",
		makeRect(left - s(14), footerY - s(4), max(s(200), right - exportW - importW - s(24) - left + s(14)), s(40)), vAutoCapsMacro != 0, false);
}

/*-------------------------------- list --------------------------------*/

int MacroWindow::maxScroll() const {
	return max(0, (int)rows.size() * rowHeight() - (int)(listArea.bottom - listArea.top));
}

void MacroWindow::scrollTo(const int& y) {
	scroll = max(0, min(y, maxScroll()));
	InvalidateRect(hWnd, NULL, FALSE);
}

bool MacroWindow::getThumb(RECT& track, RECT& thumb) const {
	int view = listArea.bottom - listArea.top;
	int total = (int)rows.size() * rowHeight();
	if (total <= view || view <= 0)
		return false;
	track = makeRect(listArea.right - s(10), listArea.top + s(4), s(8), view - s(8));
	int trackH = track.bottom - track.top;
	int thumbH = max(s(28), trackH * view / total);
	int thumbTop = track.top + (trackH - thumbH) * scroll / max(1, maxScroll());
	thumb = makeRect(track.left + s(2), thumbTop, s(4), thumbH);
	return true;
}

int MacroWindow::rowAt(const POINT& pt) const {
	if (!PtInRect(&listArea, pt))
		return -1;
	int row = (pt.y - listArea.top + scroll) / rowHeight();
	return row < (int)rows.size() ? row : -1;
}

void MacroWindow::select(const int& row) {
	if (row < 0 || row >= (int)rows.size())
		return;
	selected = row;
	setEditText(ID_NAME, rows[row].first);
	setEditText(ID_CONTENT, rows[row].second);
	//keep it in view
	int top = row * rowHeight(), view = listArea.bottom - listArea.top;
	if (top < scroll)
		scrollTo(top);
	else if (top + rowHeight() > scroll + view)
		scrollTo(top + rowHeight() - view);
	relayout();
}

void MacroWindow::paintCustom(Gdiplus::Graphics& g, const Control& c) {
	Gdiplus::Region oldClip;
	g.GetClip(&oldClip);
	g.SetClip(Gdiplus::Rect(c.rc.left, c.rc.top, c.rc.right - c.rc.left, c.rc.bottom - c.rc.top));
	int rowH = rowHeight();
	if (rows.empty()) {
		RECT empty = c.rc;
		InflateRect(&empty, -s(24), 0);
		drawText(g, TR("Chưa có từ gõ tắt nào. Nhập từ gõ tắt và nội dung ở trên rồi bấm Thêm.",
			"No shortcuts yet. Type a shortcut and its text above, then press Add."), empty, fontBody, pal.sub, 1, 1, true);
	}
	int first = scroll / rowH;
	int nameRight = c.rc.left - 1 + s(NAME_WIDTH);
	for (int i = first; i < (int)rows.size(); i++) {
		int top = c.rc.top + i * rowH - scroll;
		if (top >= c.rc.bottom)
			break;
		RECT row = makeRect(c.rc.left, top, c.rc.right - c.rc.left, rowH);
		if (i == selected) {
			RECT fill = row;
			InflateRect(&fill, -s(4), -s(2));
			fillRound(g, fill, (float)s(4), fade(pal.accent, 18));
			RECT bar = makeRect(fill.left, centerY(fill) - s(8), s(3), s(16));
			fillRound(g, bar, (float)s(1), pal.accent);
		} else if (i == hoverRow) {
			RECT fill = row;
			InflateRect(&fill, -s(4), -s(2));
			fillRound(g, fill, (float)s(4), pal.cardHover);
		}
		drawText(g, rows[i].first, makeRect(row.left + s(13), top, nameRight - row.left - s(13), rowH), fontBody, pal.text, 0, 1);
		drawText(g, rows[i].second, makeRect(nameRight + s(8), top, row.right - nameRight - s(28), rowH), fontBody, pal.sub, 0, 1);
	}
	RECT track, thumb;
	if (getThumb(track, thumb))
		fillRound(g, thumb, (thumb.right - thumb.left) / 2.0f, fade(pal.sub, draggingThumb ? 80 : 50));
	g.SetClip(&oldClip);
}

bool MacroWindow::onCustomMouse(const Control& c, const UINT& uMsg, const POINT& pt) {
	RECT track, thumb;
	switch (uMsg) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if (getThumb(track, thumb) && PtInRect(&track, pt)) {
			if (PtInRect(&thumb, pt)) {
				draggingThumb = true;
				dragStartY = pt.y;
				dragStartScroll = scroll;
				return true;
			}
			scrollTo(scroll + (pt.y < thumb.top ? -1 : 1) * (listArea.bottom - listArea.top) * 9 / 10);
			return false;
		}
		select(rowAt(pt));
		if (uMsg == WM_LBUTTONDBLCLK)
			focusEdit(ID_CONTENT);
		return false;
	case WM_MOUSEMOVE:
		if (draggingThumb) {
			if (getThumb(track, thumb)) {
				int range = (track.bottom - track.top) - (thumb.bottom - thumb.top);
				if (range > 0)
					scrollTo(dragStartScroll + (pt.y - dragStartY) * maxScroll() / range);
			}
		} else if (rowAt(pt) != hoverRow) {
			hoverRow = rowAt(pt);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		return false;
	case WM_MOUSELEAVE:
		hoverRow = -1;
		InvalidateRect(hWnd, NULL, FALSE);
		return false;
	case WM_LBUTTONUP:
		draggingThumb = false;
		InvalidateRect(hWnd, NULL, FALSE);
		return false;
	}
	return false;
}

bool MacroWindow::onCustomKey(const Control& c, const WPARAM& key) {
	int page = max(1, (int)(listArea.bottom - listArea.top) / rowHeight() - 1);
	switch (key) {
	case VK_UP:
		select(selected < 0 ? 0 : max(0, selected - 1));
		return true;
	case VK_DOWN:
		select(selected < 0 ? 0 : min((int)rows.size() - 1, selected + 1));
		return true;
	case VK_PRIOR:
		select(max(0, selected - page));
		return true;
	case VK_NEXT:
		select(min((int)rows.size() - 1, selected + page));
		return true;
	case VK_HOME:
		select(0);
		return true;
	case VK_END:
		select((int)rows.size() - 1);
		return true;
	case VK_DELETE:
		deleteShortcut();
		return true;
	}
	return false;
}

bool MacroWindow::onWheel(const POINT& pt, const int& delta) {
	if (!PtInRect(&listArea, pt))
		return false;
	scrollTo(scroll - delta * rowHeight() * 3 / WHEEL_DELTA);
	return true;
}

/*-------------------------------- actions --------------------------------*/

void MacroWindow::onEditChange(const int& id) {
	relayout(); //Add or Save, and whether Delete applies
}

void MacroWindow::onEditEnter(const int& id) {
	if (id == ID_NAME && getEditText(ID_CONTENT).empty())
		focusEdit(ID_CONTENT);
	else
		addShortcut();
}

void MacroWindow::onActivate(const int& id) {
	switch (id) {
	case ID_ADD:
		addShortcut();
		break;
	case ID_DELETE:
		deleteShortcut();
		break;
	case ID_IMPORT:
		importFile();
		break;
	case ID_EXPORT:
		exportFile();
		break;
	}
}

void MacroWindow::onToggle(const int& id) {
	if (id == ID_AUTO_CAPS) {
		APP_SET_DATA(vAutoCapsMacro, vAutoCapsMacro ? 0 : 1);
		AppDelegate::getInstance()->onSettingsChanged();
	}
}

void MacroWindow::addShortcut() {
	wstring name = getEditText(ID_NAME), content = getEditText(ID_CONTENT);
	if (name.empty() || content.empty()) {
		MessageBeep(MB_OK);
		focusEdit(name.empty() ? ID_NAME : ID_CONTENT);
		return;
	}
	addMacro(wideStringToUtf8(name), wideStringToUtf8(content));
	save();
	selected = -1;
	setEditText(ID_NAME, L"");
	setEditText(ID_CONTENT, L"");
	scrollTo(0); //new ones are at the top
	relayout();
	focusEdit(ID_NAME);
}

void MacroWindow::deleteShortcut() {
	wstring name = getEditText(ID_NAME);
	if (name.empty() || !deleteMacro(wideStringToUtf8(name))) {
		MessageBeep(MB_OK);
		return;
	}
	save();
	selected = -1;
	setEditText(ID_NAME, L"");
	setEditText(ID_CONTENT, L"");
	relayout();
	focusEdit(ID_NAME);
}

void MacroWindow::importFile() {
	OPENFILENAME ofn = { sizeof(ofn) };
	TCHAR file[MAX_PATH] = { 0 };
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = file;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = TR("Tệp văn bản (*.txt)\0*.txt\0Mọi tệp (*.*)\0*.*\0", "Text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0");
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (!GetOpenFileName(&ofn))
		return;
	bool append = true;
	if (!rows.empty()) {
		MessageWindow::Options options;
		options.heading = TR("Nạp gõ tắt từ file", "Import shortcuts");
		options.text = TR("Giữ các từ gõ tắt đang có và thêm từ file vào, hay thay toàn bộ bằng nội dung file?",
			"Keep the shortcuts you have and add the file's, or replace them all with the file?");
		options.buttons = { TR("Giữ và thêm vào", "Keep and add"), TR("Thay toàn bộ", "Replace all") };
		int answer = MessageWindow::show(hWnd, options);
		if (answer < 0)
			return;
		append = answer == 0;
	}
	readFromFile(wideStringToUtf8(file), append);
	save();
	relayout();
}

void MacroWindow::exportFile() {
	OPENFILENAME ofn = { sizeof(ofn) };
	TCHAR file[MAX_PATH] = _T("EasyKeyMacro.txt");
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = file;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = TR("Tệp văn bản (*.txt)\0*.txt\0", "Text files (*.txt)\0*.txt\0");
	ofn.lpstrDefExt = _T("txt");
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	if (!GetSaveFileName(&ofn))
		return;
	saveToFile(wideStringToUtf8(file));
	AppDelegate::getInstance()->showNotice(TR("Đã xuất bảng gõ tắt", "Shortcuts exported"), file);
}
