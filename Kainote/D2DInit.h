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
#pragma warning(disable : 4005 )

#undef GetHwnd
typedef struct DXGI_JPEG_DC_HUFFMAN_TABLE
{
    unsigned char CodeCounts[12];
    unsigned char CodeValues[12];
} DXGI_JPEG_DC_HUFFMAN_TABLE;

typedef struct DXGI_JPEG_AC_HUFFMAN_TABLE
{
    unsigned char CodeCounts[16];
    unsigned char CodeValues[162];
} DXGI_JPEG_AC_HUFFMAN_TABLE;

typedef struct DXGI_JPEG_QUANTIZATION_TABLE
{
    unsigned char Elements[64];
} DXGI_JPEG_QUANTIZATION_TABLE; 

#include <windows.h>

#include <dxgitype.h>
#include <d2d1.h>
#include <dwrite.h>

#include <wincodec.h>

IWICImagingFactory* wxWICImagingFactory();
ID2D1Factory* wxD2D1Factory();
IDWriteFactory* wxDWriteFactory();