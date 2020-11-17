//  Copyright (c) 2020, Marcin Drob

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


#include "RendererDirectShow.h"
#include "DShowPlayer.h"
#include "DshowRenderer.h"
#include "kainoteApp.h"
#include "CsriMod.h"
#include "OpennWrite.h"

#pragma comment(lib, "Dxva2.lib")
const IID IID_IDirectXVideoProcessorService = { 0xfc51a552, 0xd5e7, 0x11d9, { 0xaf, 0x55, 0x00, 0x05, 0x4e, 0x43, 0xff, 0x02 } };

RendererDirectShow::RendererDirectShow(VideoCtrl *control)
	: RendererVideo(control)
	, m_DirectShowPlayer(NULL)
{

}

RendererDirectShow::~RendererDirectShow()
{
	Stop();

	m_State = None;
	SAFE_DELETE(m_DirectShowPlayer);
	if (m_SubtitlesBuffer)
		delete[] m_SubtitlesBuffer;
}

bool RendererDirectShow::InitRendererDX()
{
	HR(m_D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_BlackBarsSurface), _("Nie można stworzyć powierzchni"));
	HR(DXVA2CreateVideoService(m_D3DDevice, IID_IDirectXVideoProcessorService, (VOID**)&m_DXVAService),
		_("Nie można stworzyć DXVA processor service"));
	DXVA2_VideoDesc videoDesc;
	videoDesc.SampleWidth = m_Width;
	videoDesc.SampleHeight = m_Height;
	videoDesc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	videoDesc.SampleFormat.NominalRange = DXVA2_NominalRange_0_255;
	videoDesc.SampleFormat.VideoTransferMatrix = DXVA2_VideoTransferMatrix_BT709;//EX_COLOR_INFO[g_ExColorInfo][0];
	videoDesc.SampleFormat.VideoLighting = DXVA2_VideoLighting_dim;
	videoDesc.SampleFormat.VideoPrimaries = DXVA2_VideoPrimaries_BT709;
	videoDesc.SampleFormat.VideoTransferFunction = DXVA2_VideoTransFunc_709;
	videoDesc.SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
	videoDesc.Format = D3DFMT_X8R8G8B8;
	videoDesc.InputSampleFreq.Numerator = 60;
	videoDesc.InputSampleFreq.Denominator = 1;
	videoDesc.OutputFrameFreq.Numerator = 60;
	videoDesc.OutputFrameFreq.Denominator = 1;

	UINT count, count1;//, count2;
	GUID* guids = NULL;

	HR(m_DXVAService->GetVideoProcessorDeviceGuids(&videoDesc, &count, &guids), _("Nie można pobrać GUIDów DXVA"));
	D3DFORMAT* formats = NULL;
	//D3DFORMAT* formats2 = NULL;
	bool isgood = false;
	GUID dxvaGuid;
	DXVA2_VideoProcessorCaps DXVAcaps;
	HRESULT hr;
	for (UINT i = 0; i < count; i++){
		hr = m_DXVAService->GetVideoProcessorRenderTargets(guids[i], &videoDesc, &count1, &formats);
		if (FAILED(hr)){ KaiLog(_("Nie można uzyskać formatów DXVA")); continue; }
		for (UINT j = 0; j < count1; j++)
		{
			if (formats[j] == D3DFMT_X8R8G8B8)
			{
				isgood = true; //break;
			}

		}

		CoTaskMemFree(formats);
		if (!isgood){ KaiLog(_("Ten format nie jest obsługiwany przez DXVA")); continue; }
		isgood = false;

		hr = m_DXVAService->GetVideoProcessorCaps(guids[i], &videoDesc, D3DFMT_X8R8G8B8, &DXVAcaps);
		if (FAILED(hr)){ KaiLog(_("GetVideoProcessorCaps zawiodło")); continue; }
		if (DXVAcaps.NumForwardRefSamples > 0 || DXVAcaps.NumBackwardRefSamples > 0){
			continue;
		}

		//if(DXVAcaps.DeviceCaps!=4){continue;}//DXVAcaps.InputPool
		hr = m_DXVAService->CreateSurface(m_Width, m_Height, 0, m_D3DFormat, D3DPOOL_DEFAULT, 0,
			DXVA2_VideoSoftwareRenderTarget, &m_MainSurface, NULL);
		if (FAILED(hr)){ KaiLog(wxString::Format(_("Nie można stworzyć powierzchni DXVA %i"), (int)i)); continue; }

		hr = m_DXVAService->CreateVideoProcessor(guids[i], &videoDesc, D3DFMT_X8R8G8B8, 0, &m_DXVAProcessor);
		if (FAILED(hr)){ KaiLog(_("Nie można stworzyć processora DXVA")); continue; }
		dxvaGuid = guids[i]; isgood = true;
		break;
	}
	CoTaskMemFree(guids);
	PTR(isgood, L"Nie ma żadnych guidów");

	/*HR(m_D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_BlackBarsSurface), _("Nie można stworzyć powierzchni"));

	HR(m_D3DDevice->CreateOffscreenPlainSurface(m_Width, m_Height, m_D3DFormat, D3DPOOL_DEFAULT, &m_MainSurface, 0),
		_("Nie można stworzyć plain surface"));*/

	//hr = m_D3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	//hr = m_D3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// Add filtering
	//hr = m_D3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//hr = m_D3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	m_WindowWidth = m_WindowRect.right - m_WindowRect.left ;
	m_WindowHeight = m_WindowRect.bottom - m_WindowRect.top ;
	/*if (m_WindowWidth % 2 != 0)
		m_WindowWidth--;*/
	/*if (m_WindowHeight % 2 != 0)
		m_WindowHeight--;*/

	m_LastBufferSize = m_WindowWidth * m_WindowHeight * 4;
	if (m_SubtitlesBuffer)
		delete[] m_SubtitlesBuffer;
	m_SubtitlesBuffer = new unsigned char[m_LastBufferSize];
	
	
	m_SubsProvider->SetVideoParameters(wxSize(m_WindowWidth, m_WindowHeight), ARGB32, m_SwapFrame);

	HR(m_D3DDevice->CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_D3DVertex, NULL),
		"Nie można utworzyć bufora wertex")
	CUSTOMVERTEX* pVertices;
	HR(hr = m_D3DVertex->Lock(0, 0, (void**)&pVertices, 0), "nie można zablokować bufora vertex");
	// looks like it places 1px border that's position is moved by 1

	pVertices[0].position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
	pVertices[0].tu = 0.0f;
	pVertices[0].tv = 0.0f;
	pVertices[1].position = D3DXVECTOR3(m_WindowWidth - 1.f, -1.0f, 0.0f);
	pVertices[1].tu = 1.0f;
	pVertices[1].tv = 0.0f;
	pVertices[2].position = D3DXVECTOR3(m_WindowWidth - 1.f, m_WindowHeight - 1.f, 0.0f);
	pVertices[2].tu = 1.0f;
	pVertices[2].tv = 1.0f;
	pVertices[3].position = D3DXVECTOR3(-1.0f, m_WindowHeight - 1.f, 0.0f);
	pVertices[3].tu = 0.0f;
	pVertices[3].tv = 1.0f;

	m_D3DVertex->Unlock();
	
	
	HR(m_D3DDevice->CreateTexture(m_WindowWidth, m_WindowHeight, 1,
		D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &m_SubtitlesTexture, NULL),
		_("Nie można storzyć tekstury napisów"));

	HR(m_D3DDevice->CreateTexture(m_WindowWidth, m_WindowHeight, 1,
		D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_BlitTexture, NULL),
		_("Nie można storzyć tekstury napisów"));

	return true;
}

