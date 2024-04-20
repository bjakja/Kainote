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



#include "AudioBox.h"
#include "EditBox.h"
#include "kainoteApp.h"
#include "OpennWrite.h"
#include "DshowRenderer.h"
#include "RendererFFMS2.h"
#include "VisualDrawingShapes.h"
#include "Visuals.h"
#include "VideoFullscreen.h"
#include "SubtitlesProviderManager.h"

RendererFFMS2::RendererFFMS2(VideoBox *control, bool visualDisabled)
	: RendererVideo(control, visualDisabled)
	, m_FFMS2(nullptr)
{
	
}

RendererFFMS2::~RendererFFMS2()
{
	Stop();

	m_State = None;
	SAFE_DELETE(m_FFMS2);
}
//made function to destroy it before FFMS2
void RendererFFMS2::DestroyFFMS2()
{
	//SAFE_DELETE(m_FFMS2);
}

bool RendererFFMS2::DrawTexture(unsigned char *nframe, bool copy)
{
	wxCriticalSectionLocker lock(m_MutexRendering);
	if (!m_MainSurface)
		return false;

	unsigned char * fdata = nullptr;
	unsigned char * texbuf;
	unsigned char bytes = 4;

	D3DLOCKED_RECT d3dlr;

	if (nframe) {
		fdata = nframe;
		if (copy) {
			byte *cpy = m_FrameBuffer;
			memcpy(cpy, fdata, m_Height * m_Pitch);
		}
	}
	else {
		fdata = m_FrameBuffer;
		m_FFMS2->GetFrameBuffer(&fdata);
		if (!fdata)
			return false;
	}


	m_SubsProvider->Draw(fdata, m_Time);


#ifdef byvertices
	HR(m_MainSurface->LockRect(&d3dlr, 0, 0), _("Nie można zablokować bufora tekstury"));//D3DLOCK_NOSYSLOCK
#else
	HR(m_MainSurface->LockRect(&d3dlr, 0, D3DLOCK_NOSYSLOCK), _("Nie można zablokować bufora tekstury"));
#endif
	texbuf = static_cast<unsigned char*>(d3dlr.pBits);

	diff = d3dlr.Pitch - (m_Width*bytes);
	if (m_SwapFrame) {
		int framePitch = m_Width * bytes;
		unsigned char* reversebyte = fdata + (framePitch * m_Height) - framePitch;
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

		int fwidth = m_Width * bytes;
		for (int i = 0; i < m_Height; i++) {
			memcpy(texbuf, fdata, fwidth);
			texbuf += (fwidth + diff);
			fdata += fwidth;
		}

	}
	else {
		KaiLog(wxString::Format(L"bad pitch diff %i pitch %i dxpitch %i", diff, m_Pitch, d3dlr.Pitch));
	}

	HR(m_MainSurface->UnlockRect(), _("Nie można odblokować bufora tekstury"));

	return true;
}

void RendererFFMS2::Render(bool redrawSubsOnFrame, bool wait)
{
	if (redrawSubsOnFrame && !m_DeviceLost){
		//no need to return cause of render do not send an event and need to be safe from start.
		if (!DrawTexture())
			return;
		//m_VideoResized = false;
		//return;
	}
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
				//make sure that dx is initialized 
				//without device it crashes in SizeChanged
				while (!InitDX()) {
					Sleep(20);
				}
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

	hr = m_D3DDevice->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	
	hr = m_D3DDevice->StretchRect(m_MainSurface, &m_MainStreamRect, m_BlackBarsSurface, &m_BackBufferRect, D3DTEXF_LINEAR);
	if (FAILED(hr)){ KaiLog(_("Nie można nałożyć powierzchni na siebie")); }


	hr = m_D3DDevice->BeginScene();

#if byvertices


	// Render the vertex buffer contents
	hr = m_D3DDevice->SetStreamSource(0, vertex, 0, sizeof(CUSTOMVERTEX));
	hr = m_D3DDevice->SetVertexShader(nullptr);
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
		hr = m_D3DLine->SetWidth(m_ProgressBarLineWidth);
		hr = m_D3DLine->Begin();
		hr = m_D3DLine->Draw(&vectors[10], 2, 0xFFFFFFFF);
		hr = m_D3DLine->End();
	}
	if (m_HasZoom){ DrawZoom(); }
	// End the scene
	hr = m_D3DDevice->EndScene();
	hr = m_D3DDevice->Present(nullptr, &m_WindowRect, nullptr, nullptr);
	if (D3DERR_DEVICELOST == hr ||
		D3DERR_DRIVERINTERNALERROR == hr){
		if (!m_DeviceLost){
			m_DeviceLost = true;
		}
		Render(true, false);
	}

}

