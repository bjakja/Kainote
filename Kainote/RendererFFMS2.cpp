//  Copyright (c) 2020 - 2026, Marcin Drob

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
#ifndef _WIN32
#include <wx/bitmap.h>
#include <wx/app.h>
#include <wx/dcclient.h>
#include <wx/image.h>
#include <wx/thread.h>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#endif

#ifndef _WIN32
namespace
{
	void DrawBgraFrameWithWxDc(wxDC& dc, const unsigned char* frame, int width, int height,
		int pitch, const RECT& targetRect)
	{
		if (!frame || width <= 0 || height <= 0 || pitch < width * 4)
			return;

		const size_t rgbSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 3;
		unsigned char* rgb = static_cast<unsigned char*>(std::malloc(rgbSize));
		if (!rgb)
			return;
		for (int y = 0; y < height; ++y) {
			const unsigned char* src = frame + (y * pitch);
			unsigned char* dst = rgb + (y * width * 3);
			for (int x = 0; x < width; ++x) {
				// FFMS2 output is requested as BGRA. wxImage expects RGB.
				dst[(x * 3) + 0] = src[(x * 4) + 2];
				dst[(x * 3) + 1] = src[(x * 4) + 1];
				dst[(x * 3) + 2] = src[(x * 4) + 0];
			}
		}

		wxImage image(width, height, rgb, false);
		int targetWidth = targetRect.right - targetRect.left;
		int targetHeight = targetRect.bottom - targetRect.top;
		if (targetWidth <= 0 || targetHeight <= 0)
			return;

		if (targetWidth != width || targetHeight != height)
			image.Rescale(targetWidth, targetHeight, wxIMAGE_QUALITY_BILINEAR);
		if (!image.IsOk())
			return;
		wxBitmap bitmap(image);
		if (!bitmap.IsOk())
			return;

		dc.DrawBitmap(bitmap, targetRect.left, targetRect.top, false);
	}

	void DrawBgraFrameWithWx(wxWindow* window, const unsigned char* frame, int width, int height,
		int pitch, const RECT& targetRect)
	{
		if (!window)
			return;
		wxClientDC dc(window);
		DrawBgraFrameWithWxDc(dc, frame, width, height, pitch, targetRect);
	}
}
#endif

RendererFFMS2::RendererFFMS2(VideoBox *control, bool visualDisabled)
	: RendererVideo(control, visualDisabled)
	, m_FFMS2(nullptr)
{
	
}

#ifndef _WIN32
void RendererFFMS2::QueueLinuxRender()
{
	if (!videoControl)
		return;

	bool expected = false;
	if (!m_LinuxRenderQueued.compare_exchange_strong(expected, true))
		return;

	wxTheApp->CallAfter([this]() {
		m_LinuxRenderQueued.store(false);
		{
			std::lock_guard<std::mutex> lock(m_LinuxPendingFrameMutex);
			m_LinuxPresentFrame.swap(m_LinuxPendingFrame);
		}
		if (m_State != None && !m_LinuxPresentFrame.empty()) {
			wxWindow* renderWindow = (videoControl->m_IsFullscreen && videoControl->m_FullScreenWindow) ?
				static_cast<wxWindow*>(videoControl->m_FullScreenWindow) : static_cast<wxWindow*>(videoControl);
			if (renderWindow)
				renderWindow->Refresh(false);
		}
	});
	wxWakeUpIdle();
}

void RendererFFMS2::PresentLinuxFrame(const unsigned char* frame)
{
	if (!frame || m_Height <= 0 || m_Pitch <= 0)
		return;
	{
		std::lock_guard<std::mutex> pendingLock(m_LinuxPendingFrameMutex);
		const size_t frameBytes = static_cast<size_t>(m_Height) * static_cast<size_t>(m_Pitch);
		if (m_LinuxPresentFrame.size() != frameBytes)
			m_LinuxPresentFrame.resize(frameBytes);
		if (m_LinuxPresentFrame.data() != frame)
			memcpy(m_LinuxPresentFrame.data(), frame, frameBytes);
	}
	wxWindow* renderWindow = (videoControl->m_IsFullscreen && videoControl->m_FullScreenWindow) ?
		static_cast<wxWindow*>(videoControl->m_FullScreenWindow) : static_cast<wxWindow*>(videoControl);
	if (renderWindow)
		renderWindow->Refresh(false);
}

