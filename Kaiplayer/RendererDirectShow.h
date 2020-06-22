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


class RendererDirectShow : public RendererVideo
{
	friend class RendererVideo;
	friend class VideoCtrl;
public:
	RendererDirectShow(VideoCtrl *control);
	virtual ~RendererDirectShow();

	bool OpenFile(const wxString &fname, wxString *textsubs, bool vobsub, bool changeAudio = true);
	bool OpenSubs(wxString *textsubs, bool redraw = true, bool fromFile = false);
	bool Play(int end = -1);
	bool Pause();
	bool Stop();
	void SetPosition(int _time, bool starttime = true, bool corect = true, bool async = true);
	int GetFrameTime(bool start = true);
	void GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd);
	int GetFrameTimeFromTime(int time, bool start = true);
	int GetFrameTimeFromFrame(int frame, bool start = true);
	int GetPlayEndTime(int time);
	int GetDuration();
	int GetVolume();
	void GetVideoSize(int *width, int *height);
	void GetFpsnRatio(float *fps, long *arx, long *ary);
	void SetVolume(int vol);

	void Render(bool RecreateFrame = true, bool wait = true);
	void RecreateSurface();
	void EnableStream(long index);
	void ChangePositionByFrame(int cpos);
	void ChangeVobsub(bool vobsub = false);
	wxArrayString GetStreams();
	byte *GetFramewithSubs(bool subs, bool *del);
	
	bool EnumFilters(Menu *menu);
	bool FilterConfig(wxString name, int idx, wxPoint pos);
	bool InitRendererDX();
	void OpenKeyframes(const wxString &filename);

	DShowPlayer *m_DirectShowPlayer;
	
};