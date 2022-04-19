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


#include "Provider.h"
//#include <dsound.h>


class DirectSoundPlayer2Thread {
	static unsigned int __stdcall ThreadProc(void* parameter);
	void Run();

	unsigned int FillAndUnlockBuffers(unsigned char* buf1, unsigned int buf1sz, unsigned char* buf2,
		unsigned int buf2sz, long long& input_frame, IDirectSoundBuffer8* audioBuffer);

	void CheckError();

	HANDLE thread_handle;

	// Used to signal state-changes to thread
	HANDLE
		event_start_playback,
		event_stop_playback,
		event_update_end_time,
		event_set_volume,
		event_kill_self;

	// Thread communicating back
	HANDLE
		thread_running,
		is_playing,
		error_happened;

	double volume =1.0;
	long long start_frame = 0;
	long long end_frame = 0;

	int wanted_latency;
	int buffer_length;

	//std::chrono::system_clock::time_point last_playback_restart;
	int last_playback_restart;

	Provider* provider;

public:
	DirectSoundPlayer2Thread(Provider* provider, int WantedLatency, int BufferLength);
	~DirectSoundPlayer2Thread();

	void Play(long long start, long long count);
	void Stop();
	void SetEndFrame(long long new_end_frame);
	void SetVolume(double new_volume);

	bool IsPlaying();
	long long GetStartFrame();
	long long GetCurrentFrame();
	int GetCurrentMS();
	long long GetEndFrame();
	double GetVolume();
	bool IsDead();

};

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
	

	Provider * provider;
	
};


