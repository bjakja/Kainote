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

RendererFFMS2::RendererFFMS2(VideoBox *control, bool visualDisabled)
	: RendererVideo(control, visualDisabled)
	, m_FFMS2(nullptr)
{
	
}

#ifndef _WIN32
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
		std::fprintf(stderr, "[linux-playback] start time=%d frame=%d end=%d duration=%d\n", m_Time.load(), m_Frame.load(), m_PlayEndTime.load(), GetDuration());
	int lastPresentedFrame = -1;
	while (!m_LinuxPlaybackStop.load()) {
		int playTime = static_cast<int>(timeGetTime() - m_LastTime);
		if (playTime < 0)
			playTime = 0;
		if ((m_PlayEndTime > 0 && playTime >= m_PlayEndTime) || playTime >= GetDuration()) {
			wxCommandEvent* evt = new wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED, ID_END_OF_STREAM);
			wxQueueEvent(videoControl, evt);
			break;
		}

		const Timebase &timebase = GetTimebase();
		int nextFrame = timebase.ClampFrame(timebase.FrameAt(playTime));
		if (nextFrame != lastPresentedFrame) {
			m_Frame = nextFrame;
			m_Time = timebase.MsAt(m_Frame);
			m_FFMS2->GetFrame(m_Frame, frameBuffer.data());
			DrawTexture(frameBuffer.data(), true);
			++decodedFrames;
			if (debugPlayback && (decodedFrames <= 5 || (decodedFrames % 30) == 0))
				std::fprintf(stderr, "[linux-playback] decode=%u playTime=%d time=%d frame=%d\n", decodedFrames, playTime, m_Time.load(), m_Frame.load());
			lastPresentedFrame = nextFrame;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(4));
	}
	if (debugPlayback)
		std::fprintf(stderr, "[linux-playback] stop decoded=%u presented=%u time=%d frame=%d\n", decodedFrames, m_LinuxPresentedFrames.load(), m_Time.load(), m_Frame.load());
}

#endif

