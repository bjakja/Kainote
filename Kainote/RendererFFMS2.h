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
	bool Play(int end = -1);
	bool Pause();
	bool Stop();
	void SetPosition(int _time, bool starttime = true, bool corect = true, bool async = true, bool refreshAudio = true) override;
	void SetFFMS2Position(int time, bool starttime, bool refreshAudio = true) override;
	void GoToNextKeyframe();
	void GoToPrevKeyframe();
	int GetFrameTime(bool start = true);
	void GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd);
	int GetFrameTimeFromTime(int time, bool start = true);
	int GetFrameTimeFromFrame(int frame, bool start = true);
	//if nothing loaded or loaded via Direct Show VFF is nullptr
	//return true if VFF is present
	//bool GetStartEndDurationFromMS(Dialogue *dial, SubsTime &duration);
	int GetPlayEndTime(int time);
	int GetDuration();
	int GetVolume();
	void GetVideoSize(int *width, int *height);
	void GetFpsnRatio(float *fps, long *arx, long *ary);
	void SetVolume(int vol);
	bool DrawTexture(unsigned char * nframe = nullptr, bool copy = false);
#ifndef _WIN32
	void RenderToDc(wxDC& dc);
#endif
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
			m_FFMS2->SetColorSpace(matrix);
			if (m_State == Paused)
				Render();
		}
	}
	void OpenKeyframes(const wxString &filename);
	bool HasFFMS2();
	bool InitRendererDX();
	Provider* GetFFMS2();
	Provider *m_FFMS2 = nullptr;
#ifndef _WIN32
	std::atomic_bool m_LinuxRenderQueued{ false };
	std::mutex m_LinuxPendingFrameMutex;
	std::vector<unsigned char> m_LinuxPendingFrame;
	std::vector<unsigned char> m_LinuxPresentFrame;
	std::thread m_LinuxPlaybackThread;
	std::atomic_bool m_LinuxPlaybackStop{ false };
	std::atomic_uint m_LinuxPresentedFrames{ 0 };
#endif
protected:
	void DestroyFFMS2();
#ifndef _WIN32
	void QueueLinuxRender();
	void PresentLinuxFrame(const unsigned char* frame);
	void StartLinuxPlaybackThread();
	void StopLinuxPlaybackThread();
	void LinuxPlaybackLoop();
#endif
};