bool RendererDirectShow::DrawTexture(byte *nframe, bool copy)
{

	wxCriticalSectionLocker lock(m_MutexRendering);
	byte *fdata = NULL;
	byte *texbuf;
	byte bytes = (m_Format == RGB32) ? 4 : (m_Format == YUY2) ? 2 : 1;

	D3DLOCKED_RECT d3dlr;
	D3DLOCKED_RECT d3dSubslr;

	if (nframe) {
		fdata = nframe;
		if (copy) {
			byte *cpy = m_FrameBuffer;
			memcpy(cpy, fdata, m_Height * m_Pitch);
		}
	}
	else {
		KaiLog(_("Brak bufora klatki")); return false;
	}
	int size = m_LastBufferSize / 4;
	memset(m_SubtitlesBuffer, 0, m_LastBufferSize);
	byte* buff1 = m_SubtitlesBuffer;

	m_SubsProvider->Draw(m_SubtitlesBuffer, m_Time);
	byte* buff = m_SubtitlesBuffer;
	

	RECT dirtySubs = { 0, 0, m_WindowWidth, m_WindowHeight };
	HR(m_SubtitlesTexture->LockRect(0, &d3dSubslr, &dirtySubs, NULL), _("Nie można zablokować bufora tekstury napisów"));
	memcpy(d3dSubslr.pBits, m_SubtitlesBuffer, m_LastBufferSize);
	m_SubtitlesTexture->UnlockRect(0);

	m_D3DDevice->UpdateTexture(m_SubtitlesTexture, m_BlitTexture);

	RECT dirty = { 0, 0, m_Width, m_Height };
#ifdef byvertices
	HR(m_MainSurface->LockRect(&d3dlr, 0, 0), _("Nie można zablokować bufora tekstury"));//D3DLOCK_NOSYSLOCK
#else
	HR(m_MainSurface->LockRect(&d3dlr, &dirty, D3DLOCK_NOSYSLOCK), _("Nie można zablokować bufora tekstury"));
#endif
	texbuf = static_cast<byte *>(d3dlr.pBits);

	diff = d3dlr.Pitch - (m_Width*bytes);
	if (m_SwapFrame) {
		int framePitch = m_Width * bytes;
		byte * reversebyte = fdata + (framePitch * m_Height) - framePitch;
		for (int j = 0; j < m_Height; ++j) {
			memcpy(texbuf, reversebyte, framePitch);
			texbuf += framePitch + diff;
			reversebyte -= framePitch;
		}
	}
	else if (!diff) {
		memcpy(texbuf, fdata, (m_Height * m_Pitch));
	}
	else if (diff > 0) {

		if (m_Format >= YV12) {
			for (int i = 0; i < m_Height; ++i) {
				memcpy(texbuf, fdata, m_Width);
				texbuf += (m_Width + diff);
				fdata += m_Width;
			}
			int hheight = m_Height / 2;
			int fwidth = (m_Format == NV12) ? m_Width : m_Width / 2;
			int fdiff = (m_Format == NV12) ? diff : diff / 2;

			for (int i = 0; i < hheight; i++) {
				memcpy(texbuf, fdata, fwidth);
				texbuf += (fwidth + fdiff);
				fdata += fwidth;
			}
			if (m_Format < NV12) {
				for (int i = 0; i < hheight; ++i) {
					memcpy(texbuf, fdata, fwidth);
					texbuf += (fwidth + fdiff);
					fdata += fwidth;
				}
			}
		}
		else
		{
			int fwidth = m_Width * bytes;
			for (int i = 0; i < m_Height; i++) {
				memcpy(texbuf, fdata, fwidth);
				texbuf += (fwidth + diff);
				fdata += fwidth;
			}
		}

	}
	else {
		KaiLog(wxString::Format(L"bad pitch diff %i pitch %i dxpitch %i", diff, m_Pitch, d3dlr.Pitch));
	}

	m_MainSurface->UnlockRect();

	return true;
}

