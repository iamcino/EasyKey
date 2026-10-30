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
#include "FluentWindow.h"

//the shortcut list: add, change, delete, import and export shortcuts
class MacroWindow : public FluentWindow {
private:
	vector<pair<wstring, wstring>> rows; //newest first
	int selected = -1, hoverRow = -1;
	int scroll = 0;                      //pixels
	bool draggingThumb = false;
	int dragStartY = 0, dragStartScroll = 0;
	RECT listArea = {};
private:
	void load();
	void save();
	int rowHeight() const;
	int maxScroll() const;
	void scrollTo(const int& y);
	bool getThumb(RECT& track, RECT& thumb) const;
	int rowAt(const POINT& pt) const;
	void select(const int& row);
	void addShortcut();
	void deleteShortcut();
	void importFile();
	void exportFile();
public:
	MacroWindow(const HINSTANCE& hInstance);
	void show();
	void fillData() override;
protected:
	void layout() override;
	wstring title() const override;
	void onActivate(const int& id) override;
	void onToggle(const int& id) override;
	void onEditChange(const int& id) override;
	void onEditEnter(const int& id) override;
	void paintCustom(Gdiplus::Graphics& g, const Control& c) override;
	bool onCustomMouse(const Control& c, const UINT& uMsg, const POINT& pt) override;
	bool onCustomKey(const Control& c, const WPARAM& key) override;
	bool onWheel(const POINT& pt, const int& delta) override;
};
