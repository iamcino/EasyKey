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
#include "AboutWindow.h"
#include "Updater.h"

#define WINDOW_WIDTH 400

enum {
	ID_HOME = 1, ID_RELEASES, ID_OPENKEY, ID_UPDATE
};

AboutWindow::AboutWindow(const HINSTANCE& hInstance) : FluentWindow(hInstance) {
}

void AboutWindow::show() {
	create(title(), WINDOW_WIDTH, NULL);
}

wstring AboutWindow::title() const {
	return TR("Giới thiệu EasyKey", "About EasyKey");
}

void AboutWindow::layout() {
	int width = clientWidth();
	int pad = s(24);
	int y = s(24);

	//the V badge, the name and the version
	int badge = s(56);
	addBadge(makeRect(pad, y, badge, badge));
	int x = pad + badge + s(16);
	int textW = width - pad - x;
	addText(L"EasyKey", makeRect(x, y - s(4), textW, s(38)), fontDisplay);
	addText(TR("Bộ gõ tiếng Việt cho Windows 11", "Vietnamese keyboard for Windows 11"), makeRect(x, y + s(34), textW, s(20)), fontBody, pal.sub);
	addText(TR("Phiên bản ", "Version ") + EasyKeyHelper::getVersionString(), makeRect(x, y + s(54), textW, s(18)), fontCaption, pal.sub);
	y += s(84);

	//links, one row each
	struct { int id; LPCTSTR label; LPCTSTR link; } rows[] = {
		{ ID_HOME, TR("Trang chủ", "Home page"), _T("github.com/iamcino/EasyKey") },
		{ ID_RELEASES, TR("Các phiên bản", "Releases"), TR("Xem trên GitHub", "See on GitHub") },
		{ ID_OPENKEY, TR("Dựa trên", "Based on"), TR("OpenKey của Mai Vũ Tuyên (GPLv3)", "OpenKey by Mai Vũ Tuyên (GPLv3)") },
	};
	int rowH = s(44), left = pad, right = width - pad;
	addCard(makeRect(left, y, right - left, rowH * 3));
	for (int i = 0; i < 3; i++) {
		if (i > 0)
			addDivider(left + 1, right - 1, y);
		addText(rows[i].label, makeRect(left + s(14), y, s(120), rowH), fontBody);
		int linkW = textWidth(fontSmall, rows[i].link);
		addLink(rows[i].id, rows[i].link, right - s(14) - linkW, y + s(10), rowH - s(20));
		y += rowH;
	}
	y += s(16);

	wstring update = TR("Kiểm tra cập nhật", "Check for updates");
	addButton(ID_UPDATE, update, makeRect(pad, y, buttonWidth(update), s(32)), true);
	addText(L"© 2026 Cino", makeRect(pad, y, width - pad * 2, s(32)), fontCaption, pal.sub, 2);
	y += s(32) + s(22);
	contentHeight = y;
}

void AboutWindow::onActivate(const int& id) {
	switch (id) {
	case ID_HOME:
		ShellExecute(NULL, _T("open"), _T("https://github.com/iamcino/EasyKey"), NULL, NULL, SW_SHOWNORMAL);
		break;
	case ID_RELEASES:
		ShellExecute(NULL, _T("open"), _T("https://github.com/iamcino/EasyKey/releases"), NULL, NULL, SW_SHOWNORMAL);
		break;
	case ID_OPENKEY:
		ShellExecute(NULL, _T("open"), _T("https://github.com/tuyenvm/OpenKey"), NULL, NULL, SW_SHOWNORMAL);
		break;
	case ID_UPDATE:
		Updater::checkAndOffer(hWnd, false);
		break;
	}
}