void RendererDirectShow::Render(bool redrawSubsOnFrame, bool wait)
{
	wxCriticalSectionLocker lock(m_MutexRendering);
	m_VideoResized = false;
	HRESULT hr = S_OK;

	if (m_DeviceLost)
	{
		if (FAILED(hr = m_D3DDevice->TestCooperativeLevel()))
		{
			if (D3DERR_DEVICELOST == hr ||
				D3DERR_DRIVERINTERNALERROR == hr){
				return;
			}

			if (D3DERR_DEVICENOTRESET == hr)
			{
				Clear();
				InitDX();
				RecreateSurface();
				if (m_Visual){
					m_Visual->SizeChanged(wxRect(m_BackBufferRect.left, m_BackBufferRect.top,
						m_BackBufferRect.right, m_BackBufferRect.bottom), m_D3DLine, m_D3DFont, m_D3DDevice);
				}
				m_DeviceLost = false;
				Render(true, false);
				return;
			}
			return;
		}
		m_DeviceLost = false;
	}

	hr = m_D3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	DXVA2_VideoProcessBltParams blt = { 0 };
	DXVA2_VideoSample samples = { 0 };
	LONGLONG start_100ns = m_Time * 10000;
	LONGLONG end_100ns = start_100ns + 170000;
	blt.TargetFrame = start_100ns;
	blt.TargetRect = m_WindowRect;

	// DXVA2_VideoProcess_Constriction
	blt.ConstrictionSize.cx = m_WindowRect.right - m_WindowRect.left;
	blt.ConstrictionSize.cy = m_WindowRect.bottom - m_WindowRect.top;
	DXVA2_AYUVSample16 color;

	color.Cr = 0x8000;
	color.Cb = 0x8000;
	color.Y = 0x0F00;
	color.Alpha = 0xFFFF;
	blt.BackgroundColor = color;

	// DXVA2_VideoProcess_YUV2RGBExtended
	blt.DestFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_Unknown;
	blt.DestFormat.NominalRange = DXVA2_NominalRange_0_255;//EX_COLOR_INFO[g_ExColorInfo][1];
	blt.DestFormat.VideoTransferMatrix = DXVA2_VideoTransferMatrix_BT709;
	blt.DestFormat.VideoLighting = DXVA2_VideoLighting_dim;
	blt.DestFormat.VideoPrimaries = DXVA2_VideoPrimaries_BT709;
	blt.DestFormat.VideoTransferFunction = DXVA2_VideoTransFunc_709;

	blt.DestFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
	// Initialize main stream video sample.
	//
	samples.Start = start_100ns;
	samples.End = end_100ns;

	// DXVA2_VideoProcess_YUV2RGBExtended
	samples.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	samples.SampleFormat.NominalRange = DXVA2_NominalRange_0_255;
	samples.SampleFormat.VideoTransferMatrix = DXVA2_VideoTransferMatrix_BT709;//EX_COLOR_INFO[g_ExColorInfo][0];
	samples.SampleFormat.VideoLighting = DXVA2_VideoLighting_dim;
	samples.SampleFormat.VideoPrimaries = DXVA2_VideoPrimaries_BT709;
	samples.SampleFormat.VideoTransferFunction = DXVA2_VideoTransFunc_709;

	samples.SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;

	samples.SrcSurface = m_MainSurface;

	samples.SrcRect = m_MainStreamRect;

	samples.DstRect = m_BackBufferRect;

	// DXVA2_VideoProcess_PlanarAlpha
	samples.PlanarAlpha = DXVA2_Fixed32OpaqueAlpha();

	hr = m_DXVAProcessor->VideoProcessBlt(m_BlackBarsSurface, &blt, &samples, 1, NULL);
	/*hr = m_D3DDevice->StretchRect(m_MainSurface, &m_MainStreamRect, m_BlackBarsSurface, &m_BackBufferRect, D3DTEXF_LINEAR);
	if (FAILED(hr)) { KaiLog(_("Nie można nałożyć powierzchni na siebie")); }*/
	
	hr = m_D3DDevice->BeginScene();
	hr = m_D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	hr = m_D3DDevice->SetStreamSource(0, m_D3DVertex, 0, sizeof(CUSTOMVERTEX));
	hr = m_D3DDevice->SetTexture(0, m_BlitTexture);
	hr = m_D3DDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	hr = m_D3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	hr = m_D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	hr = m_D3DDevice->SetTexture(0, NULL);
	hr = m_D3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	
#if byvertices


	// Render the vertex buffer contents
	hr = m_D3DDevice->SetStreamSource(0, vertex, 0, sizeof(CUSTOMVERTEX));
	hr = m_D3DDevice->SetVertexShader(NULL);
	hr = m_D3DDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	hr = m_D3DDevice->SetTexture(0, texture);
	hr = m_D3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
#endif

	if (m_Visual && !m_HasZoom){ m_Visual->Draw(m_Time); }

	if (videoControl->m_FullScreenProgressBar){
		DRAWOUTTEXT(m_D3DFont, m_ProgressBarTime, m_ProgressBarRect, DT_LEFT | DT_TOP, 0xFFFFFFFF)
			hr = m_D3DLine->SetWidth(1);
		hr = m_D3DLine->Begin();
		hr = m_D3DLine->Draw(&vectors[0], 5, 0xFF000000);
		hr = m_D3DLine->Draw(&vectors[5], 5, 0xFFFFFFFF);
		hr = m_D3DLine->End();
		hr = m_D3DLine->SetWidth(7);
		hr = m_D3DLine->Begin();
		hr = m_D3DLine->Draw(&vectors[10], 2, 0xFFFFFFFF);
		hr = m_D3DLine->End();
	}
	if (m_HasZoom){ DrawZoom(); }
	// End the scene
	hr = m_D3DDevice->EndScene();
	hr = m_D3DDevice->Present(NULL, &m_WindowRect, NULL, NULL);
	if (D3DERR_DEVICELOST == hr ||
		D3DERR_DRIVERINTERNALERROR == hr){
		if (!m_DeviceLost){
			m_DeviceLost = true;
		}
		Render(true, false);
	}

}

