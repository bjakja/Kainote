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

#include "DshowRenderer.h"


class DShowPlayer : public VideoPlayer
{
public:

	DShowPlayer(VideoCtrl* _parent);
	~DShowPlayer();
	bool OpenFile(wxString fname, bool vobsub=false);
	void Play();
	bool Pause();
	bool Stop();
	void SetPosition(int pos);

	int GetPosition();
	int GetDuration();

	void SetVolume(int volume);
	int GetVolume();

	void GetFpsnRatio(float *fps, long *arx, long *ary);
	bool EnumFilters(Menu *menu);
	bool FilterConfig(wxString name, int idx, wxPoint pos);
	void GetChapters(std::vector<chapter> *chapters);
	void GetVideoSize(int *width, int *height);
	wxSize GetVideoSize();
	wxArrayString GetStreams();
	void RecreateSurface();
	void TearDownGraph();
	VideoInf inf;
	IMediaControl *m_pControl;
	IAMStreamSelect *stream;
	IAMExtendedSeeking *chapters;
private:
	bool InitializeGraph();		

	IGraphBuilder	*m_pGraph;
	IMediaSeeking	*m_pSeek;
	IBasicAudio		*m_pBA;
	//IBaseFilter		*frend;

};

