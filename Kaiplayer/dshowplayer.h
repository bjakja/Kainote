//  Copyright (c) 2016 - 2020, Marcin Drob

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

#include "VideoRenderer.h"

#include <wx/wx.h>
#include <dshow.h>
#include <qnetwork.h>

#include "DshowRenderer.h"


class DShowPlayer
{
public:

	DShowPlayer(wxWindow*_parent);
	~DShowPlayer();
	bool OpenFile(wxString fname, bool vobsub=false);
	void Play();
	void Pause();
	void Stop();
	void SetPosition(int pos);

	int GetPosition();
	int GetDuration();

	void SetVolume(long volume);
	long GetVolume();

	void GetFpsnRatio(float *fps, long *arx, long *ary);
	bool EnumFilters(Menu *menu);
	bool FilterConfig(wxString name, int idx, wxPoint pos);
	void GetChapters(std::vector<chapter> *chapters);
	wxSize GetVideoSize();
	void TearDownGraph();
	bool HasVobsub(){ return hasVobsub; }
	PlaybackState m_state;
	VideoInf inf;
	IMediaControl	*m_pControl;
	wxArrayString GetStreams();
	IAMStreamSelect *stream;
	IAMExtendedSeeking *chapters;
private:
	bool InitializeGraph();
	HWND hwndVid;			

	IGraphBuilder	*m_pGraph;
	IMediaSeeking	*m_pSeek;
	IBasicAudio		*m_pBA;
	//IBaseFilter		*frend;



	wxWindow *parent;
	bool hasVobsub;
};

