/***************************************************************
 * Copyright (c) 2012 - 2026, Marcin Drob
 *
 * Kainote is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Kainote is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Kainote.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************/

// Just enough JSON to read GitHub's releases API. wx ships no JSON parser and
// Thirdparty/boost is only hydrated on Windows, so there is nothing in tree to
// reuse.

#pragma once

#include <wx/string.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

class JsonValue
{
public:
	enum class Type { Null, Bool, Number, String, Array, Object };

	JsonValue() = default;

	// Returns a Null value if the input is malformed, too deeply nested or
	// larger than the cap. Callers check the type rather than an error code.
	static JsonValue Parse(const std::string &utf8);

	Type GetType() const { return type; }
	bool IsNull() const { return type == Type::Null; }

	// Array access. Empty for anything that is not an array.
	const std::vector<JsonValue> &Items() const { return items; }

	// Object member, or nullptr when absent or when this is not an object.
	const JsonValue *Find(const std::string &key) const;

	// Typed member accessors, each falling back when the member is missing or
	// of the wrong type.
	wxString GetString(const std::string &key, const wxString &fallback = wxString()) const;
	bool     GetBool(const std::string &key, bool fallback = false) const;

	wxString AsString() const;

private:
	Type type = Type::Null;
	bool boolean = false;
	double number = 0.0;
	std::string text;
	std::vector<JsonValue> items;
	std::map<std::string, JsonValue> members;

	friend class JsonParser;
};
