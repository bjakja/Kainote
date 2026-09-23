#include "check.h"
#include "../Kainote/UndoHistory.h"

namespace {

int g_freed = 0;

void Free(int *snapshot)
{
	++g_freed;
	delete snapshot;
}

// A history of the steps 0..last, one snapshot per step holding its number.
struct History
{
	UndoHistory<int> steps{ Free };
	explicit History(int last)
	{
		g_freed = 0;
		for (int i = 0; i <= last; i++)
			steps.Record(new int(i));
	}
	int Current() const { return *steps.Current(); }
};

} // namespace

TEST(recording_moves_to_the_new_step)
{
	History h(2);
	CHECK_EQ(h.steps.Size(), 3);
	CHECK_EQ(h.steps.Step(), 2);
	CHECK_EQ(h.Current(), 2);
	CHECK(h.steps.CanUndo());
	CHECK(!h.steps.CanRedo());
}

TEST(undo_and_redo_stop_at_the_ends)
{
	History h(1);
	CHECK(h.steps.Undo());
	CHECK_EQ(h.Current(), 0);
	CHECK(!h.steps.Undo());
	CHECK(h.steps.Redo());
	CHECK(!h.steps.Redo());
	CHECK_EQ(h.Current(), 1);
}

TEST(recording_after_undo_drops_the_steps_that_could_be_redone)
{
	History h(3);
	h.steps.Undo();
	h.steps.Undo();
	h.steps.Record(new int(10));
	CHECK_EQ(g_freed, 2);
	CHECK_EQ(h.steps.Size(), 3);
	CHECK_EQ(h.Current(), 10);
	CHECK(!h.steps.CanRedo());
}

TEST(a_new_history_is_saved_until_a_step_is_recorded)
{
	History h(0);
	CHECK(!h.steps.IsModified());
	h.steps.Record(new int(1));
	CHECK(h.steps.IsModified());
	h.steps.Undo();
	CHECK(!h.steps.IsModified());
}

TEST(steps_from_save_count_from_the_saved_step)
{
	History h(4);
	h.steps.GoTo(1);
	h.steps.MarkSaved();
	h.steps.GoTo(3);
	CHECK_EQ(h.steps.StepsFromSave(), 2);
	h.steps.ForgetSaved();
	CHECK_EQ(h.steps.StepsFromSave(), 3);
	CHECK(h.steps.IsModified());
}

TEST(dropping_the_saved_step_forgets_it)
{
	History h(3);
	h.steps.MarkSaved();
	h.steps.GoTo(1);
	h.steps.Record(new int(10));
	CHECK_EQ(h.steps.SavedStep(), -1);
	CHECK(h.steps.IsModified());
}

TEST(rewind_goes_back_and_drops_what_came_after)
{
	History h(3);
	CHECK(h.steps.Rewind(1));
	CHECK_EQ(h.steps.Size(), 2);
	CHECK_EQ(h.Current(), 1);
	CHECK_EQ(g_freed, 2);
	CHECK(!h.steps.Rewind(5));
}

TEST(dropping_the_oldest_steps_keeps_the_first_and_shifts_the_rest)
{
	History h(5);
	h.steps.GoTo(4);
	h.steps.MarkSaved();
	h.steps.DropOldest(3);
	CHECK_EQ(g_freed, 2);
	CHECK_EQ(h.steps.Size(), 4);
	CHECK_EQ(*h.steps.At(0), 0);
	CHECK_EQ(*h.steps.At(1), 3);
	CHECK_EQ(h.steps.Step(), 2);
	CHECK_EQ(h.Current(), 4);
	CHECK_EQ(h.steps.SavedStep(), 2);
	CHECK(!h.steps.IsModified());
}

TEST(dropping_the_current_or_saved_step_falls_back_to_the_first)
{
	History h(5);
	h.steps.GoTo(2);
	h.steps.MarkSaved();
	h.steps.DropOldest(4);
	CHECK_EQ(h.steps.Step(), 0);
	CHECK_EQ(h.steps.SavedStep(), -1);
}

TEST(clearing_frees_every_step)
{
	History h(2);
	h.steps.Clear();
	CHECK_EQ(g_freed, 3);
	CHECK_EQ(h.steps.Size(), 0);
	CHECK(h.steps.Current() == nullptr);
}
