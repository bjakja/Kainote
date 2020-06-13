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
#include "kainoteApp.h"
#include "CsriMod.h"
#include "OpennWrite.h"

RendererFFMS2::RendererFFMS2(VideoCtrl *control)
	: RendererVideo(control)
	, VFF(NULL)
{
	
}
RendererFFMS2::~RendererFFMS2()
{
	SAFE_DELETE(VFF);
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

bool RendererFFMS2::OpenFile(const wxString &fname, wxString *textsubs, bool vobsub, bool changeAudio)
{
	wxMutexLocker lock(mutexOpenFile);
	kainoteApp *Kaia = (kainoteApp*)wxTheApp;
	VideoFfmpeg *tmpvff = NULL;
	if (vstate == Playing){ videoControl->Stop(); }

	bool success;
	tmpvff = new VideoFfmpeg(fname, this, (videoControl->isFullscreen) ? videoControl->TD : (wxWindow *)Kaia->Frame, &success);
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

	SAFE_DELETE(VFF);

	if (vstate != None){
		resized = seek = cross = fullScreenProgressBar = false;
		vstate = None;
		Clear();
	}

	time = 0;
	numframe = 0;

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
	VFF->GetChapters(&chapters);

	if (Visual){
		Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top,
			backBufferRect.right, backBufferRect.bottom), lines, m_font, d3device);
	}
	return true;
}

bool RendererFFMS2::OpenSubs(wxString *textsubs, bool redraw, bool fromFile)
{
	wxCriticalSectionLocker lock(mutexRender);
	if (instance) csri_close(instance);
	instance = NULL;

	if (!textsubs) {
		if (redraw && vstate != None && frameBuffer){
			RecreateSurface();
		}
		hasDummySubs = true;
		return true;
	}

	if (hasVisualEdition && Visual->Visual == VECTORCLIP && Visual->dummytext){
		wxString toAppend = Visual->dummytext->Trim().AfterLast(L'\n') + L"\r\n";
		if (fromFile){
			OpenWrite ow(*textsubs, false);
			ow.PartFileWrite(toAppend);
			ow.CloseFile();
		}
		else{
			(*textsubs) << toAppend;
		}
	}

	hasDummySubs = !fromFile;

	wxScopedCharBuffer buffer = textsubs->mb_str(wxConvUTF8);
	int size = strlen(buffer);


	// Select renderer
	csri_rend *vobsub = Options.GetVSFilter();
	if (!vobsub){ KaiLog(_("CSRI odmówi³o pos³uszeñstwa.")); delete textsubs; return false; }

	instance = (fromFile) ? csri_open_file(vobsub, buffer, NULL) : csri_open_mem(vobsub, buffer, size, NULL);
	if (!instance){ KaiLog(_("B³¹d, instancja VobSuba nie zosta³a utworzona.")); delete textsubs; return false; }

	if (!format || csri_request_fmt(instance, format)){
		KaiLog(_("CSRI nie obs³uguje tego formatu."));
		csri_close(instance);
		instance = NULL;
		delete textsubs; return false;
	}

	if (redraw && vstate != None && frameBuffer){
		RecreateSurface();
	}

	delete textsubs;
	return true;
}

bool RendererFFMS2::Play(int end)
{
	if (time >= GetDuration()){ return false; }
	SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS);
	if (!(videoControl->IsShown() || (videoControl->TD && videoControl->TD->IsShown()))){ return false; }
	if (hasVisualEdition){
		wxString *txt = tab->Grid->SaveText();
		OpenSubs(txt, false, true);
		SAFE_DELETE(Visual->dummytext);
		hasVisualEdition = false;
	}
	else if (hasDummySubs && tab->editor){
		OpenSubs(tab->Grid->SaveText(), false, true);
	}

	if (end > 0){ playend = end; }
	playend = GetDuration();

	vstate = Playing;

	time = VFF->Timecodes[numframe];
	lasttime = timeGetTime() - time;
	if (player){ player->Play(time, -1, false); }
	VFF->Play();

	return true;
}


