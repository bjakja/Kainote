#include "check.h"
#include "../Kainote/Timebase.h"

#include <climits>

namespace {

const float NTSC_FILM = 24000.f / 1001.f;

// What FFMS2 reports for constant frame rate video: truncated milliseconds.
std::vector<int> CfrTimecodes(float fps, int count)
{
	std::vector<int> timecodes;
	for (int i = 0; i < count; i++)
		timecodes.push_back((int)(i * (1000.0 / fps)));
	return timecodes;
}

// A variable frame rate stretch: 24 fps, then 60 fps, then 30 fps.
Timebase Vfr()
{
	return Timebase::FromTimecodes({ 0, 42, 83, 125, 142, 158, 175, 192, 225, 258, 292 }, 0.f);
}

int ToCentiseconds(int ms)
{
	return (ms / 10) * 10;
}

void CheckRoundTrip(const Timebase &tb, int frames)
{
	for (int f = 0; f < frames; f++)
		CHECK_EQ(tb.FrameAt(tb.MsAt(f)), f);
}

void CheckLineTimesLandOnFrames(const Timebase &tb, int frames)
{
	for (int f = 1; f < frames; f++) {
		CHECK_EQ(tb.FrameAt(tb.StartTimeFor(f)), f);
		CHECK_EQ(tb.FrameAt(ToCentiseconds(tb.StartTimeFor(f))), f);
		CHECK_EQ(tb.FrameAt(tb.EndTimeFor(f)) - 1, f);
		CHECK_EQ(tb.FrameAt(ToCentiseconds(tb.EndTimeFor(f))) - 1, f);
	}
}

} // namespace

TEST(empty_timebase_answers_zero)
{
	Timebase tb;
	CHECK(tb.IsEmpty());
	CHECK_EQ(tb.FrameAt(5000), 0);
	CHECK_EQ(tb.MsAt(100), 0);
	CHECK_EQ(tb.NextKeyframe(0), -1);
}

TEST(frame_at_is_the_first_frame_starting_at_or_after)
{
	Timebase tb = Timebase::FromTimecodes(CfrTimecodes(NTSC_FILM, 100), NTSC_FILM);
	CHECK_EQ(tb.FrameAt(0), 0);
	CHECK_EQ(tb.FrameAt(-40), 0);
	CHECK_EQ(tb.FrameAt(1), 1);
	CHECK_EQ(tb.FrameAt(41), 1);
	CHECK_EQ(tb.FrameAt(42), 2);
	CHECK_EQ(tb.FrameAt(83), 2);
}

TEST(frame_shown_at_is_the_frame_on_screen)
{
	Timebase tb = Timebase::FromTimecodes(CfrTimecodes(NTSC_FILM, 100), NTSC_FILM);
	CHECK_EQ(tb.FrameShownAt(0), 0);
	CHECK_EQ(tb.FrameShownAt(40), 0);
	CHECK_EQ(tb.FrameShownAt(41), 1);
	CHECK_EQ(tb.FrameShownAt(82), 1);
	CHECK_EQ(tb.FrameShownAt(83), 2);
	for (int f = 0; f < 100; f++)
		CHECK_EQ(tb.FrameShownAt(tb.MsAt(f)), f);
}

TEST(timecodes_and_fps_agree_on_constant_frame_rate)
{
	Timebase fromTimecodes = Timebase::FromTimecodes(CfrTimecodes(NTSC_FILM, 500), NTSC_FILM);
	Timebase fromFps = Timebase::FromFps(NTSC_FILM, 500);
	for (int f = 0; f < 500; f++)
		CHECK_EQ(fromFps.MsAt(f), fromTimecodes.MsAt(f));
	for (int ms = 0; ms <= fromFps.MsAt(499); ms += 7)
		CHECK_EQ(fromFps.FrameAt(ms), fromTimecodes.FrameAt(ms));
}

TEST(frame_and_time_round_trip)
{
	CheckRoundTrip(Timebase::FromTimecodes(CfrTimecodes(NTSC_FILM, 300), NTSC_FILM), 400);
	CheckRoundTrip(Timebase::FromFps(30000.f / 1001.f, 300), 400);
	CheckRoundTrip(Timebase::FromFps(25.f, 300), 400);
	CheckRoundTrip(Vfr(), 20);
}

