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

#include "Playback.h"

// How late a frame may be before playback skips ahead instead.
static const int LATE_FRAME_MARGIN = 20;

int SeekFrame(const Timebase &timebase, int ms, bool startTime)
{
	return timebase.ClampFrame(startTime ? timebase.FrameAt(ms) : timebase.FrameShownAt(ms - 1));
}

void PendingSeek::Post(const SeekRequest &request)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_request = request;
	m_pending = true;
}

bool PendingSeek::Take(SeekRequest *request)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_pending)
		return false;
	*request = m_request;
	m_pending = false;
	return true;
}

bool PlaybackReachedEnd(int frame, int ms, int playEndMs, int frameCount)
{
	return (playEndMs > 0 && ms >= playEndMs) || frame >= frameCount - 1;
}

PlaybackStep NextPlaybackFrame(const Timebase &timebase, int frame, int nowMs,
	int playEndMs, int frameCount)
{
	int next = frame + 1;
	int early = timebase.MsAt(next) - nowMs;
	if (early > 0)
		return { next, early };

	if (early < -LATE_FRAME_MARGIN) {
		while (next < frameCount) {
			int frameTime = timebase.MsAt(next);
			if (frameTime >= nowMs || (playEndMs > 0 && frameTime >= playEndMs))
				break;
			next++;
		}
		if (next >= frameCount)
			next = frameCount - 1;
	}
	return { next, 0 };
}
