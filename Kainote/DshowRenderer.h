//  Copyright (c) 2016 - 2020, Marcin Drob

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

typedef wchar_t* PTCHAR;

#include <streams.h>

enum {    
	RGB32,
	RGB24,
	YUY2,
	YV12,
	IYUV,
	NV12,
	ARGB32
};

struct VideoInf{
	int width;
	int height;
	float fps;
	int ARatioX;
	int ARatioY;
	unsigned char CT;
	float bytes;
};

#define HR1(x) if(FAILED(x)) { return x; }

class RendererVideo;

class CD2DVideoRender : public CBaseVideoRenderer
{
public:

	CD2DVideoRender(RendererVideo *_Vrend, HRESULT* phr);
	virtual ~CD2DVideoRender();

	long Render(IMediaSample *pMediaSample);
	long DoRenderSample(IMediaSample *pMediaSample){return 0;}
	long CheckMediaType(const CMediaType *pmt);
	long SetMediaType(const CMediaType *pmt);
	long EndOfStream();
	long StopStreaming();
	void OnReceiveFirstSample(IMediaSample *pMediaSample);
	
	long GetVidInfo(VideoInf &vi);

private:
	RendererVideo *Vrend;
	VideoInf Vinfo;
	int time;
	bool norender;
	bool noRefresh;
};

