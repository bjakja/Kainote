//  Copyright (c) 2016-2022, Marcin Drob

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

//this code piervously was taken from Aegisub 2 it's rewritten by me almost all.

#pragma once

//#include <wx/wxprec.h>
//#include <stdint.h>
//#include <mmsystem.h>
//
#include "Provider.h"


class DirectSoundPlayer2Thread;

class DirectSoundPlayer2 : public wxEvtHandler {
	DirectSoundPlayer2Thread *thread;

protected:
	int WantedLatency;
	int BufferLength;

	bool IsThreadAlive();

public:
	DirectSoundPlayer2();
	virtual ~DirectSoundPlayer2();

	void OpenStream();
	void CloseStream();
	void SetProvider(Provider *_provider);

	void Play(long long start, long long count);
	void Stop(bool timerToo = true);
	bool IsPlaying();

	long long GetStartPosition();
	long long GetEndPosition();
	long long GetCurrentPosition();
	int GetCurPositionMS();
	void SetEndPosition(long long pos);
	void SetCurrentPosition(long long pos);

	void SetVolume(double vol);
	double GetVolume();
	//void SetDisplayTimer(wxTimer *Timer);

	Provider * provider;
	//wxTimer *displayTimer;
};