bool RendererFFMS2::OpenFile(const wxString &fname, int subsFlag, bool vobsub, bool changeAudio)
{
	wxMutexLocker lock(m_MutexOpen);
	kainoteApp *Kaia = (kainoteApp*)wxTheApp;
	Provider *tmpvff = nullptr;
	if (m_State == Playing){ videoControl->Stop(); }

	bool success;
	tmpvff = Provider::Get(fname, this, videoControl->GetMessageWindowParent(), &success);
	//this is safe mode, when new video not load, 
	//the last opened will not be released
	if (!success || !tmpvff){
		SAFE_DELETE(tmpvff);
		return false;
	}
	
	//when loading only audio do not remove video
	if (tmpvff->m_width < 0 && tmpvff->GetSampleRate() > 0){
		Provider *tmp = m_FFMS2;
		m_FFMS2 = tmpvff;
		Kaia->Frame->OpenAudioInTab(tab, 40000, fname);
		m_AudioPlayer = tab->edit->ABox->audioDisplay;
		m_FFMS2 = tmp;
		return false;
	}

	SAFE_DELETE(m_FFMS2);

	if (m_State != None){
		m_VideoResized = videoControl->m_FullScreenProgressBar = false;
		m_State = None;
		Clear();
	}

	m_Time = 0;
	m_Frame = 0;

	m_FFMS2 = tmpvff;
	m_D3DFormat = D3DFMT_X8R8G8B8;
	m_Format = RGB32;
	m_Width = m_FFMS2->m_width;
	m_Height = m_FFMS2->m_height;
	videoControl->m_FPS = m_FFMS2->m_FPS;
	videoControl->m_AspectRatioX = m_FFMS2->m_arwidth;
	videoControl->m_AspectRatioY = m_FFMS2->m_arheight;
	if (m_Width % 2 != 0){ m_Width++; }
	m_Pitch = m_Width * 4;
	if (changeAudio){
		if (m_FFMS2->GetSampleRate() > 0){
			Kaia->Frame->OpenAudioInTab(tab, 40000, fname);
			m_AudioPlayer = tab->edit->ABox->audioDisplay;
		}
		else if (m_AudioPlayer){ Kaia->Frame->OpenAudioInTab(tab, GLOBAL_CLOSE_AUDIO, emptyString); }
	}
	if (!m_FFMS2 || m_FFMS2->m_width < 0){
		return false;
	}
	
	diff = 0;
	m_FrameDuration = (1000.0f / videoControl->m_FPS);
	if (videoControl->m_AspectRatioY == 0 || videoControl->m_AspectRatioX == 0){ videoControl->m_AspectRatio = 0.0f; }
	else{ videoControl->m_AspectRatio = (float)videoControl->m_AspectRatioY / (float)videoControl->m_AspectRatioX; }

	m_MainStreamRect.bottom = m_Height;
	m_MainStreamRect.right = m_Width;
	m_MainStreamRect.left = 0;
	m_MainStreamRect.top = 0;
	if (m_FrameBuffer){ delete[] m_FrameBuffer; m_FrameBuffer = nullptr; }
	m_FrameBuffer = new byte[m_Height * m_Pitch];

	UpdateRects();

	if (!InitDX()){ 
		return false; 
	}
	
	m_SubsProvider->SetVideoParameters(wxSize(m_Width, m_Height), RGB32, false);

	OpenSubs(subsFlag, false);
	
	m_State = Stopped;
	m_FFMS2->GetChapters(&m_Chapters);

	if (m_Visual){
		m_Visual->SizeChanged(wxRect(m_BackBufferRect.left, m_BackBufferRect.top,
			m_BackBufferRect.right, m_BackBufferRect.bottom), m_D3DLine, m_D3DFont, m_D3DDevice);
	}
	return true;
}

bool RendererFFMS2::OpenSubs(int flag, bool redraw, wxString *text, bool resetParameters)
{
	wxCriticalSectionLocker lock(m_MutexRendering);
	if (resetParameters)
		m_SubsProvider->SetVideoParameters(wxSize(m_Width, m_Height), m_Format, m_SwapFrame);

	return m_SubsProvider->Open(tab, flag, text);
}

bool RendererFFMS2::Play(int end)
{
	if (m_Time >= GetDuration()){ return false; }
	SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS);
	if (!(videoControl->IsShown() || 
		(videoControl->m_FullScreenWindow && 
			videoControl->m_FullScreenWindow->IsShown()))){ return false; }
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
		m_PlayEndTime = GetDuration();

	m_State = Playing;

	m_Time = m_FFMS2->m_timecodes[m_Frame];
	m_LastTime = timeGetTime() - m_Time;
	if (m_AudioPlayer){ m_AudioPlayer->Play(m_Time, -1, false); }
	m_FFMS2->Play();

	return true;
}


