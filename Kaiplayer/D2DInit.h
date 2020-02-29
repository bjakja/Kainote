/////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/graphics.h
// Purpose:     private graphics context header
// Author:      Stefan Csomor
// Modified by:
// Created:
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#pragma once

#undef GetHwnd

//#include "wx/msw/wrapwin.h"
//#include <minwindef.h>
//typedef unsigned int        UINT;
//typedef unsigned char       BYTE;
//typedef int                 BOOL;
//#include <DXGIType.h>  
typedef struct DXGI_JPEG_DC_HUFFMAN_TABLE
{
	BYTE CodeCounts[12];
	BYTE CodeValues[12];
} DXGI_JPEG_DC_HUFFMAN_TABLE;

typedef struct DXGI_JPEG_AC_HUFFMAN_TABLE
{
	BYTE CodeCounts[16];
	BYTE CodeValues[162];
} DXGI_JPEG_AC_HUFFMAN_TABLE;

typedef struct DXGI_JPEG_QUANTIZATION_TABLE
{
	BYTE Elements[64];
} DXGI_JPEG_QUANTIZATION_TABLE;

#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

IWICImagingFactory* wxWICImagingFactory();
ID2D1Factory* wxD2D1Factory();
IDWriteFactory* wxDWriteFactory();