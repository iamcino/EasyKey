/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11, based on OpenKey.
Copyright (C) 2026 Cino

This file is belong to the OpenKey project, Win32 version
which is released under GPL license.
You can fork, modify, improve this program. If you
redistribute your new version, it MUST be open source.
-----------------------------------------------------------*/
#include "MessageWindow.h"

#define WINDOW_WIDTH 420
#define ID_BUTTON 100
#define ID_LINK 1
#define ID_CHECK 2

MessageWindow::MessageWindow() : FluentWindow(GetModuleHandle(NULL)) {
}

int MessageWindow::show(const HWND& owner, Options& options) {
	MessageWindow window;
	window.options = &options;
	HWND top = owner ? GetAncestor(owner, GA_ROOT) : NULL;
	bool disable = top && IsWindowEnabled(top);
	if (disable)
		EnableWindow(top, FALSE);
	window.create(window.title(), WINDOW_WIDTH, top);

	//wait for the answer; the keyboard hook keeps running, it is served by this loop too
	MSG msg;
	bool quit = false;
	while (!window.done && window.getHwnd()) {
		if (GetMessage(&msg, NULL, 0, 0) <= 0) {
			quit = true;
			break;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (disable)
		EnableWindow(top, TRUE);
	window.destroy();
	if (top)
		SetForegroundWindow(top);
	if (quit)
		PostQuitMessage((int)msg.wParam);
	return window.result;
}

void MessageWindow::notice(const HWND& owner, const wstring& heading, const wstring& text) {
	Options options;
	options.heading = heading;
	options.text = text;
	options.buttons.push_back(TR("OK", "OK"));
	show(owner, options);
}

bool MessageWindow::ask(const HWND& owner, const wstring& heading, const wstring& text, const wstring& yes, const wstring& no) {
	Options options;
	options.heading = heading;
	options.text = text;
	options.buttons.push_back(yes);
	options.buttons.push_back(no);
	return show(owner, options) == 0;
}

wstring MessageWindow::title() const {
	return L"EasyKey";
}

void MessageWindow::layout() {
	int width = clientWidth();
	int pad = s(24);
	int inner = width - pad * 2;
	int y = s(22);
	if (!options->heading.empty()) {
		int h = textHeight(fontTitle, options->heading, inner);
		addText(options->heading, makeRect(pad, y, inner, h), fontTitle, 0, 0, true);
		y += h + s(8);
	}
	if (!options->text.empty()) {
		int h = textHeight(fontBody, options->text, inner);
		addText(options->text, makeRect(pad, y, inner, h), fontBody, pal.sub, 0, true);
		y += h + s(10);
	}
	if (!options->link.empty()) {
		addLink(ID_LINK, options->link, pad, y, s(24));
		y += s(28);
	}
	if (!options->check.empty()) {
		addSwitchRow(ID_CHECK, options->check, L"", makeRect(pad - s(14), y, inner + s(28), s(40)), options->checked, false);
		y += s(40);
	}

	//buttons of one width at the bottom right, the main one first
	y += s(14);
	int buttonW = 0;
	for (auto& text : options->buttons)
		buttonW = max(buttonW, buttonWidth(text));
	int gap = s(8), count = (int)options->buttons.size();
	int x = width - pad - count * buttonW - (count - 1) * gap;
	int first = -1;
	for (int i = 0; i < count; i++) {
		int index = addButton(ID_BUTTON + i, options->buttons[i], makeRect(x, y, buttonW, s(32)), i == 0);
		if (i == 0)
			first = index;
		x += buttonW + gap;
	}
	y += s(32) + s(22);
	contentHeight = y;
	//Enter answers with the main button
	if (focus < 0)
		focus = first;
}

void MessageWindow::onActivate(const int& id) {
	if (id == ID_LINK) {
		ShellExecute(NULL, _T("open"), options->linkUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
		return;
	}
	result = id - ID_BUTTON;
	done = true;
}

void MessageWindow::onToggle(const int& id) {
	options->checked = !options->checked;
	relayout();
}

void MessageWindow::onClosed() {
	done = true;
}