void RendererDirectShow::RecreateSurface()
{
	/*int all = m_Height * m_Pitch;
	byte *cpy = new byte[all];
	byte *cpy1 = cpy;
	byte *data1 = m_FrameBuffer;
	memcpy(cpy1, data1, all);*/
	DrawTexture(m_FrameBuffer/*cpy1*/);
	//delete[] cpy;
}

bool RendererDirectShow::OpenFile(const wxString &fname, int subsFlag, bool vobsub, bool changeAudio)
{
	wxMutexLocker lock(m_MutexOpen);
	kainoteApp *Kaia = (kainoteApp*)wxTheApp;
	if (m_State == Playing){ videoControl->Stop(); }

	if (m_State != None){
		m_VideoResized = m_DirectShowSeeking = videoControl->m_FullScreenProgressBar = false;
		m_State = None;
		Clear();
	}
	m_Time = 0;
	m_Frame = 0;


	if (!m_DirectShowPlayer){ m_DirectShowPlayer = new DShowPlayer(videoControl); }

	if (!m_DirectShowPlayer->OpenFile(fname, vobsub)){
		return false;
	}
	wxSize videoSize = m_DirectShowPlayer->GetVideoSize();
	m_Width = videoSize.x; m_Height = videoSize.y;
	if (m_Width % 2 != 0){ m_Width++; }

	m_Pitch = m_Width * m_DirectShowPlayer->inf.bytes;
	videoControl->m_FPS = m_DirectShowPlayer->inf.fps;
	m_Format = m_DirectShowPlayer->inf.CT;
	videoControl->m_AspectRatioX = m_DirectShowPlayer->inf.ARatioX;
	videoControl->m_AspectRatioY = m_DirectShowPlayer->inf.ARatioY;
	m_D3DFormat = (m_Format == NV12) ? D3DFORMAT('21VN') : (m_Format == YV12) ? D3DFORMAT('21VY') :
		(m_Format == YUY2) ? D3DFMT_YUY2 : D3DFMT_X8R8G8B8;

	m_SwapFrame = (m_Format == RGB32 && !m_DirectShowPlayer->HasVobsub());
	Kaia->Frame->OpenAudioInTab(tab, GLOBAL_CLOSE_AUDIO, L"");

	diff = 0;
	m_FrameDuration = (1000.0f / videoControl->m_FPS);
	if (videoControl->m_AspectRatioY == 0 || videoControl->m_AspectRatioX == 0){ videoControl->m_AspectRatio = 0.0f; }
	else{ videoControl->m_AspectRatio = (float)videoControl->m_AspectRatioY / (float)videoControl->m_AspectRatioX; }

	m_MainStreamRect.bottom = m_Height;
	m_MainStreamRect.right = m_Width;
	m_MainStreamRect.left = 0;
	m_MainStreamRect.top = 0;
	if (m_FrameBuffer ){ delete[] m_FrameBuffer; m_FrameBuffer = NULL; }
	m_FrameBuffer = new byte[m_Height * m_Pitch];
	
	UpdateRects();

	if (!InitDX()){ return false; }
	
	if (vobsub)
		subsFlag = CLOSE_SUBTITLES;

	OpenSubs(subsFlag, false, NULL, true);

	m_State = Stopped;
	m_DirectShowPlayer->GetChapters(&m_Chapters);

	if (m_Visual){
		m_Visual->SizeChanged(wxRect(m_BackBufferRect.left, m_BackBufferRect.top,
			m_BackBufferRect.right, m_BackBufferRect.bottom), m_D3DLine, m_D3DFont, m_D3DDevice);
	}
	return true;
}

