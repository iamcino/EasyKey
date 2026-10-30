/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
//Direct2D needs the full windows.h, include it before stdafx.h trims windows.h down
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <cmath>
#include <shellapi.h>
#include "FlyoutWindow.h"
#include "FluentText.h"
#include "ThemeHelper.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

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

static ID2D1Factory* drawFactory() {
	static ID2D1Factory* factory = NULL;
	static bool tried = false;
	if (!tried) {
		tried = true;
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
	}
	return factory;
}

static IDWriteFactory* writeFactory() {
	static IDWriteFactory* factory = NULL;
	static bool tried = false;
	if (!tried) {
		tried = true;
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&factory);
	}
	return factory;
}

static IDWriteTextFormat* textFormat(const DWRITE_FONT_WEIGHT& weight, const float& pixels) {
	IDWriteTextFormat* format = NULL;
	//the same font as the other windows; vi-VN picks the Vietnamese accent shapes of Segoe UI when it stands in
	if (!writeFactory() || FAILED(writeFactory()->CreateTextFormat(fluentFontFamily(), fluentFontCollection(), weight, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, pixels, L"vi-VN", &format)))
		return NULL;
	format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	return format;
}

static IDWriteTextLayout* textLayout(const wstring& text, IDWriteTextFormat* format, const float& height) {
	IDWriteTextLayout* layout = NULL;
	if (!format || FAILED(writeFactory()->CreateTextLayout(text.c_str(), (UINT32)text.length(), format, 10000, height, &layout)))
		return NULL;
	return layout;
}

static float textWidthOf(IDWriteTextLayout* layout) {
	DWRITE_TEXT_METRICS metrics;
	if (!layout || FAILED(layout->GetMetrics(&metrics)))
		return 0;
	return metrics.widthIncludingTrailingWhitespace;
}

static D2D1_COLOR_F colorOf(const DWORD& argb) {
	return D2D1::ColorF(((argb >> 16) & 0xFF) / 255.0f, ((argb >> 8) & 0xFF) / 255.0f, (argb & 0xFF) / 255.0f, (argb >> 24) / 255.0f);
}

static void fillPill(ID2D1RenderTarget* target, const D2D1_RECT_F& rect, const DWORD& argb) {
	ID2D1SolidColorBrush* brush = NULL;
	if (FAILED(target->CreateSolidColorBrush(colorOf(argb), &brush)))
		return;
	float radius = (rect.bottom - rect.top) / 2;
	target->FillRoundedRectangle(D2D1::RoundedRect(rect, radius, radius), brush);
	brush->Release();
}

static void drawLayout(ID2D1RenderTarget* target, IDWriteTextLayout* layout, const float& x, const float& y, const DWORD& argb) {
	ID2D1SolidColorBrush* brush = NULL;
	if (!layout || FAILED(target->CreateSolidColorBrush(colorOf(argb), &brush)))
		return;
	target->DrawTextLayout(D2D1::Point2F(x, y), layout, brush);
	brush->Release();
}

