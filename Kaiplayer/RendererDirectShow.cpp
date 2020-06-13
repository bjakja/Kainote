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
#include "kainoteApp.h"
#include "CsriMod.h"

#pragma comment(lib, "Dxva2.lib")
const IID IID_IDirectXVideoProcessorService = { 0xfc51a552, 0xd5e7, 0x11d9, { 0xaf, 0x55, 0x00, 0x05, 0x4e, 0x43, 0xff, 0x02 } };

RendererDirectShow::RendererDirectShow(VideoCtrl *control)
	: RendererVideo(control)
	, DShowPlayer(control)
{

}

RendererDirectShow::~RendererDirectShow()
{

}

bool RendererDirectShow::InitRendererDX()
{
	HR(d3device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bars), _("Nie mo¿na stworzyæ powierzchni"));
	HR(DXVA2CreateVideoService(d3device, IID_IDirectXVideoProcessorService, (VOID**)&dxvaService),
		_("Nie mo¿na stworzyæ DXVA processor service"));
	DXVA2_VideoDesc videoDesc;
	videoDesc.SampleWidth = vwidth;
	videoDesc.SampleHeight = vheight;
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

	HR(dxvaService->GetVideoProcessorDeviceGuids(&videoDesc, &count, &guids), _("Nie mo¿na pobraæ GUIDów DXVA"));
	D3DFORMAT* formats = NULL;
	//D3DFORMAT* formats2 = NULL;
	bool isgood = false;
	GUID dxvaGuid;
	DXVA2_VideoProcessorCaps DXVAcaps;
	HRESULT hr;
	for (UINT i = 0; i < count; i++){
		hr = dxvaService->GetVideoProcessorRenderTargets(guids[i], &videoDesc, &count1, &formats);
		if (FAILED(hr)){ KaiLog(_("Nie mo¿na uzyskaæ formatów DXVA")); continue; }
		for (UINT j = 0; j < count1; j++)
		{
			if (formats[j] == D3DFMT_X8R8G8B8)
			{
				isgood = true; //break;
			}

		}

		CoTaskMemFree(formats);
		if (!isgood){ KaiLog(_("Ten format nie jest obs³ugiwany przez DXVA")); continue; }
		isgood = false;

		hr = dxvaService->GetVideoProcessorCaps(guids[i], &videoDesc, D3DFMT_X8R8G8B8, &DXVAcaps);
		if (FAILED(hr)){ KaiLog(_("GetVideoProcessorCaps zawiod³o")); continue; }
		if (DXVAcaps.NumForwardRefSamples > 0 || DXVAcaps.NumBackwardRefSamples > 0){
			continue;
		}

		//if(DXVAcaps.DeviceCaps!=4){continue;}//DXVAcaps.InputPool
		hr = dxvaService->CreateSurface(vwidth, vheight, 0, d3dformat, D3DPOOL_DEFAULT, 0,
			DXVA2_VideoSoftwareRenderTarget, &MainStream, NULL);
		if (FAILED(hr)){ KaiLog(wxString::Format(_("Nie mo¿na stworzyæ powierzchni DXVA %i"), (int)i)); continue; }

		hr = dxvaService->CreateVideoProcessor(guids[i], &videoDesc, D3DFMT_X8R8G8B8, 0, &dxvaProcessor);
		if (FAILED(hr)){ KaiLog(_("Nie mo¿na stworzyæ processora DXVA")); continue; }
		dxvaGuid = guids[i]; isgood = true;
		break;
	}
	CoTaskMemFree(guids);
	PTR(isgood, L"Nie ma ¿adnych guidów");

}

void RendererDirectShow::Render(bool redrawSubsOnFrame, bool wait)
{
	wxCriticalSectionLocker lock(mutexRender);
	resized = false;
	HRESULT hr = S_OK;

	if (devicelost)
	{
		if (FAILED(hr = d3device->TestCooperativeLevel()))
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
				if (Visual){
					Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top,
						backBufferRect.right, backBufferRect.bottom), lines, m_font, d3device);
				}
				devicelost = false;
				Render(true, false);
				return;
			}
			return;
		}
		devicelost = false;
	}

	hr = d3device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	DXVA2_VideoProcessBltParams blt = { 0 };
	DXVA2_VideoSample samples = { 0 };
	LONGLONG start_100ns = time * 10000;
	LONGLONG end_100ns = start_100ns + 170000;
	blt.TargetFrame = start_100ns;
	blt.TargetRect = windowRect;

	// DXVA2_VideoProcess_Constriction
	blt.ConstrictionSize.cx = windowRect.right - windowRect.left;
	blt.ConstrictionSize.cy = windowRect.bottom - windowRect.top;
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

	samples.SrcSurface = MainStream;

	samples.SrcRect = mainStreamRect;

	samples.DstRect = backBufferRect;

	// DXVA2_VideoProcess_PlanarAlpha
	samples.PlanarAlpha = DXVA2_Fixed32OpaqueAlpha();

	hr = dxvaProcessor->VideoProcessBlt(bars, &blt, &samples, 1, NULL);
	
	hr = d3device->BeginScene();

#if byvertices


	// Render the vertex buffer contents
	hr = d3device->SetStreamSource(0, vertex, 0, sizeof(CUSTOMVERTEX));
	hr = d3device->SetVertexShader(NULL);
	hr = d3device->SetFVF(D3DFVF_CUSTOMVERTEX);
	hr = d3device->SetTexture(0, texture);
	hr = d3device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