bool RendererDirectShow::OpenSubs(int flag, bool redraw, wxString *text, bool resetParameters)
{
	wxCriticalSectionLocker lock(m_MutexRendering);
	if (resetParameters)
		m_SubsProvider->SetVideoParameters(wxSize(m_WindowWidth, m_WindowHeight), ARGB32, m_SwapFrame);

	bool result = m_SubsProvider->Open(tab, flag, text);

	if (redraw && m_State != None && m_FrameBuffer){
		RecreateSurface();
	}

	return result;
}


bool RendererDirectShow::Play(int end)
{
	SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS);
	if (!(videoControl->IsShown() || (videoControl->m_FullScreenWindow && videoControl->m_FullScreenWindow->IsShown()))){ return false; }
	if (m_HasVisualEdition){
		OpenSubs(OPEN_WHOLE_SUBTITLES, false);
		SAFE_DELETE(m_Visual->dummytext);
		m_HasVisualEdition = false;
	}
	else if (m_HasDummySubs && tab->editor){
		OpenSubs(OPEN_WHOLE_SUBTITLES, false);
	}

	if (end > 0){ m_PlayEndTime = end; }
	else
		m_PlayEndTime = 0;
	if (m_Time < GetDuration() - m_FrameDuration) 
		m_DirectShowPlayer->Play(); 

	m_State = Playing;
	return true;
}


