// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
// Copyright (c) 2016, Marcin Drob
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


extern "C" {
#include <lua.hpp>
}
#include <algorithm>
//#include <boost/range/irange.hpp>
//#include <string>
#include <vector>
#include <memory>
//#include <type_traits>
#include <wx/wx.h>

struct tm;


namespace Auto{
	
	/// Clamp `b` to the range [`a`,`c`]
	template<typename T>
	static inline T mid(T a, T b, T c) {
		return std::max(a, std::min(b, c));
	}

	bool try_parse(wxString const& str, double *out);
	bool try_parse(wxString const& str, int *out);

	/// strftime, but on wxString rather than a fixed buffer
	/// @param fmt strftime format string
	/// @param tmptr Time to format, or nullptr for current time
	/// @return The strftime-formatted string
	wxString strftime(const char *fmt, const tm *tmptr = nullptr);

	/// Case-insensitive find with proper case folding
	/// @param haystack String to search
	/// @param needle String to look for
	/// @return make_pair(-1,-1) if `needle` could not be found, or a range equivalent to `needle` in `haystack` if it could
	///
	/// `needle` and `haystack` must both be in Normalization Form D. The size
	/// of the match might be different from the size of `needle`, since it's
	/// based on the unfolded length.
	std::pair<size_t, size_t> ifind(wxString const& haystack, wxString const& needle);

	class tagless_find_helper {
		std::vector<std::pair<size_t, size_t>> blocks;
		size_t start;
		//upewniæ siê, ¿e ta klase bêdzie mia³a start na 0;
		tagless_find_helper(){start=0;}
	public:
		/// Strip ASS override tags at or after `start` in `str`, and initialize
		/// state for mapping ranges back to the input string
		wxString strip_tags(wxString const& str, size_t start);

		/// Convert a range in the string returned by `strip_tags()` to a range
		/// int the string last passed to `strip_tags()`
		void map_range(size_t& start, size_t& end);
	};

	/// Set the name of the calling thread in the Visual Studio debugger
	/// @param name New name for the thread
	void SetThreadName(const char *name);

	/// A thin wrapper around this_thread::sleep_for that uses std::thread on
	/// Windows (to avoid having to compile boost.thread) and boost::thread
	/// elsewhere (because libstcc++ 4.7 is missing it).
	void sleep_for(int ms);

	// boost.range doesn't have wrappers for the C++11 stuff
	template<typename Range, typename Predicate>
	bool any_of(Range&& r, Predicate&& p) {
		return std::any_of(std::begin(r), std::end(r), std::forward<Predicate>(p));
	}

	wxString ErrorString(int error);

	template<typename Integer>
	auto range(Integer end) -> decltype(boost::irange<Integer>(0, end)) {
		return boost::irange<Integer>(0, end);
	}
	//Modules 
	void preload_modules(lua_State *L);
	//ffi
	void do_register_lib_function(lua_State *L, const char *name, const char *type_name, void *func);
	void do_register_lib_table(lua_State *L, std::vector<const char *> types);

	//static void register_lib_functions(lua_State *) {
	//	// Base case of recursion; nothing to do
	//}

	//template<typename Func, typename... Rest>
	//void register_lib_functions(lua_State *L, const char *name, Func *func, Rest... rest) {
	//	// This cast isn't legal, but LuaJIT internally requires that it work, so we can rely on it too
	//	do_register_lib_function(L, name, type_name<Func*>::name().c_str(), (void *)func);
	//	register_lib_functions(L, rest...);
	//}

	//template<typename... Args>
	//void register_lib_table(lua_State *L, std::vector<const char *> types, Args... functions) {
	//	static_assert((sizeof...(functions) & 1) == 0, "Functions must be alternating names and function pointers");

	//	do_register_lib_table(L, types); // leaves ffi.cast on the stack
	//	lua_createtable(L, 0, sizeof...(functions) / 2);
	//	register_lib_functions(L, functions...);
	//	lua_remove(L, -2); // ffi.cast function
	//	// Leaves lib table on the stack
	//}

	template<typename T>
	char *strndup(T const& str) {
		char *ret = static_cast<char*>(malloc(str.size() + 1));
		memcpy(ret, str.data(), str.size());
		ret[str.size()] = 0;
		return ret;
	}

	//second utils

	struct error_tag {};

