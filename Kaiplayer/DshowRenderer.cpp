
//#include "VideoRenderer.h"
//#include "Videobox.h"
#include "DshowRenderer.h"
//#include <Dvdmedia.h>
#include <wmsdkidl.h>
//#include <thread>


static const GUID CLSID_KVideoRenderer = 
{ 0x269ba141, 0x1fde, 0x494b, { 0x91, 0x24, 0x45, 0x3a, 0x17, 0x83, 0x8b, 0x9f } };

CD2DVideoRender::CD2DVideoRender(VideoRend *_Vrend, HRESULT* phr)
	: CBaseVideoRenderer(CLSID_KVideoRenderer, L"Video Renderer", NULL, phr)
{
	Vrend=_Vrend;
	norender=block=false;
	time=0;
	Vinfo.fps=23.976f;
	Vinfo.width=1280;
	Vinfo.height=720;
	Vinfo.ARatioX=16;
	Vinfo.ARatioY=9;

}

CD2DVideoRender::~CD2DVideoRender()
{
}
void CD2DVideoRender::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
	//CAutoLock m_lock(this->m_pLock);
 
	if(!pMediaSample){return;}
 
	REFERENCE_TIME start=0, end=0;
    pMediaSample->GetTime(&start,&end);

	BYTE* pBuffer = NULL;
	pMediaSample->GetPointer(&pBuffer);
	
	if(Vrend->seek){
		//wxLogStatus("start %i %i", (int)(start/10000.0), (int)(end/10000.0));
		time=Vrend->time;
		Vrend->time= time+(start/10000.0);}
	Vrend->seek=false;
	if(Vrend->vstate==Playing||Vrend->block){return;}//||Vrend->block
	norender=true;

	Vrend->DrawTexture(pBuffer, true);
	Vrend->Render();
}

HRESULT CD2DVideoRender::Render(IMediaSample *pMediaSample)
{
	//CAutoLock m_lock(this->m_pLock);
	if(!pMediaSample||m_bStreaming==FALSE) return E_POINTER;//
	//wxLogStatus("playing");
	if(pMediaSample->IsPreroll()==S_OK||norender){norender = false; return S_OK;}
	
	BYTE* pBuffer = NULL;
	HR1(pMediaSample->GetPointer(&pBuffer));
	//wxLogMessage("leng %i",(int)pMediaSample->GetActualDataLength());
	
	REFERENCE_TIME start=0, end=0;
    pMediaSample->GetTime(&start,&end);
	Vrend->time=time+(start/10000.0);
	if(!Vrend->block){
		if(Vrend->playend && Vrend->time>=Vrend->playend){ 
			wxCommandEvent *evt=new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,2021);
			wxQueueEvent(Vrend, evt);
		}
		Vrend->DrawTexture(pBuffer);
		Vrend->Render();
	}else{byte *cpy = (byte*) Vrend->datas; memcpy(cpy,pBuffer,pMediaSample->GetSize());}
	
	return S_OK;
}