bool RendererDirectShow::Pause()
{
	if (m_State == Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		m_State = Paused;
		m_DirectShowPlayer->Pause();
		
	}
	else if (m_State != None){
		Play();
	}
	else{ return false; }
	return true;
}

bool RendererDirectShow::Stop()
{
	if (m_State == Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		m_State = Stopped;
		m_DirectShowPlayer->Stop();
		m_PlayEndTime = 0;
		m_Time = 0;
		return true;
	}
	return false;
}

void RendererDirectShow::SetPosition(int _time, bool starttime/*=true*/, bool corect/*=true*/, bool async /*= true*/)
{

	bool playing = m_State == Playing;
	m_Time = MID(0, _time, GetDuration());
	if (corect){
		m_Time /= m_FrameDuration;
		if (starttime){ m_Time++; }
		m_Time *= m_FrameDuration;
	}
	m_PlayEndTime = 0;
	m_DirectShowSeeking = true;
	m_DirectShowPlayer->SetPosition(m_Time);
	if (m_HasVisualEdition){
		SAFE_DELETE(m_Visual->dummytext);
		//if (m_Visual->Visual == VECTORCLIP){
			//m_Visual->SetClip(m_Visual->GetVisual(), true, false, false);
		//}
		//else{
			OpenSubs((playing) ? OPEN_WHOLE_SUBTITLES : OPEN_DUMMY, true);
			if (m_State == Playing){ m_HasVisualEdition = false; }
		//}
	}
	else if (m_HasDummySubs && tab->editor){
		OpenSubs((playing) ? OPEN_WHOLE_SUBTITLES : OPEN_DUMMY, true);
	}
	
}

