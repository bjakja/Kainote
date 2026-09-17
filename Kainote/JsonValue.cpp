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

#include "JsonValue.h"
#include <cstdlib>
#include <cstring>
#include <cctype>

namespace
{
	// Both caps exist because the input comes off the network: a hostile or
	// broken response must not be able to exhaust the stack or memory.
	constexpr size_t kMaxDepth = 64;
	constexpr size_t kMaxInput = 4u * 1024u * 1024u;

	void AppendUtf8(std::string &out, unsigned int cp)
	{
		if (cp < 0x80) {
			out += (char)cp;
		}
		else if (cp < 0x800) {
			out += (char)(0xC0 | (cp >> 6));
			out += (char)(0x80 | (cp & 0x3F));
		}
		else if (cp < 0x10000) {
			out += (char)(0xE0 | (cp >> 12));
			out += (char)(0x80 | ((cp >> 6) & 0x3F));
			out += (char)(0x80 | (cp & 0x3F));
		}
		else {
			out += (char)(0xF0 | (cp >> 18));
			out += (char)(0x80 | ((cp >> 12) & 0x3F));
			out += (char)(0x80 | ((cp >> 6) & 0x3F));
			out += (char)(0x80 | (cp & 0x3F));
		}
	}
}

class JsonParser
{
public:
	JsonParser(const std::string &input) : s(input) {}

	bool ParseValue(JsonValue &out, size_t depth)
	{
		if (depth > kMaxDepth)
			return false;

		SkipWhitespace();
		if (pos >= s.size())
			return false;

		switch (s[pos]) {
		case '{': return ParseObject(out, depth);
		case '[': return ParseArray(out, depth);
		case '"': {
			out.type = JsonValue::Type::String;
			return ParseString(out.text);
		}
		case 't':
			if (!Literal("true")) return false;
			out.type = JsonValue::Type::Bool;
			out.boolean = true;
			return true;
		case 'f':
			if (!Literal("false")) return false;
			out.type = JsonValue::Type::Bool;
			out.boolean = false;
			return true;
		case 'n':
			if (!Literal("null")) return false;
			out.type = JsonValue::Type::Null;
			return true;
		default:
			return ParseNumber(out);
		}
	}

