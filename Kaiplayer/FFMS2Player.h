//  Copyright (c) 2018, Marcin Drob

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

class FFMS2Player : public VideoPlayer
{
public:
	FFMS2Player(VideoCtrl *window);
	virtual ~FFMS2Player();
	bool OpenFile(const wxString &fname, wxString *textsubs, bool vobsub, bool changeAudio = true);
	bool OpenSubs(wxString *textsubs, bool redraw = true, bool fromFile = false);
	bool Play(int end = -1);
	bool Pause(bool skipWhenOnEnd = true);
	bool Stop();
	void SetPosition(int time, bool starttime = true, bool corect = true);
	void SetFFMS2Position(int time, bool starttime);
	void SetColorSpace(const wxString& matrix){
		VFF->SetColorSpace(matrix);
		if (vstate == Paused)
			Render();
	}
	void GoToNextKeyframe();
	void GoToPrevKeyframe();
	int GetFrameTime(bool start = true);
	void GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd);
	int GetFrameTimeFromTime(int time, bool start = true);
	int GetFrameTimeFromFrame(int frame, bool start = true);

	int GetPlayEndTime(int time);
	int GetDuration();
	int GetVolume();
	void GetVideoSize(int *width, int *height);
	wxSize GetVideoSize();
	void GetFpsnRatio(float *fps, long *arx, long *ary);
	void UpdateVideoWindow();
	void SetVolume(int vol);

	void Render(bool RecreateFrame = true);
	void ChangePositionByFrame(int cpos);
	void DeleteAudioCache(){ VFF->DeleteOldAudioCache(); }
	byte *GetFramewithSubs(bool subs, bool *del);
	
	VideoFfmpeg *VFF = NULL;
private:
	bool InitDX(bool reset = false);
	AudioDisplay *player = NULL;
};