RendererFFMS2::~RendererFFMS2()
{
#ifndef _WIN32
	if (m_LinuxAlive) m_LinuxAlive->store(false); // stop any queued present touching us
#endif
	Stop();
#ifndef _WIN32
	StopLinuxPlaybackThread();
#endif

	m_State = None;
	SAFE_DELETE(m_FFMS2);
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
				m_FFMS2->GetFrameBuffer(m_Frame, &fdata);
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

	bool overlay = UsesOverlay();
	if (nframe) {
		fdata = nframe;
		if (copy) {
			byte *cpy = m_FrameBuffer;
			memcpy(cpy, fdata, FrameBytes());
		}
	}
	else if (overlay && m_UploadedFrame == m_Frame) {
		// only the subtitles can have changed, as while dragging a visual tool
		UpdateOverlay();
		return true;
	}
	else {
		fdata = m_FrameBuffer;
		m_FFMS2->GetFrameBuffer(m_Frame, &fdata);
		if (!fdata)
			return false;
	}

	if (overlay)
		UpdateOverlay();
	else
		m_SubsProvider->Draw(fdata, m_Time);
	IDirect3DSurface9 *upload = m_UploadSurfaces[m_UploadIndex];
	if (!upload)
		return false;
	HR(upload->LockRect(&d3dlr, 0, D3DLOCK_NOSYSLOCK), _("Cannot lock texture buffer"));
	texbuf = static_cast<unsigned char*>(d3dlr.pBits);

	diff = d3dlr.Pitch - (m_Width*bytes);
	if (m_Nv12) {
		// the chroma plane of an NV12 surface starts right below the luma plane
		for (int y = 0; y < m_Height + m_Height / 2; y++)
			memcpy(texbuf + (size_t)y * d3dlr.Pitch, fdata + (size_t)y * m_Width, m_Width);
	}
	else if (m_SwapFrame) {
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

	HR(upload->UnlockRect(), _("Cannot unlock texture buffer"));
	upload->AddRef();
	SAFE_RELEASE(m_MainSurface);
	m_MainSurface = upload;
	m_UploadIndex ^= 1;
	m_UploadedFrame = overlay ? (int)m_Frame : -1;

	return true;
}

bool RendererFFMS2::UsesOverlay()
{
	return m_OverlayTexture && (m_Nv12 || m_SubsProvider->IsLibass());
}

// call with m_MutexRendering locked
void RendererFFMS2::UpdateOverlay()
{
	size_t bytes = (size_t)m_Width * m_Height * 4;
	if (m_Overlay.size() != bytes) {
		m_Overlay.assign(bytes, 0);
		m_OverlayUploadAll = true;
	}
	wxRect dirty;
	bool changed = m_SubsProvider->DrawOverlay(m_Overlay.data(), m_Time, &dirty);
	if (m_OverlayUploadAll) {
		dirty = wxRect(0, 0, m_Width, m_Height);
		changed = true;
		m_OverlayUploadAll = false;
	}
	dirty.Intersect(wxRect(0, 0, m_Width, m_Height));
	if (!changed || dirty.IsEmpty())
		return;

	RECT rect = { dirty.x, dirty.y, dirty.x + dirty.width, dirty.y + dirty.height };
	D3DLOCKED_RECT locked;
	if (FAILED(m_OverlayStaging->LockRect(0, &locked, &rect, 0))) {
		m_OverlayUploadAll = true;
		return;
	}
	int pitch = m_Width * 4;
	const unsigned char *src = m_Overlay.data() + dirty.y * pitch + dirty.x * 4;
	unsigned char *dst = static_cast<unsigned char*>(locked.pBits);
	for (int y = 0; y < dirty.height; y++)
		memcpy(dst + y * locked.Pitch, src + y * pitch, dirty.width * 4);
	m_OverlayStaging->UnlockRect(0);
	if (FAILED(m_D3DDevice->UpdateTexture(m_OverlayStaging, m_OverlayTexture)))
		m_OverlayUploadAll = true;
}

// call inside BeginScene with m_MutexRendering locked
void RendererFFMS2::DrawOverlay()
{
	struct OverlayVertex { float x, y, z, u, v; };
	float u0 = (float)m_MainStreamRect.left / m_Width, u1 = (float)m_MainStreamRect.right / m_Width;
	float v0 = (float)m_MainStreamRect.top / m_Height, v1 = (float)m_MainStreamRect.bottom / m_Height;
	float x0 = m_BackBufferRect.left, x1 = m_BackBufferRect.right;
	float y0 = m_BackBufferRect.top, y1 = m_BackBufferRect.bottom;
	OverlayVertex quad[4] = {
		{ x0, y0, 0.f, u0, v0 }, { x1, y0, 0.f, u1, v0 },
		{ x0, y1, 0.f, u0, v1 }, { x1, y1, 0.f, u1, v1 },
	};
	m_D3DDevice->SetTexture(0, m_OverlayTexture);
	m_D3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
	m_D3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_D3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_D3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	m_D3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	// libass draws it premultiplied, VSFilter with straight alpha
	if (m_SubsProvider->IsLibass())
		m_D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	m_D3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(OverlayVertex));
	m_D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_D3DDevice->SetTexture(0, nullptr);
	m_D3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
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
		if (m_D3DDevice)
			hr = m_D3DDevice->TestCooperativeLevel();
		if (m_D3DDevice && FAILED(hr) && D3DERR_DEVICENOTRESET != hr)
			return;
		if (!m_D3DDevice || FAILED(hr))
		{
			{
				Clear();
				// without a device SizeChanged crashes, so try again at the next render
				if (!InitDX())
					return;
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

	
	if (m_Nv12)
		BlitNv12();
	else {
		hr = m_D3DDevice->StretchRect(m_MainSurface, &m_MainStreamRect, m_BlackBarsSurface, &m_BackBufferRect, D3DTEXF_LINEAR);
		if (FAILED(hr)){ KaiLog(_("Cannot overlay surfaces")); }
	}


	hr = m_D3DDevice->BeginScene();

	if (UsesOverlay())
		DrawOverlay();

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
	hr = m_D3DDevice->Present(&m_WindowRect, &m_WindowRect, nullptr, nullptr);
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
	m_Nv12 = m_FFMS2->IsNv12();
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
	videoControl->SetVideoTimebase(m_FFMS2->TakeTimebase());
	
	diff = 0;
	m_FrameDuration = (1000.0f / videoControl->m_FPS);
	if (videoControl->m_AspectRatioY == 0 || videoControl->m_AspectRatioX == 0){ videoControl->m_AspectRatio = 0.0f; }
	else{ videoControl->m_AspectRatio = (float)videoControl->m_AspectRatioY / (float)videoControl->m_AspectRatioX; }

	m_MainStreamRect.bottom = m_Height;
	m_MainStreamRect.right = m_Width;
	m_MainStreamRect.left = 0;
	m_MainStreamRect.top = 0;
	if (m_FrameBuffer){ delete[] m_FrameBuffer; m_FrameBuffer = nullptr; }
	m_FrameBuffer = new byte[FrameBytes()];

	UpdateRects();

#ifdef _WIN32
	if (!InitDX()){
		return false;
	}
#endif

	// NV12 video always shows subtitles in the overlay, which has alpha
	m_SubsProvider->SetVideoParameters(wxSize(m_Width, m_Height), m_Nv12 ? ARGB32 : RGB32, false);

	OpenSubs(subsFlag, false);
	
	m_State = Paused;
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
			m_SubsProvider->SetVideoParameters(wxSize(m_Width, m_Height), m_Nv12 ? ARGB32 : m_Format, m_SwapFrame);

		result = m_SubsProvider->Open(flag, SubtitlesText(flag, text));
	}

#ifndef _WIN32
	if (result && redraw && m_State != None && m_FrameBuffer) {
		Render(true);
	}
#endif
	return result;
}

void RendererFFMS2::StartStream()
{
	m_Time = GetTimebase().MsAt(m_Frame);
	m_LastTime = timeGetTime() - m_Time;
	if (m_AudioPlayer){ m_AudioPlayer->Play(m_Time, -1, false); }
	SetAudioPosition((m_AudioPlayer && m_AudioPlayer->player) ? m_AudioPlayer->player->Position() : nullptr);
#ifdef _WIN32
	m_FFMS2->Play();
#else
	StartLinuxPlaybackThread();
#endif
}

void RendererFFMS2::PauseStream()
{
#ifndef _WIN32
	StopLinuxPlaybackThread();
#endif
	if (m_AudioPlayer){ m_AudioPlayer->Stop(false); }
	SetAudioPosition(nullptr);
}

void RendererFFMS2::StopStream()
{
#ifndef _WIN32
	StopLinuxPlaybackThread();
#endif
	if (m_AudioPlayer){ m_AudioPlayer->Stop(); }
	SetAudioPosition(nullptr);
}

void RendererFFMS2::SetPosition(int time, bool startTime, int flags)
{
	bool refreshAudio = !(flags & SEEK_KEEP_AUDIO);
#ifndef _WIN32
	// Linux playback decodes on m_LinuxPlaybackThread, while subtitle reopening
	// reads the grid and edit controls. Finish any in-flight decode before the
	// main thread seeks, then resume from the new clock position.
	wxASSERT_MSG(wxIsMainThread(), "Linux video seeks must run on the wx main thread");
	const bool wasPlaying = m_State == Playing;
	if (wasPlaying)
		StopLinuxPlaybackThread();
	SetFFMS2Position(time, startTime, refreshAudio);
	if (wasPlaying)
		StartLinuxPlaybackThread();
#else
	//while playing, the playback thread owns the frame and seeks itself
	if ((flags & SEEK_WAIT) && m_State != Playing)
		SetFFMS2Position(time, startTime, refreshAudio);
	else
		m_FFMS2->SetPosition(time, startTime, refreshAudio);
#endif
}

// Windows calls this on the provider thread; Linux calls it on the main thread
// after joining the frame decoder.
void RendererFFMS2::SetFFMS2Position(int time, bool startTime, bool refreshAudio/* = true*/){
	bool playing = m_State == Playing;
	const Timebase &timebase = GetTimebase();
	m_Frame = SeekFrame(timebase, time, startTime);
	m_Time = timebase.MsAt(m_Frame);
	m_LastTime = timeGetTime() - m_Time;
	m_PlayEndTime = 0;

	// the audio itself seeks on the UI thread; until then the clock holds here
	std::shared_ptr<AudioPosition> position = playing ? GetAudioPosition() : nullptr;
	if (position)
		position->Restart((long long)m_Time * position->SampleRate() / 1000);
	// decode here, off the UI thread, so only copying is left for it
	if (!playing)
		m_FFMS2->PrefetchFrame(m_Frame);

	QueueSeekRefresh(playing, refreshAudio);
}

int RendererFFMS2::GetDuration()
{
	return m_FFMS2 ? m_FFMS2->m_duration * 1000.0 : 0;
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
		m_Time = GetTimebase().MsAt(m_Frame);
		if (m_HasVisualEdition || !m_SubsProvider->ShowsWholeSubtitles()){
			OpenSubs(OPEN_WHOLE_SUBTITLES, false);
			m_HasVisualEdition = false;
		}
		if (m_AudioPlayer){ m_AudioPlayer->UpdateImage(true, true); }
		Render(true, false);
	
	
	videoControl->RefreshTime();

}

byte *RendererFFMS2::GetFrameWithSubs(bool subs, bool *del)
{
	int all = m_Height * m_Pitch;
	if (subs && !UsesOverlay()){
		*del = false;
		return m_FrameBuffer;
	}
	*del = true;
	byte *cpy = new byte[all];
	if (subs && m_Nv12){
		// the frame is NV12; the provider gives it as BGRA
		m_FFMS2->GetFrame(m_Frame, cpy);
		wxCriticalSectionLocker lock(m_MutexRendering);
		m_SubsProvider->Draw(cpy, m_Time);
	}
	else if (subs){
		// the subtitles are in the overlay, not in the frame
		wxCriticalSectionLocker lock(m_MutexRendering);
		memcpy(cpy, m_FrameBuffer, all);
		m_SubsProvider->Draw(cpy, m_Time);
	}
	else{
		m_FFMS2->GetFrame(m_Frame, cpy);
	}
	return cpy;
}

unsigned char* RendererFFMS2::GetFrame(int frame, bool subs)
{
	int all = m_Height * m_Pitch;
	byte* newFrame = new byte[all];
	m_FFMS2->GetFrame(frame, newFrame);
	if (subs) {
		m_SubsProvider->Draw(newFrame, GetTimebase().MsAt(frame));
	}
	return newFrame;
}

Provider* RendererFFMS2::GetFFMS2()
{
	return m_FFMS2;
}


bool RendererFFMS2::InitRendererDX()
{
#ifndef byvertices
	HR(m_D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_BlackBarsSurface), _("Cannot create surface"));

	if (FAILED(m_D3DDevice->CreateTexture(m_Width, m_Height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &m_OverlayStaging, nullptr)) ||
		FAILED(m_D3DDevice->CreateTexture(m_Width, m_Height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_OverlayTexture, nullptr))) {
		// without the overlay, subtitles are drawn into the frame
		SAFE_RELEASE(m_OverlayStaging);
		SAFE_RELEASE(m_OverlayTexture);
	}
	m_OverlayUploadAll = true;

	// NV12 needs the overlay for subtitles and a DXVA2 processor; a video
	// just opened can still go back to BGRA frames, a playing one cannot
	if (m_Nv12 && (!m_OverlayTexture || !InitNv12())) {
		if (m_State != None)
			return false;
		m_Nv12 = false;
		m_FFMS2->UseRgbOutput();
		delete[] m_FrameBuffer;
		m_FrameBuffer = new byte[FrameBytes()];
	}
	if (!m_Nv12) {
		for (IDirect3DSurface9 *&surface : m_UploadSurfaces) {
			HR(m_D3DDevice->CreateOffscreenPlainSurface(m_Width, m_Height, m_D3DFormat, D3DPOOL_DEFAULT, &surface, 0),
				_("Cannot create plain surface"));
		}
	}
	m_UploadIndex = 0;
	m_MainSurface = m_UploadSurfaces[1];
	m_MainSurface->AddRef();
	m_UploadedFrame = -1;

#endif
	return true;
}