bool RendererFFMS2::Pause()
{
	if (m_State == Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		m_State = Paused;
		if (m_AudioPlayer){ m_AudioPlayer->Stop(false); }
	}
	else if (m_State != None){
		Play();
	}
	else{ return false; }
	return true;
}

bool RendererFFMS2::Stop()
{
	if (m_State == Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		m_State = Stopped;
		if (m_AudioPlayer){
			m_AudioPlayer->Stop();
			m_PlayEndTime = GetDuration();
		}
		m_Time = 0;
		return true;
	}
	return false;
}

void RendererFFMS2::SetPosition(int _time, bool starttime/*=true*/, bool corect/*=true*/, bool async /*= true*/)
{
	if (m_State == Playing || !async)
		SetFFMS2Position(_time, starttime);
	else
		m_FFMS2->SetPosition(_time, starttime);
}

//is from video thread make safe any deletion
void RendererFFMS2::SetFFMS2Position(int _time, bool starttime){
	bool playing = m_State == Playing;
	m_Frame = m_FFMS2->GetFramefromMS(_time, (m_Time > _time) ? 0 : m_Frame);
	if (!starttime){
		m_Frame--;
		if (m_FFMS2->m_timecodes[m_Frame] >= _time){ m_Frame--; }
	}
	m_Time = m_FFMS2->m_timecodes[m_Frame];
	m_LastTime = timeGetTime() - m_Time;
	m_PlayEndTime = GetDuration();

	if (m_HasVisualEdition){
		//block removing or changing visual from main thread
		wxMutexLocker lock(m_MutexVisualChange);
		SAFE_DELETE(m_Visual->dummytext);
		/*if (m_Visual->Visual == VECTORCLIP){
			m_Visual->SetClip(m_Visual->GetVisual(), true, false, false);
		}
		else{*/
			OpenSubs((playing) ? OPEN_WHOLE_SUBTITLES : OPEN_DUMMY, true);
			if (playing){ m_HasVisualEdition = false; }
		//}
	}
	else if (m_HasDummySubs){
		OpenSubs((playing) ? OPEN_WHOLE_SUBTITLES : OPEN_DUMMY, true);
	}
	if (playing){
		if (m_AudioPlayer){
			m_AudioPlayer->player->SetCurrentPosition(m_AudioPlayer->GetSampleAtMS(m_Time));
		}
	}
	else{
		//rebuild spectrum cause position can be changed
		//and it causes random bugs
		if (m_AudioPlayer){ m_AudioPlayer->UpdateImage(false, true); }
		
		videoControl->RefreshTime();
		Render();
	}
}

int RendererFFMS2::GetDuration()
{
	return m_FFMS2 ? m_FFMS2->m_duration * 1000.0 : 0;
}

int RendererFFMS2::GetFrameTime(bool start)
{
	if (start){
		int prevFrameTime = m_FFMS2->GetMSfromFrame(m_Frame - 1);
		return m_Time + ((prevFrameTime - m_Time) / 2);
	}
	else{
		if (m_Frame + 1 >= m_FFMS2->m_numFrames){
			int prevFrameTime = m_FFMS2->GetMSfromFrame(m_Frame - 1);
			return m_Time + ((m_Time - prevFrameTime) / 2);
		}
		else{
			int nextFrameTime = m_FFMS2->GetMSfromFrame(m_Frame + 1);
			return m_Time + ((nextFrameTime - m_Time) / 2);
		}
	}
}

void RendererFFMS2::GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd)
{
	if (!retStart || !retEnd){ return; }
	
	int frameStartTime = m_FFMS2->GetFramefromMS(startTime);
	int frameEndTime = m_FFMS2->GetFramefromMS(endTime, frameStartTime);
	*retStart = m_FFMS2->GetMSfromFrame(frameStartTime) - startTime;
	*retEnd = m_FFMS2->GetMSfromFrame(frameEndTime) - endTime;
}

int RendererFFMS2::GetFrameTimeFromTime(int _time, bool start)
{
	if (start){
		int frameFromTime = m_FFMS2->GetFramefromMS(_time);
		int prevFrameTime = m_FFMS2->GetMSfromFrame(frameFromTime - 1);
		int frameTime = m_FFMS2->GetMSfromFrame(frameFromTime);
		return frameTime + ((prevFrameTime - frameTime) / 2);
	}
	else{
		int frameFromTime = m_FFMS2->GetFramefromMS(_time);
		int nextFrameTime = m_FFMS2->GetMSfromFrame(frameFromTime + 1);
		int frameTime = m_FFMS2->GetMSfromFrame(frameFromTime);
		return frameTime + ((nextFrameTime - frameTime) / 2);
	}
}

