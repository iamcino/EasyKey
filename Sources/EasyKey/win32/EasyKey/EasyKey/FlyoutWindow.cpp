/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11, based on OpenKey.
Copyright (C) 2026 Cino

This file is belong to the OpenKey project, Win32 version
which is released under GPL license.
You can fork, modify, improve this program. If you
redistribute your new version, it MUST be open source.
-----------------------------------------------------------*/
//GDI+ needs the full windows.h and objidl.h, include them before stdafx.h trims windows.h down
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <cmath>
#include <shellapi.h>
#include "FlyoutWindow.h"
#include "ThemeHelper.h"

#define FLYOUT_CLASS _T("EasyKeySwitchFlyout")
#define WM_FLYOUT_SHOW (WM_APP + 1)
#define WM_FLYOUT_MESSAGE (WM_APP + 2)
#define MESSAGE_HOLD_TIME 4000
#define TIMER_HOLD 1
#define TIMER_FADE 2
#define HOLD_TIME 1200
#define FADE_STEP 16
#define FADE_FRAMES 12
#define BOTTOM_GAP 36 //above the taskbar, in DIPs
#define SHADOW 16

typedef HRESULT(WINAPI *GetDpiForMonitorFunc)(HMONITOR, int, UINT*, UINT*);

static LPCTSTR codeTableName(const int& code) {
	static LPCTSTR names[] = { _T("Unicode"), _T("TCVN3"), _T("VNI Windows"), NULL, _T("CP 1258") };
	if (code == 3)
		return TR("Unicode tổ hợp", "Unicode combining");
	return code >= 0 && code < 5 ? names[code] : names[0];
}

//a game, video or presentation fills the screen: stay out of the way
static bool isFullScreenBusy() {
	QUERY_USER_NOTIFICATION_STATE state;
	if (FAILED(SHQueryUserNotificationState(&state)))
		return false;
	return state == QUNS_BUSY || state == QUNS_RUNNING_D3D_FULL_SCREEN || state == QUNS_PRESENTATION_MODE;
}

static UINT monitorDpi(const HMONITOR& monitor) {
	static GetDpiForMonitorFunc getDpiForMonitor = NULL;
	static bool loaded = false;
	if (!loaded) {
		loaded = true;
		HMODULE shcore = LoadLibrary(_T("shcore.dll"));
		if (shcore)
			getDpiForMonitor = (GetDpiForMonitorFunc)GetProcAddress(shcore, "GetDpiForMonitor");
	}
	UINT x = USER_DEFAULT_SCREEN_DPI, y = USER_DEFAULT_SCREEN_DPI;
	if (!getDpiForMonitor || FAILED(getDpiForMonitor(monitor, 0, &x, &y))) //MDT_EFFECTIVE_DPI
		x = USER_DEFAULT_SCREEN_DPI;
	return x;
}

static void addPill(Gdiplus::GraphicsPath& path, const float& x, const float& y, const float& w, const float& h) {
	float d = h;
	path.AddArc(x, y, d, d, 90, 180);
	path.AddArc(x + w - d, y, d, d, 270, 180);
	path.CloseFigure();
}

FlyoutWindow::FlyoutWindow(const HINSTANCE& hInstance) {
	this->hInstance = hInstance;
	ThemeHelper::startGdiplus();
	WNDCLASSEX wc = { sizeof(wc) };
	wc.lpfnWndProc = windowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = FLYOUT_CLASS;
	RegisterClassEx(&wc);
	CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
		FLYOUT_CLASS, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, NULL, hInstance, this);
}

FlyoutWindow::~FlyoutWindow() {
	if (hWnd) {
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		DestroyWindow(hWnd);
		hWnd = NULL;
	}
	freeBitmap();
}

void FlyoutWindow::notify() {
	if (hWnd && vShowSwitchFlyout)
		PostMessage(hWnd, WM_FLYOUT_SHOW, 0, 0);
}

void FlyoutWindow::showMessage(const wstring& title, const wstring& detail, const bool& warning) {
	if (!hWnd)
		return;
	messageTitle = title;
	messageDetail = detail;
	messageWarning = warning;
	PostMessage(hWnd, WM_FLYOUT_MESSAGE, 0, 0);
}

void FlyoutWindow::freeBitmap() {
	if (memDC) {
		DeleteDC(memDC);
		memDC = NULL;
	}
	if (bitmap) {
		DeleteObject(bitmap);
		bitmap = NULL;
	}
}