FlyoutWindow::FlyoutWindow(const HINSTANCE& hInstance) {
	this->hInstance = hInstance;
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

void FlyoutWindow::notify(const int& what) {
	if (hWnd && vShowSwitchFlyout)
		PostMessage(hWnd, WM_FLYOUT_SHOW, what, 0);
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

	wstring title = !vietnamese ? TR("Tiếng Anh", "English") : vMixedEnglish ? TR("Anh–Việt", "English + Vietnamese") : TR("Tiếng Việt", "Vietnamese");
	wstring detail;
	vector<LPCTSTR>& inputTypes = EasyKeyManager::getInputType();
	wstring inputType = vInputType >= 0 && vInputType < (int)inputTypes.size() ? inputTypes[vInputType] : _T("");
	if (message) {
		title = messageTitle;
		detail = messageDetail;
	} else if (shown == FLYOUT_CODE_TABLE) {
		title = codeTableName(vCodeTable);
		detail = TR("Bảng mã · ", "Character encoding · ") +
			(vietnamese ? inputType : wstring(TR("dùng khi gõ tiếng Việt", "used when typing Vietnamese")));
	} else if (shown == FLYOUT_INPUT_TYPE) {
		title = inputType;
		detail = TR("Kiểu gõ · ", "Input method · ") +
			(vietnamese ? wstring(codeTableName(vCodeTable)) : wstring(TR("dùng khi gõ tiếng Việt", "used when typing Vietnamese")));
	} else if (vietnamese) {
		detail = inputType + wstring(_T(" · ")) + codeTableName(vCodeTable);
	} else {
		detail = TR("Bấm ", "Press ") + EasyKeyHelper::getHotKeyString(vSwitchKeyStatus) + TR(" để gõ tiếng Việt", " to type Vietnamese");
	}

	//the text goes through DirectWrite: GDI+ snaps glyphs to the pixel grid,
	//which breaks them up when the screen is scaled above 100%
	float margin = s(SHADOW);
	float pillHeight = s(64);
	float titleHeight = pillHeight / 2 + s(1);
	IDWriteTextFormat* titleFormat = textFormat(DWRITE_FONT_WEIGHT_SEMI_BOLD, s(16));
	IDWriteTextFormat* detailFormat = textFormat(DWRITE_FONT_WEIGHT_NORMAL, s(12));
	IDWriteTextFormat* badgeFormat = textFormat(DWRITE_FONT_WEIGHT_BOLD, s(17));
	IDWriteTextLayout* titleLayout = textLayout(title, titleFormat, titleHeight);
	IDWriteTextLayout* detailLayout = textLayout(detail, detailFormat, pillHeight / 2);
	IDWriteTextLayout* badgeLayout = textLayout(message && messageWarning ? _T("!") : !vietnamese ? _T("E") : vMixedEnglish ? _T("EV") : _T("V"), badgeFormat, s(40));
	if (titleLayout)
		titleLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
	if (badgeLayout) {
		badgeLayout->SetMaxWidth(s(40));
		badgeLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		badgeLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}

	//measure the text, then size the pill around it
	float textWidth = max(textWidthOf(titleLayout), textWidthOf(detailLayout));
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
	float badgeX = margin + s(12), badgeY = margin + (pillHeight - s(40)) / 2;
	bool filled = vietnamese || message;
	if (bitmap) {
		SelectObject(memDC, bitmap);
		ID2D1DCRenderTarget* target = NULL;
		D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), USER_DEFAULT_SCREEN_DPI, USER_DEFAULT_SCREEN_DPI);
		RECT whole = { 0, 0, size.cx, size.cy };
		if (drawFactory() && SUCCEEDED(drawFactory()->CreateDCRenderTarget(&properties, &target)) && SUCCEEDED(target->BindDC(memDC, &whole))) {
			target->BeginDraw();
			target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			target->Clear(D2D1::ColorF(0, 0));
			//soft shadow from a few widening pills, a little lower than the pill
			int steps = 8;
			for (int i = steps; i >= 1; i--) {
				float grow = margin * i / steps;
				fillPill(target, D2D1::RectF(margin - grow, margin - grow + s(4), margin + pillWidth + grow, margin + s(4) + pillHeight + grow), (dark ? 14 : 8) << 24);
			}
			D2D1_RECT_F pill = D2D1::RectF(margin + 0.5f, margin + 0.5f, margin + pillWidth - 0.5f, margin + pillHeight - 0.5f);
			fillPill(target, pill, fill);
			ID2D1SolidColorBrush* brush = NULL;
			if (SUCCEEDED(target->CreateSolidColorBrush(colorOf(frame), &brush))) {
				target->DrawRoundedRectangle(D2D1::RoundedRect(pill, (pillHeight - 1) / 2, (pillHeight - 1) / 2), brush);
				brush->Release();
			}
			if (SUCCEEDED(target->CreateSolidColorBrush(colorOf(filled ? accentColor : (dark ? 0x14FFFFFF : 0x0D000000)), &brush))) {
				target->FillEllipse(D2D1::Ellipse(D2D1::Point2F(badgeX + s(20), badgeY + s(20)), s(20), s(20)), brush);
				brush->Release();
			}

			//the window is see-through, ClearType needs an opaque background
			target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
			if (badgeLayout) {
				//center the letter's ink, not its line box: the line box has more room
				//above the caps than below, which put the letter low
				DWRITE_OVERHANG_METRICS ink;
				float dx = 0, dy = 0;
				if (SUCCEEDED(badgeLayout->GetOverhangMetrics(&ink))) {
					dx = (ink.left - ink.right) / 2;
					dy = (ink.top - ink.bottom) / 2;
				}
				drawLayout(target, badgeLayout, badgeX + dx, badgeY + dy, filled ? (dark ? 0xFF000000 : 0xFFFFFFFF) : text);
			}
			float textX = badgeX + s(40) + s(14);
			float middle = margin + pillHeight / 2;
			drawLayout(target, titleLayout, textX, margin, text);
			drawLayout(target, detailLayout, textX, middle + s(2), sub);
			target->EndDraw();
		}
		if (target)
			target->Release();
	}

	IUnknown* objects[] = { titleFormat, detailFormat, badgeFormat, titleLayout, detailLayout, badgeLayout };
	for (IUnknown* object : objects)
		if (object)
			object->Release();
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
		shown = (int)wParam;
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