int RendererFFMS2::GetFrameTimeFromFrame(int frame, bool start)
{
	if (start){
		int prevFrameTime = m_FFMS2->GetMSfromFrame(frame - 1);
		int frameTime = m_FFMS2->GetMSfromFrame(frame);
		return frameTime + ((prevFrameTime - frameTime) / 2);
	}
	else{
		int nextFrameTime = m_FFMS2->GetMSfromFrame(frame + 1);
		int frameTime = m_FFMS2->GetMSfromFrame(frame);
		return frameTime + ((nextFrameTime - frameTime) / 2);
	}
}

int RendererFFMS2::GetPlayEndTime(int _time)
{
	int frameFromTime = m_FFMS2->GetFramefromMS(_time);
	int prevFrameTime = m_FFMS2->GetMSfromFrame(frameFromTime - 1);
	return prevFrameTime;
}

void RendererFFMS2::OpenKeyframes(const wxString &filename)
{
	m_FFMS2->OpenKeyframes(filename);
}

void RendererFFMS2::GetFpsnRatio(float *fps, long *arx, long *ary)
{
	*fps = m_FFMS2->m_FPS;
	*arx = m_FFMS2->m_arwidth;
	*ary = m_FFMS2->m_arheight;
}

void RendererFFMS2::GetVideoSize(int *width, int *height)
{
	*width = m_FFMS2->m_width;
	*height = m_FFMS2->m_height;
}

void RendererFFMS2::SetVolume(int vol)
{
	if (m_State == None || !m_AudioPlayer){ return; }
	
	vol = 7600 + vol;
	double dvol = vol / 7600.0;
	int sliderValue = (dvol * 99) + 1;
	if (tab->edit->ABox){
		tab->edit->ABox->SetVolume(sliderValue);
	}
}

int RendererFFMS2::GetVolume()
{
	if (m_State == None || !m_AudioPlayer){ return 0; }
	double dvol = m_AudioPlayer->player->GetVolume();
	dvol = sqrt(dvol);
	dvol *= 8100.0;
	dvol -= 8100.0;
	return dvol;
}

void RendererFFMS2::ChangePositionByFrame(int step)
{
	if (m_State == Playing || m_State == None){ return; }
	
		m_Frame = MID(0, m_Frame + step, m_FFMS2->m_numFrames - 1);
		m_Time = m_FFMS2->m_timecodes[m_Frame];
		if (m_HasVisualEdition || m_HasDummySubs){
			OpenSubs(OPEN_WHOLE_SUBTITLES, false);
			m_HasVisualEdition = false;
		}
		if (m_AudioPlayer){ m_AudioPlayer->UpdateImage(true, true); }
		Render(true, false);
	
	
	videoControl->RefreshTime();

}

byte *RendererFFMS2::GetFrameWithSubs(bool subs, bool *del)
{
	byte *cpy1;
	int all = m_Height * m_Pitch;
	if (!subs){
		*del = true;
		byte *cpy = new byte[all];
		cpy1 = cpy;
		m_FFMS2->GetFrame(m_Time, cpy1);
	}
	else{ *del = false; }
	return (!subs) ? cpy1 : m_FrameBuffer;
}

unsigned char* RendererFFMS2::GetFrame(int frame, bool subs)
{
	int all = m_Height * m_Pitch;
	byte* newFrame = new byte[all];
	m_FFMS2->GetFrame(frame, newFrame);
	if (subs) {
		m_SubsProvider->Draw(newFrame, frame);
	}
	return newFrame;
}

void RendererFFMS2::GoToNextKeyframe()
{
	for (size_t i = 0; i < m_FFMS2->m_keyFrames.size(); i++){
		if (m_FFMS2->m_keyFrames[i] > m_Time){
			SetPosition(m_FFMS2->m_keyFrames[i]);
			return;
		}
	}
	SetPosition(m_FFMS2->m_keyFrames[0]);
}
void RendererFFMS2::GoToPrevKeyframe()
{
	for (int i = m_FFMS2->m_keyFrames.size() - 1; i >= 0; i--){
		if (m_FFMS2->m_keyFrames[i] < m_Time){
			SetPosition(m_FFMS2->m_keyFrames[i]);
			return;
		}
	}
	SetPosition(m_FFMS2->m_keyFrames[m_FFMS2->m_keyFrames.size() - 1]);
}

bool RendererFFMS2::HasFFMS2()
{
	return m_FFMS2 != nullptr;
}

Provider* RendererFFMS2::GetFFMS2()
{
	return m_FFMS2;
}


bool RendererFFMS2::InitRendererDX()
{
#ifndef byvertices
	HR(m_D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_BlackBarsSurface), _("Nie można stworzyć powierzchni"));

	HR(m_D3DDevice->CreateOffscreenPlainSurface(m_Width, m_Height, m_D3DFormat, D3DPOOL_DEFAULT, &m_MainSurface, 0),
		_("Nie można stworzyć plain surface"));//D3DPOOL_DEFAULT

#endif
	return true;
}

