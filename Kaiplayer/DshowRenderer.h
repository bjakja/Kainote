//  Copyright (c) 2016, Marcin Drob

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

//#pragma once
#ifndef __DSHOW_RENDERER__
#define __DSHOW_RENDERER__

typedef wchar_t* PTCHAR;

#include "VideoRenderer.h"
#include <Dvdmedia.h>
#include <streams.h>

enum {    
	RGB32,
    RGB24,
	YUY2,
	YV12,
    IYUV,
    NV12,
};

struct VideoInf{
	int width;
	int height;
	float fps;
	int ARatioX;
	int ARatioY;
	byte CT;
	float bytes;
	};

#define HR1(x) if(FAILED(x)) { return x; }

#ifndef SRELEASE
#define SRELEASE(x) if (x != NULL) { x->Release(); x = NULL; }
#endif


class CD2DVideoRender : public CBaseVideoRenderer
{
public:

	CD2DVideoRender(VideoRend *_Vrend, HRESULT* phr);
	virtual ~CD2DVideoRender();

	HRESULT Render(IMediaSample *pMediaSample);
	HRESULT DoRenderSample(IMediaSample *pMediaSample){return 0;}
    HRESULT CheckMediaType(const CMediaType *pmt);
	HRESULT SetMediaType(const CMediaType *pmt);
	HRESULT EndOfStream();
	HRESULT StopStreaming();
	void OnReceiveFirstSample(IMediaSample *pMediaSample);
	
	HRESULT GetVidInfo(VideoInf &vi);

private:
	VideoRend *Vrend;
	VideoInf Vinfo;
	int time;
	bool norender;
	bool block;
	
};

#endif