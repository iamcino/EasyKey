/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
#pragma once
#include "FluentWindow.h"

/*
 * A question or notice in EasyKey's own style, in place of MessageBox, so it
 * follows the color mode. It waits for an answer like MessageBox does.
 */
class MessageWindow : public FluentWindow {
public:
	struct Options {
		wstring heading;          //bold first line
		wstring text;             //wraps
		vector<wstring> buttons;  //left to right; the first one is filled with the accent
		wstring check;            //a switch below the text, such as "Don't remind me again"
		bool checked = false;     //in and out
		wstring link, linkUrl;    //a link below the text
	};
	//index of the button pressed, -1 when the window was closed
	static int show(const HWND& owner, Options& options);
	//one OK button
	static void notice(const HWND& owner, const wstring& heading, const wstring& text);
	//true for yes
	static bool ask(const HWND& owner, const wstring& heading, const wstring& text, const wstring& yes, const wstring& no);
private:
	Options* options = NULL;
	int result = -1;
	bool done = false;
	MessageWindow();
protected:
	void layout() override;
	wstring title() const override;
	void onActivate(const int& id) override;
	void onToggle(const int& id) override;
	void onClosed() override;
};