void RendererFFMS2::RenderToDc(wxDC& dc)
{
	wxCriticalSectionLocker lock(m_MutexRendering);
	if (m_Width <= 0 || m_Height <= 0 || m_Pitch <= 0)
		return;
	std::lock_guard<std::mutex> pendingLock(m_LinuxPendingFrameMutex);
	const unsigned char* frame = !m_LinuxPresentFrame.empty() ? m_LinuxPresentFrame.data() : m_FrameBuffer;
	if (!frame)
		return;
	DrawBgraFrameWithWxDc(dc, frame, m_Width, m_Height, m_Pitch, m_BackBufferRect);
	++m_LinuxPresentedFrames;
	if (m_Visual && !m_HasZoom)
		m_Visual->DrawWx(dc, m_Time);
}

void RendererFFMS2::StartLinuxPlaybackThread()
{
	StopLinuxPlaybackThread();
	m_LinuxPresentedFrames.store(0);
	m_LinuxPlaybackStop.store(false);
	m_LinuxPlaybackThread = std::thread(&RendererFFMS2::LinuxPlaybackLoop, this);
}

void RendererFFMS2::StopLinuxPlaybackThread()
{
	m_LinuxPlaybackStop.store(true);
	if (m_LinuxPlaybackThread.joinable())
		m_LinuxPlaybackThread.join();
}

void RendererFFMS2::LinuxPlaybackLoop()
{
	if (!m_FFMS2 || m_Width <= 0 || m_Height <= 0 || m_Pitch <= 0)
		return;

	std::vector<unsigned char> frameBuffer(static_cast<size_t>(m_Height) * static_cast<size_t>(m_Pitch));
	const bool debugPlayback = std::getenv("KAINOTE_DEBUG_LINUX_PLAYBACK") != nullptr;
	unsigned int decodedFrames = 0;
	if (debugPlayback)
		std::fprintf(stderr, "[linux-playback] start time=%d frame=%d end=%d duration=%d\n", m_Time, m_Frame, m_PlayEndTime, GetDuration());
	int lastPresentedFrame = -1;
	int lastPresentedTime = -1;
	while (!m_LinuxPlaybackStop.load()) {
		int playTime = static_cast<int>(timeGetTime() - m_LastTime);
		if (playTime < 0)
			playTime = 0;
		if (playTime >= m_PlayEndTime || playTime >= GetDuration()) {
			wxCommandEvent* evt = new wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED, ID_END_OF_STREAM);
			wxQueueEvent(videoControl, evt);
			break;
		}

		const int seekFrom = (lastPresentedFrame >= 0 && playTime >= lastPresentedTime) ? lastPresentedFrame : 0;
		int nextFrame = m_FFMS2->GetFramefromMS(playTime, seekFrom, true);
		nextFrame = std::max(0, std::min(nextFrame, m_FFMS2->m_numFrames - 1));
		if (nextFrame != lastPresentedFrame) {
			m_Frame = nextFrame;
			m_Time = m_FFMS2->m_timecodes[m_Frame];
			m_FFMS2->GetFrame(m_Frame, frameBuffer.data());
			DrawTexture(frameBuffer.data(), true);
			++decodedFrames;
			if (debugPlayback && (decodedFrames <= 5 || (decodedFrames % 30) == 0))
				std::fprintf(stderr, "[linux-playback] decode=%u playTime=%d time=%d frame=%d\n", decodedFrames, playTime, m_Time, m_Frame);
			lastPresentedFrame = nextFrame;
			lastPresentedTime = playTime;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(4));
	}
	if (debugPlayback)
		std::fprintf(stderr, "[linux-playback] stop decoded=%u presented=%u time=%d frame=%d\n", decodedFrames, m_LinuxPresentedFrames.load(), m_Time, m_Frame);
}