#endif

	if (Visual){ Visual->Draw(time); }

	if (cross){
		DRAWOUTTEXT(m_font, coords, crossRect, (crossRect.left < vectors[0].x) ? 10 : 8, 0xFFFFFFFF)
			hr = lines->SetWidth(3);
		hr = lines->Begin();
		hr = lines->Draw(&vectors[0], 2, 0xFF000000);
		hr = lines->Draw(&vectors[2], 2, 0xFF000000);
		hr = lines->End();
		hr = lines->SetWidth(1);
		D3DXVECTOR2 v1[4];
		v1[0] = vectors[0];
		v1[0].x += 0.5f;
		v1[1] = vectors[1];
		v1[1].x += 0.5f;
		v1[2] = vectors[2];
		v1[2].y += 0.5f;
		v1[3] = vectors[3];
		v1[3].y += 0.5f;
		hr = lines->Begin();
		hr = lines->Draw(&v1[0], 2, 0xFFFFFFFF);
		hr = lines->Draw(&v1[2], 2, 0xFFFFFFFF);
		hr = lines->End();
	}

	if (fullScreenProgressBar){
		DRAWOUTTEXT(m_font, pbtime, progressBarRect, DT_LEFT | DT_TOP, 0xFFFFFFFF)
			hr = lines->SetWidth(1);
		hr = lines->Begin();
		hr = lines->Draw(&vectors[4], 5, 0xFF000000);
		hr = lines->Draw(&vectors[9], 5, 0xFFFFFFFF);
		hr = lines->End();
		hr = lines->SetWidth(7);
		hr = lines->Begin();
		hr = lines->Draw(&vectors[14], 2, 0xFFFFFFFF);
		hr = lines->End();
	}
	if (hasZoom){ DrawZoom(); }
	// End the scene
	hr = d3device->EndScene();
	hr = d3device->Present(NULL, &windowRect, NULL, NULL);
	if (D3DERR_DEVICELOST == hr ||
		D3DERR_DRIVERINTERNALERROR == hr){
		if (!devicelost){
			devicelost = true;
		}
		Render(true, false);
	}

}

void RendererDirectShow::RecreateSurface()
{
	int all = vheight * pitch;
	char *cpy = new char[all];
	byte *cpy1 = (byte*)cpy;
	byte *data1 = (byte*)frameBuffer;
	memcpy(cpy1, data1, all);
	DrawTexture(cpy1);
	delete[] cpy;
}

bool RendererDirectShow::OpenFile(const wxString &fname, wxString *textsubs, bool Dshow, bool vobsub, bool changeAudio)
{
	wxMutexLocker lock(mutexOpenFile);
	kainoteApp *Kaia = (kainoteApp*)wxTheApp;
	TabPanel *tab = ((TabPanel*)videoControl->GetParent());
	VideoFfmpeg *tmpvff = NULL;
	if (vstate == Playing){ videoControl->Stop(); }

	if (vstate != None){
		resized = seek = cross = fullScreenProgressBar = false;
		vstate = None;
		Clear();
	}
	time = 0;
	numframe = 0;


	if (!vplayer){ vplayer = new DShowPlayer(videoControl); }

	if (!vplayer->OpenFile(fname, vobsub)){
		return false;
	}
	wxSize videoSize = vplayer->GetVideoSize();
	vwidth = videoSize.x; vheight = videoSize.y;
	if (vwidth % 2 != 0){ vwidth++; }

	pitch = vwidth * vplayer->inf.bytes;
	fps = vplayer->inf.fps;
	vformat = vplayer->inf.CT;
	ax = vplayer->inf.ARatioX;
	ay = vplayer->inf.ARatioY;
	d3dformat = (vformat == 5) ? D3DFORMAT('21VN') : (vformat == 3) ? D3DFORMAT('21VY') :
		(vformat == 2) ? D3DFMT_YUY2 : D3DFMT_X8R8G8B8;
	//KaiLog(wxString::Format(L"vformat %i", (int)vformat));
	swapFrame = (vformat == 0 && !vplayer->HasVobsub());
	if (player){
		Kaia->Frame->OpenAudioInTab(((TabPanel*)videoControl->GetParent()), GLOBAL_CLOSE_AUDIO, L"");
	}

	diff = 0;
	frameDuration = (1000.0f / fps);
	if (ay == 0 || ax == 0){ AR = 0.0f; }
	else{ AR = (float)ay / (float)ax; }

	mainStreamRect.bottom = vheight;
	mainStreamRect.right = vwidth;
	mainStreamRect.left = 0;
	mainStreamRect.top = 0;
	if (frameBuffer){ delete[] frameBuffer; frameBuffer = NULL; }
	frameBuffer = new char[vheight*pitch];

	if (!InitDX()){ return false; }
	UpdateRects();

	if (!framee){ framee = new csri_frame; }
	if (!format){ format = new csri_fmt; }
	for (int i = 1; i < 4; i++){
		framee->planes[i] = NULL;
		framee->strides[i] = NULL;
	}

	framee->pixfmt = (vformat == 5) ? CSRI_F_YV12A : (vformat == 3) ? CSRI_F_YV12 :
		(vformat == 2) ? CSRI_F_YUY2 : CSRI_F_BGR_;

	format->width = vwidth;
	format->height = vheight;
	format->pixfmt = framee->pixfmt;

	if (!vobsub){
		OpenSubs(textsubs, false);
	}
	else{
		SAFE_DELETE(textsubs);
		OpenSubs(0, false);
	}
	vstate = Stopped;
	vplayer->GetChapters(&videoControl->chapters);

	if (Visual){
		Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top,
			backBufferRect.right, backBufferRect.bottom), lines, m_font, d3device);
	}
	return true;
}