int RendererDirectShow::GetDuration()
{
	return m_DirectShowPlayer->GetDuration();
}

int RendererDirectShow::GetFrameTime(bool start)
{
	int halfFrame = (start) ? -(m_FrameDuration / 2.0f) : (m_FrameDuration / 2.0f) + 1;
	return m_Time + halfFrame;
}

void RendererDirectShow::GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd)
{
	if (!retStart || !retEnd){ return; }
	
	int frameStartTime = (((float)startTime / 1000.f) * videoControl->m_FPS);
	int frameEndTime = (((float)endTime / 1000.f) * videoControl->m_FPS);
	frameStartTime++;
	frameEndTime++;
	*retStart = (((frameStartTime * 1000) / videoControl->m_FPS) + 0.5f) - startTime;
	*retEnd = (((frameEndTime * 1000) / videoControl->m_FPS) + 0.5f) - endTime;

}

int RendererDirectShow::GetFrameTimeFromTime(int _time, bool start)
{
	int halfFrame = (start) ? -(m_FrameDuration / 2.0f) : (m_FrameDuration / 2.0f) + 1;
	return _time + halfFrame;
}

int RendererDirectShow::GetFrameTimeFromFrame(int frame, bool start)
{
	int halfFrame = (start) ? -(m_FrameDuration / 2.0f) : (m_FrameDuration / 2.0f) + 1;
	return (frame * (1000.f / videoControl->m_FPS)) + halfFrame;
}

int RendererDirectShow::GetPlayEndTime(int _time)
{
	int newTime = _time;
	newTime /= m_FrameDuration;
	newTime = (newTime * m_FrameDuration) + 1.f;
	if (_time == newTime && newTime % 10 == 0){ newTime -= 5; }
	return newTime;
}

void RendererDirectShow::OpenKeyframes(const wxString &filename)
{
	
}

void RendererDirectShow::ClearObject()
{
	SAFE_RELEASE(m_SubtitlesTexture);
	SAFE_RELEASE(m_BlitTexture);
}


void RendererDirectShow::GetFpsnRatio(float *fps, long *arx, long *ary)
{
	m_DirectShowPlayer->GetFpsnRatio(fps, arx, ary);
}

void RendererDirectShow::GetVideoSize(int *width, int *height)
{
	wxSize sz = m_DirectShowPlayer->GetVideoSize();
	*width = sz.x;
	*height = sz.y;
}

void RendererDirectShow::SetVolume(int vol)
{
	if (m_State == None){ return; }
	m_DirectShowPlayer->SetVolume(vol);
}

int RendererDirectShow::GetVolume()
{
	if (m_State == None){ return 0; }
	return m_DirectShowPlayer->GetVolume();
}

