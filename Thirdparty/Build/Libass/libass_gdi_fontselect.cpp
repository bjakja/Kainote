// Copyright (c) 2016, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#define UNICODE
#define M_PI       3.14159265358979323846   // pi
extern "C" {
#include "ass_utils.h"
#include "ass_fontselect.h"
}

#undef inline
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


#include <memory>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <xstring>

//#include "H:\Kainote\Kaiplayer\LogHandler.h"

#define U_IS_SURROGATE(c) (((c)&0xfffff800)==0xd800)

//ASS_Library *g_lib = NULL;

namespace {
class GdiFont {
	HFONT font;
	std::shared_ptr<HDC__> dc;

	size_t size = 0;
	std::unique_ptr<char[]> font_data;

public:
	GdiFont(HFONT font, std::shared_ptr<HDC__> dc) : font(font), dc(dc) { }
	~GdiFont() { DeleteObject(font); }

	size_t GetData(unsigned char *data, size_t offset, size_t len);
	bool CheckPostscript() { return false; }
	bool CheckGlyph(uint32_t codepoint) { return true; }
	void Destroy() { delete this; }
};

//bool GdiFont::CheckGlyph(uint32_t codepoint) 
//{ 
//	if (!codepoint)
//		return true;
//
//	std::wstring glyph; 
//	glyph.insert(glyph.begin(), ((wchar_t)codepoint));
//	WORD *indices = new WORD[glyph.size()];
//
//	bool succeeded = (GetGlyphIndicesW(dc.get(), glyph.data(), glyph.size(),
//		indices, GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR);
//	
//	
//	for (size_t i = 0; i < glyph.size(); ++i) {
//		
//		if (U_IS_SURROGATE(glyph[i]))
//			continue;
//		if (indices[i] == 0xFFFF) {
//			succeeded = false;
//		}
//	}
//	delete[] indices;
//	return succeeded;
//	//return true;
//}

size_t GdiFont::GetData(unsigned char *data, size_t offset, size_t len) {
	if (!font_data) {
		HDC DC = dc.get();
		
		SelectObject(DC, font);
		DWORD ttcf = 0x66637474;
		size = GetFontData(DC, ttcf, 0, 0, 0);
		if (size == GDI_ERROR) {
			ttcf = 0;
			size = GetFontData(DC, 0, 0, 0, 0);
		}
		if (size == GDI_ERROR) {
			return 0;
		}
		font_data.reset(new char[size]);
		DWORD obtainedSize = GetFontData(DC, ttcf, 0, font_data.get(), size);
	}

	if (!data) {
		return size;
	}
	memcpy(data, font_data.get() + offset, len);
	return len;
}
bool hasFont = false;
char *get_fallback(void* priv, ASS_Library* lib, const char *search_family, uint32_t code)
{
	std::shared_ptr<HDC__> dc(CreateCompatibleDC(nullptr), [](HDC dc) { DeleteDC(dc); });
	
	LOGFONTW lf{};
	lf.lfCharSet = DEFAULT_CHARSET;
	int numWritten = MultiByteToWideChar(CP_UTF8, 0, search_family, -1, lf.lfFaceName, LF_FACESIZE);
	if(!numWritten)
		return NULL;
	
	EnumFontFamiliesEx(dc.get(), &lf, [](const LOGFONT *lf, const TEXTMETRIC *, DWORD, LPARAM lParam) -> int {
		hasFont = true;
		return 1;
	}, (LPARAM)&hasFont, 0);

	if (!hasFont) {
		return NULL;
	}
	char *fontName = static_cast<char *>(malloc(LF_FACESIZE * 4));
	auto len = wcsnlen(lf.lfFaceName, LF_FACESIZE);
	auto written = WideCharToMultiByte(CP_UTF8, 0, lf.lfFaceName, len,
		fontName, LF_FACESIZE * 4, nullptr, nullptr);
	if (!written) {
		return NULL;
	}
	fontName[written] = 0;
	return fontName;
}


void match_fonts(void* priv, ASS_Library *lib, ASS_FontProvider *provider, char *name) {
	std::shared_ptr<HDC__> dc(CreateCompatibleDC(nullptr), [](HDC dc) { DeleteDC(dc); });
	//if (!g_lib)
		//g_lib = lib;

	
	LOGFONTW lf{};
	lf.lfCharSet = DEFAULT_CHARSET;
	MultiByteToWideChar(CP_UTF8, 0, name, -1, lf.lfFaceName, LF_FACESIZE);
	auto cb = [=](LOGFONTW const& lf) {
		ASS_FontProviderMetaData meta{};
		meta.weight = lf.lfWeight;
		meta.slant = lf.lfItalic ? FONT_SLANT_ITALIC : FONT_SLANT_NONE;
		meta.width = FONT_WIDTH_NORMAL;

		meta.families= static_cast<char **>(malloc(sizeof(char *)));
		meta.n_family = 1;

		auto name = static_cast<char *>(malloc(LF_FACESIZE * 4));
		auto len = wcsnlen(lf.lfFaceName, LF_FACESIZE);
		auto written = WideCharToMultiByte(CP_UTF8, 0, lf.lfFaceName, len,
										   name, LF_FACESIZE * 4, nullptr, nullptr);
		name[written] = 0;
		//remove spaces on the end to avoid not match in libass
		for (int i = written - 1; i >= 0; i--) {
			if (name[i] == ' ')
				name[i] = 0;
			else
				break;
		}
		meta.families[0] = name;

		auto hfont = CreateFontIndirectW(&lf);


		ass_font_provider_add_font(provider, &meta, nullptr, 0, new GdiFont(hfont, dc));

		if (name)
			free(name);
		if (meta.families)
			free(meta.families);
	};
	using type = decltype(cb);
	EnumFontFamiliesEx(dc.get(), &lf, [](const LOGFONT *lf, const TEXTMETRIC *, DWORD, LPARAM lParam) -> int {
		(*reinterpret_cast<type*>(lParam))(*lf);
		return 1;
	}, (LPARAM)&cb, 0);


}



template <typename T, T> struct wrapper;
template <typename T, typename R, typename... Args, R (T::*method)(Args...)>
struct wrapper<R (T::*)(Args...), method> {
	static R call(void *obj, Args... args) {
		return (static_cast<T*>(obj)->*method)(args...);
	}
};
}

extern "C"
ASS_FontProvider *ass_directwrite_add_provider(ASS_Library *lib,
											   ASS_FontSelector *selector,
											   const char *) {
	//g_lib = lib;

#define WRAP(method) &wrapper<decltype(&method), &method>::call
	static ASS_FontProviderFuncs callbacks = {
		WRAP(GdiFont::GetData),
		WRAP(GdiFont::CheckPostscript),
		WRAP(GdiFont::CheckGlyph),
		WRAP(GdiFont::Destroy),
		nullptr, // destroy_provider
		&match_fonts,
		nullptr, // get_substitution
		nullptr,//&get_fallback,
		nullptr
	};
	return ass_font_provider_new(selector, &callbacks, nullptr);
}
