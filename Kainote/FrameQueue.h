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

#include <condition_variable>
#include <mutex>
#include <vector>

// Frames a decoder thread decodes ahead of playback. The decoder asks for
// the next frame to decode and a slot to put it in; the player takes the
// frame it wants, and every earlier frame is dropped and never decoded.
class FrameQueue
{
public:
	struct Result
	{
		// nullptr at the end of the video, on a failed frame or when stopped
		unsigned char *slot = nullptr;
		int frame = -1;
		bool failed = false;
	};

	FrameQueue(size_t slots, size_t frameBytes, int frameCount);

	// Decoder side: waits for a free slot and returns it with the frame to
	// decode into it, or nullptr once stopped.
	unsigned char *NextToDecode(int *frame);
	void Decoded(unsigned char *slot, bool ok);

	// Player side: waits for wanted, or the first later frame decoded.
	Result Take(int wanted);
	void Release(unsigned char *slot);

	// Decoding starts over at frame, as after a seek.
	void Reset(int frame);
	void Stop();

private:
	enum State { FREE, DECODING, READY, TAKEN };
	struct Slot
	{
		std::vector<unsigned char> data;
		int frame = -1;
		State state = FREE;
		unsigned generation = 0;
	};

	Slot *Find(unsigned char *data);

	std::mutex m_mutex;
	std::condition_variable m_changed;
	std::vector<Slot> m_slots;
	int m_frameCount;
	int m_next = 0;
	int m_wanted = 0;
	unsigned m_generation = 0;
	bool m_failed = false;
	bool m_stopped = false;
};
