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

#include <vector>

// Maps times in milliseconds to video frames and back, from either the
// video's own timecodes or a constant frame rate.
class Timebase
{
public:
	Timebase() = default;
	static Timebase FromTimecodes(std::vector<int> timecodes, float fps);
	static Timebase FromFps(float fps, int frameCount);

	bool IsEmpty() const { return m_fps <= 0.f; }
	int FrameCount() const { return m_frameCount; }
	float Fps() const { return m_fps; }

	// The first frame that starts at or after ms, the frame a line starting
	// at ms first appears on. Past the last frame it extrapolates.
	int FrameAt(int ms) const;
	// The frame on screen at ms, the last one that starts at or before it.
	int FrameShownAt(int ms) const;
	// When frame starts; 0 for negative frames, extrapolated past the end.
	int MsAt(int frame) const;
	int ClampFrame(int frame) const;

	// Line times that land exactly on a frame: a Start that first shows on
	// frame, and an End that last shows frame. Both sit mid-frame.
	int StartTimeFor(int frame) const;
	int EndTimeFor(int frame) const;
	// Start of the last frame before ms, where playing up to ms should stop.
	int PlayEndBefore(int ms) const;

	const std::vector<int> &Keyframes() const { return m_keyframes; }
	void SetKeyframes(std::vector<int> keyframesMs);
	bool IsKeyframe(int ms) const;
	// Wrap around at either end; -1 when there are no keyframes.
	int NextKeyframe(int ms) const;
	int PrevKeyframe(int ms) const;

private:
	float FrameDuration() const;

	std::vector<int> m_timecodes;
	std::vector<int> m_keyframes;
	float m_fps = 0.f;
	int m_frameCount = 0;
};
