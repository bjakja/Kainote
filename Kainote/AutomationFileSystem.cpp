// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
// Copyright (c) 2016 - 2020, Marcin Drob
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
#include <windows.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace bfs = boost::filesystem;

typedef boost::filesystem::path path;




template<class T, class Del = void(*)(T)>
class scoped_holder {
	T value;
	Del destructor;

	scoped_holder(scoped_holder const&);
	scoped_holder& operator=(scoped_holder const&);
public:
	operator T() const { return value; }
	T operator->() const { return value; }

	scoped_holder& operator=(T new_value) {
		if (value){
			destructor=FindClose;
		}
		value = new_value;
		return *this;
	}

	scoped_holder(T value, Del destructor)
		: value(value)
		, destructor(destructor)
	{
	}
	scoped_holder(){};

	~scoped_holder() { if (value) destructor(value); }
};

wxString ErrorString(int error) {
	LPWSTR lpstr = nullptr;

	if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, 0, reinterpret_cast<LPWSTR>(&lpstr), 0, nullptr) == 0) {
		/// @todo Return the actual 'unknown error' string from windows.
		return "Unknown Error";
	}

	wxString str = wxString(lpstr);
	LocalFree(lpstr);
	return str;
}

FINDEX_INFO_LEVELS find_info_level() {
	OSVERSIONINFO osvi = {};
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	if (osvi.dwMajorVersion > 6 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 1))
		return FindExInfoBasic;
	else
		return FindExInfoStandard;
}


void Touch(path const &file) {
	bfs::create_directories(file.parent_path());

	SYSTEMTIME st;
	FILETIME ft;
	GetSystemTime(&st);
	if(!SystemTimeToFileTime(&st, &ft))
		throw wxString(L"SystemTimeToFileTime failed with error: " + ErrorString(GetLastError()));

	scoped_holder<HANDLE, BOOL (__stdcall *)(HANDLE)>
		h(CreateFile(file.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr), CloseHandle);
	// error handling etc.
	if (!SetFileTime(h, nullptr, nullptr, &ft))
		throw wxString(L"SetFileTime failed with error: " + ErrorString(GetLastError()));
}

void Copy(wxString const& from, wxString const& to) {
	bfs::is_regular_file(from.wc_str());
	bfs::create_directories(to.wc_str());
	bfs::is_directory(to.wc_str());

	if (!CopyFile(from.wc_str(), to.wc_str(), false)) {
		switch (GetLastError()) {
		case ERROR_FILE_NOT_FOUND:
			throw wxString(L"File not found: " + from + L".");
		case ERROR_ACCESS_DENIED:
			throw wxString(L"Could not overwrite " + to + L".");
		default:
			throw wxString(L"Could not copy: " + ErrorString(GetLastError()));
		}
	}
}

class DirectoryIterator {
	//struct PrivData;
	//std::shared_ptr<PrivData> privdata;
	//PrivData *privdata;
	HANDLE handle;
public:
	std::string value;
	typedef path value_type;
	typedef path* pointer;
	typedef path& reference;
	typedef size_t difference_type;
	typedef std::forward_iterator_tag iterator_category;

	bool operator==(DirectoryIterator const&) const;
	bool operator!=(DirectoryIterator const& rhs) const { return !(*this == rhs); }
	DirectoryIterator& operator++();
	std::string const& operator*() const { return value; }

	DirectoryIterator(path const& p, std::string const& filter);
	DirectoryIterator();
	~DirectoryIterator();

	template<typename T> void GetAll(T& cont);
};

static inline DirectoryIterator& begin(DirectoryIterator &it) { return it; }
static inline DirectoryIterator end(DirectoryIterator &) { return DirectoryIterator(); }

template<typename T>
inline void DirectoryIterator::GetAll(T& cont) {
	copy(*this, end(*this), std::back_inserter(cont));
}

//struct DirectoryIterator::PrivData {
//	scoped_holder<HANDLE, BOOL (__stdcall *)(HANDLE)> h;//{INVALID_HANDLE_VALUE, FindClose};
//};

DirectoryIterator::DirectoryIterator() {handle = nullptr; }
DirectoryIterator::DirectoryIterator(path const& p, std::string const& filter)
//: privdata(new PrivData)
{
	handle = nullptr;
	WIN32_FIND_DATA data;
	handle = FindFirstFileEx((p/(filter.empty() ? "*.*" : filter)).c_str(), find_info_level(), &data, FindExSearchNameMatch, nullptr, 0);
	if (handle == INVALID_HANDLE_VALUE) {
		//privdata.reset();
		//if(privdata){delete privdata; privdata=nullptr;}
		handle = nullptr;
		return;
	}

	value = wxString(data.cFileName);
	while (value[0] == '.' && (value[1] == 0 || value[1] == '.'))
		++*this;
}

bool DirectoryIterator::operator==(DirectoryIterator const& rhs) const {
	return handle == rhs.handle;
}

