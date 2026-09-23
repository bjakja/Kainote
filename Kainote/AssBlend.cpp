//  Copyright (c) 2026, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

#include "AssBlend.h"

#include <cstring>

#if defined(_M_X64) || defined(__SSE2__)
#include <emmintrin.h>
#define ASS_BLEND_SSE2 1
#endif

// x / 255 rounded, exact for 0 <= x <= 255 * 255
static inline unsigned Div255(unsigned x)
{
	x += 128;
	return (x + (x >> 8)) >> 8;
}

#ifdef ASS_BLEND_SSE2
static inline __m128i Div255(__m128i x)
{
	x = _mm_add_epi16(x, _mm_set1_epi16(128));
	return _mm_srli_epi16(_mm_add_epi16(x, _mm_srli_epi16(x, 8)), 8);
}
#endif

void BlendAssBitmap(unsigned char *dst, int dstPitch, const unsigned char *src, int srcStride,
	int w, int h, uint32_t color)
{
	unsigned opacity = 255 - (color & 0xFF);
	if (!opacity)
		return;
	const unsigned channels[4] = { (color >> 8) & 0xFF, (color >> 16) & 0xFF, color >> 24, 255 };

#ifdef ASS_BLEND_SSE2
	const __m128i zero = _mm_setzero_si128();
	const __m128i full = _mm_set1_epi16(255);
	const __m128i colour = _mm_setr_epi16(channels[0], channels[1], channels[2], 255,
		channels[0], channels[1], channels[2], 255);
	const __m128i opacityVec = _mm_set1_epi16((short)opacity);
#endif

	for (int y = 0; y < h; y++) {
		unsigned char *row = dst + y * dstPitch;
		const unsigned char *cover = src + y * srcStride;
		int x = 0;
#ifdef ASS_BLEND_SSE2
		for (; x + 4 <= w; x += 4) {
			uint32_t coverage;
			memcpy(&coverage, cover + x, 4);
			if (!coverage)
				continue;
			// alpha of each of the four pixels, spread over its four channels
			__m128i v = _mm_unpacklo_epi8(_mm_cvtsi32_si128((int)coverage), zero);
			__m128i alpha = Div255(_mm_mullo_epi16(v, opacityVec));
			alpha = _mm_unpacklo_epi16(alpha, alpha);
			__m128i alphaLo = _mm_unpacklo_epi32(alpha, alpha);
			__m128i alphaHi = _mm_unpackhi_epi32(alpha, alpha);

			__m128i pixels = _mm_loadu_si128((const __m128i *)(row + x * 4));
			__m128i lo = _mm_unpacklo_epi8(pixels, zero);
			__m128i hi = _mm_unpackhi_epi8(pixels, zero);
			lo = Div255(_mm_add_epi16(_mm_mullo_epi16(colour, alphaLo),
				_mm_mullo_epi16(lo, _mm_sub_epi16(full, alphaLo))));
			hi = Div255(_mm_add_epi16(_mm_mullo_epi16(colour, alphaHi),
				_mm_mullo_epi16(hi, _mm_sub_epi16(full, alphaHi))));
			_mm_storeu_si128((__m128i *)(row + x * 4), _mm_packus_epi16(lo, hi));
		}
#endif
		for (; x < w; x++) {
			unsigned alpha = Div255(opacity * cover[x]);
			if (!alpha)
				continue;
			unsigned char *pixel = row + x * 4;
			for (int c = 0; c < 4; c++)
				pixel[c] = (unsigned char)Div255(channels[c] * alpha + pixel[c] * (255 - alpha));
		}
	}
}
