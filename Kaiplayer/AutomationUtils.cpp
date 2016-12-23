// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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
//
// Aegisub Project http://www.aegisub.org/

#include "AutomationUtils.h"
#include "AutomationLPeg.h"


#include <cstdlib>
#include <iterator>
#include <wx/wx.h>
#include <wx/regex.h>

#include <boost/regex/icu.hpp>
#include <boost/locale/conversion.hpp>

#ifdef _MSC_VER
// Disable warnings for noreturn functions having return types
#pragma warning(disable: 4645 4646)
#endif

extern "C" int luaopen_luabins(lua_State *L);
extern "C" int luaopen_re_impl(lua_State *L);
extern "C" int luaopen_unicode_impl(lua_State *L);
extern "C" int luaopen_lfs_impl(lua_State *L);    
extern "C" int luaopen_lpeg(lua_State *L);

namespace Auto{
wxString get_string_or_default(lua_State *L, int idx) {
	size_t len = 0;
	const char *str = lua_tolstring(L, idx, &len);
	if (!str)
		return "<not a string>";
	return wxString(str, wxConvUTF8, len);
}

wxString get_string(lua_State *L, int idx) {
	if (lua_isstring(L, -1))
		return wxString(lua_tostring(L, -1),wxConvUTF8);

	return "";
}

wxString get_global_string(lua_State *L, const char *name) {
	wxString ret;
	lua_getglobal(L, name);
	if (lua_isstring(L, -1))
		ret = wxString(lua_tostring(L, -1), wxConvUTF8);
	lua_pop(L, 1);
	return ret;
}

wxString check_string(lua_State *L, int idx) {
	size_t len = 0;
	const char *str = lua_tolstring(L, idx, &len);
	if (!str) typerror(L, idx, "string");
	return wxString(str, wxConvUTF8, len);
}

int check_int(lua_State *L, int idx) {
	auto v = lua_tointeger(L, idx);
	if (v == 0 && !lua_isnumber(L, idx))
		typerror(L, idx, "number");
	return v;
}

size_t check_uint(lua_State *L, int idx) {
	auto v = lua_tointeger(L, idx);
	if (v == 0 && !lua_isnumber(L, idx))
		typerror(L, idx, "number");
	if (v < 0)
		argerror(L, idx, "must be >= 0");
	return static_cast<size_t>(v);
}

void *check_udata(lua_State *L, int idx, const char *mt) {
	void *p = lua_touserdata(L, idx);
	if (!p) typerror(L, idx, mt);
	if (!lua_getmetatable(L, idx)) typerror(L, idx, mt);

	lua_getfield(L, LUA_REGISTRYINDEX, mt);
	if (!lua_rawequal(L, -1, -2)) typerror(L, idx, mt);

	lua_pop(L, 2);
	return p;
}

static int moon_line(lua_State *L, int lua_line, wxString const& file) {
	if (luaL_dostring(L, "return require 'moonscript.line_tables'")) {
		lua_pop(L, 1); // pop error message
		return lua_line;
	}

	push_value(L, file);
	lua_rawget(L, -2);

	if (!lua_istable(L, -1)) {
		lua_pop(L, 2);
		return lua_line;
	}

	lua_rawgeti(L, -1, lua_line);
	if (!lua_isnumber(L, -1)) {
		lua_pop(L, 3);
		return lua_line;
	}

	auto char_pos = static_cast<size_t>(lua_tonumber(L, -1));
	lua_pop(L, 3);

	// The moonscript line tables give us a character offset into the file,
	// so now we need to map that to a line number
	lua_getfield(L, LUA_REGISTRYINDEX, ("raw moonscript: " + file).c_str());
	if (!lua_isstring(L, -1)) {
		lua_pop(L, 1);
		return lua_line;
	}

	size_t moon_len;
	auto moon = lua_tolstring(L, -1, &moon_len);
	return std::count(moon, moon + std::min(moon_len, char_pos), '\n') + 1;
}

int add_stack_trace(lua_State *L) {
	int level = 1;
	if (lua_isnumber(L, 2)) {
		level = (int)lua_tointeger(L, 2);
		lua_pop(L, 1);
	}

	const char *err = lua_tostring(L, 1);
	if (!err) return 1;

	wxString message = err;
	if (lua_gettop(L))
		lua_pop(L, 1);

	// Strip the location from the error message since it's redundant with
	// the stack trace
	wxRegEx rx("(^\\[string (.*)\\]:[0-9]+: )");
	rx.Replace(&message,"");

	wxString frames;

	lua_Debug ar;
	while (lua_getstack(L, level++, &ar)) {
		lua_getinfo(L, "Snl", &ar);

		if (ar.what[0] == 't')
			frames << "(tail call)";
		else {
			bool is_moon = false;
			wxString file = ar.source;
			if (file == "=[C]")
				file = "<C function>";
			else if (file.EndsWith(".moon"))
				is_moon = true;

			auto real_line = [&](int line) {
				return is_moon ? moon_line(L, line, file) : line;
			};

			wxString function = ar.name ? ar.name : "";
			if (*ar.what == 'm')
				function += " <main>";
			else if (*ar.what == 'C')
				function += '?';
			else if (!*ar.namewhat)
				function += wxString::Format(" <anonymous function at lines %d-%d>", real_line(ar.linedefined), real_line(ar.lastlinedefined - 1));

			frames << wxString::Format("File \"%s\", line %d %s\n%s\n\n", file, real_line(ar.currentline), function, message);
		}
	}
	
	push_value(L, frames);

	return 1;
}

int error(lua_State *L, const char *fmt, ...) {
	va_list argp;
	va_start(argp, fmt);
	luaL_where(L, 1);
	lua_pushvfstring(L, fmt, argp);
	va_end(argp);
	lua_concat(L, 2);
	throw error_tag();
	//return lua_error(L);
}

int argerror(lua_State *L, int narg, const char *extramsg) {
	lua_Debug ar;
	if (!lua_getstack(L, 0, &ar))
		error(L, "bad argument #%d (%s)", narg, extramsg);
	lua_getinfo(L, "n", &ar);
	if (strcmp(ar.namewhat, "method") == 0 && --narg == 0)
		error(L, "calling '%s' on bad self (%s)", ar.name, extramsg);
	if (!ar.name) ar.name = "?";
	error(L, "bad argument #%d to '%s' (%s)",
		narg, ar.name, extramsg);
	return 0;
}

int typerror(lua_State *L, int narg, const char *tname) {
	const char *msg = lua_pushfstring(L, "%s expected, got %s",
		tname, luaL_typename(L, narg));
	argerror(L, narg, msg);
	return 0;
}

void argcheck(lua_State *L, bool cond, int narg, const char *msg) {
	if (!cond) argerror(L, narg, msg);
}

int exception_wrapper(lua_State *L, int (*func)(lua_State *L)) {
	try {
		return func(L);
	}
	catch (std::exception const& e) {
		push_value(L, e.what());
		return lua_error(L);
	}
	catch (error_tag) {
		// Error message is already on the stack
		return lua_error(L);
	}
	catch (...) {
		push_value(L, "Lua fatal error");
		return lua_error(L);
	}
}

#ifdef _DEBUG
void LuaStackcheck::check_stack(int additional) {
	int top = lua_gettop(L);
	if (top - additional != startstack) {
		//wxLogStatus("automation/lua: lua stack size mismatch." );
		dump();
		//assert(top - additional == startstack); 
	}
}

void LuaStackcheck::dump() {
	int top = lua_gettop(L);
	//wxLogStatus("automation/lua/stackdump: dumping lua stack...");
	for (int i = top; i > 0; i--) {
		lua_pushvalue(L, i);
		wxString type(lua_typename(L, lua_type(L, -1)));
		//if (lua_isstring(L, i))
			//wxLogStatus("automation/lua/stackdump %s: %s",type, lua_tostring(L, -1));
		//else
			//wxLogStatus("automation/lua/stackdump: %s", type);
		lua_pop(L, 1);
	}
	//wxLogStatus("automation/lua: --- end dump");
}

#endif

int regex_init(lua_State *L);

//extern "C" int luaopen_lpeg(lua_State *L);
void preload_modules(lua_State *L) {
	luaL_openlibs(L);

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	set_field(L, "kainote.__re_impl", luaopen_re_impl);
	set_field(L, "kainote.__unicode_impl", luaopen_unicode_impl);
	set_field(L, "kainote.__lfs_impl", luaopen_lfs_impl);
	set_field(L, "lpeg", luaopen_lpeg);
	set_field(L, "luabins", luaopen_luabins);

	lua_pop(L, 2);

	//register_lib_functions(L); // silence an unused static function warning
}

void do_register_lib_function(lua_State *L, const char *name, const char *type_name, void *func) {
	lua_pushvalue(L, -2); // push cast function
	lua_pushstring(L, type_name);
	lua_pushlightuserdata(L, func);
	lua_call(L, 2, 1);
	lua_setfield(L, -2, name);
}

void do_register_lib_table(lua_State *L, std::vector<const char *> types) {
	lua_getglobal(L, "require");
	lua_pushstring(L, "ffi");
	lua_call(L, 1, 1);

	// Register all passed type with the ffi
	for (auto type : types) {
		lua_getfield(L, -1, "cdef");
		lua_pushfstring(L, "typedef struct %s %s;", type, type);
		lua_call(L, 1, 0);
	}

	lua_getfield(L, -1, "cast");
	lua_remove(L, -2); // ffi table

	// leaves ffi.cast on the stack
}


char *wrapu(const char *str, char **err) {
	try {
		return strndup(boost::locale::to_upper(str));
	} catch (std::exception const& e) {
		wxLogStatus("uppercase error: " + wxString(e.what()));
		*err = _strdup(e.what());
		return nullptr;
	}
}

char *wrapl(const char *str, char **err) {
	try {
		return strndup(boost::locale::to_lower(str));
	} catch (std::exception const& e) {
		wxLogStatus("lowercase error: " + wxString(e.what()));
		*err = _strdup(e.what());
		return nullptr;
	}
}
char *wrapf(const char *str, char **err) {
	try {
		return strndup(boost::locale::fold_case(str));
	} catch (std::exception const& e) {
		wxLogStatus("foldcase error: " + wxString(e.what()));
		*err = _strdup(e.what());
		return nullptr;
	}
}

extern "C" int luaopen_unicode_impl(lua_State *L) {
	do_register_lib_table(L, std::vector<const char *>());
	lua_createtable(L, 0, 3);
	do_register_lib_function(L, "to_upper_case", "char * (*)(const char *, char **)", wrapu);
	do_register_lib_function(L, "to_lower_case", "char * (*)(const char *, char **)", wrapl);
	do_register_lib_function(L, "to_fold_case", "char * (*)(const char *, char **)", wrapf);
	lua_remove(L, -2); // ffi.cast function

	return 1;
}



using boost::u32regex;

// A cmatch with a match range attached to it so that we can return a pointer to
// an int pair without an extra heap allocation each time (LuaJIT can't compile
// ffi calls which return aggregates by value)
struct agi_re_match {
	boost::cmatch m;
	int range[2];
};

struct agi_re_flag {
	const char *name;
	int value;
};



