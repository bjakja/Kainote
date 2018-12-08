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


#include "DirectShowRenderer.h"
#include "Videobox.h"
#include <wmsdkidl.h>


static const GUID CLSID_KVideoRenderer =
{ 0x269ba141, 0x1fde, 0x494b, { 0x91, 0x24, 0x45, 0x3a, 0x17, 0x83, 0x8b, 0x9f } };

CD2DVideoRender::CD2DVideoRender(VideoPlayer *_Vplayer, HRESULT* phr)
	: CBaseVideoRenderer(CLSID_KVideoRenderer, L"Video Renderer", NULL, phr)
{
	Vplayer = _Vplayer;
	noRefresh = norender = false;
	time = 0;
	Vinfo.fps = 23.976f;
	Vinfo.width = 1280;
	Vinfo.height = 720;
	Vinfo.ARatioX = 16;
	Vinfo.ARatioY = 9;

}

CD2DVideoRender::~CD2DVideoRender()
{
}
void CD2DVideoRender::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
	//CAutoLock m_lock(this->m_pLock);
	if (!pMediaSample || Vplayer->vstate >= Stopped){ return; }

	REFERENCE_TIME start = 0, end = 0;
	pMediaSample->GetTime(&start, &end);

	BYTE* pBuffer = NULL;
	pMediaSample->GetPointer(&pBuffer);

	if (Vplayer->seek){
		time = Vplayer->time;
		Vplayer->time = time + (start / 10000.0);
		Vplayer->videoWindow->RefreshTime();
	}
	//po testach to przestawić
	Vplayer->seek = false;
	if (Vplayer->vstate == Playing || noRefresh){ noRefresh = false; return; }
	norender = true;


	Vplayer->DrawTexture(pBuffer, true);
	Vplayer->Render();
	if (Vplayer->block){ Vplayer->block = false; }
}

HRESULT CD2DVideoRender::Render(IMediaSample *pMediaSample)
{
	//CAutoLock m_lock(this->m_pLock);
	if (!pMediaSample || m_bStreaming == FALSE) return E_POINTER;//

	if (pMediaSample->IsPreroll() == S_OK || norender){ norender = false; return S_OK; }

	BYTE* pBuffer = NULL;
	HR1(pMediaSample->GetPointer(&pBuffer));

	REFERENCE_TIME start = 0, end = 0;
	pMediaSample->GetTime(&start, &end);
	/*if(!Vrend->block){*/
	bool endOfPlaying = Vplayer->playend && time + (end / 10000.0) >= Vplayer->playend;
	if (endOfPlaying){
		wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, 2021);
		wxQueueEvent(Vplayer, evt);
		wxCommandEvent *evtRefreshTime = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, 23334);
		wxQueueEvent(Vplayer, evtRefreshTime);
		noRefresh = true;
		//return S_OK;
		//Vrend->Pause();
	}
	Vplayer->time = time + (start / 10000.0);
	//kończąc odtwarzanie trzeba skopiować klatkę bo będzie później edytować na pierwszej, 
	//stop streaming nie działa, first sample jest zablokowane
	Vplayer->DrawTexture(pBuffer, endOfPlaying || Vplayer->resized);
	Vplayer->Render();
	/*}else{byte *cpy = (byte*) Vrend->datas; memcpy(cpy,pBuffer,pMediaSample->GetSize());}*/

	return S_OK;
}

HRESULT CD2DVideoRender::CheckMediaType(const CMediaType *pmt)
{

	CheckPointer(pmt, E_POINTER);
	if (pmt->majortype != MEDIATYPE_Video)
	{
		return E_FAIL;
	}
	const GUID *SubType = pmt->Subtype();

	if (SubType == NULL)
	{
		return E_FAIL;
	}

	if (*SubType != MEDIASUBTYPE_YV12 &&
		//*SubType != WMMEDIASUBTYPE_I420 &&
		//*SubType != MEDIASUBTYPE_IYUV //&&
		*SubType != MEDIASUBTYPE_NV12 &&
		*SubType != MEDIASUBTYPE_YUY2
		/*SubType != MEDIASUBTYPE_RGB555 &&
		*SubType != MEDIASUBTYPE_RGB565 &&*/
		//*SubType != MEDIASUBTYPE_RGB24 &&
		&& *SubType != MEDIASUBTYPE_RGB32
		)
	{
		return E_FAIL;
	}

	if (pmt->formattype != FORMAT_VideoInfo && pmt->formattype != FORMAT_VideoInfo2)
	{
		return E_FAIL;
	}
	/*OLECHAR bstrGuid[100];
	StringFromGUID2(*SubType,bstrGuid,100);
	wLogStatus(wxString(bstrGuid));*/
	return S_OK;
}

