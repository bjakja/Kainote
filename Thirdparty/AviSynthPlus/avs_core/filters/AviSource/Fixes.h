// Packaged with Avisynth v1.0 beta.
// http://www.math.berkeley.edu/~benrg/avisynth.html

//	VirtualDub - Video processing and capture application
//	Copyright (C) 1998-2000 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef f_FIXES_H
#define f_FIXES_H

// Those stupid idiots at Microsoft forgot to change this structure
// when making the VfW headers for Win32.  The result is that the
// AVIStreamHeader is correct in 16-bit Windows, but the 32-bit
// headers define RECT as LONGs instead of SHORTs.  This creates
// invalid AVI files!!!!

typedef struct {
	SHORT	left;
	SHORT	top;
	SHORT	right;
	SHORT	bottom;
} RECT16;

typedef struct {
    FOURCC		fccType;
    FOURCC		fccHandler;
    DWORD		dwFlags;
    WORD		wPriority;
    WORD		wLanguage;
    DWORD		dwInitialFrames;
    DWORD		dwScale;
    DWORD		dwRate;
    DWORD		dwStart;
    DWORD		dwLength;
    DWORD		dwSuggestedBufferSize;
    DWORD		dwQuality;
    DWORD		dwSampleSize;
    RECT16		rcFrame;
} AVIStreamHeader_fixed;

#endif