bool RendererFFMS2::InitNv12()
{
#ifndef _WIN32
	return false;
#else
	const D3DFORMAT nv12 = (D3DFORMAT)MAKEFOURCC('N', 'V', '1', '2');
	if (FAILED(DXVA2CreateVideoService(m_D3DDevice, __uuidof(IDirectXVideoProcessorService), (void**)&m_DXVAService)))
		return false;
	DXVA2_VideoDesc desc = {};
	desc.SampleWidth = m_Width;
	desc.SampleHeight = m_Height;
	desc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	desc.SampleFormat.NominalRange = DXVA2_NominalRange_16_235;
	desc.SampleFormat.VideoTransferMatrix = DXVA2_VideoTransferMatrix_BT709;
	desc.SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
	desc.Format = nv12;
	desc.InputSampleFreq.Numerator = desc.OutputFrameFreq.Numerator = 60;
	desc.InputSampleFreq.Denominator = desc.OutputFrameFreq.Denominator = 1;

	UINT count = 0;
	GUID *guids = nullptr;
	if (FAILED(m_DXVAService->GetVideoProcessorDeviceGuids(&desc, &count, &guids)))
		return false;
	for (UINT i = 0; i < count && !m_DXVAProcessor; i++) {
		DXVA2_VideoProcessorCaps caps;
		if (FAILED(m_DXVAService->GetVideoProcessorCaps(guids[i], &desc, D3DFMT_X8R8G8B8, &caps)) ||
			caps.NumForwardRefSamples > 0 || caps.NumBackwardRefSamples > 0 ||
			!(caps.VideoProcessorOperations & DXVA2_VideoProcess_YUV2RGBExtended))
			continue;
		m_DXVAService->CreateVideoProcessor(guids[i], &desc, D3DFMT_X8R8G8B8, 0, &m_DXVAProcessor);
	}
	CoTaskMemFree(guids);
	if (!m_DXVAProcessor)
		return false;
	for (IDirect3DSurface9 *&surface : m_UploadSurfaces) {
		if (FAILED(m_DXVAService->CreateSurface(m_Width, m_Height, 0, nv12, D3DPOOL_DEFAULT, 0,
			DXVA2_VideoSoftwareRenderTarget, &surface, nullptr))) {
			SAFE_RELEASE(m_UploadSurfaces[0]);
			SAFE_RELEASE(m_UploadSurfaces[1]);
			SAFE_RELEASE(m_DXVAProcessor);
			return false;
		}
	}
	return true;
#endif
}