HRESULT CD2DVideoRender::StopStreaming()
{
	//CAutoLock m_lock(this->m_pLock);
	if (m_pMediaSample && Vplayer->vstate == Paused)
	{
		BYTE* pBuffer = NULL;
		m_pMediaSample->GetPointer(&pBuffer);
		if (pBuffer){
			Vplayer->DrawTexture(pBuffer, true);
			Vplayer->Render();
			norender = true;
			if (Vplayer->block){ Vplayer->block = false; }
		}

	}


	return CBaseVideoRenderer::StopStreaming();
}

HRESULT CD2DVideoRender::SetMediaType(const CMediaType *pmt)
{
	CheckPointer(pmt, E_POINTER);
	CAutoLock m_lock(this->m_pLock);


	VIDEOINFOHEADER* vInfo = NULL;
	VIDEOINFOHEADER2* vInfo2 = NULL;
	double tpf;

	if (pmt->formattype == FORMAT_VideoInfo)
	{
		vInfo = (VIDEOINFOHEADER*)pmt->pbFormat;
		if (vInfo == NULL || vInfo->bmiHeader.biSize != sizeof(BITMAPINFOHEADER))
		{
			return E_INVALIDARG;
		}
		tpf = (double)vInfo->AvgTimePerFrame;
		Vinfo.width = vInfo->bmiHeader.biWidth;
		Vinfo.height = vInfo->bmiHeader.biHeight;
		Vinfo.ARatioX = Vinfo.width;
		Vinfo.ARatioY = Vinfo.height;
	}
	else if (pmt->formattype == FORMAT_VideoInfo2)
	{
		vInfo2 = (VIDEOINFOHEADER2*)pmt->pbFormat;
		if (vInfo2 == NULL || vInfo2->bmiHeader.biSize != sizeof(BITMAPINFOHEADER))
		{
			return E_INVALIDARG;
		}

		tpf = (double)vInfo2->AvgTimePerFrame;
		Vinfo.width = vInfo2->bmiHeader.biWidth;
		Vinfo.height = vInfo2->bmiHeader.biHeight;
		Vinfo.ARatioX = vInfo2->dwPictAspectRatioX;
		Vinfo.ARatioY = vInfo2->dwPictAspectRatioY;

	}
	else
	{
		return E_INVALIDARG;
	}
	//if(Vinfo.height<0){Vinfo.height=abs(Vinfo.height);}
	//if(Vinfo.width<0){Vinfo.width=abs(Vinfo.width);}
	if (*pmt->Subtype() == MEDIASUBTYPE_YV12){
		Vinfo.bytes = 1.5;
		Vinfo.CT = YV12;
	}
	/*else if(*pmt->Subtype()==WMMEDIASUBTYPE_I420 || *pmt->Subtype()== MEDIASUBTYPE_IYUV){
		Vinfo.bytes=1.5;
		Vinfo.CT= IYUV;}*/
	else if (*pmt->Subtype() == MEDIASUBTYPE_NV12){
		Vinfo.bytes = 1.5;
		Vinfo.CT = NV12;
	}
	else if (*pmt->Subtype() == MEDIASUBTYPE_YUY2){
		Vinfo.bytes = 2;
		Vinfo.CT = YUY2;
	}
	/*else if(*pmt->Subtype()==MEDIASUBTYPE_RGB24){
		Vinfo.bytes=3;
		Vinfo.CT= RGB24;}*/
	else if (*pmt->Subtype() == MEDIASUBTYPE_RGB32){
		Vinfo.bytes = 4;
		Vinfo.CT = RGB32;
	}

	if (tpf < 1){ tpf = 417083; }
	Vinfo.fps = (float)(10000000.0 / tpf);
	return S_OK;
}

HRESULT CD2DVideoRender::EndOfStream()
{
	HRESULT hr = CBaseRenderer::EndOfStream();
	wxCommandEvent *evt = new wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED, 23333);
	wxQueueEvent(Vplayer, evt);//EndofStream();
	return hr;
}


HRESULT CD2DVideoRender::GetVidInfo(VideoInfo &vi)
{
	vi = Vinfo;
	return S_OK;
}
