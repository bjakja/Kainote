#include "check.h"
#include "../Kainote/SemVer.h"
#include "../Kainote/VersionKainote.h"

#include <iterator>

namespace {

bool IsValid(const char *text)
{
	return ParseSemVer(text).has_value();
}

int Compare(const char *a, const char *b)
{
	return CompareSemVer(*ParseSemVer(a), *ParseSemVer(b));
}

} // namespace

TEST(semver_parses_core_prerelease_and_tag_prefix)
{
	std::optional<SemVer> version = ParseSemVer("v12.3.40-rc.1+build.5");
	CHECK(version.has_value());
	CHECK_EQ(version->major, 12LL);
	CHECK_EQ(version->minor, 3LL);
	CHECK_EQ(version->patch, 40LL);
	CHECK(version->prerelease == "rc.1");
}

TEST(semver_rejects_malformed_versions)
{
	CHECK(IsValid("0.0.0"));
	CHECK(IsValid("1.0.0-0A.is.legal"));
	CHECK(IsValid("1.0.0+0.build.1-rc.10000aaa-kk-0.1"));
	CHECK(!IsValid(""));
	CHECK(!IsValid("1.2"));
	CHECK(!IsValid("1.2.3.4"));
	CHECK(!IsValid("v1.0.0.1537"));
	CHECK(!IsValid("01.1.1"));
	CHECK(!IsValid("1.2.3-"));
	CHECK(!IsValid("1.2.3-beta..1"));
	CHECK(!IsValid("1.2.3-01"));
	CHECK(!IsValid("1.2.3+"));
	CHECK(!IsValid("1.2.3-be$ta"));
	CHECK(!IsValid("1.2.x"));
}

TEST(semver_orders_by_precedence)
{
	// The ordering example from semver.org, section 11.
	const char *ordered[] = {
		"1.0.0-alpha", "1.0.0-alpha.1", "1.0.0-alpha.beta", "1.0.0-beta",
		"1.0.0-beta.2", "1.0.0-beta.11", "1.0.0-rc.1", "1.0.0",
		"2.0.0", "2.1.0-rc.1", "2.1.0-rc.2", "2.1.0-rc.10", "2.1.0", "2.1.1", "10.0.0",
	};
	for (size_t i = 0; i + 1 < std::size(ordered); i++) {
		CHECK(Compare(ordered[i], ordered[i + 1]) < 0);
		CHECK(Compare(ordered[i + 1], ordered[i]) > 0);
	}
}

TEST(semver_ignores_build_metadata)
{
	CHECK_EQ(Compare("1.0.0+a", "1.0.0+b"), 0);
	CHECK_EQ(Compare("v1.0.0", "1.0.0"), 0);
}

TEST(semver_declared_version_is_valid)
{
	CHECK(IsValid(VersionKainote));
}
