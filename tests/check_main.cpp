#include "check.h"

int main()
{
	for (const auto &test : check::Registry()) {
		int before = check::Failures();
		test.body();
		std::printf("%s %s\n", (check::Failures() == before) ? "ok  " : "FAIL", test.name);
	}
	std::printf("%zu tests, %d failed checks\n", check::Registry().size(), check::Failures());
	return check::Failures() ? 1 : 0;
}