	void SkipWhitespace()
	{
		while (pos < s.size() &&
			(s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r'))
		{
			pos++;
		}
	}

	size_t pos = 0;

private:
	const std::string &s;

	bool Literal(const char *word)
	{
		size_t len = strlen(word);
		if (s.compare(pos, len, word) != 0)
			return false;
		pos += len;
		return true;
	}

	bool ParseNumber(JsonValue &out)
	{
		size_t start = pos;
		if (pos < s.size() && (s[pos] == '-' || s[pos] == '+'))
			pos++;
		bool anyDigit = false;
		while (pos < s.size() &&
			(isdigit((unsigned char)s[pos]) || s[pos] == '.' ||
			 s[pos] == 'e' || s[pos] == 'E' || s[pos] == '-' || s[pos] == '+'))
		{
			if (isdigit((unsigned char)s[pos]))
				anyDigit = true;
			pos++;
		}
		if (!anyDigit)
			return false;

		out.type = JsonValue::Type::Number;
		out.number = strtod(s.substr(start, pos - start).c_str(), nullptr);
		return true;
	}

	bool ParseHex4(unsigned int &value)
	{
		if (pos + 4 > s.size())
			return false;
		value = 0;
		for (int i = 0; i < 4; i++) {
			char c = s[pos++];
			value <<= 4;
			if (c >= '0' && c <= '9') value |= (unsigned)(c - '0');
			else if (c >= 'a' && c <= 'f') value |= (unsigned)(c - 'a' + 10);
			else if (c >= 'A' && c <= 'F') value |= (unsigned)(c - 'A' + 10);
			else return false;
		}
		return true;
	}

	bool ParseString(std::string &out)
	{
		if (pos >= s.size() || s[pos] != '"')
			return false;
		pos++;

		while (pos < s.size()) {
			char c = s[pos++];
			if (c == '"')
				return true;

			if (c != '\\') {
				out += c;
				continue;
			}

			if (pos >= s.size())
				return false;
			char esc = s[pos++];
			switch (esc) {
			case '"':  out += '"';  break;
			case '\\': out += '\\'; break;
			case '/':  out += '/';  break;
			case 'b':  out += '\b'; break;
			case 'f':  out += '\f'; break;
			case 'n':  out += '\n'; break;
			case 'r':  out += '\r'; break;
			case 't':  out += '\t'; break;
			case 'u': {
				unsigned int cp = 0;
				if (!ParseHex4(cp))
					return false;
				// A high surrogate has to be paired with the low one that
				// follows, or release notes with emoji come out mangled.
				if (cp >= 0xD800 && cp <= 0xDBFF && pos + 1 < s.size() &&
					s[pos] == '\\' && s[pos + 1] == 'u')
				{
					size_t save = pos;
					pos += 2;
					unsigned int low = 0;
					if (ParseHex4(low) && low >= 0xDC00 && low <= 0xDFFF)
						cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
					else
						pos = save;
				}
				AppendUtf8(out, cp);
				break;
			}
			default:
				return false;
			}
		}
		return false;
	}

	bool ParseArray(JsonValue &out, size_t depth)
	{
		pos++;  // '['
		out.type = JsonValue::Type::Array;
		SkipWhitespace();
		if (pos < s.size() && s[pos] == ']') { pos++; return true; }

		for (;;) {
			JsonValue item;
			if (!ParseValue(item, depth + 1))
				return false;
			out.items.push_back(std::move(item));

			SkipWhitespace();
			if (pos >= s.size())
				return false;
			if (s[pos] == ',') { pos++; continue; }
			if (s[pos] == ']') { pos++; return true; }
			return false;
		}
	}

	bool ParseObject(JsonValue &out, size_t depth)
	{
		pos++;  // '{'
		out.type = JsonValue::Type::Object;
		SkipWhitespace();
		if (pos < s.size() && s[pos] == '}') { pos++; return true; }

		for (;;) {
			SkipWhitespace();
			std::string key;
			if (!ParseString(key))
				return false;

			SkipWhitespace();
			if (pos >= s.size() || s[pos] != ':')
				return false;
			pos++;

			JsonValue value;
			if (!ParseValue(value, depth + 1))
				return false;
			out.members[key] = std::move(value);

			SkipWhitespace();
			if (pos >= s.size())
				return false;
			if (s[pos] == ',') { pos++; continue; }
			if (s[pos] == '}') { pos++; return true; }
			return false;
		}
	}
};

JsonValue JsonValue::Parse(const std::string &utf8)
{
	JsonValue result;
	if (utf8.empty() || utf8.size() > kMaxInput)
		return result;

	JsonParser parser(utf8);
	JsonValue parsed;
	if (!parser.ParseValue(parsed, 0))
		return result;

	// Trailing content means we did not understand the document.
	parser.SkipWhitespace();
	if (parser.pos != utf8.size())
		return result;

	return parsed;
}

const JsonValue *JsonValue::Find(const std::string &key) const
{
	if (type != Type::Object)
		return nullptr;
	auto it = members.find(key);
	return it == members.end() ? nullptr : &it->second;
}

wxString JsonValue::AsString() const
{
	return type == Type::String ? wxString::FromUTF8(text.c_str()) : wxString();
}

wxString JsonValue::GetString(const std::string &key, const wxString &fallback) const
{
	const JsonValue *value = Find(key);
	if (!value || value->type != Type::String || value->text.empty())
		return fallback;
	return wxString::FromUTF8(value->text.c_str());
}

bool JsonValue::GetBool(const std::string &key, bool fallback) const
{
	const JsonValue *value = Find(key);
	return (value && value->type == Type::Bool) ? value->boolean : fallback;
}