bool RendererFFMS2::Pause()
{
	if (vstate == Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		vstate = Paused;
		if (player){ player->Stop(false); }
	}
	else if (vstate != None){
		Play();
	}
	else{ return false; }
	return true;
}

bool RendererFFMS2::Stop()
{
	if (vstate == Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		vstate = Stopped;
		if (player){
			player->Stop();
			playend = GetDuration();
		}
		time = 0;
		return true;
	}
	return false;
}

void RendererFFMS2::SetPosition(int _time, bool starttime/*=true*/, bool corect/*=true*/, bool async /*= true*/)
{
	if (vstate == Playing || !async)
		SetFFMS2Position(_time, starttime);
	else
		VFF->SetPosition(_time, starttime);
}

//is from video thread make safe any deletion
void RendererFFMS2::SetFFMS2Position(int _time, bool starttime){
	bool playing = vstate == Playing;
	numframe = VFF->GetFramefromMS(_time, (time > _time) ? 0 : numframe);
	if (!starttime){
		numframe--;
		if (VFF->Timecodes[numframe] >= _time){ numframe--; }
	}
	time = VFF->Timecodes[numframe];
	lasttime = timeGetTime() - time;
	playend = GetDuration();

	if (hasVisualEdition){
		//block removing or changing visual from main thread
		wxMutexLocker lock(mutexVisualChange);
		SAFE_DELETE(Visual->dummytext);
		if (Visual->Visual == VECTORCLIP){
			Visual->SetClip(Visual->GetVisual(), true, false, false);
		}
		else{
			OpenSubs((playing) ? tab->Grid->SaveText() : tab->Grid->GetVisible(), true, playing);
			if (playing){ hasVisualEdition = false; }
		}
	}
	else if (hasDummySubs){
		OpenSubs((playing) ? tab->Grid->SaveText() : tab->Grid->GetVisible(), true, playing);
	}
	if (playing){
		if (player){
			player->player->SetCurrentPosition(player->GetSampleAtMS(time));
		}
	}
	else{
		//rebuild spectrum cause position can be changed
		//and it causes random bugs
		if (player){ player->UpdateImage(false, true); }
		VFF->Render();
		videoControl->RefreshTime();
	}
}

int RendererFFMS2::GetDuration()
{
	return VFF->Duration * 1000.0;
}

int RendererFFMS2::GetFrameTime(bool start)
{
	if (start){
		int prevFrameTime = VFF->GetMSfromFrame(numframe - 1);
		return time + ((prevFrameTime - time) / 2);
	}
	else{
		if (numframe + 1 >= VFF->NumFrames){
			int prevFrameTime = VFF->GetMSfromFrame(numframe - 1);
			return time + ((time - prevFrameTime) / 2);
		}
		else{
			int nextFrameTime = VFF->GetMSfromFrame(numframe + 1);
			return time + ((nextFrameTime - time) / 2);
		}
	}
}

void RendererFFMS2::GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd)
{
	if (!retStart || !retEnd){ return; }
	
	int frameStartTime = VFF->GetFramefromMS(startTime);
	int frameEndTime = VFF->GetFramefromMS(endTime, frameStartTime);
	*retStart = VFF->GetMSfromFrame(frameStartTime) - startTime;
	*retEnd = VFF->GetMSfromFrame(frameEndTime) - endTime;
}

int RendererFFMS2::GetFrameTimeFromTime(int _time, bool start)
{
	if (start){
		int frameFromTime = VFF->GetFramefromMS(_time);
		int prevFrameTime = VFF->GetMSfromFrame(frameFromTime - 1);
		int frameTime = VFF->GetMSfromFrame(frameFromTime);
		return frameTime + ((prevFrameTime - frameTime) / 2);
	}
	else{
		int frameFromTime = VFF->GetFramefromMS(_time);
		int nextFrameTime = VFF->GetMSfromFrame(frameFromTime + 1);
		int frameTime = VFF->GetMSfromFrame(frameFromTime);
		return frameTime + ((nextFrameTime - frameTime) / 2);
	}
}

