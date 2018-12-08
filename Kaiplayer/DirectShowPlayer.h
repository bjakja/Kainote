//  Copyright (c) 2016-2018, Marcin Drob

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

#include "VideoPlayer.h"

#include <wx/wx.h>
#include <dshow.h>
#include <qnetwork.h>

#include "DirectShowRenderer.h"


class DirectShowPlayer : public VideoPlayer
{
public:

	DirectShowPlayer(VideoCtrl* _parent);
	virtual ~DirectShowPlayer();
	bool OpenFile(wxString fname, bool vobsub=false);
	bool OpenFile(const wxString &fname, wxString *textsubs, bool vobsub, bool changeAudio);
	bool OpenSubs(wxString *textsubs, bool redraw = true, bool fromFile = false);
	void Render(bool RecreateFrame = true);
	bool Play(int end = -1);
	bool Pause(bool skipWhenOnEnd = true);
	bool Stop();
	void SetPosition(int _time, bool starttime = true, bool corect =true);
	void ChangePositionByFrame(int step);
	int GetPosition();
	int GetDuration();

	void SetVolume(int volume);
	int GetVolume();

	void GetFpsnRatio(float *fps, long *arx, long *ary);
	int GetFrameTime(bool start = true);
	void GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd);
	int GetFrameTimeFromTime(int time, bool start = true);
	int GetFrameTimeFromFrame(int frame, bool start = true);
	int GetPlayEndTime(int time);

	bool EnumFilters(Menu *menu);
	bool FilterConfig(wxString name, int idx, wxPoint pos);
	void GetChapters(std::vector<chapter> *chapters);
	void GetVideoSize(int *width, int *height);
	wxSize GetVideoSize();
	wxArrayString GetStreams();
	void EnableStream(long index);
	void ChangeVobsub(bool vobsub);
	void RecreateSurface();
	byte *DirectShowPlayer::GetFramewithSubs(bool subs, bool *del);
	void TearDownGraph();
	VideoInfo videoInfo;
	IMediaControl *m_pControl;
	IAMStreamSelect *stream;
	IAMExtendedSeeking *chaptersControl;
private:
	bool InitializeGraph();	
	bool InitDX(bool reset = false);

	IGraphBuilder	*m_pGraph;
	IMediaSeeking	*m_pSeek;
	IBasicAudio		*m_pBA;
	//IBaseFilter		*frend;

};

