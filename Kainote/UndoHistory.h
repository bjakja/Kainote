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

// Snapshots of a document to undo and redo through, and which one was saved.
// It owns the snapshots and frees them with the function it is given.
template <typename T>
class UndoHistory
{
public:
	using Destroy = void (*)(T *);

	explicit UndoHistory(Destroy destroy) : m_destroy(destroy) {}
	~UndoHistory() { Clear(); }
	UndoHistory(const UndoHistory &) = delete;
	UndoHistory &operator=(const UndoHistory &) = delete;

	// A new history, saved at its first step.
	void Clear()
	{
		for (T *snapshot : m_steps)
			m_destroy(snapshot);
		m_steps.clear();
		m_step = 0;
		m_saved = 0;
	}

	// Adds a step after the current one; the steps that could be redone go.
	void Record(T *snapshot)
	{
		if (!m_steps.empty())
			DropAfter(m_step);
		m_steps.push_back(snapshot);
		m_step = Size() - 1;
	}

	// These return false when there is no such step.
	bool Undo() { return GoTo(m_step - 1); }
	bool Redo() { return GoTo(m_step + 1); }
	bool GoTo(int step)
	{
		if (step < 0 || step >= Size())
			return false;
		m_step = step;
		return true;
	}

	// Goes back to step for good: the steps after it go.
	bool Rewind(int step)
	{
		if (!GoTo(step))
			return false;
		DropAfter(step);
		return true;
	}

	// Frees memory by dropping the oldest steps but the first, keeping count - 1.
	void DropOldest(int count)
	{
		int last = (count < Size()) ? count : Size();
		if (last <= 1)
			return;
		for (int i = 1; i < last; i++)
			m_destroy(m_steps[i]);
		m_steps.erase(m_steps.begin() + 1, m_steps.begin() + last);
		int dropped = last - 1;
		m_step = (m_step >= last) ? m_step - dropped : 0;
		if (m_saved >= last)
			m_saved -= dropped;
		else if (m_saved > 0)
			m_saved = -1;
	}

	T *Current() const { return m_steps.empty() ? nullptr : m_steps[m_step]; }
	T *At(int step) const { return (step >= 0 && step < Size()) ? m_steps[step] : nullptr; }
	int Step() const { return m_step; }
	int Size() const { return (int)m_steps.size(); }
	bool CanUndo() const { return m_step > 0; }
	bool CanRedo() const { return m_step < Size() - 1; }

	void MarkSaved() { m_saved = m_step; }
	// The saved state is not in the history, as after opening a backup.
	void ForgetSaved() { m_saved = -1; }
	// -1 when the saved state is not in the history
	int SavedStep() const { return m_saved; }
	bool IsModified() const { return m_step != m_saved; }
	// Steps from the saved one, or from the start when it is not in the history.
	int StepsFromSave() const { return (m_saved < 0) ? m_step : m_step - m_saved; }

private:
	void DropAfter(int step)
	{
		for (int i = step + 1; i < Size(); i++)
			m_destroy(m_steps[i]);
		m_steps.erase(m_steps.begin() + step + 1, m_steps.end());
		if (m_saved >= Size())
			m_saved = -1;
	}

	std::vector<T *> m_steps;
	Destroy m_destroy;
	int m_step = 0;
	int m_saved = 0;
};