// call with m_MutexRendering locked
void RendererFFMS2::BlitNv12()
{
#ifdef _WIN32
	DXVA2_ExtendedFormat source = {};
	source.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	source.NominalRange = m_FFMS2->YuvFullRange() ? DXVA2_NominalRange_0_255 : DXVA2_NominalRange_16_235;
	source.VideoTransferMatrix = m_FFMS2->YuvMatrix();
	source.SampleFormat = DXVA2_SampleProgressiveFrame;
	DXVA2_ExtendedFormat target = source;
	target.NominalRange = DXVA2_NominalRange_0_255;

	DXVA2_VideoProcessBltParams blt = {};
	blt.TargetFrame = (LONGLONG)m_Time * 10000;
	blt.TargetRect = m_WindowRect;
	blt.ConstrictionSize.cx = m_WindowRect.right - m_WindowRect.left;
	blt.ConstrictionSize.cy = m_WindowRect.bottom - m_WindowRect.top;
	// black: luma 16 and neutral chroma, in 16-bit steps
	blt.BackgroundColor.Y = 0x1000;
	blt.BackgroundColor.Cb = blt.BackgroundColor.Cr = 0x8000;
	blt.BackgroundColor.Alpha = 0xFFFF;
	blt.DestFormat = target;
	blt.ProcAmpValues.Brightness.ll = 0;
	blt.ProcAmpValues.Contrast.ll = 0x10000;
	blt.ProcAmpValues.Hue.ll = 0;
	blt.ProcAmpValues.Saturation.ll = 0x10000;
	blt.Alpha = DXVA2_Fixed32OpaqueAlpha();

	DXVA2_VideoSample sample = {};
	sample.Start = blt.TargetFrame;
	sample.End = sample.Start + 170000;
	sample.SampleFormat = source;
	sample.SrcSurface = m_MainSurface;
	sample.SrcRect = m_MainStreamRect;
	sample.DstRect = m_BackBufferRect;
	sample.PlanarAlpha = DXVA2_Fixed32OpaqueAlpha();
	if (FAILED(m_DXVAProcessor->VideoProcessBlt(m_BlackBarsSurface, &blt, &sample, 1, nullptr)))
		KaiLog(_("Cannot overlay surfaces"));
#endif
}

void RendererFFMS2::ClearObject()
{
	SAFE_RELEASE(m_OverlayStaging);
	SAFE_RELEASE(m_OverlayTexture);
	SAFE_RELEASE(m_UploadSurfaces[0]);
	SAFE_RELEASE(m_UploadSurfaces[1]);
}
