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
#include <dwrite_3.h>
#include <cmath>
#include "FluentText.h"
#include "resource.h"

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

#define APP_FONT_FAMILY L"Be Vietnam Pro"

//the bytes of a font in the resources; WOFF2 is unpacked, a plain font file is taken as it is
static bool readFontResource(IDWriteFactory5* factory, const int& id, vector<BYTE>& bytes) {
	HRSRC resource = FindResource(NULL, MAKEINTRESOURCE(id), RT_RCDATA);
	HGLOBAL loaded = resource ? LoadResource(NULL, resource) : NULL;
	const BYTE* data = loaded ? (const BYTE*)LockResource(loaded) : NULL;
	DWORD size = resource ? SizeofResource(NULL, resource) : 0;
	if (!data || size < 4)
		return false;
	if (memcmp(data, "wOF2", 4) != 0) {
		bytes.assign(data, data + size);
		return true;
	}
	IDWriteFontFileStream* stream = NULL;
	if (FAILED(factory->UnpackFontFile(DWRITE_CONTAINER_TYPE_WOFF2, data, size, &stream)))
		return false;
	UINT64 length = 0;
	const void* fragment = NULL;
	void* context = NULL;
	bool read = SUCCEEDED(stream->GetFileSize(&length)) && length > 0 &&
		SUCCEEDED(stream->ReadFileFragment(&fragment, 0, length, &context));
	if (read) {
		bytes.assign((const BYTE*)fragment, (const BYTE*)fragment + length);
		stream->ReleaseFileFragment(context);
	}
	stream->Release();
	return read;
}

static bool gdiHasAppFont = false;

//Be Vietnam Pro, regular and semibold, carried inside the exe so nothing gets installed;
//NULL where DirectWrite cannot load fonts from memory (before Windows 10 1703)
static IDWriteFontCollection* appFonts() {
	static IDWriteFontCollection* fonts = NULL;
	static bool tried = false;
	if (tried)
		return fonts;
	tried = true;
	IDWriteFactory5* factory = NULL;
	if (!writeFactory() || FAILED(writeFactory()->QueryInterface(__uuidof(IDWriteFactory5), (void**)&factory)))
		return NULL;
	IDWriteInMemoryFontFileLoader* loader = NULL;
	IDWriteFontSetBuilder1* builder = NULL;
	//the loader stays registered for as long as the app runs, the fonts live in it
	if (SUCCEEDED(factory->CreateInMemoryFontFileLoader(&loader)) && SUCCEEDED(factory->RegisterFontFileLoader(loader)) &&
		SUCCEEDED(factory->CreateFontSetBuilder(&builder))) {
		int ids[] = { IDR_FONT_REGULAR, IDR_FONT_SEMIBOLD };
		int added = 0;
		for (int id : ids) {
			vector<BYTE> bytes;
			IDWriteFontFile* file = NULL;
			if (!readFontResource(factory, id, bytes) ||
				FAILED(loader->CreateInMemoryFontFileReference(factory, bytes.data(), (UINT32)bytes.size(), NULL, &file)))
				continue;
			if (SUCCEEDED(builder->AddFontFile(file)))
				added++;
			file->Release();
			//the edit boxes draw with GDI, which keeps its own copy for this process only
			DWORD count = 0;
			if (AddFontMemResourceEx(bytes.data(), (DWORD)bytes.size(), NULL, &count))
				gdiHasAppFont = true;
		}
		IDWriteFontSet* set = NULL;
		IDWriteFontCollection1* collection = NULL;
		if (added == 2 && SUCCEEDED(builder->CreateFontSet(&set)) && SUCCEEDED(factory->CreateFontCollectionFromFontSet(set, &collection))) {
			UINT32 index = 0;
			BOOL exists = FALSE;
			if (SUCCEEDED(collection->FindFamilyName(APP_FONT_FAMILY, &index, &exists)) && exists)
				fonts = collection;
			else
				collection->Release();
		}
		if (set)
			set->Release();
	}
	if (builder)
		builder->Release();
	if (loader)
		loader->Release();
	factory->Release();
	return fonts;
}

static LPCWSTR fontFamily() {
	static LPCWSTR family = NULL;
	if (!family) {
		family = L"Segoe UI";
		IDWriteFontCollection* fonts = NULL;
		if (appFonts()) {
			family = APP_FONT_FAMILY;
		} else if (writeFactory() && SUCCEEDED(writeFactory()->GetSystemFontCollection(&fonts))) {
			UINT32 index = 0;
			BOOL exists = FALSE;
			if (SUCCEEDED(fonts->FindFamilyName(L"Segoe UI Variable Text", &index, &exists)) && exists)
				family = L"Segoe UI Variable Text";
			fonts->Release();
		}
	}
	return family;
}

IDWriteFontCollection* fluentFontCollection() {
	return appFonts();
}

LPCWSTR fluentFontFamily() {
	return fontFamily();
}

LPCWSTR fluentGdiFontFace() {
	appFonts();
	return gdiHasAppFont ? APP_FONT_FAMILY : NULL;
}

static IDWriteTextFormat* createFormat(const DWRITE_FONT_WEIGHT& weight, const float& pixels, const bool& wrap) {
	IDWriteTextFormat* format = NULL;
	//vi-VN picks the Vietnamese accent shapes of Segoe UI when it stands in
	if (!writeFactory() || FAILED(writeFactory()->CreateTextFormat(fontFamily(), appFonts(), weight, DWRITE_FONT_STYLE_NORMAL,
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