DirectoryIterator& DirectoryIterator::operator++() {
	WIN32_FIND_DATA data;
	if (handle && FindNextFile(handle, &data))
		value = wxString(data.cFileName);
	else {
		//if(privdata){privdata.reset();}
		//if(privdata){delete privdata; privdata=nullptr;}
		handle = nullptr;
		value.clear();
	}
	return *this;
}

DirectoryIterator::~DirectoryIterator() { if (handle){ FindClose(handle); handle=nullptr; }}

template<typename Func>
auto wrap(char **err, Func f) -> decltype(f()) {
	try {
		return f();
	}
	catch (std::exception const& e) {
		*err = _strdup(e.what());
	}
	catch (wxString const& e) {
		*err = _strdup(e.utf8_str());
	}
	catch (...) {
		*err = _strdup("Unknown error");
	}
	return 0;
}

template<typename Ret>
bool setter(const char *path, char **err, Ret (*f)(bfs::path const&)) {
	return wrap(err, [=]{
		f(path);
		return true;
	});
}

bool lfs_chdir(const char *dir, char **err) {
	return setter(dir, err, &bfs::current_path);
}

char *currentdir(char **err) {
	return wrap(err, []{
		return strndup(bfs::current_path().string());
	});
}

bool mkdir(const char *dir, char **err) {
	return setter(dir, err, &bfs::create_directories);
}

bool lfs_rmdir(const char *dir, char **err) {
	return setter(dir, err, &bfs::remove);
}

bool touch(const char *path, char **err) {
	return setter(path, err, &Touch);
}

char *dir_next(DirectoryIterator &it, char **err) {
	if (it == end(it)) return nullptr;
	return wrap(err, [&]{
		auto str = strndup(*it);
		++it;
		return str;
	});
}

void dir_close(DirectoryIterator &it) {
	it = DirectoryIterator();
}

void dir_free(DirectoryIterator *it) {
	delete it;
}

DirectoryIterator *dir_new(const char *path, char **err) {
	return wrap(err, [=]{
		return new DirectoryIterator(path, "");
	});
}

char *get_mode(const char *path, char **err) {
	return wrap(err, [=]() -> char * {
		switch (bfs::status(path).type()) {
		case bfs::file_not_found: return nullptr;         break;
		case bfs::regular_file:   return strndup(wxString("file"));          break;
		case bfs::directory_file: return strndup(wxString("directory"));     break;
		case bfs::symlink_file:   return strndup(wxString("link"));          break;
		case bfs::block_file:     return strndup(wxString("block device"));  break;
		case bfs::character_file: return strndup(wxString("char device"));   break;
		case bfs::fifo_file:      return strndup(wxString("fifo"));          break;
		case bfs::socket_file:    return strndup(wxString("socket"));        break;
		case bfs::reparse_file:   return strndup(wxString("reparse point")); break;
		default:                  return strndup(wxString("other"));         break;
		}
	});
}

long long get_mtime(const char *path, char **err) {
	return wrap(err, [=] { return bfs::last_write_time(path); });
}

unsigned long long get_size(const char *path, char **err) {
	return wrap(err, [=] { 
		if (bfs::is_directory(path))
			throw "Not a file";
		return bfs::file_size(path); 
	});
}

extern "C" int luaopen_lfs_impl(lua_State *L) {
	std::vector<const char *> types;
	types.push_back("DirectoryIterator");
	//types.push_back("time_t");
	//types.push_back("uintmax_t");
	do_register_lib_table(L, types);
	lua_createtable(L, 0, 12);
	do_register_lib_function(L, "chdir", "bool (*)(const char *, char **)", lfs_chdir);
	do_register_lib_function(L, "currentdir", "char * (*)(char **)", currentdir);
	do_register_lib_function(L, "mkdir", "bool (*)(const char *, char **)", mkdir);
	do_register_lib_function(L, "rmdir", "bool (*)(const char *, char **)", lfs_rmdir);
	do_register_lib_function(L, "touch", "bool (*)(const char *, char **)", touch);
	do_register_lib_function(L, "get_mtime", "long long (*)(const char *, char **)", get_mtime);
	do_register_lib_function(L, "get_mode", "char * (*)(const char *, char **)", get_mode);
	do_register_lib_function(L, "get_size", "unsigned long long (*)(const char *, char **)", get_size);
	do_register_lib_function(L, "dir_new", "DirectoryIterator * (*)(const char *, char **)", dir_new);
	do_register_lib_function(L, "dir_free", "void (*)(DirectoryIterator *)", dir_free);
	do_register_lib_function(L, "dir_next", "char * (*)(DirectoryIterator &, char **)", dir_next);
	do_register_lib_function(L, "dir_close", "void (*)(DirectoryIterator &)", dir_close);
	lua_remove(L, -2);
		
	return 1;
}

