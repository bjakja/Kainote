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

#include "RendererFFMS2.h"

RendererFFMS2::RendererFFMS2(VideoCtrl *control, const wxString &filename, wxWindow *progressSinkWindow, bool *success)
	: RendererVideo(control)
	, VideoFfmpeg(filename, this, progressSinkWindow, success)
{
	
}
RendererFFMS2::~RendererFFMS2()
{

}

void RendererFFMS2::Render(bool redrawSubsOnFrame, bool wait)
{
	if (redrawSubsOnFrame && !devicelost){
		VFF->Render(wait);
		resized = false;
		return;
	}
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

	
	hr = d3device->StretchRect(MainStream, &mainStreamRect, bars, &backBufferRect, D3DTEXF_LINEAR);
	if (FAILED(hr)){ KaiLog(_("Nie mo¿na na³o¿yæ powierzchni na siebie")); }


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

bool VideoRenderer::OpenFile(const wxString &fname, wxString *textsubs, bool Dshow, bool vobsub, bool changeAudio)
{
	wxMutexLocker lock(mutexOpenFile);
	kainoteApp *Kaia = (kainoteApp*)wxTheApp;
	TabPanel *tab = ((TabPanel*)GetParent());
	VideoFfmpeg *tmpvff = NULL;
	if (vstate == Playing){ ((VideoCtrl*)this)->Stop(); }

	if (!Dshow){
		bool success;
		tmpvff = new VideoFfmpeg(fname, this, (isFullscreen) ? ((VideoCtrl*)this)->TD : (wxWindow *)Kaia->Frame, &success);
		//this is safe mode, when new video not load, 
		//the last opened will not be released
		if (!success || !tmpvff){
			SAFE_DELETE(tmpvff);
			return false;
		}
		//when loading only audio do not remove video
		if (tmpvff->width < 0 && tmpvff->GetSampleRate() > 0){
			VideoFfmpeg *tmp = VFF;
			VFF = tmpvff;
			Kaia->Frame->OpenAudioInTab(tab, 40000, fname);
			player = tab->Edit->ABox->audioDisplay;
			VFF = tmp;
			return false;
		}
	}
	SAFE_DELETE(VFF);

	if (vstate != None){
		resized = seek = cross = fullScreenProgressBar = false;
		vstate = None;
		Clear();
	}
	IsDshow = Dshow;
	time = 0;
	numframe = 0;

	if (!Dshow){
		SAFE_DELETE(vplayer);
		VFF = tmpvff;
		d3dformat = D3DFMT_X8R8G8B8;
		vformat = RGB32;
		vwidth = VFF->width;
		vheight = VFF->height;
		fps = VFF->fps;
		ax = VFF->arwidth;
		ay = VFF->arheight;
		if (vwidth % 2 != 0){ vwidth++; }
		pitch = vwidth * 4;
		if (changeAudio){
			if (VFF->GetSampleRate() > 0){
				Kaia->Frame->OpenAudioInTab(tab, 40000, fname);
				player = tab->Edit->ABox->audioDisplay;
			}
			else if (player){ Kaia->Frame->OpenAudioInTab(tab, GLOBAL_CLOSE_AUDIO, L""); }
		}
		if (!VFF || VFF->width < 0){
			return false;
		}
	}
	else{

		if (!vplayer){ vplayer = new DShowPlayer(this); }

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
			Kaia->Frame->OpenAudioInTab(((TabPanel*)GetParent()), GLOBAL_CLOSE_AUDIO, L"");
		}
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
	if (IsDshow && vplayer)
		vplayer->GetChapters(&chapters);
	else if (!IsDshow)
		VFF->GetChapters(&chapters);

	if (Visual){
		Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top,
			backBufferRect.right, backBufferRect.bottom), lines, m_font, d3device);
	}
	return true;
}