#endif

RendererFFMS2::~RendererFFMS2()
{
	Stop();
#ifndef _WIN32
	StopLinuxPlaybackThread();
#endif

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

#ifndef _WIN32
	{
		unsigned char* fdata = nullptr;
		if (nframe) {
			fdata = nframe;
		}
		else {
			fdata = m_FrameBuffer;
			if (!fdata && m_FFMS2)
				m_FFMS2->GetFrameBuffer(&fdata);
			if (!fdata)
				return false;
		}

		m_SubsProvider->Draw(fdata, m_Time);
		if (nframe && m_FrameBuffer) {
			// Keep the software backbuffer in sync for wxGTK redraws after ASS rendering.
			memcpy(m_FrameBuffer, fdata, m_Height * m_Pitch);
		}
		if (!wxIsMainThread()) {
			{
				std::lock_guard<std::mutex> pendingLock(m_LinuxPendingFrameMutex);
				const size_t frameBytes = static_cast<size_t>(m_Height) * static_cast<size_t>(m_Pitch);
				if (m_LinuxPendingFrame.size() != frameBytes)
					m_LinuxPendingFrame.resize(frameBytes);
				memcpy(m_LinuxPendingFrame.data(), fdata, frameBytes);
			}
			QueueLinuxRender();
			return true;
		}
		PresentLinuxFrame(fdata);
		return true;
	}
#endif

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
#ifndef _WIN32
	{
		if (!wxIsMainThread()) {
			QueueLinuxRender();
			return;
		}
		if (redrawSubsOnFrame){
			{
				wxCriticalSectionLocker lock(m_MutexRendering);
				if (m_FrameBuffer && m_FFMS2 && m_Frame >= 0 && m_Frame < m_FFMS2->m_numFrames) {
					m_FFMS2->GetFrame(m_Frame, m_FrameBuffer);
				}
			}
			DrawTexture();
			return;
		}
		wxCriticalSectionLocker lock(m_MutexRendering);
		if (m_FrameBuffer) {
			PresentLinuxFrame(m_FrameBuffer);
		}
		m_VideoResized = false;
		return;
	}
#endif

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
				int i = 0;
				while (!InitDX()) {
					Sleep(500);
					if (i > 10) {
						//cannot render without initialized DX
						//return when device will be active again it should work after refresh.
						return;
					}
					i++;
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
		KainoteFrame::Get()->OpenAudioInTab(tab, 40000, fname);
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
			KainoteFrame::Get()->OpenAudioInTab(tab, 40000, fname);
			m_AudioPlayer = tab->edit->ABox->audioDisplay;
		}
		else if (m_AudioPlayer){ KainoteFrame::Get()->OpenAudioInTab(tab, GLOBAL_CLOSE_AUDIO, emptyString); }
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

#ifdef _WIN32
	if (!InitDX()){
		return false;
	}
#endif
	
	m_SubsProvider->SetVideoParameters(wxSize(m_Width, m_Height), RGB32, false);

	OpenSubs(subsFlag, false);
	
	m_State = Stopped;
	m_FFMS2->GetChapters(&m_Chapters);
#ifndef _WIN32
	// Prime wxGTK software backbuffer before fullscreen/first render.
	if (m_FrameBuffer && m_FFMS2 && m_FFMS2->m_numFrames > 0) {
		m_FFMS2->GetFrame(m_Frame, m_FrameBuffer);
	}
#endif

	if (m_Visual){
		m_Visual->SizeChanged(wxRect(m_BackBufferRect.left, m_BackBufferRect.top,
			m_BackBufferRect.right, m_BackBufferRect.bottom), m_D3DLine, m_D3DFont, m_D3DDevice);
	}
	return true;
}

