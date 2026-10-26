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
#include "ConvertToolWindow.h"
#include "AppDelegate.h"

#define WINDOW_WIDTH 440

enum {
	ID_FROM = 1, ID_TO, ID_SWAP, ID_CASE, ID_REMOVE_MARK, ID_HOTKEY, ID_NOTIFY, ID_CONVERT, ID_CLOSE
};

//same order as the code table numbers
static LPCTSTR codeName(const int& code) {
	switch (code) {
	case 1: return _T("TCVN3 (ABC)");
	case 2: return _T("VNI Windows");
	case 3: return TR("Unicode tổ hợp", "Unicode combining");
	case 4: return _T("CP 1258");
	}
	return _T("Unicode");
}

static vector<wstring> letterCases() {
	return { TR("Giữ nguyên", "Keep as is"), TR("CHỮ HOA", "UPPERCASE"), TR("chữ thường", "lowercase"),
		TR("Hoa đầu câu", "Sentence case"), TR("Hoa Mỗi Từ", "Capitalize Each Word") };
}

static int letterCase() {
	if (convertToolToAllCaps) return 1;
	if (convertToolToAllNonCaps) return 2;
	if (convertToolToCapsFirstLetter) return 3;
	if (convertToolToCapsEachWord) return 4;
	return 0;
}

ConvertToolWindow::ConvertToolWindow(const HINSTANCE& hInstance) : FluentWindow(hInstance) {
}

void ConvertToolWindow::show() {
	create(title(), WINDOW_WIDTH, NULL);
}

wstring ConvertToolWindow::title() const {
	return TR("Chuyển mã", "Convert text");
}

void ConvertToolWindow::layout() {
	int width = clientWidth();
	int left = s(16), right = width - s(16);
	int rowH = s(48), fieldW = s(170);
	int y = s(14);
	auto section = [&](const wstring& text) {
		addText(text, makeRect(left + s(2), y, right - left, s(20)), fontStrong);
		y += s(26);
	};

	//code tables, with a link to swap them on the heading line
	section(TR("Bảng mã", "Encoding"));
	wstring swap = TR("⇅ Đảo chiều", "⇅ Swap");
	addLink(ID_SWAP, swap, right - s(2) - textWidth(fontSmall, swap), y - s(26), s(20));
	addCard(makeRect(left, y, right - left, rowH * 2));
	addComboRow(ID_FROM, TR("Từ bảng mã", "From"), codeName(convertToolFromCode), makeRect(left, y, right - left, rowH), fieldW, false);
	y += rowH;
	addComboRow(ID_TO, TR("Sang bảng mã", "To"), codeName(convertToolToCode), makeRect(left, y, right - left, rowH), fieldW, true);
	y += rowH + s(16);

	section(TR("Chữ", "Text"));
	addCard(makeRect(left, y, right - left, rowH * 2));
	addComboRow(ID_CASE, TR("Kiểu chữ", "Letter case"), letterCases()[letterCase()], makeRect(left, y, right - left, rowH), fieldW, false);
	y += rowH;
	addSwitchRow(ID_REMOVE_MARK, TR("Bỏ dấu", "Remove tone marks"), TR("Tiếng Việt → Tieng Viet", "Tiếng Việt → Tieng Viet"),
		makeRect(left, y, right - left, rowH), convertToolRemoveMark != 0, true);
	y += rowH + s(16);

	section(TR("Chuyển nhanh", "Quick convert"));
	addCard(makeRect(left, y, right - left, rowH * 2));
	addHotKeyRow(ID_HOTKEY, TR("Phím tắt", "Keys"), capturing == ID_HOTKEY ? 0 : convertToolHotKey, makeRect(left, y, right - left, rowH), false);
	y += rowH;
	addSwitchRow(ID_NOTIFY, TR("Báo khi chuyển xong", "Tell me when done"), L"", makeRect(left, y, right - left, rowH),
		convertToolDontAlertWhenCompleted == 0, true);
	y += rowH + s(12);

	wstring note = TR("EasyKey chuyển chữ đang có trong Clipboard: sao chép chữ cần chuyển, bấm Chuyển mã hoặc phím tắt, rồi dán ra.",
		"EasyKey converts the text in the clipboard: copy the text, press Convert or the keys, then paste it.");
	int noteH = textHeight(fontCaption, note, right - left - s(4));
	addText(note, makeRect(left + s(2), y, right - left - s(4), noteH), fontCaption, pal.sub, 0, true);
	y += noteH + s(16);

	//main button on the right, like Windows dialogs
	wstring convert = TR("Chuyển mã", "Convert"), closeText = TR("Đóng", "Close");
	int buttonW = max(buttonWidth(convert), buttonWidth(closeText));
	addButton(ID_CLOSE, closeText, makeRect(right - buttonW, y, buttonW, s(32)));
	addButton(ID_CONVERT, convert, makeRect(right - buttonW * 2 - s(8), y, buttonW, s(32)), true);
	y += s(32) + s(18);
	contentHeight = y;
}

