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

#include <cstddef>
#include <functional>
#include <string_view>
#include <vector>

inline size_t ScriptTextHash(std::wstring_view text)
{
	return std::hash<std::wstring_view>()(text);
}

// The last scripts parsed, by a hash of their text: editing switches between
// the whole script and one of the edited line, and switching back to one that
// has not changed then needs no parse.
template <typename T>
class ParsedScripts
{
public:
	explicit ParsedScripts(void (*release)(T*), size_t capacity = 3)
		: m_release(release), m_capacity(capacity) {}
	~ParsedScripts() { Clear(); }
	ParsedScripts(const ParsedScripts &) = delete;
	ParsedScripts &operator=(const ParsedScripts &) = delete;

	T *Find(size_t hash)
	{
		for (size_t i = 0; i < m_items.size(); i++) {
			if (m_items[i].hash == hash) {
				Entry found = m_items[i];
				m_items.erase(m_items.begin() + i);
				m_items.insert(m_items.begin(), found);
				return found.item;
			}
		}
		return nullptr;
	}
	// Past capacity the least recently used one goes, but never inUse.
	void Add(size_t hash, T *item, T *inUse)
	{
		m_items.insert(m_items.begin(), Entry{ hash, item });
		for (size_t i = m_items.size(); m_items.size() > m_capacity && i-- > 0;) {
			if (m_items[i].item == inUse)
				continue;
			m_release(m_items[i].item);
			m_items.erase(m_items.begin() + i);
		}
	}
	void Remove(T *item)
	{
		for (size_t i = 0; i < m_items.size(); i++) {
			if (m_items[i].item == item) {
				m_release(item);
				m_items.erase(m_items.begin() + i);
				return;
			}
		}
	}
	void Clear()
	{
		for (Entry &entry : m_items)
			m_release(entry.item);
		m_items.clear();
	}
	size_t Size() const { return m_items.size(); }

private:
	struct Entry { size_t hash; T *item; };
	std::vector<Entry> m_items;
	void (*m_release)(T*);
	size_t m_capacity;
};
