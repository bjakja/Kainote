#pragma once

// A minimal test harness: TEST registers a function, CHECK/CHECK_EQ record
// failures, and main() in check_main.cpp runs everything.

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace check {

struct Test {
	const char *name;
	std::function<void()> body;
};

inline std::vector<Test> &Registry()
{
	static std::vector<Test> tests;
	return tests;
}

inline int &Failures()
{
	static int failures = 0;
	return failures;
}

struct Registrar {
	Registrar(const char *name, std::function<void()> body) { Registry().push_back({ name, std::move(body) }); }
};

template <typename A, typename B>
void ExpectEqual(const A &actual, const B &expected, const char *actualText, const char *expectedText,
	const char *file, int line)
{
	if (actual == expected)
		return;
	++Failures();
	std::printf("%s:%d: CHECK_EQ(%s, %s) failed: got %s, expected %s\n", file, line, actualText, expectedText,
		std::to_string(actual).c_str(), std::to_string(expected).c_str());
}

} // namespace check

#define CHECK_CONCAT_(a, b) a##b
#define CHECK_CONCAT(a, b) CHECK_CONCAT_(a, b)

#define TEST(name) \
	static void CHECK_CONCAT(test_, name)(); \
	static check::Registrar CHECK_CONCAT(registrar_, name)(#name, CHECK_CONCAT(test_, name)); \
	static void CHECK_CONCAT(test_, name)()

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			++check::Failures(); \
			std::printf("%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond); \
		} \
	} while (0)

#define CHECK_EQ(actual, expected) \
	check::ExpectEqual((actual), (expected), #actual, #expected, __FILE__, __LINE__)
