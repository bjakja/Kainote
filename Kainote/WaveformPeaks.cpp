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

#include "WaveformPeaks.h"

#include <algorithm>

void WaveformPeaks::Append(const short *samples, long long count)
{
	for (long long i = 0; i < count; i++, m_samples++) {
		short sample = samples[i];
		if (m_samples % m_block == 0) {
			m_lo.push_back(sample);
			m_hi.push_back(sample);
		}
		else {
			m_lo.back() = std::min(m_lo.back(), sample);
			m_hi.back() = std::max(m_hi.back(), sample);
		}
	}
}

bool WaveformPeaks::Range(long long first, long long count, short *lo, short *hi) const
{
	if (first < 0)
		first = 0;
	if (first >= m_samples || count <= 0)
		return false;
	size_t begin = (size_t)(first / m_block);
	size_t end = std::min((size_t)((first + count - 1) / m_block) + 1, m_lo.size());
	*lo = *std::min_element(m_lo.begin() + begin, m_lo.begin() + end);
	*hi = *std::max_element(m_hi.begin() + begin, m_hi.begin() + end);
	return true;
}
