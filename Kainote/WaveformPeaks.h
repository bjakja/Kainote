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

// The lowest and highest sample of each block of audio, so a zoomed out
// waveform reads one entry per block instead of every sample.
class WaveformPeaks
{
public:
	explicit WaveformPeaks(int blockSamples) : m_block(blockSamples) {}

	void Append(const short *samples, long long count);
	// lo and hi over every block the range touches; false when it starts past the end
	bool Range(long long first, long long count, short *lo, short *hi) const;
	long long Samples() const { return m_samples; }
	int BlockSamples() const { return m_block; }

private:
	int m_block;
	long long m_samples = 0;
	std::vector<short> m_lo;
	std::vector<short> m_hi;
};