HRESULT CD2DVideoRender::CheckMediaType(const CMediaType *pmt)
{
	
	CheckPointer(pmt, E_POINTER);
	if(pmt->majortype != MEDIATYPE_Video)
    {
        return E_FAIL;
    }
    const GUID *SubType = pmt->Subtype();
	
    if (SubType == NULL)
	{ return E_FAIL;}

    if(*SubType != MEDIASUBTYPE_YV12 &&
       //*SubType != WMMEDIASUBTYPE_I420 &&
	   //*SubType != MEDIASUBTYPE_IYUV //&&
	   *SubType != MEDIASUBTYPE_NV12 &&
	   *SubType != MEDIASUBTYPE_YUY2
	   /*SubType != MEDIASUBTYPE_RGB555 &&
	   *SubType != MEDIASUBTYPE_RGB565 &&*/
	   //*SubType != MEDIASUBTYPE_RGB24 &&
	   //*SubType != MEDIASUBTYPE_RGB32
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
	wxLogStatus(wxString(bstrGuid));*/
    return S_OK;
}

HRESULT CD2DVideoRender::StopStreaming()
{
	//CAutoLock m_lock(this->m_pLock);
	if(m_pMediaSample && Vrend->vstate==Paused)
	{
		BYTE* pBuffer = NULL;
		m_pMediaSample->GetPointer(&pBuffer);
		if(pBuffer){
			Vrend->DrawTexture(pBuffer, true);
			Vrend->Render();
			norender=true;
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
	
	if(pmt->formattype == FORMAT_VideoInfo)
	{
		vInfo = (VIDEOINFOHEADER*)pmt->pbFormat;
		if(vInfo == NULL || vInfo->bmiHeader.biSize != sizeof(BITMAPINFOHEADER))
		{
			return E_INVALIDARG;
		}
		tpf=(double)vInfo->AvgTimePerFrame;
		Vinfo.width=vInfo->bmiHeader.biWidth;
		Vinfo.height=vInfo->bmiHeader.biHeight;
		Vinfo.ARatioX=Vinfo.width;
		Vinfo.ARatioY=Vinfo.height;
		//wxLogStatus("Bitcount %i", (int)vInfo->bmiHeader.biBitCount);
	}
	else if(pmt->formattype == FORMAT_VideoInfo2)
	{
		vInfo2 = (VIDEOINFOHEADER2*)pmt->pbFormat;
		if(vInfo2 == NULL || vInfo2->bmiHeader.biSize != sizeof(BITMAPINFOHEADER))
		{
			return E_INVALIDARG;
		}
		
		tpf=(double)vInfo2->AvgTimePerFrame;
		Vinfo.width=vInfo2->bmiHeader.biWidth;
		Vinfo.height=vInfo2->bmiHeader.biHeight;
		Vinfo.ARatioX=vInfo2->dwPictAspectRatioX;
		Vinfo.ARatioY=vInfo2->dwPictAspectRatioY;
		
	}
	else
	{
		return E_INVALIDARG;
	}
	//if(Vinfo.height<0){Vinfo.height=abs(Vinfo.height);}
	//if(Vinfo.width<0){Vinfo.width=abs(Vinfo.width);}
	if(*pmt->Subtype()==MEDIASUBTYPE_YV12){
		Vinfo.bytes=1.5;
		Vinfo.CT= YV12;}
	/*else if(*pmt->Subtype()==WMMEDIASUBTYPE_I420 || *pmt->Subtype()== MEDIASUBTYPE_IYUV){
		Vinfo.bytes=1.5;
		Vinfo.CT= IYUV;}*/
	else if(*pmt->Subtype()==MEDIASUBTYPE_NV12){
		Vinfo.bytes=1.5;
		Vinfo.CT= NV12;}
	else if(*pmt->Subtype()==MEDIASUBTYPE_YUY2){
		Vinfo.bytes=2;
		Vinfo.CT= YUY2;}
	/*else if(*pmt->Subtype()==MEDIASUBTYPE_RGB24){
		Vinfo.bytes=3;
		Vinfo.CT= RGB24;}*/
	else if(*pmt->Subtype()==MEDIASUBTYPE_RGB32){
		Vinfo.bytes=4;
		Vinfo.CT= RGB32;}

	if(tpf<1){tpf=417083;}
	Vinfo.fps=(float)(10000000.0/tpf);
	//wxLogStatus("format %i", (int)Vinfo.CT);
	return S_OK;
}

 HRESULT CD2DVideoRender::EndOfStream()
 {
	 //wxLogStatus("End of stream");
	 HRESULT hr=CBaseRenderer::EndOfStream();
	 wxCommandEvent *evt=new wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,23333);
	 wxQueueEvent(Vrend, evt);//EndofStream();
	 return hr;
 }


HRESULT CD2DVideoRender::GetVidInfo(VideoInf &vi)
{
	vi=Vinfo;
	return S_OK;
}