void RendererDirectShow::ChangePositionByFrame(int step)
{
	if (m_State == Playing || m_State == None){ return; }
	
	m_Time += (m_FrameDuration * step);
	SetPosition(m_Time, true, false);
	videoControl->RefreshTime();

}


wxArrayString RendererDirectShow::GetStreams()
{
	return m_DirectShowPlayer->GetStreams();
}

void RendererDirectShow::EnableStream(long index)
{
	if (m_DirectShowPlayer->stream){
		m_DirectShowSeeking = true;
		auto hr = m_DirectShowPlayer->stream->Enable(index, AMSTREAMSELECTENABLE_ENABLE);
		if (FAILED(hr)){
			KaiLog(L"Cannot change stream");
		}
	}
}



void RendererDirectShow::ChangeVobsub(bool vobsub)
{
	if (!m_DirectShowPlayer){ return; }
	int tmptime = m_Time;
	OpenSubs((vobsub) ? CLOSE_SUBTITLES : OPEN_WHOLE_SUBTITLES, true);
	m_DirectShowPlayer->OpenFile(tab->VideoPath, vobsub);
	m_Format = m_DirectShowPlayer->inf.CT;
	D3DFORMAT tmpd3dformat = (m_Format == 5) ? D3DFORMAT('21VN') : (m_Format == 3) ? D3DFORMAT('21VY') :
		(m_Format == 2) ? D3DFMT_YUY2 : D3DFMT_X8R8G8B8;
	m_SwapFrame = (m_Format == 0 && !m_DirectShowPlayer->HasVobsub());
	if (tmpd3dformat != m_D3DFormat){
		m_D3DFormat = tmpd3dformat;
		int tmppitch = m_Width * m_DirectShowPlayer->inf.bytes;
		if (tmppitch != m_Pitch){
			m_Pitch = tmppitch;
			if (m_FrameBuffer){ delete[] m_FrameBuffer; m_FrameBuffer = NULL; }
			m_FrameBuffer = new byte[m_Height * m_Pitch];
		}
		UpdateVideoWindow();
	}
	SetPosition(tmptime);
	if (m_State == Paused){ m_DirectShowPlayer->Play(); m_DirectShowPlayer->Pause(); }
	else if (m_State == Playing){ m_DirectShowPlayer->Play(); }
	int pos = tab->Video->m_VolumeSlider->GetValue();
	SetVolume(-(pos * pos));
	tab->Video->ChangeStream();
}

bool RendererDirectShow::EnumFilters(Menu *menu)
{
	return m_DirectShowPlayer->EnumFilters(menu); 
}

bool RendererDirectShow::FilterConfig(wxString name, int idx, wxPoint pos)
{
	return m_DirectShowPlayer->FilterConfig(name, idx, pos);
}

byte *RendererDirectShow::GetFramewithSubs(bool subs, bool *del)
{
	bool dssubs = (videoControl->m_IsDirectShow && subs && Notebook::GetTab()->editor);
	byte *cpy1;
	byte bytes = (m_Format == RGB32) ? 4 : (m_Format == YUY2) ? 2 : 1;
	int all = m_Height * m_Pitch;
	if (dssubs){
		*del = true;
		byte *cpy = new byte[all];
		cpy1 = cpy;
	}
	else{ *del = false; }
	if (dssubs){
		byte *data1 = m_FrameBuffer;
		memcpy(cpy1, data1, all);
		m_SubsProvider->SetVideoParameters(wxSize(m_Width, m_Height), RGB32, m_SwapFrame);
		m_SubsProvider->Draw(cpy1, m_Time);
		m_SubsProvider->SetVideoParameters(wxSize(m_WindowWidth, m_WindowHeight), ARGB32, m_SwapFrame);
	}
	return (dssubs) ? cpy1 : m_FrameBuffer;
}
