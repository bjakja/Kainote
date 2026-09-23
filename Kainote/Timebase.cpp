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

#include "Timebase.h"

#include <algorithm>
#include <climits>
#include <cmath>

// Keeps a mid-frame time on the same frame after ASS rounds it to centiseconds.
static const int CENTISECOND_MARGIN = 5;

// Frame numbers from scripts or typed in can be far past any video.
static int SaturateToInt(double value)
{
	return (value >= (double)INT_MAX) ? INT_MAX : (int)value;
}

Timebase Timebase::FromTimecodes(std::vector<int> timecodes, float fps)
{
	Timebase tb;
	tb.m_frameCount = (int)timecodes.size();
	if (fps <= 0.f && timecodes.size() > 1 && timecodes.back() > timecodes.front())
		fps = 1000.f * (timecodes.size() - 1) / (timecodes.back() - timecodes.front());
	tb.m_fps = (tb.m_frameCount > 0) ? fps : 0.f;
	tb.m_timecodes = std::move(timecodes);
	return tb;
}

Timebase Timebase::FromFps(float fps, int frameCount)
{
	Timebase tb;
	tb.m_fps = std::max(fps, 0.f);
	tb.m_frameCount = std::max(frameCount, 0);
	return tb;
}

float Timebase::FrameDuration() const
{
	return (m_fps > 0.f) ? 1000.f / m_fps : 0.f;
}

int Timebase::MsAt(int frame) const
{
	if (frame < 0 || IsEmpty())
		return 0;
	if (m_timecodes.empty())
		return SaturateToInt(frame * (double)FrameDuration());

	int last = (int)m_timecodes.size() - 1;
	if (frame <= last)
		return m_timecodes[frame];
	return SaturateToInt(m_timecodes[last] + (frame - last) * (double)FrameDuration());
}

int Timebase::FrameAt(int ms) const
{
	if (ms <= 0 || IsEmpty())
		return 0;

	double estimate;
	if (!m_timecodes.empty()) {
		auto it = std::lower_bound(m_timecodes.begin(), m_timecodes.end(), ms);
		if (it != m_timecodes.end())
			return (int)(it - m_timecodes.begin());
		int last = (int)m_timecodes.size() - 1;
		estimate = last + std::ceil((ms - m_timecodes[last]) / FrameDuration());
	}
	else {
		estimate = std::ceil(ms / FrameDuration());
	}
	int frame = SaturateToInt(estimate);
	// the float estimate can be one off where MsAt truncates
	while (frame < INT_MAX && MsAt(frame) < ms)
		frame++;
	while (frame > 0 && MsAt(frame - 1) >= ms)
		frame--;
	return frame;
}

int Timebase::FrameShownAt(int ms) const
{
	if (ms < 0)
		return 0;
	return std::max(FrameAt((ms < INT_MAX) ? ms + 1 : ms) - 1, 0);
}

int Timebase::ClampFrame(int frame) const
{
	if (frame < 0)
		return 0;
	if (m_frameCount > 0 && frame >= m_frameCount)
		return m_frameCount - 1;
	return frame;
}

int Timebase::StartTimeFor(int frame) const
{
	if (frame <= 0)
		return 0;
	int frameTime = MsAt(frame);
	int prevTime = MsAt(frame - 1);
	return std::min(prevTime + (frameTime - prevTime) / 2 + CENTISECOND_MARGIN, frameTime);
}

int Timebase::EndTimeFor(int frame) const
{
	if (frame < 0)
		return 0;
	int frameTime = MsAt(frame);
	int nextTime = MsAt(frame + 1);
	return std::min(frameTime + (nextTime - frameTime) / 2 + CENTISECOND_MARGIN, nextTime);
}

int Timebase::PlayEndBefore(int ms) const
{
	return MsAt(FrameAt(ms) - 1);
}

void Timebase::SetKeyframes(std::vector<int> keyframesMs)
{
	std::sort(keyframesMs.begin(), keyframesMs.end());
	keyframesMs.erase(std::unique(keyframesMs.begin(), keyframesMs.end()), keyframesMs.end());
	m_keyframes = std::move(keyframesMs);
}

bool Timebase::IsKeyframe(int ms) const
{
	return std::binary_search(m_keyframes.begin(), m_keyframes.end(), ms);
}

int Timebase::NextKeyframe(int ms) const
{
	if (m_keyframes.empty())
		return -1;
	auto it = std::upper_bound(m_keyframes.begin(), m_keyframes.end(), ms);
	return (it != m_keyframes.end()) ? *it : m_keyframes.front();
}

int Timebase::PrevKeyframe(int ms) const
{
	if (m_keyframes.empty())
		return -1;
	auto it = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), ms);
	return (it != m_keyframes.begin()) ? *(it - 1) : m_keyframes.back();
}
