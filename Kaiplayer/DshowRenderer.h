//#pragma once

#define UNICODE
//#include <wxdebug.h>

typedef wchar_t* PTCHAR;

#include "VideoRenderer.h"
#include <streams.h>
#include <Dvdmedia.h>


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
