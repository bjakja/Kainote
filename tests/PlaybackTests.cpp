#include "check.h"
#include "../Kainote/Playback.h"

namespace {

// 25 fps: frames every 40 ms.
Timebase Pal(int frames)
{
	return Timebase::FromFps(25.f, frames);
}

} // namespace

TEST(seek_to_a_start_time_lands_on_the_frame_it_first_shows_on)
{
	Timebase tb = Pal(100);
	CHECK_EQ(SeekFrame(tb, 40, true), 1);
	CHECK_EQ(SeekFrame(tb, 41, true), 2);
	CHECK_EQ(SeekFrame(tb, 0, true), 0);
}

TEST(seek_to_an_end_time_lands_on_the_last_frame_it_shows)
{
	Timebase tb = Pal(100);
	CHECK_EQ(SeekFrame(tb, 80, false), 1);
	CHECK_EQ(SeekFrame(tb, 81, false), 2);
}

TEST(seek_past_the_video_lands_on_its_last_frame)
{
	Timebase tb = Pal(100);
	CHECK_EQ(SeekFrame(tb, 1000000, true), 99);
	CHECK_EQ(SeekFrame(tb, 1000000, false), 99);
}

TEST(pending_seek_is_empty_until_posted)
{
	PendingSeek pending;
	SeekRequest request;
	CHECK(!pending.Take(&request));
}

TEST(pending_seek_keeps_the_latest_request_with_its_own_flags)
{
	PendingSeek pending;
	pending.Post({ 1000, true, true });
	pending.Post({ 2000, false, false });
	SeekRequest request;
	CHECK(pending.Take(&request));
	CHECK_EQ(request.time, 2000);
	CHECK(!request.startTime);
	CHECK(!request.refreshAudio);
	CHECK(!pending.Take(&request));
}

TEST(playback_ends_at_the_play_end_or_the_last_frame)
{
	CHECK(!PlaybackReachedEnd(10, 400, 0, 100));
	CHECK(PlaybackReachedEnd(99, 3960, 0, 100));
	CHECK(PlaybackReachedEnd(10, 400, 400, 100));
	CHECK(!PlaybackReachedEnd(10, 399, 400, 100));
}

TEST(next_frame_on_time_waits_until_it_is_due)
{
	Timebase tb = Pal(100);
	PlaybackStep step = NextPlaybackFrame(tb, 10, 405, 0, 100);
	CHECK_EQ(step.frame, 11);
	CHECK_EQ(step.sleepMs, 35);
}

TEST(next_frame_slightly_late_is_shown_without_skipping)
{
	Timebase tb = Pal(100);
	PlaybackStep step = NextPlaybackFrame(tb, 10, 450, 0, 100);
	CHECK_EQ(step.frame, 11);
	CHECK_EQ(step.sleepMs, 0);
}

TEST(next_frame_far_behind_skips_to_the_frame_due_now)
{
	Timebase tb = Pal(100);
	PlaybackStep step = NextPlaybackFrame(tb, 10, 1000, 0, 100);
	CHECK_EQ(step.frame, 25);
	CHECK_EQ(step.sleepMs, 0);
}

TEST(skipping_stops_at_the_play_end)
{
	Timebase tb = Pal(100);
	CHECK_EQ(NextPlaybackFrame(tb, 10, 1000, 600, 100).frame, 15);
}

TEST(skipping_stops_at_the_last_frame)
{
	Timebase tb = Pal(30);
	CHECK_EQ(NextPlaybackFrame(tb, 10, 100000, 0, 30).frame, 29);
}
