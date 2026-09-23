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

#pragma once

#include "RendererVideo.h"
#include "Provider.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "UndoD3DXMacros.h"
#ifndef _WIN32
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
class wxDC;
#endif

class RendererFFMS2 : public RendererVideo
{
	friend class RendererVideo;
	friend class VideoBox;
	friend class Provider;
public:
	RendererFFMS2(VideoBox *control, bool visualDisabled);
	virtual ~RendererFFMS2();

	bool OpenFile(const wxString &fname, int subsFlag, bool vobsub, bool changeAudio = true);
	bool OpenSubs(int flag, bool redraw = true, wxString *text = nullptr, bool resetParameters = false);
	void SetPosition(int time, bool startTime = true, int flags = 0) override;
	// Seek implementation: provider thread on Windows, main thread after
	// stopping the frame decoder on Linux.
	void SetFFMS2Position(int time, bool startTime, bool refreshAudio = true);
	int GetCurrentFrame() override { return m_Frame; }
	//if nothing loaded or loaded via Direct Show VFF is nullptr
	//return true if VFF is present
	//bool GetStartEndDurationFromMS(Dialogue *dial, SubsTime &duration);
	int GetDuration();
	int GetVolume();
	void GetVideoSize(int *width, int *height);
	void GetFpsnRatio(float *fps, long *arx, long *ary);
	void SetVolume(int vol);
	bool DrawTexture(unsigned char * nframe = nullptr, bool copy = false);
	void Render(bool RecreateFrame = true, bool wait = true);
	void ChangePositionByFrame(int cpos);
	//it's safe to not exist visual
	//returns true if removed
	//bool RemoveVisual(bool noRefresh = false);
	unsigned char * GetFrameWithSubs(bool subs, bool *del) override;
	unsigned char* GetFrame(int frame, bool subs) override;
	void DeleteAudioCache(){ if (m_FFMS2){ m_FFMS2->DeleteOldAudioCache(); } }
	void SetColorSpace(const wxString& matrix, bool render = true){
		if (m_FFMS2){
			m_UploadedFrame = -1;
			m_FFMS2->SetColorSpace(matrix);
			if (m_State == Paused)
				Render();
		}
	}
	bool InitRendererDX();
	void ClearObject() override;
	Provider* GetFFMS2();
	Provider *m_FFMS2 = nullptr;
	// frames alternate between these, so the one the GPU may still be
	// reading is never locked; m_MainSurface holds the last one filled
	IDirect3DSurface9 *m_UploadSurfaces[2] = { nullptr, nullptr };
	int m_UploadIndex = 0;
	// libass subtitles live in their own overlay, drawn over the video on the
	// GPU and redrawn only when they change; the frame then stays clean
	bool UsesOverlay();
	void UpdateOverlay();
	void DrawOverlay();
	std::vector<unsigned char> m_Overlay;
	IDirect3DTexture9 *m_OverlayStaging = nullptr;
	IDirect3DTexture9 *m_OverlayTexture = nullptr;
	bool m_OverlayUploadAll = true;
	// the frame in m_MainSurface when it holds no subtitles, else -1
	int m_UploadedFrame = -1;
#ifndef _WIN32
	// Frame-accurate playback driver; the shared software present (buffers,
	// QueueLinuxRender/PresentLinuxFrame/RenderToDc) lives in RendererVideo.
	std::thread m_LinuxPlaybackThread;
	std::atomic_bool m_LinuxPlaybackStop{ false };
#endif
protected:
	void StartStream() override;
	void PauseStream() override;
	void StopStream() override;
#ifndef _WIN32
	void StartLinuxPlaybackThread();
	void StopLinuxPlaybackThread();
	void LinuxPlaybackLoop();
#endif
};
