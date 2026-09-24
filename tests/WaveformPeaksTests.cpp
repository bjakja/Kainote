#include "check.h"
#include "../Kainote/WaveformPeaks.h"

#include <vector>

namespace {

std::vector<short> Ramp(int count)
{
	std::vector<short> samples(count);
	for (int i = 0; i < count; i++)
		samples[i] = (short)((i % 2) ? i : -i);
	return samples;
}

} // namespace

TEST(peaks_cover_whole_blocks)
{
	std::vector<short> samples = Ramp(1000);
	WaveformPeaks peaks(100);
	peaks.Append(samples.data(), 1000);
	short lo, hi;
	CHECK(peaks.Range(0, 100, &lo, &hi));
	CHECK_EQ((int)lo, -98);
	CHECK_EQ((int)hi, 99);
	CHECK(peaks.Range(200, 300, &lo, &hi));
	CHECK_EQ((int)lo, -498);
	CHECK_EQ((int)hi, 499);
}

TEST(appending_in_pieces_gives_the_same_blocks)
{
	std::vector<short> samples = Ramp(1000);
	WaveformPeaks whole(100), pieces(100);
	whole.Append(samples.data(), 1000);
	pieces.Append(samples.data(), 37);
	pieces.Append(samples.data() + 37, 500);
	pieces.Append(samples.data() + 537, 463);
	for (int start = 0; start < 1000; start += 100) {
		short a, b, c, d;
		CHECK(whole.Range(start, 100, &a, &b));
		CHECK(pieces.Range(start, 100, &c, &d));
		CHECK_EQ((int)a, (int)c);
		CHECK_EQ((int)b, (int)d);
	}
}

TEST(a_range_takes_every_block_it_touches)
{
	std::vector<short> samples = Ramp(1000);
	WaveformPeaks peaks(100);
	peaks.Append(samples.data(), 1000);
	short lo, hi;
	CHECK(peaks.Range(150, 100, &lo, &hi));
	CHECK_EQ((int)lo, -298);
	CHECK_EQ((int)hi, 299);
}

TEST(the_last_partial_block_counts)
{
	std::vector<short> samples = Ramp(250);
	WaveformPeaks peaks(100);
	peaks.Append(samples.data(), 250);
	short lo, hi;
	CHECK(peaks.Range(200, 50, &lo, &hi));
	CHECK_EQ((int)hi, 249);
	CHECK_EQ(peaks.Samples(), (long long)250);
}

TEST(ranges_past_the_end_are_empty)
{
	std::vector<short> samples = Ramp(250);
	WaveformPeaks peaks(100);
	peaks.Append(samples.data(), 250);
	short lo, hi;
	CHECK(!peaks.Range(300, 100, &lo, &hi));
	CHECK(peaks.Range(200, 1000, &lo, &hi));
}