	// Below are functionally equivalent to the luaL_ functions, but using a C++
	// exception for stack unwinding
	int error(lua_State *L, const char *fmt, ...);
	int argerror(lua_State *L, int narg, const char *extramsg);
	int typerror(lua_State *L, int narg, const char *tname);
	void argcheck(lua_State *L, bool cond, int narg, const char *msg);

	inline void push_value(lua_State *L, bool value) { lua_pushboolean(L, value); }
	inline void push_value(lua_State *L, const char *value) { lua_pushstring(L, value); }
	inline void push_value(lua_State *L, double value) { lua_pushnumber(L, value); }
	inline void push_value(lua_State *L, int value) { lua_pushinteger(L, value); }
	inline void push_value(lua_State *L, void *p) { lua_pushlightuserdata(L, p); }

	template<typename Integer>
	typename std::enable_if<std::is_integral<Integer>::value>::type
	push_value(lua_State *L, Integer value) {
		lua_pushinteger(L, static_cast<lua_Integer>(value));
	}

	inline void push_value(lua_State *L, wxString const& value) {
		lua_pushlstring(L, value.c_str(), value.size());
	}

	inline void push_value(lua_State *L, lua_CFunction value) {
		if (lua_gettop(L) >= 2 && lua_type(L, -2) == LUA_TUSERDATA) {
			lua_pushvalue(L, -2);
			lua_pushcclosure(L, value, 1);
		}
		else
			lua_pushcclosure(L, value, 0);
	}

	template<typename T>
	void push_value(lua_State *L, std::vector<T> const& value) {
		lua_createtable(L, value.size(), 0);
		for (size_t i = 0; i < value.size(); ++i) {
			push_value(L, value[i]);
			lua_rawseti(L, -2, i + 1);
		}
	}

	int exception_wrapper(lua_State *L, int (*func)(lua_State *L));
	/// Wrap a function which may throw exceptions and make it trigger lua errors
	/// whenever it throws
	template<int (*func)(lua_State *L)>
	int exception_wrapper(lua_State *L) {
		return exception_wrapper(L, func);
	}

	template<typename T>
	void set_field(lua_State *L, const char *name, T value) {
		push_value(L, value);
		lua_setfield(L, -2, name);
	}

	template<int (*func)(lua_State *L)>
	void set_field(lua_State *L, const char *name) {
		push_value(L, exception_wrapper<func>);
		lua_setfield(L, -2, name);
	}

	wxString get_string_or_default(lua_State *L, int idx);
	wxString get_string(lua_State *L, int idx);
	wxString get_global_string(lua_State *L, const char *name);

	wxString check_string(lua_State *L, int idx);
	int check_int(lua_State *L, int idx);
	size_t check_uint(lua_State *L, int idx);
	void *check_udata(lua_State *L, int idx, const char *mt);

	/*template<typename T, typename... Args>
	T *make(lua_State *L, const char *mt, Args&&... args) {
		auto obj = static_cast<T*>(lua_newuserdata(L, sizeof(T)));
		new(obj) T(std::forward<Args>(args)...);
		luaL_getmetatable(L, mt);
		lua_setmetatable(L, -2);
		return obj;
	}*/

	template<typename T>
	T& get(lua_State *L, int idx, const char *mt) {
		return *static_cast<T *>(check_udata(L, idx, mt));
	}

	#ifdef _DEBUG
	struct LuaStackcheck {
		lua_State *L;
		int startstack;

		void check_stack(int additional);
		void dump();

		LuaStackcheck(lua_State *L) : L(L), startstack(lua_gettop(L)) { }
		~LuaStackcheck() { check_stack(0); }
	};
	#else
	struct LuaStackcheck {
		void check_stack(int) { }
		void dump() { }
		LuaStackcheck(lua_State*) { }
	};
	#endif

	struct LuaForEachBreak {};

	template<typename Func>
	void lua_for_each(lua_State *L, Func&& func) {
		{
			LuaStackcheck stackcheck(L);
			lua_pushnil(L); // initial key
			while (lua_next(L, -2)) {
				try {
					func();
				}
				catch (LuaForEachBreak) {
					lua_pop(L, 2); // pop value and key
					break;
				}
				lua_pop(L, 1); // pop value, leave key
			}
			stackcheck.check_stack(0);
		}
		lua_pop(L, 1); // pop table
	}

	/*template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}*/

	/// Lua error handler which adds the stack trace to the error message, with
	/// moonscript line rewriting support
	int add_stack_trace(lua_State *L);

}