void FlyoutWindow::showNow(const bool& message) {
	if (isFullScreenBusy())
		return;
	//on the screen being typed on
	HWND foreground = GetForegroundWindow();
	HMONITOR monitor = foreground ? MonitorFromWindow(foreground, MONITOR_DEFAULTTOPRIMARY) : MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi = { sizeof(mi) };
	GetMonitorInfo(monitor, &mi);
	render(monitorDpi(monitor), mi.rcWork, message);
	alpha = 255;
	present();
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	KillTimer(hWnd, TIMER_FADE);
	SetTimer(hWnd, TIMER_HOLD, message ? MESSAGE_HOLD_TIME : HOLD_TIME, NULL);
}

void FlyoutWindow::render(const UINT& dpi, const RECT& workArea, const bool& message) {
	auto s = [&](const float& value) { return value * dpi / USER_DEFAULT_SCREEN_DPI; };
	bool dark = ThemeHelper::isAppDarkMode();
	bool vietnamese = vLanguage != 0;
	DWORD text = dark ? 0xFFFFFFFF : 0xFF1B1B1B;
	DWORD sub = dark ? 0xFFC8C8C8 : 0xFF595959;
	DWORD fill = dark ? 0xF52C2C2C : 0xF7F9F9F9;
	DWORD frame = dark ? 0x1AFFFFFF : 0x1A000000;
	COLORREF accent = ThemeHelper::getAccentColor(dark, false);
	DWORD accentColor = 0xFF000000 | ((DWORD)GetRValue(accent) << 16) | ((DWORD)GetGValue(accent) << 8) | GetBValue(accent);

	wstring title = vietnamese ? TR("Tiếng Việt", "Vietnamese") : TR("Tiếng Anh", "English");
	wstring detail;
	if (message) {
		title = messageTitle;
		detail = messageDetail;
	} else if (vietnamese) {
		vector<LPCTSTR>& inputTypes = EasyKeyManager::getInputType();
		detail = (vInputType >= 0 && vInputType < (int)inputTypes.size() ? inputTypes[vInputType] : _T("")) + wstring(_T(" · ")) + codeTableName(vCodeTable);
	} else {
		detail = TR("Bấm ", "Press ") + EasyKeyHelper::getHotKeyString(vSwitchKeyStatus) + TR(" để gõ tiếng Việt", " to type Vietnamese");
	}

	Gdiplus::Font* titleFont = ThemeHelper::createFont(true, s(16), Gdiplus::FontStyleRegular);
	Gdiplus::Font* detailFont = ThemeHelper::createFont(false, s(12), Gdiplus::FontStyleRegular);
	Gdiplus::Font* badgeFont = ThemeHelper::createFont(false, s(17), Gdiplus::FontStyleBold);
	Gdiplus::StringFormat* format = Gdiplus::StringFormat::GenericTypographic()->Clone();
	format->SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap | Gdiplus::StringFormatFlagsMeasureTrailingSpaces);

	//measure the text, then size the pill around it
	float textWidth = 0;
	{
		HDC screen = GetDC(NULL);
		Gdiplus::Graphics g(screen);
		g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
		Gdiplus::RectF box;
		g.MeasureString(title.c_str(), -1, titleFont, Gdiplus::PointF(0, 0), format, &box);
		textWidth = box.Width;
		g.MeasureString(detail.c_str(), -1, detailFont, Gdiplus::PointF(0, 0), format, &box);
		textWidth = max(textWidth, box.Width);
		ReleaseDC(NULL, screen);
	}
	float margin = s(SHADOW);
	float pillHeight = s(64);
	float pillWidth = s(12) + s(40) + s(14) + ceil(textWidth) + s(22);
	size.cx = (LONG)ceil(pillWidth + margin * 2);
	size.cy = (LONG)ceil(pillHeight + margin * 2);
	position.x = (workArea.left + workArea.right - size.cx) / 2;
	position.y = workArea.bottom - (LONG)s(BOTTOM_GAP) - (LONG)(pillHeight + margin);

	freeBitmap();
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = size.cx;
	bmi.bmiHeader.biHeight = -size.cy;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	void* bits = NULL;
	HDC screen = GetDC(NULL);
	bitmap = CreateDIBSection(screen, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	memDC = CreateCompatibleDC(screen);
	ReleaseDC(NULL, screen);
	if (bitmap) {
		SelectObject(memDC, bitmap);
		Gdiplus::Bitmap canvas(size.cx, size.cy, size.cx * 4, PixelFormat32bppPARGB, (BYTE*)bits);
		Gdiplus::Graphics g(&canvas);
		g.Clear(Gdiplus::Color(0, 0, 0, 0));
		g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
		g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

		//soft shadow from a few widening pills, a little lower than the pill
		int steps = 8;
		for (int i = steps; i >= 1; i--) {
			float grow = margin * i / steps;
			Gdiplus::GraphicsPath shadow;
			addPill(shadow, margin - grow, margin - grow + s(4), pillWidth + grow * 2, pillHeight + grow * 2);
			Gdiplus::SolidBrush brush(Gdiplus::Color((BYTE)(dark ? 14 : 8), 0, 0, 0));
			g.FillPath(&brush, &shadow);
		}

		Gdiplus::GraphicsPath pill;
		addPill(pill, margin + 0.5f, margin + 0.5f, pillWidth - 1, pillHeight - 1);
		Gdiplus::SolidBrush fillBrush((Gdiplus::ARGB)fill);
		g.FillPath(&fillBrush, &pill);
		Gdiplus::Pen framePen((Gdiplus::ARGB)frame, 1);
		g.DrawPath(&framePen, &pill);

		float badgeX = margin + s(12), badgeY = margin + (pillHeight - s(40)) / 2;
		bool filled = vietnamese || message;
		bool warningBadge = message && messageWarning;
		Gdiplus::SolidBrush badgeBrush((Gdiplus::ARGB)(filled ? accentColor : (dark ? 0x14FFFFFF : 0x0D000000)));
		g.FillEllipse(&badgeBrush, badgeX, badgeY, s(40), s(40));
		Gdiplus::SolidBrush letterBrush((Gdiplus::ARGB)(filled ? (dark ? 0xFF000000 : 0xFFFFFFFF) : text));
		Gdiplus::StringFormat centered(format);
		centered.SetAlignment(Gdiplus::StringAlignmentCenter);
		centered.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		g.DrawString(warningBadge ? _T("!") : vietnamese ? _T("V") : _T("E"), -1, badgeFont, Gdiplus::RectF(badgeX, badgeY, s(40), s(40)), &centered, &letterBrush);

		float textX = badgeX + s(40) + s(14);
		float middle = margin + pillHeight / 2;
		Gdiplus::StringFormat bottom(format);
		bottom.SetLineAlignment(Gdiplus::StringAlignmentFar);
		Gdiplus::SolidBrush titleBrush((Gdiplus::ARGB)text);
		g.DrawString(title.c_str(), -1, titleFont, Gdiplus::RectF(textX, margin, ceil(textWidth) + 1, middle + s(1) - margin), &bottom, &titleBrush);
		Gdiplus::SolidBrush detailBrush((Gdiplus::ARGB)sub);
		g.DrawString(detail.c_str(), -1, detailFont, Gdiplus::RectF(textX, middle + s(2), ceil(textWidth) + 1, pillHeight / 2), format, &detailBrush);
	}

	delete format;
	delete titleFont;
	delete detailFont;
	delete badgeFont;
}

