//  Copyright (c) 2016-2026, Marcin Drob

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
#include <dsound.h>
#include <atomic>
#include <memory>
#include <mutex>

// Where the sound card is, for any thread. It outlives the player thread, so
// a reader never reaches a player the UI thread has since deleted.
class AudioPosition
{
public:
	// the frame audible now, or -1 when nothing plays
	long long Frame();
	int SampleRate() const { return sample_rate; }
	// from a seek until the new data is written, the position holds at frame
	void Restart(long long frame)
	{
		std::lock_guard<std::mutex> lock(mutex);
		restarting = true;
		start_frame = frame;
	}
private:
	friend class DirectSoundPlayer2Thread;
	std::mutex mutex;
	IDirectSoundBuffer8 *buffer = nullptr;
	long long start_frame = 0;
	long long written_frame = 0;
	unsigned long write_offset = 0;
	unsigned long buffer_bytes = 0;
	long long bytes_per_frame = 2;
	std::atomic<int> sample_rate{ 0 };
	bool playing = false;
	bool restarting = false;
	bool refilled = false;
};


class DirectSoundPlayer2Thread {
	static unsigned int __stdcall ThreadProc(void* parameter);
	void Run();

	unsigned int FillAndUnlockBuffers(unsigned char* buf1, unsigned int buf1sz, unsigned char* buf2,
		unsigned int buf2sz, long long& input_frame, IDirectSoundBuffer8* audioBuffer);

	void CheckError();
	void CloseHandles();

	HANDLE thread_handle = nullptr;

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

	// set by the UI thread, read by the playback thread
	std::atomic<double> volume{ 1.0 };
	std::atomic<long long> start_frame{ 0 };
	std::atomic<long long> end_frame{ 0 };

	int wanted_latency;
	int buffer_length;

	
	int last_playback_restart;

	std::shared_ptr<AudioPosition> position = std::make_shared<AudioPosition>();
	// started ends a restart; later fills leave a seek's pending restart alone
	void SetWritten(long long frame, unsigned long offset, bool refills, bool started);
	void SetStopped();

	Provider* provider;
#ifndef _WIN32
	struct LinuxAudioState;
	LinuxAudioState* linuxState = nullptr;
#endif
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
	const std::shared_ptr<AudioPosition> &Position() const { return position; }

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
	// null until a stream is open
	std::shared_ptr<AudioPosition> Position();
	

	Provider * provider;
	
};


