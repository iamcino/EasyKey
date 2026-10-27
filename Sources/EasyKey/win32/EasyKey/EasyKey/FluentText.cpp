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
#include "FluentText.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

struct FluentFont {
	IDWriteTextFormat* line = NULL; //one line, cut with an ellipsis when too long
	IDWriteTextFormat* wrap = NULL; //wraps at word ends
};

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

static LPCWSTR fontFamily() {
	static LPCWSTR family = NULL;
	if (!family) {
		family = L"Segoe UI";
		IDWriteFontCollection* fonts = NULL;
		if (writeFactory() && SUCCEEDED(writeFactory()->GetSystemFontCollection(&fonts))) {
			UINT32 index = 0;
			BOOL exists = FALSE;
			if (SUCCEEDED(fonts->FindFamilyName(L"Segoe UI Variable Text", &index, &exists)) && exists)
				family = L"Segoe UI Variable Text";
			fonts->Release();
		}
	}
	return family;
}

static IDWriteTextFormat* createFormat(const DWRITE_FONT_WEIGHT& weight, const float& pixels, const bool& wrap) {
	IDWriteTextFormat* format = NULL;
	//vi-VN picks Segoe UI's Vietnamese accent shapes
	if (!writeFactory() || FAILED(writeFactory()->CreateTextFormat(fontFamily(), NULL, weight, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, pixels, L"vi-VN", &format)))
		return NULL;
	if (wrap) {
		format->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
	} else {
		format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		IDWriteInlineObject* ellipsis = NULL;
		if (SUCCEEDED(writeFactory()->CreateEllipsisTrimmingSign(format, &ellipsis))) {
			DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
			format->SetTrimming(&trimming, ellipsis);
			ellipsis->Release();
		}
	}
	return format;
}

FluentFont* createFluentFont(const bool& strong, const float& pixels, const bool& bold) {
	DWRITE_FONT_WEIGHT weight = bold ? DWRITE_FONT_WEIGHT_BOLD : strong ? DWRITE_FONT_WEIGHT_SEMI_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
	FluentFont* font = new FluentFont();
	font->line = createFormat(weight, pixels, false);
	font->wrap = createFormat(weight, pixels, true);
	return font;
}

void freeFluentFont(FluentFont*& font) {
	if (!font)
		return;
	if (font->line)
		font->line->Release();
	if (font->wrap)
		font->wrap->Release();
	delete font;
	font = NULL;
}

static IDWriteTextLayout* createLayout(const wstring& text, FluentFont* font, const bool& wrap, const float& width, const float& height) {
	IDWriteTextFormat* format = font ? (wrap ? font->wrap : font->line) : NULL;
	IDWriteTextLayout* layout = NULL;
	if (!format || FAILED(writeFactory()->CreateTextLayout(text.c_str(), (UINT32)text.length(), format, max(width, 0.0f), max(height, 0.0f), &layout)))
		return NULL;
	return layout;
}

int fluentTextWidth(FluentFont* font, const wstring& text) {
	if (text.empty())
		return 0;
	IDWriteTextLayout* layout = createLayout(text, font, false, 100000.0f, 100000.0f);
	DWRITE_TEXT_METRICS metrics;
	int width = 0;
	if (layout && SUCCEEDED(layout->GetMetrics(&metrics)))
		width = (int)ceil(metrics.widthIncludingTrailingWhitespace);
	if (layout)
		layout->Release();
	return width;
}

int fluentTextHeight(FluentFont* font, const wstring& text, const int& width) {
	IDWriteTextLayout* layout = createLayout(text.empty() ? L" " : text, font, true, (float)width, 100000.0f);
	DWRITE_TEXT_METRICS metrics;
	int height = 0;
	if (layout && SUCCEEDED(layout->GetMetrics(&metrics)))
		height = (int)ceil(metrics.height);
	if (layout)
		layout->Release();
	return height;
}

void FluentTextBatch::add(const wstring& text, const RECT& rc, FluentFont* font, const DWORD& color,
						  const int& hAlign, const int& vAlign, const bool& wrap, const RECT& clip) {
	if (text.empty() || rc.right <= rc.left || !font)
		return;
	items.push_back({ text, rc, clip, font, color, hAlign, vAlign, wrap });
}

static D2D1_COLOR_F colorOf(const DWORD& argb) {
	return D2D1::ColorF(((argb >> 16) & 0xFF) / 255.0f, ((argb >> 8) & 0xFF) / 255.0f, (argb & 0xFF) / 255.0f, (argb >> 24) / 255.0f);
}

void FluentTextBatch::draw(const HDC& dc, const int& width, const int& height) {
	if (items.empty() || width <= 0 || height <= 0) {
		items.clear();
		return;
	}
	//a DC render target does not keep what the DC held, so the text goes on a clear layer of its own,
	//which is then laid over the shapes
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	void* bits = NULL;
	HBITMAP layer = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	HDC layerDC = layer ? CreateCompatibleDC(dc) : NULL;
	if (!layerDC) {
		if (layer)
			DeleteObject(layer);
		items.clear();
		return;
	}
	HGDIOBJ oldLayer = SelectObject(layerDC, layer);

	//pixels, not DIPs: the windows already scale their layout for the screen
	D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), USER_DEFAULT_SCREEN_DPI, USER_DEFAULT_SCREEN_DPI);
	ID2D1DCRenderTarget* target = NULL;
	RECT whole = { 0, 0, width, height };
	bool drawn = false;
	if (drawFactory() && SUCCEEDED(drawFactory()->CreateDCRenderTarget(&properties, &target)) && SUCCEEDED(target->BindDC(layerDC, &whole))) {
		target->BeginDraw();
		target->Clear(D2D1::ColorF(0, 0));
		//ClearType needs an opaque background, grayscale works on Mica too
		target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
		static const DWRITE_TEXT_ALIGNMENT hAligns[] = {
			DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_TEXT_ALIGNMENT_TRAILING
		};
		static const DWRITE_PARAGRAPH_ALIGNMENT vAligns[] = {
			DWRITE_PARAGRAPH_ALIGNMENT_NEAR, DWRITE_PARAGRAPH_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_FAR
		};
		ID2D1SolidColorBrush* brush = NULL;
		target->CreateSolidColorBrush(colorOf(0), &brush);
		for (const Item& item : items) {
			IDWriteTextLayout* layout = createLayout(item.text, item.font, item.wrap,
				(float)(item.rc.right - item.rc.left), (float)(item.rc.bottom - item.rc.top));
			if (!layout || !brush) {
				if (layout)
					layout->Release();
				continue;
			}
			layout->SetTextAlignment(hAligns[item.hAlign]);
			layout->SetParagraphAlignment(vAligns[item.vAlign]);
			brush->SetColor(colorOf(item.color));
			target->PushAxisAlignedClip(D2D1::RectF((float)item.clip.left, (float)item.clip.top, (float)item.clip.right, (float)item.clip.bottom),
				D2D1_ANTIALIAS_MODE_ALIASED);
			target->DrawTextLayout(D2D1::Point2F((float)item.rc.left, (float)item.rc.top), layout, brush);
			target->PopAxisAlignedClip();
			layout->Release();
		}
		if (brush)
			brush->Release();
		drawn = SUCCEEDED(target->EndDraw());
	}
	if (target)
		target->Release();
	if (drawn) {
		BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
		GdiAlphaBlend(dc, 0, 0, width, height, layerDC, 0, 0, width, height, blend);
	}
	SelectObject(layerDC, oldLayer);
	DeleteDC(layerDC);
	DeleteObject(layer);
	items.clear();
}
