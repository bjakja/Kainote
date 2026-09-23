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

#pragma once

#include <cstdint>

// Blends one libass bitmap (coverage bytes, src) in color (libass 0xRRGGBBAA,
// where AA is transparency) over BGRA pixels, keeping the alpha
// premultiplied so the result also works as an overlay.
void BlendAssBitmap(unsigned char *dst, int dstPitch, const unsigned char *src, int srcStride,
	int w, int h, uint32_t color);