bool RendererFFMS2::OpenSubs(int flag, bool redraw, wxString *text, bool resetParameters)
{
	bool result = false;
	{
		wxCriticalSectionLocker lock(m_MutexRendering);
		if (resetParameters)
			m_SubsProvider->SetVideoParameters(wxSize(m_Width, m_Height), m_Format, m_SwapFrame);

		result = m_SubsProvider->Open(tab, flag, text);
	}

#ifndef _WIN32
	if (result && redraw && m_State != None && m_FrameBuffer) {
		Render(true);
	}
#endif
	return result;
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
#ifdef _WIN32
	m_FFMS2->Play();
#else
	StartLinuxPlaybackThread();
#endif

	return true;
}


bool RendererFFMS2::Pause()
{
	if (m_State == Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		m_State = Paused;
#ifndef _WIN32
		StopLinuxPlaybackThread();
#endif
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
#ifndef _WIN32
		StopLinuxPlaybackThread();
#endif
		if (m_AudioPlayer){
			m_AudioPlayer->Stop();
			m_PlayEndTime = GetDuration();
		}
		m_Time = 0;
		return true;
	}
	return false;
}

void RendererFFMS2::SetPosition(int _time, bool starttime/*=true*/, bool corect/*=true*/, 
	bool async /*= true*/, bool refreshAudio/* = true*/)
{
	if (m_State == Playing || !async)
		SetFFMS2Position(_time, starttime, refreshAudio);
	else
		m_FFMS2->SetPosition(_time, starttime, refreshAudio);
}

//is from video thread make safe any deletion
void RendererFFMS2::SetFFMS2Position(int _time, bool starttime, bool refreshAudio/* = true*/){
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
		if (refreshAudio && m_AudioPlayer){ m_AudioPlayer->UpdateImage(false, false); }
		
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
	//+5ms to avoid times +27ms where + 17ms is near
	if (start){
		int prevFrameTime = m_FFMS2->GetMSfromFrame(m_Frame - 1);
		return m_Time + (((prevFrameTime - m_Time) / 2.f) + 5);
	}
	else{
		if (m_Frame + 1 >= m_FFMS2->m_numFrames){
			int prevFrameTime = m_FFMS2->GetMSfromFrame(m_Frame - 1);
			return m_Time + (((m_Time - prevFrameTime) / 2.f) + 5);
		}
		else{
			int nextFrameTime = m_FFMS2->GetMSfromFrame(m_Frame + 1);
			return m_Time + (((nextFrameTime - m_Time) / 2.f) + 5);
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
	//+5ms to avoid times +27ms where + 17ms is near
	if (start){
		int frameFromTime = m_FFMS2->GetFramefromMS(_time);
		int prevFrameTime = m_FFMS2->GetMSfromFrame(frameFromTime - 1);
		int frameTime = m_FFMS2->GetMSfromFrame(frameFromTime);
		return frameTime + ((prevFrameTime - frameTime) / 2.f) + 5;
	}
	else{
		int frameFromTime = m_FFMS2->GetFramefromMS(_time);
		int nextFrameTime = m_FFMS2->GetMSfromFrame(frameFromTime + 1);
		int frameTime = m_FFMS2->GetMSfromFrame(frameFromTime);
		return frameTime + ((nextFrameTime - frameTime) / 2.f) + 5;
	}
}

int RendererFFMS2::GetFrameTimeFromFrame(int frame, bool start)
{
	//+5ms to avoid times +27ms where + 17ms is near
	if (start){
		int prevFrameTime = m_FFMS2->GetMSfromFrame(frame - 1);
		int frameTime = m_FFMS2->GetMSfromFrame(frame);
		return frameTime + ((prevFrameTime - frameTime) / 2.f) + 5;
	}
	else{
		int nextFrameTime = m_FFMS2->GetMSfromFrame(frame + 1);
		int frameTime = m_FFMS2->GetMSfromFrame(frame);
		return frameTime + ((nextFrameTime - frameTime) / 2.f) + 5;
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
		m_FFMS2->GetFrame(m_Frame, cpy1);
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

