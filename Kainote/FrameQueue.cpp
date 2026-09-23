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

#include "FrameQueue.h"

#include <algorithm>

FrameQueue::FrameQueue(size_t slots, size_t frameBytes, int frameCount)
	: m_slots(slots)
	, m_frameCount(frameCount)
{
	for (Slot &slot : m_slots)
		slot.data.resize(frameBytes);
}

FrameQueue::Slot *FrameQueue::Find(unsigned char *data)
{
	for (Slot &slot : m_slots) {
		if (slot.data.data() == data)
			return &slot;
	}
	return nullptr;
}

unsigned char *FrameQueue::NextToDecode(int *frame)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	while (!m_stopped) {
		m_next = std::max(m_next, m_wanted);
		if (!m_failed && m_next < m_frameCount) {
			for (Slot &slot : m_slots) {
				if (slot.state != FREE)
					continue;
				slot.state = DECODING;
				slot.frame = m_next++;
				slot.generation = m_generation;
				*frame = slot.frame;
				return slot.data.data();
			}
		}
		m_changed.wait(lock);
	}
	return nullptr;
}

void FrameQueue::Decoded(unsigned char *data, bool ok)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		Slot *slot = Find(data);
		if (!slot)
			return;
		if (slot->generation != m_generation) {
			slot->state = FREE;
		}
		else if (!ok) {
			slot->state = FREE;
			m_failed = true;
		}
		else {
			slot->state = READY;
		}
	}
	m_changed.notify_all();
}

FrameQueue::Result FrameQueue::Take(int wanted)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	bool dropped = false;
	if (wanted > m_wanted) {
		m_wanted = wanted;
		dropped = true;
	}
	Result result;
	while (!m_stopped) {
		Slot *first = nullptr;
		bool decoding = false;
		for (Slot &slot : m_slots) {
			if (slot.state == READY && slot.frame < wanted) {
				slot.state = FREE;
				dropped = true;
			}
			else if (slot.state == READY && (!first || slot.frame < first->frame)) {
				first = &slot;
			}
			else if (slot.state == DECODING && slot.generation == m_generation) {
				decoding = true;
			}
		}
		if (dropped) {
			m_changed.notify_all();
			dropped = false;
		}
		if (first) {
			first->state = TAKEN;
			result.slot = first->data.data();
			result.frame = first->frame;
			return result;
		}
		if (m_failed && !decoding) {
			result.failed = true;
			return result;
		}
		if (m_next >= m_frameCount && !decoding)
			return result;
		m_changed.wait(lock);
	}
	return result;
}

void FrameQueue::Release(unsigned char *data)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		Slot *slot = Find(data);
		if (slot && slot->state == TAKEN)
			slot->state = FREE;
	}
	m_changed.notify_all();
}

void FrameQueue::Reset(int frame)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		++m_generation;
		for (Slot &slot : m_slots) {
			if (slot.state == READY)
				slot.state = FREE;
		}
		m_next = frame;
		m_wanted = frame;
		m_failed = false;
	}
	m_changed.notify_all();
}

void FrameQueue::Stop()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_stopped = true;
	}
	m_changed.notify_all();
}
