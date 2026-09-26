//  Copyright (c) 2012 - 2026, Marcin Drob

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

#include <optional>
#include <string_view>

// Semantic Versioning 2.0.0 (https://semver.org). The prerelease view points
// into the parsed text, so that text must outlive the SemVer.
struct SemVer
{
	long long major = 0;
	long long minor = 0;
	long long patch = 0;
	std::string_view prerelease;
};

namespace semver_detail
{
	constexpr bool IsDigit(char c) { return c >= '0' && c <= '9'; }

	constexpr bool IsIdentifierChar(char c)
	{
		return IsDigit(c) || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-';
	}

	constexpr bool IsNumeric(std::string_view s)
	{
		if (s.empty())
			return false;
		for (char c : s) {
			if (!IsDigit(c))
				return false;
		}
		return true;
	}

	constexpr bool HasLeadingZero(std::string_view s) { return s.size() > 1 && s[0] == '0'; }

	constexpr std::optional<long long> ParseNumber(std::string_view s)
	{
		if (!IsNumeric(s) || HasLeadingZero(s) || s.size() > 18)
			return std::nullopt;
		long long value = 0;
		for (char c : s)
			value = value * 10 + (c - '0');
		return value;
	}

	constexpr bool AreValidIdentifiers(std::string_view s, bool prerelease)
	{
		while (true) {
			size_t dot = s.find('.');
			std::string_view id = s.substr(0, dot);
			if (id.empty())
				return false;
			for (char c : id) {
				if (!IsIdentifierChar(c))
					return false;
			}
			if (prerelease && IsNumeric(id) && HasLeadingZero(id))
				return false;
			if (dot == std::string_view::npos)
				return true;
			s.remove_prefix(dot + 1);
		}
	}

	constexpr int CompareIdentifiers(std::string_view a, std::string_view b)
	{
		bool aNumeric = IsNumeric(a);
		bool bNumeric = IsNumeric(b);
		if (aNumeric != bNumeric)
			return aNumeric ? -1 : 1;
		// Without leading zeros, a longer number is a larger one.
		if (aNumeric && a.size() != b.size())
			return a.size() < b.size() ? -1 : 1;
		int result = a.compare(b);
		return result < 0 ? -1 : (result > 0 ? 1 : 0);
	}
}

// Accepts a leading "v", as release tags carry one. Build metadata is
// validated and then dropped, since it takes no part in precedence.
constexpr std::optional<SemVer> ParseSemVer(std::string_view text)
{
	using namespace semver_detail;

	if (!text.empty() && (text[0] == 'v' || text[0] == 'V'))
		text.remove_prefix(1);

	size_t plus = text.find('+');
	if (plus != std::string_view::npos) {
		if (!AreValidIdentifiers(text.substr(plus + 1), false))
			return std::nullopt;
		text = text.substr(0, plus);
	}

	SemVer version;
	size_t dash = text.find('-');
	if (dash != std::string_view::npos) {
		version.prerelease = text.substr(dash + 1);
		if (!AreValidIdentifiers(version.prerelease, true))
			return std::nullopt;
		text = text.substr(0, dash);
	}

	long long *core[] = { &version.major, &version.minor, &version.patch };
	for (int i = 0; i < 3; i++) {
		size_t dot = text.find('.');
		if ((i < 2) == (dot == std::string_view::npos))
			return std::nullopt;
		std::optional<long long> number = ParseNumber(text.substr(0, dot));
		if (!number)
			return std::nullopt;
		*core[i] = *number;
		text = i < 2 ? text.substr(dot + 1) : std::string_view();
	}
	return version;
}

// Negative, zero or positive as a has lower, equal or higher precedence than b.
constexpr int CompareSemVer(const SemVer &a, const SemVer &b)
{
	using namespace semver_detail;

	if (a.major != b.major)
		return a.major < b.major ? -1 : 1;
	if (a.minor != b.minor)
		return a.minor < b.minor ? -1 : 1;
	if (a.patch != b.patch)
		return a.patch < b.patch ? -1 : 1;

	if (a.prerelease.empty() || b.prerelease.empty())
		return a.prerelease.empty() - b.prerelease.empty();

	std::string_view left = a.prerelease;
	std::string_view right = b.prerelease;
	while (true) {
		size_t leftDot = left.find('.');
		size_t rightDot = right.find('.');
		int result = CompareIdentifiers(left.substr(0, leftDot), right.substr(0, rightDot));
		if (result != 0)
			return result;
		bool leftDone = leftDot == std::string_view::npos;
		bool rightDone = rightDot == std::string_view::npos;
		if (leftDone || rightDone)
			return rightDone - leftDone;
		left.remove_prefix(leftDot + 1);
		right.remove_prefix(rightDot + 1);
	}
}