TEST(times_past_the_last_frame_extrapolate)
{
	Timebase tb = Timebase::FromTimecodes(CfrTimecodes(25.f, 10), 25.f);
	CHECK_EQ(tb.MsAt(9), 360);
	CHECK_EQ(tb.MsAt(10), 400);
	CHECK_EQ(tb.MsAt(12), 480);
	CHECK_EQ(tb.FrameAt(401), 11);
	CHECK_EQ(tb.ClampFrame(tb.FrameAt(401)), 9);
	CHECK_EQ(tb.ClampFrame(-3), 0);
}

TEST(line_times_land_on_their_frame_after_centisecond_rounding)
{
	CheckLineTimesLandOnFrames(Timebase::FromTimecodes(CfrTimecodes(NTSC_FILM, 300), NTSC_FILM), 300);
	CheckLineTimesLandOnFrames(Timebase::FromFps(30000.f / 1001.f, 300), 300);
	CheckLineTimesLandOnFrames(Timebase::FromFps(25.f, 300), 300);
}

TEST(line_times_land_on_their_frame_with_variable_frame_rate)
{
	Timebase tb = Vfr();
	for (int f = 1; f < tb.FrameCount(); f++) {
		CHECK_EQ(tb.FrameAt(tb.StartTimeFor(f)), f);
		CHECK_EQ(tb.FrameAt(tb.EndTimeFor(f)) - 1, f);
	}
}

TEST(line_times_stay_inside_short_frames)
{
	Timebase tb = Timebase::FromFps(120.f, 100);
	for (int f = 1; f < 100; f++) {
		CHECK_EQ(tb.FrameAt(tb.StartTimeFor(f)), f);
		CHECK_EQ(tb.FrameAt(tb.EndTimeFor(f)) - 1, f);
	}
}

TEST(first_frame_starts_at_zero)
{
	Timebase tb = Timebase::FromFps(NTSC_FILM, 100);
	CHECK_EQ(tb.StartTimeFor(0), 0);
	CHECK_EQ(tb.FrameAt(tb.StartTimeFor(0)), 0);
}

TEST(play_end_is_the_start_of_the_last_frame_before)
{
	Timebase tb = Timebase::FromTimecodes(CfrTimecodes(25.f, 100), 25.f);
	CHECK_EQ(tb.PlayEndBefore(400), 360);
	CHECK_EQ(tb.PlayEndBefore(401), 400);
	CHECK_EQ(tb.PlayEndBefore(0), 0);
}

TEST(keyframes_are_sorted_and_wrap_around)
{
	Timebase tb = Timebase::FromFps(25.f, 1000);
	tb.SetKeyframes({ 4000, 0, 2000, 2000 });
	CHECK_EQ((int)tb.Keyframes().size(), 3);
	CHECK(tb.IsKeyframe(2000));
	CHECK(!tb.IsKeyframe(2001));
	CHECK_EQ(tb.NextKeyframe(0), 2000);
	CHECK_EQ(tb.NextKeyframe(2500), 4000);
	CHECK_EQ(tb.NextKeyframe(4000), 0);
	CHECK_EQ(tb.PrevKeyframe(2000), 0);
	CHECK_EQ(tb.PrevKeyframe(2001), 2000);
	CHECK_EQ(tb.PrevKeyframe(0), 4000);
}

TEST(huge_times_and_frames_saturate_instead_of_overflowing)
{
	Timebase tb = Timebase::FromTimecodes(CfrTimecodes(NTSC_FILM, 100), NTSC_FILM);
	CHECK_EQ(tb.MsAt(INT_MAX), INT_MAX);
	CHECK(tb.FrameAt(INT_MAX) > 0);
	CHECK(tb.FrameShownAt(INT_MAX) > 0);
	Timebase fast = Timebase::FromFps(2000.f, 0);
	CHECK_EQ(fast.MsAt(INT_MAX), INT_MAX / 2);
	CHECK(fast.FrameAt(INT_MAX) > 0);
}

TEST(fps_is_derived_from_timecodes_when_unknown)
{
	Timebase tb = Timebase::FromTimecodes(CfrTimecodes(25.f, 101), 0.f);
	CHECK(tb.Fps() > 24.99f && tb.Fps() < 25.01f);
	CHECK_EQ(tb.FrameCount(), 101);
}

TEST(frames_from_the_video_are_exact_and_estimated_ones_are_not)
{
	CHECK(!Timebase().IsExact());
	CHECK(Timebase::FromTimecodes({ 0, 40, 80 }, 25.f).IsExact());
	CHECK(Timebase::FromFps(25.f, 100).IsExact());
	CHECK(!Timebase::Estimated(25.f, 100).IsExact());
	CHECK_EQ(Timebase::Estimated(25.f, 100).FrameAt(41), 2);
}