int RendererFFMS2::GetFrameTimeFromFrame(int frame, bool start)
{
	if (start){
		int prevFrameTime = VFF->GetMSfromFrame(frame - 1);
		int frameTime = VFF->GetMSfromFrame(frame);
		return frameTime + ((prevFrameTime - frameTime) / 2);
	}
	else{
		int nextFrameTime = VFF->GetMSfromFrame(frame + 1);
		int frameTime = VFF->GetMSfromFrame(frame);
		return frameTime + ((nextFrameTime - frameTime) / 2);
	}
}

int RendererFFMS2::GetPlayEndTime(int _time)
{
	int frameFromTime = VFF->GetFramefromMS(_time);
	int prevFrameTime = VFF->GetMSfromFrame(frameFromTime - 1);
	return prevFrameTime;
}

void RendererFFMS2::OpenKeyframes(const wxString &filename)
{
	VFF->OpenKeyframes(filename);
}

void RendererFFMS2::GetFpsnRatio(float *fps, long *arx, long *ary)
{
	*fps = VFF->fps;
	*arx = VFF->arwidth;
	*ary = VFF->arheight;
}

void RendererFFMS2::GetVideoSize(int *width, int *height)
{
	*width = VFF->width;
	*height = VFF->height;
}

wxSize RendererFFMS2::GetVideoSize()
{
	wxSize sz;
	sz.x = VFF->width;
	sz.y = VFF->height;
	return sz;
}

void RendererFFMS2::SetVolume(int vol)
{
	if (vstate == None || !player){ return; }
	
	vol = 7600 + vol;
	double dvol = vol / 7600.0;
	int sliderValue = (dvol * 99) + 1;
	if (tab->Edit->ABox){
		tab->Edit->ABox->SetVolume(sliderValue);
	}
}

int RendererFFMS2::GetVolume()
{
	if (vstate == None || !player){ return 0; }
	double dvol = player->player->GetVolume();
	dvol = sqrt(dvol);
	dvol *= 8100.0;
	dvol -= 8100.0;
	return dvol;
}

void RendererFFMS2::ChangePositionByFrame(int step)
{
	if (vstate == Playing || vstate == None){ return; }
	
		numframe = MID(0, numframe + step, VFF->NumFrames - 1);
		time = VFF->Timecodes[numframe];
		if (hasVisualEdition || hasDummySubs){
			OpenSubs(tab->Grid->SaveText(), false, true);
			hasVisualEdition = false;
		}
		if (player){ player->UpdateImage(true, true); }
		Render(true, false);
	
	
	videoControl->RefreshTime();

}

//bool VideoRenderer::EnumFilters(Menu *menu)
//{
//	if (vplayer){ return vplayer->EnumFilters(menu); }
//	return false;
//}
//
//bool VideoRenderer::FilterConfig(wxString name, int idx, wxPoint pos)
//{
//	if (vplayer){ return vplayer->FilterConfig(name, idx, pos); }
//	return false;
//}

byte *RendererFFMS2::GetFramewithSubs(bool subs, bool *del)
{
	bool ffnsubs = (!subs);
	byte *cpy1;
	byte bytes = (vformat == RGB32) ? 4 : (vformat == YUY2) ? 2 : 1;
	int all = vheight * pitch;
	if (ffnsubs){
		*del = true;
		char *cpy = new char[all];
		cpy1 = (byte*)cpy;
		VFF->GetFrame(time, cpy1);
	}
	else{ *del = false; }
	return (ffnsubs) ? cpy1 : (byte*)frameBuffer;
}

void RendererFFMS2::GoToNextKeyframe()
{
	for (size_t i = 0; i < VFF->KeyFrames.size(); i++){
		if (VFF->KeyFrames[i] > time){
			SetPosition(VFF->KeyFrames[i]);
			return;
		}
	}
	SetPosition(VFF->KeyFrames[0]);
}
void RendererFFMS2::GoToPrevKeyframe()
{
	for (int i = VFF->KeyFrames.size() - 1; i >= 0; i--){
		if (VFF->KeyFrames[i] < time){
			SetPosition(VFF->KeyFrames[i]);
			return;
		}
	}
	SetPosition(VFF->KeyFrames[VFF->KeyFrames.size() - 1]);
}