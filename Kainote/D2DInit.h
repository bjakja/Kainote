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

#include <..\shared\dxgitype.h>
#include <..\include\d2d1.h>
#include <..\include\dwrite.h>

#include <..\um\wincodec.h>

IWICImagingFactory* wxWICImagingFactory();
ID2D1Factory* wxD2D1Factory();
IDWriteFactory* wxDWriteFactory();