void ConvertToolWindow::onActivate(const int& id) {
	switch (id) {
	case ID_FROM:
	case ID_TO: {
		vector<wstring> names;
		for (int code = 0; code < 5; code++)
			names.push_back(codeName(code));
		for (auto& c : controls) {
			if (c.kind == F_COMBO && c.id == id) {
				int chosen = showMenu(c.ctl, names, id == ID_FROM ? convertToolFromCode : convertToolToCode);
				if (chosen >= 0) {
					if (id == ID_FROM) {
						APP_SET_DATA(convertToolFromCode, chosen);
					} else {
						APP_SET_DATA(convertToolToCode, chosen);
					}
				}
				break;
			}
		}
		relayout();
		break;
	}
	case ID_SWAP: {
		int from = convertToolFromCode;
		APP_SET_DATA(convertToolFromCode, convertToolToCode);
		APP_SET_DATA(convertToolToCode, from);
		relayout();
		break;
	}
	case ID_CASE: {
		for (auto& c : controls) {
			if (c.kind == F_COMBO && c.id == ID_CASE) {
				int chosen = showMenu(c.ctl, letterCases(), letterCase());
				if (chosen >= 0) {
					//one case at a time
					APP_SET_DATA(convertToolToAllCaps, chosen == 1 ? 1 : 0);
					APP_SET_DATA(convertToolToAllNonCaps, chosen == 2 ? 1 : 0);
					APP_SET_DATA(convertToolToCapsFirstLetter, chosen == 3 ? 1 : 0);
					APP_SET_DATA(convertToolToCapsEachWord, chosen == 4 ? 1 : 0);
				}
				break;
			}
		}
		relayout();
		break;
	}
	case ID_CONVERT:
		AppDelegate::getInstance()->onQuickConvert();
		break;
	case ID_CLOSE:
		close();
		break;
	}
}

void ConvertToolWindow::onToggle(const int& id) {
	if (id == ID_REMOVE_MARK) {
		APP_SET_DATA(convertToolRemoveMark, convertToolRemoveMark ? 0 : 1);
	} else if (id == ID_NOTIFY) {
		APP_SET_DATA(convertToolDontAlertWhenCompleted, convertToolDontAlertWhenCompleted ? 0 : 1);
	}
	relayout();
}

bool ConvertToolWindow::onHotKey(const int& id, const int& hotKey) {
	//the language and input method keys keep theirs
	if (hotKey != EMPTY_HOTKEY_DATA &&
		(hotKey == (vSwitchKeyStatus & ~0x8000) || (vSwitchInputType && hotKey == vInputTypeHotKey)))
		return false;
	savedHotKey = hotKey;
	return true;
}

void ConvertToolWindow::onCaptureChange(const int& id, const bool& started) {
	if (started) {
		//the keyboard hook must not convert while the new keys are typed
		savedHotKey = convertToolHotKey;
		convertToolHotKey = EMPTY_HOTKEY_DATA;
	} else {
		APP_SET_DATA(convertToolHotKey, savedHotKey);
	}
}
