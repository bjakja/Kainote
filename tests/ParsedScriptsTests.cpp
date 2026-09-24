#include "check.h"
#include "../Kainote/ParsedScripts.h"

#include <vector>

namespace {

std::vector<int *> released;

void Release(int *item)
{
	released.push_back(item);
}

} // namespace

TEST(a_parsed_script_is_found_by_its_hash)
{
	released.clear();
	int a = 1, b = 2;
	ParsedScripts<int> scripts(Release, 2);
	scripts.Add(10, &a, &a);
	scripts.Add(20, &b, &b);
	CHECK(scripts.Find(10) == &a);
	CHECK(scripts.Find(20) == &b);
	CHECK(scripts.Find(30) == nullptr);
	scripts.Clear();
}

TEST(the_least_recently_used_script_goes_first)
{
	released.clear();
	int a = 1, b = 2, c = 3;
	{
		ParsedScripts<int> scripts(Release, 2);
		scripts.Add(10, &a, &a);
		scripts.Add(20, &b, &b);
		scripts.Find(10);
		scripts.Add(30, &c, &c);
		CHECK_EQ(released.size(), (size_t)1);
		CHECK(released[0] == &b);
		CHECK(scripts.Find(10) == &a);
	}
	CHECK_EQ(released.size(), (size_t)3);
}

TEST(the_script_in_use_is_never_released)
{
	released.clear();
	int shown = 1, b = 2, c = 3;
	ParsedScripts<int> scripts(Release, 2);
	scripts.Add(10, &shown, &shown);
	// scripts parsed ahead, while shown stays on screen
	scripts.Add(20, &b, &shown);
	scripts.Add(30, &c, &shown);
	CHECK_EQ(released.size(), (size_t)1);
	CHECK(released[0] == &b);
	CHECK(scripts.Find(10) == &shown);
	CHECK(scripts.Find(30) == &c);
	scripts.Clear();
}
