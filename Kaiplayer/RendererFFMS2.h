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

#pragma once

#include "RendererVideo.h"
#include "VideoFfmpeg.h"

class RendererFFMS2 : public RendererVideo
{
	friend class RendererVideo;
	friend class VideoCtrl;
public:
	RendererFFMS2(VideoCtrl *control);
	~RendererFFMS2();

	bool OpenFile(const wxString &fname, wxString *textsubs, bool vobsub, bool changeAudio = true);
	bool OpenSubs(wxString *textsubs, bool redraw = true, bool fromFile = false);
	bool Play(int end = -1);
	bool PlayLine(int start, int end);
	bool Pause();
	bool Stop();
	void SetPosition(int _time, bool starttime = true, bool corect = true, bool async = true);
	void SetFFMS2Position(int time, bool starttime);
	void GoToNextKeyframe();
	void GoToPrevKeyframe();
	int GetCurrentPosition();
	int GetCurrentFrame();
	int GetFrameTime(bool start = true);
	void GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd);
	int GetFrameTimeFromTime(int time, bool start = true);
	int GetFrameTimeFromFrame(int frame, bool start = true);
	//if nothing loaded or loaded via Direct Show VFF is NULL
	//return true if VFF is present
	//bool GetStartEndDurationFromMS(Dialogue *dial, STime &duration);
	int GetPlayEndTime(int time);
	int GetDuration();
	int GetVolume();
	void GetVideoSize(int *width, int *height);
	void GetFpsnRatio(float *fps, long *arx, long *ary);
	void UpdateVideoWindow();
	void SetVolume(int vol);

	void Render(bool RecreateFrame = true, bool wait = true);
	void DrawLines(wxPoint point);
	void DrawProgBar();
	bool DrawTexture(byte *nframe = NULL, bool copy = false);
	//void RecreateSurface();
	void EnableStream(long index);
	void ChangePositionByFrame(int cpos);
	void ChangeVobsub(bool vobsub = false);
	wxArrayString GetStreams();
	void SetVisual(bool settext = false, bool noRefresh = false);
	void ResetVisual();
	//it's safe to not exist visual
	//returns true if removed
	bool RemoveVisual(bool noRefresh = false);
	byte *GetFramewithSubs(bool subs, bool *del);
	bool UpdateRects(bool changeZoom = true);
	void Zoom(const wxSize &size);
	void DrawZoom();
	void ZoomMouseHandle(wxMouseEvent &evt);
	void SetZoom();
	void ResetZoom();
	void SetVisualZoom();
	//int GetPreciseTime(bool start = true);
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
	VideoFfmpeg * GetFFMS2();
	//bool InitRendererDX(bool reset = false)
	VideoFfmpeg *m_FFMS2;
};