	//AGI_DEFINE_TYPE_NAME(u32regex);
	//AGI_DEFINE_TYPE_NAME(agi_re_match);
	//AGI_DEFINE_TYPE_NAME(agi_re_flag);


//using match = agi_re_match;
bool search(u32regex& re, const char *str, size_t len, int start, boost::cmatch& result) {
	return u32regex_search(str + start, str + len, result, re,
		start > 0 ? boost::match_prev_avail | boost::match_not_bob : boost::match_default);
}

agi_re_match *regex_match(u32regex& re, const char *str, size_t len, int start) {
	std::unique_ptr<agi_re_match> result(new agi_re_match);
	if (!search(re, str, len, start, result->m))
		return nullptr;
	return result.release();
}

int *regex_get_match(agi_re_match& match, size_t idx) {
	if (idx > match.m.size() || !match.m[idx].matched)
		return nullptr;
	match.range[0] = std::distance(match.m.prefix().first, match.m[idx].first + 1);
	match.range[1] = std::distance(match.m.prefix().first, match.m[idx].second);
	return match.range;
}

int *regex_search(u32regex& re, const char *str, size_t len, size_t start) {
	boost::cmatch result;
	if (!search(re, str, len, start, result))
		return nullptr;

	auto ret = static_cast<int *>(malloc(sizeof(int) * 2));
	ret[0] = start + result.position() + 1;
	ret[1] = start + result.position() + result.length();
	return ret;
}

char *regex_replace(u32regex& re, const char *replacement, const char *str, size_t len, int max_count) {
	// Can't just use regex_replace here since it can only do one or infinite replacements
	auto match = boost::u32regex_iterator<const char *>(str, str + len, re);
	auto end_it = boost::u32regex_iterator<const char *>();

	auto suffix = str;

	std::string ret;
	auto out = std::back_inserter(ret);
	while (match != end_it && max_count > 0) {
		std::copy(suffix, match->prefix().second, out);
		match->format(out, replacement);
		suffix = match->suffix().first;
		++match;
		--max_count;
	}

	ret += suffix;
	return strndup(ret);
}

u32regex *regex_compile(const char *pattern, int flags, char **err) {
	std::unique_ptr<u32regex> re(new u32regex);//new u32regex;//
	try {
		*re = boost::make_u32regex(pattern, boost::regex::perl | flags);
		return re.release();
	}
	catch (std::exception const& e) {
		*err = _strdup(e.what());
		return nullptr;
	}
}

void regex_free(u32regex *re) { delete re; }
void match_free(agi_re_match *m) { delete m; }

const agi_re_flag *get_regex_flags() {
	static const agi_re_flag flags[] = {
		{"ICASE", boost::u32regex::icase},
		{"NOSUB", boost::u32regex::nosubs},
		{"COLLATE", boost::u32regex::collate},
		{"NEWLINE_ALT", boost::u32regex::newline_alt},
		{"NO_MOD_M", boost::u32regex::no_mod_m},
		{"NO_MOD_S", boost::u32regex::no_mod_s},
		{"MOD_S", boost::u32regex::mod_s},
		{"MOD_X", boost::u32regex::mod_x},
		{"NO_EMPTY_SUBEXPRESSIONS", boost::u32regex::no_empty_expressions},
		{nullptr, 0}
	};
	return flags;
}


extern "C" int luaopen_re_impl(lua_State *L) {
	std::vector<const char *> types;
	types.push_back("agi_re_match");
	types.push_back("u32regex");

	do_register_lib_table(L, types);
	lua_createtable(L, 0, 8);
	do_register_lib_function(L, "search", "int * (*)(u32regex&, const char *, size_t, size_t)", regex_search);
	do_register_lib_function(L, "match", "agi_re_match * (*)(u32regex&, const char *, size_t, int)", regex_match);
	do_register_lib_function(L, "get_match", "int * (*)(agi_re_match&, size_t)", regex_get_match);
	do_register_lib_function(L, "replace", "char * (*)(u32regex&, const char *, const char *, size_t, int)", regex_replace);
	do_register_lib_function(L, "compile", "u32regex * (*)(const char *, int, char **)", regex_compile);
	do_register_lib_function(L, "get_flags", "const agi_re_flag * (*)()", get_regex_flags);
	do_register_lib_function(L, "match_free", "void (*)(agi_re_match *)", match_free);
	do_register_lib_function(L, "regex_free", "void (*)(u32regex *)", regex_free);
	lua_remove(L, -2); // ffi.cast function
	
	return 1;
}


}



