//  Copyright (c) 2026, Marcin Drob

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

#include "Timebase.h"
#include <mutex>

enum SeekFlags
{
	// land on the exact time instead of a frame
	SEEK_NO_SNAP = 1,
	// seek before returning, even while the playback thread plays
	SEEK_WAIT = 2,
	// leave the audio display where it is
	SEEK_KEEP_AUDIO = 4,
};

// Where a seek to ms lands: the first frame a line starting at ms shows on,
// or the last frame a line ending at ms shows on.
int SeekFrame(const Timebase &timebase, int ms, bool startTime);

struct SeekRequest
{
	int time = 0;
	bool startTime = true;
	bool refreshAudio = true;
};

// Seeks asked for while the playback thread plays. The latest one wins, and
// its time and flags are taken together.
class PendingSeek
{
public:
	void Post(const SeekRequest &request);
	bool Take(SeekRequest *request);
private:
	std::mutex m_mutex;
	SeekRequest m_request;
	bool m_pending = false;
};

// A play end of 0 plays to the end of the video.
bool PlaybackReachedEnd(int frame, int ms, int playEndMs, int frameCount);

struct PlaybackStep
{
	int frame;
	int sleepMs;
};

// The frame to show after frame once nowMs of the video should have played:
// the next one after a sleep, or a later one when playback fell behind.
PlaybackStep NextPlaybackFrame(const Timebase &timebase, int frame, int nowMs,
	int playEndMs, int frameCount);
