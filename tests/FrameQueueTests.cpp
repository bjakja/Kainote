#include "check.h"
#include "../Kainote/FrameQueue.h"

#include <cstring>
#include <thread>

namespace {

// A decoder that writes each frame's number into its slot, until the queue stops.
struct Decoder
{
	FrameQueue &queue;
	int failAt;
	std::thread thread;

	Decoder(FrameQueue &q, int fail = -1) : queue(q), failAt(fail), thread([this] { Run(); }) {}
	~Decoder()
	{
		queue.Stop();
		thread.join();
	}
	void Run()
	{
		int frame;
		while (unsigned char *slot = queue.NextToDecode(&frame)) {
			std::memcpy(slot, &frame, sizeof(frame));
			queue.Decoded(slot, frame != failAt);
		}
	}
};

int FrameIn(const unsigned char *slot)
{
	int frame;
	std::memcpy(&frame, slot, sizeof(frame));
	return frame;
}

} // namespace

TEST(frames_come_out_in_order)
{
	FrameQueue queue(3, sizeof(int), 100);
	queue.Reset(10);
	Decoder decoder(queue);
	for (int wanted = 10; wanted < 20; wanted++) {
		FrameQueue::Result r = queue.Take(wanted);
		CHECK(r.slot != nullptr);
		CHECK_EQ(r.frame, wanted);
		CHECK_EQ(FrameIn(r.slot), wanted);
		queue.Release(r.slot);
	}
}

TEST(taking_a_later_frame_drops_the_earlier_ones)
{
	FrameQueue queue(3, sizeof(int), 100);
	queue.Reset(0);
	Decoder decoder(queue);
	FrameQueue::Result r = queue.Take(0);
	queue.Release(r.slot);
	r = queue.Take(40);
	CHECK(r.slot != nullptr);
	CHECK_EQ(r.frame, 40);
	CHECK_EQ(FrameIn(r.slot), 40);
	queue.Release(r.slot);
	r = queue.Take(41);
	CHECK_EQ(r.frame, 41);
	queue.Release(r.slot);
}

TEST(reset_starts_over_at_the_new_frame)
{
	FrameQueue queue(3, sizeof(int), 100);
	queue.Reset(50);
	Decoder decoder(queue);
	FrameQueue::Result r = queue.Take(50);
	queue.Release(r.slot);
	queue.Reset(5);
	r = queue.Take(5);
	CHECK_EQ(r.frame, 5);
	CHECK_EQ(FrameIn(r.slot), 5);
	queue.Release(r.slot);
}

TEST(a_failed_frame_is_reported)
{
	FrameQueue queue(3, sizeof(int), 100);
	queue.Reset(0);
	Decoder decoder(queue, 2);
	for (int wanted = 0; wanted < 2; wanted++)
		queue.Release(queue.Take(wanted).slot);
	FrameQueue::Result r = queue.Take(2);
	CHECK(r.slot == nullptr);
	CHECK(r.failed);
}

TEST(decoding_stops_after_the_last_frame)
{
	FrameQueue queue(3, sizeof(int), 5);
	queue.Reset(3);
	Decoder decoder(queue);
	FrameQueue::Result r = queue.Take(4);
	CHECK_EQ(r.frame, 4);
	queue.Release(r.slot);
	r = queue.Take(5);
	CHECK(r.slot == nullptr);
	CHECK(!r.failed);
}

TEST(stop_wakes_a_waiting_taker)
{
	FrameQueue queue(3, sizeof(int), 100);
	queue.Reset(0);
	std::thread stopper([&] { queue.Stop(); });
	FrameQueue::Result r = queue.Take(0);
	stopper.join();
	CHECK(r.slot == nullptr);
}

TEST(a_stopped_queue_plays_again_after_reset)
{
	FrameQueue queue(3, sizeof(int), 100);
	queue.Reset(0);
	{
		Decoder decoder(queue);
		FrameQueue::Result r = queue.Take(0);
		queue.Release(r.slot);
	}
	queue.Reset(40);
	Decoder decoder(queue);
	FrameQueue::Result r = queue.Take(40);
	CHECK(r.slot != nullptr);
	CHECK_EQ(r.frame, 40);
	CHECK_EQ(FrameIn(r.slot), 40);
	queue.Release(r.slot);
}