void FlyoutWindow::present() {
	if (!bitmap)
		return;
	POINT source = { 0, 0 };
	BLENDFUNCTION blend = { AC_SRC_OVER, 0, (BYTE)alpha, AC_SRC_ALPHA };
	UpdateLayeredWindow(hWnd, NULL, &position, &size, memDC, &source, 0, &blend, ULW_ALPHA);
}

LRESULT CALLBACK FlyoutWindow::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_NCCREATE) {
		FlyoutWindow* window = (FlyoutWindow*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		window->hWnd = hWnd;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
	}
	FlyoutWindow* window = (FlyoutWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (window)
		return window->handleMessage(uMsg, wParam, lParam);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT FlyoutWindow::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_FLYOUT_SHOW:
		showNow(false);
		return 0;
	case WM_FLYOUT_MESSAGE:
		showNow(true);
		return 0;
	case WM_TIMER:
		if (wParam == TIMER_HOLD) {
			KillTimer(hWnd, TIMER_HOLD);
			SetTimer(hWnd, TIMER_FADE, FADE_STEP, NULL);
		} else if (wParam == TIMER_FADE) {
			alpha -= 255 / FADE_FRAMES;
			if (alpha <= 0) {
				KillTimer(hWnd, TIMER_FADE);
				ShowWindow(hWnd, SW_HIDE);
				freeBitmap();
			} else {
				present();
			}
		}
		return 0;
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	case WM_NCHITTEST:
		return HTTRANSPARENT;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
