#pragma once

// Cross-platform shim for the Linux build.  The original project is Windows/MSVC
// first; keep Windows behaviour untouched and provide small substitutes only on
// non-Windows platforms.
#ifndef _WIN32
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <ctime>
#include <cwctype>
#include <chrono>
#include <cctype>
#include <string>
#include <filesystem>
#include <cstdio>
#include <fstream>
#include <locale>
#include <codecvt>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <poll.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cerrno>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cfloat>
#include <limits>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <memory>
#include <map>
#include <set>
#include <sys/wait.h>
#include <fontconfig/fontconfig.h>
#include <wx/app.h>
#include <wx/defs.h>
#include <wx/window.h>
#include <wx/panel.h>
#include <wx/intl.h>
#include <wx/power.h>
#include <wx/utils.h>

#ifndef __linux__
#define __linux__ 1
#endif

#ifndef __WXGTK__
#define __WXGTK__ 1
#endif

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif
#ifndef _MAX_PATH
#define _MAX_PATH MAX_PATH
#endif

#ifndef __stdcall
#define __stdcall
#endif
#ifndef _stdcall
#define _stdcall
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef CALLBACK
#define CALLBACK
#endif

using BYTE = unsigned char;
using WORD = std::uint16_t;
using DWORD = std::uint32_t;
using UINT = unsigned int;
// Win32 LONG, ULONG, and HRESULT remain 32-bit on LP64 hosts.
using ULONG = std::uint32_t;
using LONG = std::int32_t;
using BOOL = int;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
using BOOLEAN = int;
using VOID = void;
using HRESULT = std::int32_t;
using WCHAR = wchar_t;
using LPCWSTR = const wchar_t*;
using LPWSTR = wchar_t*;
using LPWORD = WORD*;
using LPCSTR = const char*;
using HWND = void*;
using HDC = void*;
using HMODULE = void*;
using FARPROC = void*;
using HGLOBAL = void*;
using HRSRC = void*;
struct ITEMIDLIST {};
using HGDIOBJ = void*;
using PVOID = void*;
using ULONGLONG = unsigned long long;
using HICON = void*;
using HANDLE = void*;
using HMONITOR = void*;
using HKEY = void*;
using LPBYTE = BYTE*;
struct LOGFONTW { wchar_t lfFaceName[128]{}; long lfHeight{}; long lfWeight{}; unsigned char lfItalic{}; unsigned char lfUnderline{}; unsigned char lfStrikeOut{}; unsigned char lfCharSet{}; unsigned char lfPitchAndFamily{}; unsigned char lfOutPrecision{}; unsigned char lfClipPrecision{}; unsigned char lfQuality{}; };
using LOGFONT = LOGFONTW;
using LPLOGFONT = LOGFONTW*;
using HFONT = HGDIOBJ;
struct TEXTMETRIC { int tmDescent{}; int tmExternalLeading{}; };
struct SIZE { long cx{}; long cy{}; };
using FONTENUMPROC = int (__stdcall *)(const LOGFONT*, const TEXTMETRIC*, DWORD, std::intptr_t);
using FONTENUMPROCW = FONTENUMPROC;
constexpr int LF_FACESIZE = 32;
struct SYSTEMTIME { WORD wYear{}, wMonth{}, wDayOfWeek{}, wDay{}, wHour{}, wMinute{}, wSecond{}, wMilliseconds{}; };
struct FILETIME { DWORD dwLowDateTime{}, dwHighDateTime{}; };
struct WIN32_FIND_DATAW { DWORD nFileSizeLow{}; FILETIME ftLastWriteTime{}; wchar_t cFileName[MAX_PATH]{}; };
using WIN32_FIND_DATA = WIN32_FIND_DATAW;
enum FINDEX_INFO_LEVELS { FindExInfoStandard = 0, FindExInfoBasic = 1 };
constexpr int FindExSearchNameMatch = 0;
struct OSVERSIONINFO { DWORD dwOSVersionInfoSize{}, dwMajorVersion{10}, dwMinorVersion{}; };
inline BOOL GetVersionExW(OSVERSIONINFO*) { return TRUE; }
using byte = unsigned char;
using WPARAM = std::uintptr_t;
using LPARAM = std::intptr_t;
using LRESULT = std::intptr_t;
using HHOOK = void*;
#ifndef WXLRESULT
using WXLRESULT = long;
#endif
#ifndef WXUINT
using WXUINT = unsigned int;
#endif
#ifndef WXWPARAM
using WXWPARAM = std::uintptr_t;
#endif
#ifndef WXLPARAM
using WXLPARAM = std::intptr_t;
#endif
using WXHDC = void*;
struct POINT { long x{}; long y{}; POINT(long _x = 0, long _y = 0) : x(_x), y(_y) {} };
struct MSG { HWND hwnd{}; UINT message{}; WPARAM wParam{}; LPARAM lParam{}; DWORD time{}; POINT pt{}; };
using LPMSG = MSG*;
constexpr WPARAM VK_MENU = 0x12;
constexpr WPARAM VK_LMENU = 0xA4;
constexpr WPARAM VK_LSHIFT = 0xA0;
constexpr WPARAM VK_RSHIFT = 0xA1;
constexpr WPARAM VK_LCONTROL = 0xA2;
constexpr WPARAM VK_RCONTROL = 0xA3;
constexpr WPARAM VK_DOWN = 0x28;
constexpr WPARAM VK_UP = 0x26;
constexpr WPARAM VK_LEFT = 0x25;
constexpr WPARAM VK_RIGHT = 0x27;
constexpr WPARAM VK_ESCAPE = 0x1B;
constexpr WPARAM VK_RETURN = 0x0D;
constexpr UINT WM_MOUSEWHEEL = 0x020A;
constexpr UINT WM_LBUTTONDOWN = 0x0201;
constexpr UINT WM_NCLBUTTONDOWN = 0x00A1;
constexpr UINT WM_MBUTTONUP = 0x0208;
constexpr UINT WM_NCMBUTTONUP = 0x00A8;
inline BOOL GetKeyboardState(BYTE state[256]) { if (state) std::memset(state, 0, 256); return TRUE; }
inline BOOL GetCursorPos(POINT* p) {
    if (!p) return FALSE;
    wxPoint pos = wxGetMousePosition();
    p->x = pos.x;
    p->y = pos.y;
    return TRUE;
}
inline HWND WindowFromPoint(POINT pt) {
    wxWindow* win = wxFindWindowAtPoint(wxPoint(pt.x, pt.y));
    return win ? reinterpret_cast<HWND>(win) : nullptr;
}
inline DWORD GetCurrentThreadId() {
    static std::atomic<DWORD> next{1};
    thread_local DWORD id = next.fetch_add(1);
    return id;
}
inline DWORD GetCurrentProcessId() { return static_cast<DWORD>(::getpid()); }
inline DWORD GetWindowThreadProcessId(HWND, DWORD* pid) { if (pid) *pid = GetCurrentProcessId(); return GetCurrentThreadId(); }
inline wxWindow* wxGetWindowFromHWND(HWND hwnd) { return reinterpret_cast<wxWindow*>(hwnd); }

inline DWORD timeGetTime() {
    using namespace std::chrono;
    return static_cast<DWORD>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

#ifndef _SPACE
#define _SPACE 0x0008
#endif
#ifndef _PUNCT
#define _PUNCT 0x0010
#endif
inline int kainote_iswctype(wint_t ch, int mask) {
    return ((mask & _SPACE) && std::iswspace(ch)) || ((mask & _PUNCT) && std::iswpunct(ch));
}
#define iswctype(ch, mask) kainote_iswctype((ch), (mask))

constexpr DWORD GENERIC_READ = 0x80000000u;
constexpr DWORD GENERIC_WRITE = 0x40000000u;
constexpr DWORD FILE_SHARE_READ = 0x00000001u;
constexpr DWORD FILE_SHARE_WRITE = 0x00000002u;
constexpr DWORD FILE_SHARE_DELETE = 0x00000004u;
constexpr DWORD OPEN_EXISTING = 3;
constexpr DWORD OPEN_ALWAYS = 4;
constexpr DWORD CREATE_ALWAYS = 2;
constexpr DWORD FILE_ATTRIBUTE_NORMAL = 0x80;
constexpr DWORD FILE_ATTRIBUTE_READONLY = 0x1;
constexpr DWORD INVALID_FILE_ATTRIBUTES = 0xffffffffu;
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

inline void kainote_tm_to_system(const std::tm& tm, SYSTEMTIME* st) {
    st->wYear = tm.tm_year + 1900; st->wMonth = tm.tm_mon + 1; st->wDay = tm.tm_mday;
    st->wDayOfWeek = tm.tm_wday; st->wHour = tm.tm_hour; st->wMinute = tm.tm_min; st->wSecond = tm.tm_sec; st->wMilliseconds = 0;
}
inline void kainote_time_t_to_system(std::time_t t, SYSTEMTIME* st) {
    std::tm tm{}; gmtime_r(&t, &tm); kainote_tm_to_system(tm, st);
}
inline void GetSystemTime(SYSTEMTIME* st) { kainote_time_t_to_system(std::time(nullptr), st); }
inline void GetLocalTime(SYSTEMTIME* st) {
    std::time_t t=std::time(nullptr); std::tm tm{}; localtime_r(&t,&tm);
    st->wYear=tm.tm_year+1900; st->wMonth=tm.tm_mon+1; st->wDay=tm.tm_mday; st->wDayOfWeek=tm.tm_wday; st->wHour=tm.tm_hour; st->wMinute=tm.tm_min; st->wSecond=tm.tm_sec; st->wMilliseconds=0;
}
constexpr std::uint64_t KAINOTE_FILETIME_TICKS_PER_SECOND = 10000000ULL;
constexpr std::int64_t KAINOTE_FILETIME_UNIX_EPOCH_OFFSET = 11644473600LL;
inline bool kainote_timespec_to_filetime(const timespec& value, FILETIME* output) {
    if (!output || value.tv_nsec < 0 || value.tv_nsec >= 1000000000L)
        return false;
    const std::int64_t unixSeconds = static_cast<std::int64_t>(value.tv_sec);
    const std::int64_t maxUnixSeconds = static_cast<std::int64_t>(
        std::numeric_limits<std::uint64_t>::max() / KAINOTE_FILETIME_TICKS_PER_SECOND) -
        KAINOTE_FILETIME_UNIX_EPOCH_OFFSET;
    if (unixSeconds < -KAINOTE_FILETIME_UNIX_EPOCH_OFFSET ||
        unixSeconds > maxUnixSeconds)
        return false;
    const std::uint64_t ticks =
        static_cast<std::uint64_t>(unixSeconds + KAINOTE_FILETIME_UNIX_EPOCH_OFFSET) *
            KAINOTE_FILETIME_TICKS_PER_SECOND +
        static_cast<std::uint64_t>(value.tv_nsec) / 100ULL;
    output->dwLowDateTime = static_cast<DWORD>(ticks);
    output->dwHighDateTime = static_cast<DWORD>(ticks >> 32);
    return true;
}
inline bool kainote_filetime_to_timespec(const FILETIME* input, timespec* output) {
    if (!input || !output)
        return false;
    const std::uint64_t ticks =
        (static_cast<std::uint64_t>(input->dwHighDateTime) << 32) |
        input->dwLowDateTime;
    const std::int64_t unixSeconds = static_cast<std::int64_t>(
        ticks / KAINOTE_FILETIME_TICKS_PER_SECOND) -
        KAINOTE_FILETIME_UNIX_EPOCH_OFFSET;
    const std::time_t seconds = static_cast<std::time_t>(unixSeconds);
    if (static_cast<std::int64_t>(seconds) != unixSeconds)
        return false;
    output->tv_sec = seconds;
    output->tv_nsec = static_cast<long>(
        (ticks % KAINOTE_FILETIME_TICKS_PER_SECOND) * 100ULL);
    return true;
}
inline bool SystemTimeToFileTime(const SYSTEMTIME* st, FILETIME* ft) {
    if (!st || !ft || st->wYear < 1601 || st->wMonth < 1 || st->wMonth > 12 ||
        st->wDay < 1 || st->wDay > 31 || st->wHour > 23 || st->wMinute > 59 ||
        st->wSecond > 59 || st->wMilliseconds > 999) return false;
    std::tm tm{}; tm.tm_year=st->wYear-1900; tm.tm_mon=st->wMonth-1; tm.tm_mday=st->wDay; tm.tm_hour=st->wHour; tm.tm_min=st->wMinute; tm.tm_sec=st->wSecond;
    const std::time_t t = timegm(&tm);
    const timespec value{t, static_cast<long>(st->wMilliseconds) * 1000000L};
    return kainote_timespec_to_filetime(value, ft);
}
inline bool FileTimeToSystemTime(const FILETIME* ft, SYSTEMTIME* st) {
    if (!ft || !st) return false;
    timespec value{};
    if (!kainote_filetime_to_timespec(ft, &value)) return false;
    std::tm tm{};
    if (!gmtime_r(&value.tv_sec, &tm)) return false;
    kainote_tm_to_system(tm, st);
    st->wMilliseconds = static_cast<WORD>(value.tv_nsec / 1000000L);
    return true;
}
inline std::time_t kainote_filetime_to_time_t(const FILETIME* ft) {
    timespec value{};
    return kainote_filetime_to_timespec(ft, &value) ? value.tv_sec : static_cast<std::time_t>(-1);
}
struct KainoteHandleBase {
    enum class Kind { File, Event, Thread, Timer, ChangeNotification } kind;
    explicit KainoteHandleBase(Kind k) : kind(k) {}
    virtual ~KainoteHandleBase() = default;
};
struct KainoteFileHandle : KainoteHandleBase {
    FILE* file{};
    explicit KainoteFileHandle(FILE* f) : KainoteHandleBase(Kind::File), file(f) {}
    ~KainoteFileHandle() override { if (file) std::fclose(file); }
};
struct KainoteEventHandle : KainoteHandleBase {
    std::mutex mutex;
    std::condition_variable cv;
    bool manualReset{};
    bool signaled{};
    KainoteEventHandle(bool manual, bool initial) : KainoteHandleBase(Kind::Event), manualReset(manual), signaled(initial) {}
};
struct KainoteThreadHandle : KainoteHandleBase {
    std::thread thread;
	// Completion may outlive a closed thread handle.
    std::shared_ptr<std::atomic<bool>> finished{std::make_shared<std::atomic<bool>>(false)};
    KainoteThreadHandle() : KainoteHandleBase(Kind::Thread) {}
    explicit KainoteThreadHandle(std::thread&& t) : KainoteHandleBase(Kind::Thread), thread(std::move(t)) {}
    ~KainoteThreadHandle() override { if (thread.joinable()) thread.detach(); }
};
struct KainoteTimerState {
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> cancelled{false};
	// Coalesce callbacks like Win32 timers.
    std::atomic<bool> callbackPending{false};

    bool wait_for_cancel_or_timeout(DWORD milliseconds) {
        if (cancelled.load()) return true;
        if (milliseconds == 0) return false;
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, std::chrono::milliseconds(milliseconds), [&] { return cancelled.load(); });
    }

    void cancel() {
        cancelled.store(true);
        cv.notify_all();
    }
};
struct KainoteTimerHandle : KainoteHandleBase {
    std::shared_ptr<KainoteTimerState> state{std::make_shared<KainoteTimerState>()};
    std::thread thread;
    KainoteTimerHandle() : KainoteHandleBase(Kind::Timer) {}
    explicit KainoteTimerHandle(std::thread&& t) : KainoteHandleBase(Kind::Timer), thread(std::move(t)) {}
    ~KainoteTimerHandle() override {
        if (state) state->cancel();
        if (thread.joinable()) {
            if (thread.get_id() == std::this_thread::get_id()) thread.detach();
            else thread.join();
        }
    }
};
struct KainoteChangeNotificationHandle : KainoteHandleBase {
    int fd{-1};
    bool recursive{};
    uint32_t mask{};
    std::map<int, std::string> watchPaths;
    std::set<std::string> watchedPaths;

    KainoteChangeNotificationHandle(int inotifyFd, bool watchRecursively, uint32_t eventMask)
        : KainoteHandleBase(Kind::ChangeNotification), fd(inotifyFd),
          recursive(watchRecursively), mask(eventMask) {}

    bool add_directory(const std::filesystem::path& directory) {
        std::error_code ec;
        if (!std::filesystem::is_directory(directory, ec)) return false;
        const std::string normalized = directory.lexically_normal().string();
        if (!watchedPaths.insert(normalized).second) return true;
        const int wd = inotify_add_watch(fd, normalized.c_str(), mask | IN_ONLYDIR);
        if (wd < 0) {
            watchedPaths.erase(normalized);
            return false;
        }
        auto previous = watchPaths.find(wd);
        if (previous != watchPaths.end()) watchedPaths.erase(previous->second);
        watchPaths[wd] = normalized;
        watchedPaths.insert(normalized);
        return true;
    }

    bool add_directory_tree(const std::filesystem::path& root) {
        bool added = add_directory(root);
        if (!recursive || !added) return added;

        std::error_code ec;
        std::filesystem::recursive_directory_iterator iterator(
            root, std::filesystem::directory_options::skip_permission_denied, ec);
        const std::filesystem::recursive_directory_iterator end;
        while (iterator != end) {
            if (!ec && iterator->is_directory(ec)) add_directory(iterator->path());
            ec.clear();
            iterator.increment(ec);
        }
        return added;
    }

    bool consume_events() {
        alignas(inotify_event) char buffer[64 * 1024];
        bool consumed = false;
        while (true) {
            const ssize_t length = read(fd, buffer, sizeof(buffer));
            if (length < 0) {
                if (errno == EINTR) continue;
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                return false;
            }
            if (length == 0) break;
            consumed = true;
            std::size_t offset = 0;
            while (offset + sizeof(inotify_event) <= static_cast<std::size_t>(length)) {
                const auto* event = reinterpret_cast<const inotify_event*>(buffer + offset);
                auto watched = watchPaths.find(event->wd);
                if (recursive && watched != watchPaths.end() && event->len &&
                    (event->mask & IN_ISDIR) &&
                    (event->mask & (IN_CREATE | IN_MOVED_TO))) {
                    add_directory_tree(std::filesystem::path(watched->second) / event->name);
                }
                if ((event->mask & IN_IGNORED) && watched != watchPaths.end()) {
                    watchedPaths.erase(watched->second);
                    watchPaths.erase(watched);
                }
                offset += sizeof(inotify_event) + event->len;
            }
        }
        return consumed;
    }

    ~KainoteChangeNotificationHandle() override {
        if (fd >= 0) {
            for (const auto& [watch, unused] : watchPaths) {
                (void)unused;
                inotify_rm_watch(fd, watch);
            }
        }
        if (fd >= 0) close(fd);
    }
};

inline HANDLE CreateFileW(const wchar_t* path, DWORD access, DWORD, void*, DWORD creation, DWORD, HANDLE) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv; std::string p=conv.to_bytes(path ? std::wstring(path) : std::wstring()); std::replace(p.begin(), p.end(), '\\', '/');
    const char* mode = (access & GENERIC_WRITE) ? "ab+" : "rb"; if (creation == CREATE_ALWAYS) mode = "wb+";
    FILE* f = std::fopen(p.c_str(), mode); return f ? reinterpret_cast<HANDLE>(new KainoteFileHandle(f)) : INVALID_HANDLE_VALUE;
}
inline HANDLE CreateFileA(const char* path, DWORD access, DWORD, void*, DWORD creation, DWORD, HANDLE) {
    std::string p = path ? std::string(path) : std::string(); std::replace(p.begin(), p.end(), '\\', '/');
    const char* mode = (access & GENERIC_WRITE) ? "ab+" : "rb"; if (creation == CREATE_ALWAYS) mode = "wb+";
    FILE* f = std::fopen(p.c_str(), mode); return f ? reinterpret_cast<HANDLE>(new KainoteFileHandle(f)) : INVALID_HANDLE_VALUE;
}
inline HANDLE CreateFile(const wchar_t* path, DWORD access, DWORD share, void* sec, DWORD creation, DWORD flags, HANDLE tmpl) { return CreateFileW(path, access, share, sec, creation, flags, tmpl); }
inline HANDLE CreateFile(const char* path, DWORD access, DWORD share, void* sec, DWORD creation, DWORD flags, HANDLE tmpl) { return CreateFileA(path, access, share, sec, creation, flags, tmpl); }
inline KainoteHandleBase* kainote_handle(HANDLE h) { return (!h || h == INVALID_HANDLE_VALUE) ? nullptr : reinterpret_cast<KainoteHandleBase*>(h); }
inline bool GetFileTime(HANDLE h, FILETIME* creation, FILETIME* access, FILETIME* write) {
    auto* base = kainote_handle(h);
    if (!base || base->kind != KainoteHandleBase::Kind::File) return false;
    FILE* f = static_cast<KainoteFileHandle*>(base)->file;
    int fd=fileno(f); struct stat st{}; if (fstat(fd,&st)!=0) return false;
	// POSIX has no portable creation time; use ctime as the closest value.
    if (creation && !kainote_timespec_to_filetime(st.st_ctim, creation)) return false;
    if (access && !kainote_timespec_to_filetime(st.st_atim, access)) return false;
    if (write && !kainote_timespec_to_filetime(st.st_mtim, write)) return false;
    return true;
}
inline bool CloseHandle(HANDLE h) { auto* base = kainote_handle(h); if (!base) return false; delete base; return true; }
inline DWORD GetLastError() { return errno; }
inline unsigned short GetSystemDefaultUILanguage() { return (wxLocale::GetSystemLanguage() == wxLANGUAGE_POLISH) ? 0x415 : 0x409; }

struct KainoteFindHandle { std::vector<std::filesystem::directory_entry> entries; std::size_t index{}; };
inline std::wstring kainote_utf8_to_wstring(const std::string& s) { std::wstring_convert<std::codecvt_utf8<wchar_t>> conv; return conv.from_bytes(s); }
inline std::string kainote_wstring_to_utf8(const std::wstring& s) { std::wstring_convert<std::codecvt_utf8<wchar_t>> conv; return conv.to_bytes(s); }
inline std::string kainote_normalize_path(const wchar_t* path) { std::string p = kainote_wstring_to_utf8(path ? std::wstring(path) : std::wstring()); std::replace(p.begin(), p.end(), '\\', '/'); return p; }
inline std::string kainote_normalize_path(const char* path) { std::string p = path ? std::string(path) : std::string(); std::replace(p.begin(), p.end(), '\\', '/'); return p; }
inline void kainote_fill_find_data(const std::filesystem::directory_entry& e, WIN32_FIND_DATAW* data) {
    if (!data) return; *data = WIN32_FIND_DATAW{};
    auto name = kainote_utf8_to_wstring(e.path().filename().string());
    std::wcsncpy(data->cFileName, name.c_str(), MAX_PATH - 1);
    std::error_code ec; auto sz = e.is_regular_file(ec) ? e.file_size(ec) : 0; data->nFileSizeLow = static_cast<DWORD>(sz & 0xffffffffu);
    struct stat metadata{};
    if (::stat(e.path().c_str(), &metadata) == 0)
        kainote_timespec_to_filetime(metadata.st_mtim, &data->ftLastWriteTime);
}
inline char kainote_ascii_fold(char ch) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
}
inline bool kainote_windows_wildcard_match(const std::string& name, const std::string& pattern) {
	// Win32 matching is case-insensitive and treats *.* as match-all.
    if (pattern == "*" || pattern == "*.*") return true;
    std::size_t namePos = 0;
    std::size_t patternPos = 0;
    std::size_t starPos = std::string::npos;
    std::size_t retryNamePos = 0;
    while (namePos < name.size()) {
        if (patternPos < pattern.size() &&
            (pattern[patternPos] == '?' ||
             kainote_ascii_fold(pattern[patternPos]) == kainote_ascii_fold(name[namePos]))) {
            ++namePos;
            ++patternPos;
        } else if (patternPos < pattern.size() && pattern[patternPos] == '*') {
            starPos = patternPos++;
            retryNamePos = namePos;
        } else if (starPos != std::string::npos) {
            patternPos = starPos + 1;
            namePos = ++retryNamePos;
        } else {
            return false;
        }
    }
    while (patternPos < pattern.size() && pattern[patternPos] == '*') ++patternPos;
    return patternPos == pattern.size();
}
inline HANDLE FindFirstFileW(const wchar_t* pattern, WIN32_FIND_DATAW* data) {
    const std::filesystem::path searchPath(kainote_normalize_path(pattern));
    std::filesystem::path dir = searchPath.parent_path();
    const std::string wildcard = searchPath.filename().string();
    if (dir.empty()) dir = ".";
    std::error_code ec;
    if (!std::filesystem::is_directory(dir, ec)) return INVALID_HANDLE_VALUE;
    auto* h = new KainoteFindHandle();
    for (auto& e : std::filesystem::directory_iterator(dir, ec)) {
        if (ec) break;
        if (kainote_windows_wildcard_match(e.path().filename().string(), wildcard))
            h->entries.push_back(e);
    }
    if (h->entries.empty()) { delete h; return INVALID_HANDLE_VALUE; }
    kainote_fill_find_data(h->entries[0], data); return static_cast<HANDLE>(h);
}
inline HANDLE FindFirstFileA(const char* pattern, WIN32_FIND_DATA* data) { auto w = kainote_utf8_to_wstring(kainote_normalize_path(pattern)); return FindFirstFileW(w.c_str(), data); }
inline HANDLE FindFirstFileEx(const char* pattern, FINDEX_INFO_LEVELS, WIN32_FIND_DATA* data, int, void*, DWORD) { return FindFirstFileA(pattern, data); }
inline HANDLE FindFirstFileEx(const wchar_t* pattern, FINDEX_INFO_LEVELS, WIN32_FIND_DATA* data, int, void*, DWORD) { return FindFirstFileW(pattern, data); }
inline BOOL FindNextFile(HANDLE handle, WIN32_FIND_DATAW* data) { if (!handle || handle == INVALID_HANDLE_VALUE) return 0; auto* h = static_cast<KainoteFindHandle*>(handle); if (++h->index >= h->entries.size()) return 0; kainote_fill_find_data(h->entries[h->index], data); return 1; }
inline BOOL FindNextFileW(HANDLE handle, WIN32_FIND_DATAW* data) { return FindNextFile(handle, data); }
inline BOOL FindClose(HANDLE handle) { if (!handle || handle == INVALID_HANDLE_VALUE) return FALSE; auto* h = static_cast<KainoteFindHandle*>(handle); delete h; return TRUE; }

constexpr DWORD ERROR_NO_MORE_FILES = 18;
constexpr DWORD ERROR_FILE_NOT_FOUND = 2;
constexpr DWORD ERROR_ACCESS_DENIED = 5;
constexpr DWORD FORMAT_MESSAGE_ALLOCATE_BUFFER = 0x100;
constexpr DWORD FORMAT_MESSAGE_FROM_SYSTEM = 0x1000;
constexpr DWORD FORMAT_MESSAGE_IGNORE_INSERTS = 0x200;
inline DWORD FormatMessage(DWORD, void*, DWORD, DWORD, LPWSTR buffer, DWORD, void*) { static wchar_t msg[] = L"System error"; if (buffer) *reinterpret_cast<LPWSTR*>(buffer) = msg; return 12; }
inline DWORD FormatMessageA(DWORD, void*, DWORD error, DWORD, char* buffer, DWORD size, void*) {
    if (!buffer || size == 0) return 0;
    const char* msg = dlerror();
    if (!msg) msg = std::strerror(error ? static_cast<int>(error) : errno);
    if (!msg) msg = "System error";
    std::snprintf(buffer, size, "%s", msg);
    return static_cast<DWORD>(std::strlen(buffer));
}
inline void* LocalFree(void*) { return nullptr; }
inline BOOL SetFileTime(HANDLE h, const FILETIME*, const FILETIME* access, const FILETIME* write) {
    auto* base = kainote_handle(h);
    if (!base || base->kind != KainoteHandleBase::Kind::File) { errno = EBADF; return FALSE; }
    FILE* f = static_cast<KainoteFileHandle*>(base)->file;
    if (!f) { errno = EBADF; return FALSE; }
    const int fd = fileno(f);
    struct stat st{};
    if (fd < 0 || fstat(fd, &st) != 0) return FALSE;
    timespec times[2]{};
    times[0] = st.st_atim;
    times[1] = st.st_mtim;
    if (access) {
        if (!kainote_filetime_to_timespec(access, &times[0])) return FALSE;
    }
    if (write) {
        if (!kainote_filetime_to_timespec(write, &times[1])) return FALSE;
    }
    return futimens(fd, times) == 0 ? TRUE : FALSE;
}
inline BOOL CopyFile(const wchar_t* from, const wchar_t* to, BOOL failIfExists) { std::error_code ec; auto src = std::filesystem::path(kainote_normalize_path(from)); auto dst = std::filesystem::path(kainote_normalize_path(to)); if (failIfExists && std::filesystem::exists(dst, ec)) return FALSE; std::filesystem::create_directories(dst.parent_path(), ec); return std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec); }
inline FILE* kainote_wfopen(const wchar_t* path, const wchar_t* mode) { std::string p = kainote_normalize_path(path); std::string m = kainote_wstring_to_utf8(mode ? std::wstring(mode) : std::wstring(L"rb")); return std::fopen(p.c_str(), m.c_str()); }
inline int kainote_wremove(const wchar_t* path) { std::string p = kainote_normalize_path(path); return std::remove(p.c_str()); }
struct TIME_ZONE_INFORMATION { LONG Bias{}; };
inline DWORD GetTimeZoneInformation(TIME_ZONE_INFORMATION* info) {
    if (info) {
        std::time_t now = std::time(nullptr);
        std::tm utc{};
        std::tm local{};
        gmtime_r(&now, &utc);
        localtime_r(&now, &local);
        info->Bias = static_cast<LONG>(std::difftime(timegm(&utc), mktime(&local)) / 60);
    }
    return 0;
}
inline BOOL SystemTimeToTzSpecificLocalTime(const TIME_ZONE_INFORMATION*, const SYSTEMTIME* universal, SYSTEMTIME* local) {
    if (!universal || !local) return FALSE;
    FILETIME ft{};
    if (!SystemTimeToFileTime(universal, &ft)) return FALSE;
    std::time_t t = kainote_filetime_to_time_t(&ft);
    std::tm tm{};
    if (!localtime_r(&t, &tm)) return FALSE;
    kainote_tm_to_system(tm, local);
    local->wMilliseconds = universal->wMilliseconds;
    return TRUE;
}
#define _wfopen kainote_wfopen
#define _wremove kainote_wremove
inline int kainote_wrename(const wchar_t* oldp, const wchar_t* newp) { return std::rename(kainote_normalize_path(oldp).c_str(), kainote_normalize_path(newp).c_str()); }
#define _wrename kainote_wrename
#define _fseeki64 fseeko
#define _int64 long long
#define __int64 long long
#define _strdup strdup
#define vsprintf_s(buffer, size, fmt, args) vsnprintf((buffer), (size), (fmt), (args))
#define _snwprintf swprintf
#define _tcsncpy wcsncpy
#ifndef __stdcall
#define __stdcall
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
constexpr int WH_CBT = 5;
constexpr int HCBT_MINMAX = 1;
using TIMERPROC = void (*)();
constexpr int WH_KEYBOARD = 2;
constexpr int WH_GETMESSAGE = 3;
inline HHOOK SetWindowsHookEx(int, LRESULT (__stdcall *)(int, WPARAM, LPARAM), void*, DWORD) { return nullptr; }
inline BOOL UnhookWindowsHookEx(HHOOK) { return 1; }
inline LRESULT CallNextHookEx(HHOOK, int, WPARAM, LPARAM) { return 0; }
inline std::mutex& kainote_window_timer_mutex() { static std::mutex m; return m; }
inline std::map<std::pair<std::uintptr_t, UINT>, std::unique_ptr<KainoteTimerHandle>>& kainote_window_timers() { static std::map<std::pair<std::uintptr_t, UINT>, std::unique_ptr<KainoteTimerHandle>> timers; return timers; }
inline void kainote_dispatch_timer_proc(const std::shared_ptr<KainoteTimerState>& state, TIMERPROC proc) {
    if (!state || !proc || state->cancelled.load()) return;
    bool expected = false;
    if (!state->callbackPending.compare_exchange_strong(expected, true)) return;
    if (wxTheApp) {
        std::weak_ptr<KainoteTimerState> weakState = state;
        wxTheApp->CallAfter([weakState, proc]() {
            auto lockedState = weakState.lock();
            if (!lockedState || lockedState->cancelled.load()) return;
            proc();
            lockedState->callbackPending.store(false);
        });
    } else {
        if (!state->cancelled.load()) proc();
        state->callbackPending.store(false);
    }
}
inline UINT SetTimer(HWND hwnd, UINT id, UINT elapsed, TIMERPROC proc) {
    static std::atomic<UINT> nextTimerId{1};
    if (id == 0) {
        id = nextTimerId.fetch_add(1);
        if (id == 0) id = nextTimerId.fetch_add(1);
    }
    auto timer = std::make_unique<KainoteTimerHandle>();
    auto state = timer->state;
    timer->thread = std::thread([state, elapsed, proc]() {
        do {
            if (state->wait_for_cancel_or_timeout(elapsed)) break;
            if (state->cancelled.load()) break;
            kainote_dispatch_timer_proc(state, proc);
        } while (!state->cancelled.load());
    });

    std::unique_ptr<KainoteTimerHandle> old;
    auto key = std::make_pair(reinterpret_cast<std::uintptr_t>(hwnd), id);
    {
        std::lock_guard<std::mutex> lock(kainote_window_timer_mutex());
        auto& timers = kainote_window_timers();
        auto it = timers.find(key);
        if (it != timers.end()) {
            old = std::move(it->second);
            timers.erase(it);
        }
        timers.emplace(key, std::move(timer));
    }
    return id;
}
inline BOOL KillTimer(HWND hwnd, UINT id) {
    std::unique_ptr<KainoteTimerHandle> timer;
    auto key = std::make_pair(reinterpret_cast<std::uintptr_t>(hwnd), id);
    {
        std::lock_guard<std::mutex> lock(kainote_window_timer_mutex());
        auto& timers = kainote_window_timers();
        auto it = timers.find(key);
        if (it == timers.end()) return FALSE;
        timer = std::move(it->second);
        timers.erase(it);
    }
    return TRUE;
}
#ifndef MAXINT
#define MAXINT INT32_MAX
#endif
#ifndef _stdcall
#define _stdcall
#endif
#ifndef swscanf_s
#define swscanf_s swscanf
#endif
#ifndef TEXT
#define TEXT(x) L##x
#endif
constexpr int FW_NORMAL = 400;
constexpr int FW_BOLD = 700;
constexpr int DEFAULT_CHARSET = 1;
constexpr int OUT_TT_ONLY_PRECIS = 7;
constexpr int OUT_DEFAULT_PRECIS = 0;
constexpr int OUT_TT_PRECIS = 4;
constexpr int CLIP_DEFAULT_PRECIS = 0;
constexpr int ANTIALIASED_QUALITY = 4;
constexpr int MM_TEXT = 1;
constexpr int DEFAULT_PITCH = 0;
constexpr int FF_DONTCARE = 0;
constexpr int CLEARTYPE_QUALITY = 5;
struct SYSTEM_INFO { DWORD dwNumberOfProcessors{1}; };
inline void GetSystemInfo(SYSTEM_INFO* si) {
    if (!si) return;
    long processors = sysconf(_SC_NPROCESSORS_ONLN);
    if (processors < 1) processors = static_cast<long>(std::thread::hardware_concurrency());
    si->dwNumberOfProcessors = static_cast<DWORD>(std::max<long>(1, processors));
}
constexpr int SM_CXICON = 11;
constexpr int SM_CYICON = 12;
constexpr DWORD WAIT_OBJECT_0 = 0;
constexpr DWORD WAIT_TIMEOUT = 258;
constexpr DWORD WAIT_FAILED = 0xffffffffu;
constexpr DWORD INFINITE = 0xffffffffu;
inline HANDLE CreateEvent(void*, BOOL manualReset, BOOL initialState, const wchar_t*) { return reinterpret_cast<HANDLE>(new KainoteEventHandle(manualReset != FALSE, initialState != FALSE)); }
inline BOOL SetEvent(HANDLE h) {
    auto* base = kainote_handle(h);
    if (!base || base->kind != KainoteHandleBase::Kind::Event) return FALSE;
    auto* ev = static_cast<KainoteEventHandle*>(base);
    { std::lock_guard<std::mutex> lock(ev->mutex); ev->signaled = true; }
    ev->cv.notify_all();
    return TRUE;
}
inline BOOL ResetEvent(HANDLE h) {
    auto* base = kainote_handle(h);
    if (!base || base->kind != KainoteHandleBase::Kind::Event) return FALSE;
    auto* ev = static_cast<KainoteEventHandle*>(base);
    std::lock_guard<std::mutex> lock(ev->mutex);
    ev->signaled = false;
    return TRUE;
}
inline DWORD WaitForSingleObject(HANDLE h, DWORD ms) {
    auto* base = kainote_handle(h);
    if (!base) return WAIT_FAILED;
    if (base->kind == KainoteHandleBase::Kind::Thread) {
        auto* th = static_cast<KainoteThreadHandle*>(base);
        if (ms == INFINITE) {
            if (th->thread.joinable()) th->thread.join();
            th->finished->store(true);
            return WAIT_OBJECT_0;
        }
        auto start = std::chrono::steady_clock::now();
        while (!th->finished->load()) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() >= ms) return WAIT_TIMEOUT;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (th->thread.joinable()) th->thread.join();
        return WAIT_OBJECT_0;
    }
    if (base->kind == KainoteHandleBase::Kind::Event) {
        auto* ev = static_cast<KainoteEventHandle*>(base);
        std::unique_lock<std::mutex> lock(ev->mutex);
        bool ok = false;
        if (ms == INFINITE) { ev->cv.wait(lock, [&]{ return ev->signaled; }); ok = true; }
        else ok = ev->cv.wait_for(lock, std::chrono::milliseconds(ms), [&]{ return ev->signaled; });
        if (!ok) return WAIT_TIMEOUT;
        if (!ev->manualReset) ev->signaled = false;
        return WAIT_OBJECT_0;
    }
    if (base->kind == KainoteHandleBase::Kind::ChangeNotification) {
        auto* change = static_cast<KainoteChangeNotificationHandle*>(base);
        if (change->fd < 0) return WAIT_FAILED;
        pollfd pfd{};
        pfd.fd = change->fd;
        pfd.events = POLLIN;
        const int timeout = (ms == INFINITE) ? -1 : static_cast<int>(ms);
        int result = poll(&pfd, 1, timeout);
        if (result == 0) return WAIT_TIMEOUT;
        if (result < 0) return WAIT_FAILED;
        if (!(pfd.revents & POLLIN) || !change->consume_events()) return WAIT_FAILED;
        return WAIT_OBJECT_0;
    }
    return WAIT_FAILED;
}
inline DWORD WaitForMultipleObjects(DWORD count, const HANDLE* handles, BOOL waitAll, DWORD ms) {
    if (!handles || !count) return WAIT_FAILED;
    for (DWORD i = 0; i < count; ++i) {
        auto* base = kainote_handle(handles[i]);
        if (!base || (base->kind != KainoteHandleBase::Kind::Thread &&
                      base->kind != KainoteHandleBase::Kind::Event &&
                      base->kind != KainoteHandleBase::Kind::ChangeNotification)) {
            return WAIT_FAILED;
        }
    }
    if (waitAll) {
        std::vector<KainoteHandleBase*> bases;
        std::vector<KainoteEventHandle*> events;
        bases.reserve(count);
        events.reserve(count);
        for (DWORD i = 0; i < count; ++i) {
            auto* base = kainote_handle(handles[i]);
            bases.push_back(base);
            if (base->kind == KainoteHandleBase::Kind::Event)
                events.push_back(static_cast<KainoteEventHandle*>(base));
        }
        std::sort(events.begin(), events.end(), std::less<KainoteEventHandle*>{});
        events.erase(std::unique(events.begin(), events.end()), events.end());

        const auto start = std::chrono::steady_clock::now();
        while (true) {
            std::vector<std::unique_lock<std::mutex>> eventLocks;
            eventLocks.reserve(events.size());
            for (auto* event : events) eventLocks.emplace_back(event->mutex);

            bool allReady = true;
            bool failed = false;
            for (auto* base : bases) {
                if (base->kind == KainoteHandleBase::Kind::Event) {
                    if (!static_cast<KainoteEventHandle*>(base)->signaled) allReady = false;
                } else if (base->kind == KainoteHandleBase::Kind::Thread) {
                    if (!static_cast<KainoteThreadHandle*>(base)->finished->load()) allReady = false;
                } else {
                    auto* change = static_cast<KainoteChangeNotificationHandle*>(base);
                    if (change->fd < 0) {
                        failed = true;
                        allReady = false;
                        continue;
                    }
                    pollfd descriptor{change->fd, POLLIN, 0};
                    const int result = poll(&descriptor, 1, 0);
                    if (result < 0 && errno != EINTR) failed = true;
                    if (result <= 0 || !(descriptor.revents & POLLIN)) allReady = false;
                    if (descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) failed = true;
                }
            }
            if (failed) return WAIT_FAILED;

            if (allReady) {
				// Consume auto-reset events only after every handle is ready.
                for (auto* base : bases) {
                    if (base->kind == KainoteHandleBase::Kind::ChangeNotification &&
                        !static_cast<KainoteChangeNotificationHandle*>(base)->consume_events()) {
                        return WAIT_FAILED;
                    }
                }
                for (auto* event : events) {
                    if (!event->manualReset) event->signaled = false;
                }
                eventLocks.clear();
                for (auto* base : bases) {
                    if (base->kind == KainoteHandleBase::Kind::Thread) {
                        auto* thread = static_cast<KainoteThreadHandle*>(base);
                        if (thread->thread.joinable()) thread->thread.join();
                    }
                }
                return WAIT_OBJECT_0;
            }

            eventLocks.clear();
            if (ms != INFINITE &&
                std::chrono::steady_clock::now() - start >= std::chrono::milliseconds(ms)) {
                return WAIT_TIMEOUT;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    auto start = std::chrono::steady_clock::now();
    while (true) {
        for (DWORD i = 0; i < count; ++i) {
            DWORD r = WaitForSingleObject(handles[i], 0);
            if (r == WAIT_OBJECT_0) return WAIT_OBJECT_0 + i;
            if (r == WAIT_FAILED) return WAIT_FAILED;
        }
        if (ms != INFINITE && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() >= ms) return WAIT_TIMEOUT;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
constexpr int THREAD_PRIORITY_TIME_CRITICAL = 15;
constexpr int THREAD_PRIORITY_LOWEST = -2;
inline BOOL SetThreadPriority(HANDLE, int) { return TRUE; }
using KainoteThreadProc = unsigned int (__stdcall *)(void*);
inline uintptr_t _beginthreadex(void*, unsigned, KainoteThreadProc proc, void* data, unsigned, unsigned int* threadid) {
    if (!proc) return 0;
    if (threadid) *threadid = GetCurrentThreadId();
    auto* handle = new KainoteThreadHandle();
    auto finished = handle->finished;
    handle->thread = std::thread([proc, data, finished]() { proc(data); finished->store(true); });
    return reinterpret_cast<uintptr_t>(handle);
}
inline void Sleep(DWORD ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
using WAITORTIMERCALLBACK = void (CALLBACK *)(PVOID, BOOLEAN);
using LPTHREAD_START_ROUTINE = DWORD (*)(void*);
inline HANDLE CreateThread(void*, size_t, LPTHREAD_START_ROUTINE proc, void* data, DWORD, DWORD* threadid) {
    if (!proc) return nullptr;
    unsigned int tid{};
    auto h = _beginthreadex(nullptr, 0, reinterpret_cast<KainoteThreadProc>(proc), data, 0, &tid);
    if (threadid) *threadid = tid;
    return reinterpret_cast<HANDLE>(h);
}
inline BOOL CreateTimerQueueTimer(HANDLE* out, HANDLE, WAITORTIMERCALLBACK cb, PVOID param, DWORD due, DWORD period, ULONG) {
    if (!cb) return FALSE;
    auto* timer = new KainoteTimerHandle();
    auto state = timer->state;
    timer->thread = std::thread([state, cb, param, due, period]() {
        if (state->wait_for_cancel_or_timeout(due)) return;
        do {
            if (state->cancelled.load()) break;
            cb(param, TRUE);
            if (!period) break;
        } while (!state->wait_for_cancel_or_timeout(period));
    });
    if (out) *out = reinterpret_cast<HANDLE>(timer);
    return TRUE;
}
inline BOOL DeleteTimerQueueTimer(HANDLE, HANDLE timer, HANDLE) { return CloseHandle(timer); }
constexpr int WM_SETICON = 0x0080;
constexpr unsigned ES_DISPLAY_REQUIRED = 0x2;
constexpr unsigned ES_CONTINUOUS = 0x80000000u;
inline unsigned SetThreadExecutionState(unsigned state) {
    static bool kainoteScreenInhibited = false;
    const bool want = (state & ES_DISPLAY_REQUIRED) != 0;
    if (want && !kainoteScreenInhibited) {
        if (wxPowerResource::Acquire(wxPOWER_RESOURCE_SCREEN, wxS("Kainote video playback")))
            kainoteScreenInhibited = true;
    } else if (!want && kainoteScreenInhibited) {
        wxPowerResource::Release(wxPOWER_RESOURCE_SCREEN);
        kainoteScreenInhibited = false;
    }
    return state;
}
constexpr int ICON_BIG = 1;
inline int GetSystemMetrics(int metric) { return (metric == SM_CXICON || metric == SM_CYICON) ? 32 : 0; }
constexpr int LOGPIXELSY = 90;
constexpr DWORD FILE_NOTIFY_CHANGE_FILE_NAME = 0x1;
constexpr DWORD FR_PRIVATE = 0x10;
constexpr int CSIDL_LOCAL_APPDATA = 0x001c;
constexpr int CSIDL_FLAG_CREATE = 0x8000;
constexpr DWORD GGI_MARK_NONEXISTING_GLYPHS = 0x1;
constexpr DWORD GDI_ERROR = 0xffffffffu;
inline HRESULT SHGetFolderPath(HWND, int, HANDLE, DWORD, LPWSTR path) { if (!path) return -1; const char* home = std::getenv("HOME"); std::wstring w = kainote_utf8_to_wstring(std::string(home ? home : "/tmp") + "/.local/share"); std::wcsncpy(path, w.c_str(), MAX_PATH - 1); path[MAX_PATH - 1] = L'\0'; return 0; }

struct KainoteLinuxFontFace {
    std::wstring family;
    std::string file;
    int weight{FW_NORMAL};
    bool italic{};
};

inline std::wstring kainote_lower_wstring(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t ch) { return static_cast<wchar_t>(std::towlower(ch)); });
    return value;
}

inline int kainote_fc_to_win_weight(int weight) {
    if (weight >= FC_WEIGHT_BOLD) return FW_BOLD;
    if (weight >= FC_WEIGHT_DEMIBOLD) return 600;
    return FW_NORMAL;
}

inline int kainote_win_to_fc_weight(int weight) {
    return weight >= FW_BOLD ? FC_WEIGHT_BOLD : FC_WEIGHT_REGULAR;
}

inline LOGFONTW kainote_logfont_from_face(const KainoteLinuxFontFace& face) {
    LOGFONTW lf{};
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    lf.lfOutPrecision = OUT_TT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = ANTIALIASED_QUALITY;
    lf.lfWeight = face.weight;
    lf.lfItalic = face.italic ? 0xff : 0;
    std::wcsncpy(lf.lfFaceName, face.family.c_str(), WXSIZEOF(lf.lfFaceName) - 1);
    lf.lfFaceName[WXSIZEOF(lf.lfFaceName) - 1] = L'\0';
    return lf;
}

inline std::vector<KainoteLinuxFontFace> kainote_linux_list_fonts(const std::wstring& familyFilter = std::wstring()) {
    std::vector<KainoteLinuxFontFace> faces;
    if (!FcInit()) return faces;

    FcPattern* pattern = FcPatternCreate();
    FcObjectSet* objects = FcObjectSetBuild(FC_FAMILY, FC_FILE, FC_WEIGHT, FC_SLANT, nullptr);
    if (!pattern || !objects) {
        if (pattern) FcPatternDestroy(pattern);
        if (objects) FcObjectSetDestroy(objects);
        return faces;
    }

    FcFontSet* fonts = FcFontList(nullptr, pattern, objects);
    const std::wstring filter = kainote_lower_wstring(familyFilter);
    if (fonts) {
        for (int i = 0; i < fonts->nfont; ++i) {
            FcPattern* font = fonts->fonts[i];
            FcChar8* familyRaw = nullptr;
            if (FcPatternGetString(font, FC_FAMILY, 0, &familyRaw) != FcResultMatch || !familyRaw) continue;
            std::wstring family = kainote_utf8_to_wstring(reinterpret_cast<const char*>(familyRaw));
            if (!filter.empty() && kainote_lower_wstring(family) != filter) continue;

            KainoteLinuxFontFace face;
            face.family = std::move(family);
            FcChar8* fileRaw = nullptr;
            if (FcPatternGetString(font, FC_FILE, 0, &fileRaw) == FcResultMatch && fileRaw)
                face.file = reinterpret_cast<const char*>(fileRaw);
            int weight = FC_WEIGHT_REGULAR;
            if (FcPatternGetInteger(font, FC_WEIGHT, 0, &weight) == FcResultMatch)
                face.weight = kainote_fc_to_win_weight(weight);
            int slant = FC_SLANT_ROMAN;
            if (FcPatternGetInteger(font, FC_SLANT, 0, &slant) == FcResultMatch)
                face.italic = slant != FC_SLANT_ROMAN;
            faces.push_back(std::move(face));
        }
        FcFontSetDestroy(fonts);
    }
    FcObjectSetDestroy(objects);
    FcPatternDestroy(pattern);
    return faces;
}

inline std::vector<std::string> kainote_linux_collect_font_files() {
    std::vector<std::string> files;
    for (const auto& face : kainote_linux_list_fonts()) {
        if (!face.file.empty()) files.push_back(face.file);
    }
    std::sort(files.begin(), files.end());
    files.erase(std::unique(files.begin(), files.end()), files.end());
    return files;
}

struct KainoteGdiBase {
    enum class Kind { Dc, Font } kind;
    explicit KainoteGdiBase(Kind k) : kind(k) {}
    virtual ~KainoteGdiBase() = default;
};

struct KainoteFontObject : KainoteGdiBase {
    LOGFONTW logfont{};
    std::string file;
    FcCharSet* charset{};
    explicit KainoteFontObject(const LOGFONTW& lf) : KainoteGdiBase(Kind::Font), logfont(lf) {}
    ~KainoteFontObject() override { if (charset) FcCharSetDestroy(charset); }
};

struct KainoteDcObject : KainoteGdiBase {
    KainoteFontObject* selectedFont{};
    KainoteDcObject() : KainoteGdiBase(Kind::Dc) {}
};

inline KainoteGdiBase* kainote_gdi_base(void* handle) {
    return handle ? reinterpret_cast<KainoteGdiBase*>(handle) : nullptr;
}

inline KainoteDcObject* kainote_dc(HDC dc) {
    auto* base = kainote_gdi_base(dc);
    return base && base->kind == KainoteGdiBase::Kind::Dc ? static_cast<KainoteDcObject*>(base) : nullptr;
}

inline KainoteFontObject* kainote_font(HGDIOBJ font) {
    auto* base = kainote_gdi_base(font);
    return base && base->kind == KainoteGdiBase::Kind::Font ? static_cast<KainoteFontObject*>(base) : nullptr;
}

inline void kainote_fill_font_object_from_match(KainoteFontObject* out) {
    if (!out || !FcInit()) return;
    FcPattern* pattern = FcPatternCreate();
    if (!pattern) return;

    if (out->logfont.lfFaceName[0]) {
        std::string family = kainote_wstring_to_utf8(out->logfont.lfFaceName);
        FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8*>(family.c_str()));
    }
    FcPatternAddInteger(pattern, FC_WEIGHT, kainote_win_to_fc_weight(static_cast<int>(out->logfont.lfWeight)));
    FcPatternAddInteger(pattern, FC_SLANT, out->logfont.lfItalic ? FC_SLANT_ITALIC : FC_SLANT_ROMAN);
    FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result = FcResultNoMatch;
    FcPattern* match = FcFontMatch(nullptr, pattern, &result);
    if (match) {
        FcChar8* fileRaw = nullptr;
        if (FcPatternGetString(match, FC_FILE, 0, &fileRaw) == FcResultMatch && fileRaw)
            out->file = reinterpret_cast<const char*>(fileRaw);
        FcCharSet* charset = nullptr;
        if (FcPatternGetCharSet(match, FC_CHARSET, 0, &charset) == FcResultMatch && charset)
            out->charset = FcCharSetCopy(charset);
        FcChar8* familyRaw = nullptr;
        if (FcPatternGetString(match, FC_FAMILY, 0, &familyRaw) == FcResultMatch && familyRaw) {
            std::wstring family = kainote_utf8_to_wstring(reinterpret_cast<const char*>(familyRaw));
            std::wcsncpy(out->logfont.lfFaceName, family.c_str(), WXSIZEOF(out->logfont.lfFaceName) - 1);
            out->logfont.lfFaceName[WXSIZEOF(out->logfont.lfFaceName) - 1] = L'\0';
        }
        FcPatternDestroy(match);
    }
    FcPatternDestroy(pattern);
}

inline DWORD GetGlyphIndicesW(HDC dc, LPCWSTR text, int count, LPWORD indices, DWORD) {
    if (!text || !indices) return GDI_ERROR;
    if (count < 0) count = std::wcslen(text);
    auto* context = kainote_dc(dc);
    auto* font = context ? context->selectedFont : nullptr;
    for (int i = 0; i < count; ++i) {
        const wchar_t ch = text[i];
        bool present = ch != 0;
        if (font && font->charset)
            present = FcCharSetHasChar(font->charset, static_cast<FcChar32>(ch));
        indices[i] = present ? 1 : 0xffff;
    }
    return static_cast<DWORD>(count);
}

inline HDC GetDC(HWND) { return reinterpret_cast<HDC>(new KainoteDcObject()); }
inline HDC CreateCompatibleDC(HDC) { return reinterpret_cast<HDC>(new KainoteDcObject()); }
inline BOOL DeleteDC(HDC dc) { auto* context = kainote_dc(dc); if (!context) return FALSE; delete context; return TRUE; }
inline int SetMapMode(HDC, int mode) { return mode; }
inline int GetDeviceCaps(HDC, int) { return 96; }
inline int ReleaseDC(HWND, HDC dc) { return DeleteDC(dc) ? 1 : 0; }
inline HGDIOBJ CreateFontIndirectW(const LOGFONTW* lf) { LOGFONTW copy = lf ? *lf : LOGFONTW{}; auto* font = new KainoteFontObject(copy); kainote_fill_font_object_from_match(font); return reinterpret_cast<HGDIOBJ>(font); }
#define CreateFontIndirect CreateFontIndirectW
inline HGDIOBJ SelectObject(HDC dc, HGDIOBJ obj) { auto* context = kainote_dc(dc); if (!context) return nullptr; HGDIOBJ old = reinterpret_cast<HGDIOBJ>(context->selectedFont); context->selectedFont = kainote_font(obj); return old; }
inline BOOL DeleteObject(HGDIOBJ obj) { auto* base = kainote_gdi_base(obj); if (!base) return TRUE; delete base; return TRUE; }
inline BOOL GetTextExtentPoint32(HDC, const wchar_t* text, int len, SIZE* size) { if (size) { size->cx = std::max(0, len) * 8 * 64; size->cy = 16 * 64; } return TRUE; }
inline BOOL GetTextMetrics(HDC, TEXTMETRIC* tm) { if (tm) { tm->tmDescent = 3 * 64; tm->tmExternalLeading = 0; } return TRUE; }
inline int EnumFontFamiliesEx(HDC, LOGFONTW* requested, FONTENUMPROC proc, LPARAM lParam, DWORD) {
    if (!proc) return 0;
    std::wstring familyFilter;
    if (requested && requested->lfFaceName[0]) familyFilter = requested->lfFaceName;
    auto faces = kainote_linux_list_fonts(familyFilter);
    TEXTMETRIC tm{};
    for (const auto& face : faces) {
        LOGFONTW lf = kainote_logfont_from_face(face);
        if (!proc(&lf, &tm, 0, lParam)) break;
    }
    return static_cast<int>(faces.size());
}
inline DWORD GetFontData(HDC dc, DWORD, DWORD offset, void* buffer, DWORD length) {
    auto* context = kainote_dc(dc);
    auto* font = context ? context->selectedFont : nullptr;
    if (!font || font->file.empty()) return GDI_ERROR;
    std::ifstream input(font->file, std::ios::binary | std::ios::ate);
    if (!input) return GDI_ERROR;
    std::streamoff size = input.tellg();
    if (size < 0 || static_cast<unsigned long long>(size) > 0xffffffffull) return GDI_ERROR;
    if (!buffer || length == 0) return static_cast<DWORD>(size);
    if (offset >= static_cast<DWORD>(size)) return GDI_ERROR;
    const DWORD available = static_cast<DWORD>(size) - offset;
    const DWORD toRead = std::min(length, available);
    input.seekg(offset, std::ios::beg);
    input.read(reinterpret_cast<char*>(buffer), toRead);
    return static_cast<DWORD>(input.gcount());
}
inline HANDLE kainote_create_change_notification(const std::vector<std::string>& paths, BOOL recursive) {
    int fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (fd < 0) return INVALID_HANDLE_VALUE;
    const uint32_t mask = IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO |
                          IN_CLOSE_WRITE | IN_ATTRIB | IN_DELETE_SELF | IN_MOVE_SELF;
    auto* handle = new KainoteChangeNotificationHandle(fd, recursive != FALSE, mask);
    for (const auto& path : paths) {
        if (!path.empty()) handle->add_directory_tree(std::filesystem::path(path));
    }
    if (handle->watchPaths.empty()) {
        delete handle;
        return INVALID_HANDLE_VALUE;
    }
    return reinterpret_cast<HANDLE>(handle);
}
inline HANDLE FindFirstChangeNotification(const wchar_t* path, BOOL recursive, DWORD) {
    if (!path) return INVALID_HANDLE_VALUE;
    return kainote_create_change_notification({kainote_normalize_path(path)}, recursive);
}
inline HANDLE kainote_find_first_change_notifications(const std::vector<std::wstring>& paths, BOOL recursive, DWORD) {
    std::vector<std::string> normalized;
    normalized.reserve(paths.size());
    for (const auto& path : paths) normalized.push_back(kainote_normalize_path(path.c_str()));
    return kainote_create_change_notification(normalized, recursive);
}
inline std::vector<std::wstring> kainote_linux_font_directories() {
    std::vector<std::wstring> directories;
    if (!FcInit()) return directories;
    FcConfig* config = FcConfigGetCurrent();
    if (!config) return directories;
    FcStrList* list = FcConfigGetFontDirs(config);
    if (!list) return directories;
    while (FcChar8* directory = FcStrListNext(list)) {
		try {
			directories.push_back(kainote_utf8_to_wstring(
				reinterpret_cast<const char*>(directory)));
		} catch (const std::range_error&) {
		}
    }
    FcStrListDone(list);
    std::sort(directories.begin(), directories.end());
    directories.erase(std::unique(directories.begin(), directories.end()), directories.end());
    return directories;
}
inline BOOL FindNextChangeNotification(HANDLE h) {
    auto* base = kainote_handle(h);
    return base && base->kind == KainoteHandleBase::Kind::ChangeNotification ? TRUE : FALSE;
}
inline BOOL FindCloseChangeNotification(HANDLE h) { return CloseHandle(h) ? TRUE : FALSE; }
inline std::mutex& kainote_app_font_mutex() {
    static std::mutex mutex;
    return mutex;
}
inline std::map<std::string, std::size_t>& kainote_app_font_references() {
    static std::map<std::string, std::size_t> references;
    return references;
}
inline bool kainote_rebuild_app_fonts(FcConfig* config) {
    FcConfigAppFontClear(config);
    bool addedAll = true;
    for (const auto& [path, references] : kainote_app_font_references()) {
        (void)references;
        if (!FcConfigAppFontAddFile(config,
                reinterpret_cast<const FcChar8*>(path.c_str()))) {
            addedAll = false;
        }
    }
    return FcConfigBuildFonts(config) != FcFalse && addedAll;
}
inline int AddFontResourceExW(const wchar_t* path, DWORD, void*) {
    if (!path || !FcInit()) return 0;
    const std::string normalized = kainote_normalize_path(path);
    FcConfig* config = FcConfigGetCurrent();
    if (!config) return 0;
    std::lock_guard<std::mutex> lock(kainote_app_font_mutex());
    auto& references = kainote_app_font_references();
    auto existing = references.find(normalized);
    if (existing != references.end()) {
        ++existing->second;
        return 1;
    }
    if (!FcConfigAppFontAddFile(config,
            reinterpret_cast<const FcChar8*>(normalized.c_str()))) {
        return 0;
    }
    references.emplace(normalized, 1);
    if (FcConfigBuildFonts(config) == FcFalse) {
        references.erase(normalized);
        kainote_rebuild_app_fonts(config);
        return 0;
    }
    return 1;
}
inline BOOL RemoveFontResourceExW(const wchar_t* path, DWORD, void*) {
    if (!path || !FcInit()) return FALSE;
    const std::string normalized = kainote_normalize_path(path);
    FcConfig* config = FcConfigGetCurrent();
    if (!config) return FALSE;
    std::lock_guard<std::mutex> lock(kainote_app_font_mutex());
    auto& references = kainote_app_font_references();
    auto existing = references.find(normalized);
    if (existing == references.end()) return FALSE;
    if (existing->second > 1) {
        --existing->second;
        return TRUE;
    }
    references.erase(existing);
    if (kainote_rebuild_app_fonts(config)) return TRUE;
    references.emplace(normalized, 1);
    kainote_rebuild_app_fonts(config);
    return FALSE;
}
inline LRESULT SendMessage(HWND, UINT, WPARAM, LPARAM) { return 0; }
inline HICON GetHiconOf(...) { return nullptr; }
inline HRESULT CoInitialize(void*) { return 0; }
inline void CoUninitialize() {}
inline ITEMIDLIST* ILCreateFromPathW(const wchar_t*) { return nullptr; }
inline void ILFree(ITEMIDLIST*) {}
inline HRESULT SHOpenFolderAndSelectItems(ITEMIDLIST*, unsigned, void*, DWORD) { return 0; }
struct SHELLEXECUTEINFO { DWORD cbSize{}; const wchar_t* lpFile{}; const wchar_t* lpVerb{}; int nShow{}; DWORD fMask{}; };
template <typename T> struct WinStruct : public T { WinStruct() { std::memset(static_cast<T*>(this), 0, sizeof(T)); this->cbSize = sizeof(T); } };
constexpr int SW_RESTORE = 9;
constexpr int SW_SHOWMINNOACTIVE = 7;
constexpr int SW_MINIMIZE = 6;
constexpr DWORD SEE_MASK_FLAG_NO_UI = 0x400;
inline BOOL ShellExecuteEx(SHELLEXECUTEINFO* sei) {
    if (!sei || !sei->lpFile) return FALSE;
    const wxString target(sei->lpFile);
    if (target.StartsWith(L"http://") || target.StartsWith(L"https://"))
        return wxLaunchDefaultBrowser(target) ? TRUE : FALSE;
    return wxLaunchDefaultApplication(target) ? TRUE : FALSE;
}
struct SHFILEOPSTRUCT { HWND hwnd{}; UINT wFunc{}; LPCWSTR pFrom{}; LPCWSTR pTo{}; unsigned short fFlags{}; };
constexpr UINT FO_DELETE = 0x0003;
constexpr unsigned short FOF_SILENT = 0x0004;
constexpr unsigned short FOF_NOERRORUI = 0x0400;
constexpr unsigned short FOF_NOCONFIRMATION = 0x0010;
constexpr unsigned short FOF_ALLOWUNDO = 0x0040;
inline int SHFileOperation(SHFILEOPSTRUCT* op) {
    if (!op || op->wFunc != FO_DELETE || !op->pFrom) return 1;
    std::error_code ec;
    std::filesystem::path p(kainote_normalize_path(op->pFrom));
    if (op->fFlags & FOF_ALLOWUNDO) {
        // Recycle-bin semantics: move to the freedesktop trash instead of an
        // unrecoverable delete. Use gio (part of the GTK/glib stack). If trashing
        // is unavailable, report failure rather than silently rm the file.
        std::string quoted = "'";
        for (char c : p.string()) { if (c == '\'') quoted += "'\\''"; else quoted += c; }
        quoted += "'";
        const std::string cmd = "gio trash -- " + quoted + " >/dev/null 2>&1";
        return std::system(cmd.c_str()) == 0 ? 0 : 1;
    }
    if (std::filesystem::is_directory(p, ec)) std::filesystem::remove_all(p, ec);
    else std::filesystem::remove(p, ec);
    return ec ? 1 : 0;
}
inline BOOL ShowWindow(HWND, int) { return TRUE; }
inline HKEY HKEY_CURRENT_USER = reinterpret_cast<HKEY>(static_cast<std::uintptr_t>(1));
constexpr DWORD KEY_ALL_ACCESS = 0xf003f;
constexpr DWORD REG_OPTION_NON_VOLATILE = 0;
constexpr DWORD REG_SZ = 1;
constexpr long ERROR_SUCCESS = 0;
constexpr long ERROR_INVALID_PARAMETER = 87;
constexpr long ERROR_MORE_DATA = 234;

struct KainoteRegistryHandle {
    std::wstring path;
    explicit KainoteRegistryHandle(std::wstring p) : path(std::move(p)) {}
};

inline std::mutex& kainote_registry_mutex() { static std::mutex m; return m; }
inline char kainote_hex_digit(unsigned value) { return static_cast<char>(value < 10 ? '0' + value : 'a' + value - 10); }
inline int kainote_hex_value(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}
inline std::string kainote_hex_encode(const std::string& input) {
    std::string out;
    out.reserve(input.size() * 2);
    for (unsigned char ch : input) {
        out.push_back(kainote_hex_digit(ch >> 4));
        out.push_back(kainote_hex_digit(ch & 0xf));
    }
    return out;
}
inline std::string kainote_hex_decode(const std::string& input) {
    std::string out;
    if (input.size() % 2) return out;
    out.reserve(input.size() / 2);
    for (std::size_t i = 0; i < input.size(); i += 2) {
        int hi = kainote_hex_value(input[i]);
        int lo = kainote_hex_value(input[i + 1]);
        if (hi < 0 || lo < 0) return {};
        out.push_back(static_cast<char>((hi << 4) | lo));
    }
    return out;
}
inline std::string kainote_registry_encode(const std::wstring& input) { return kainote_hex_encode(kainote_wstring_to_utf8(input)); }
inline std::wstring kainote_registry_decode(const std::string& input) { return kainote_utf8_to_wstring(kainote_hex_decode(input)); }
inline std::filesystem::path kainote_registry_store_path() {
    const char* xdg = std::getenv("XDG_CONFIG_HOME");
    std::filesystem::path dir;
    if (xdg && *xdg) dir = std::filesystem::path(xdg) / "kainote";
    else {
        const char* home = std::getenv("HOME");
        dir = std::filesystem::path(home && *home ? home : "/tmp") / ".config" / "kainote";
    }
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir / "registry-kv.txt";
}
using KainoteRegistryStore = std::map<std::wstring, std::map<std::wstring, std::wstring>>;
inline KainoteRegistryStore kainote_load_registry_store() {
    KainoteRegistryStore store;
    std::ifstream input(kainote_registry_store_path());
    std::string line;
    while (std::getline(input, line)) {
        auto first = line.find('\t');
        auto second = first == std::string::npos ? std::string::npos : line.find('\t', first + 1);
        if (first == std::string::npos || second == std::string::npos) continue;
        std::wstring key = kainote_registry_decode(line.substr(0, first));
        std::wstring name = kainote_registry_decode(line.substr(first + 1, second - first - 1));
        std::wstring value = kainote_registry_decode(line.substr(second + 1));
        if (!key.empty()) store[key][name] = value;
    }
    return store;
}
inline KainoteRegistryStore& kainote_registry_store() { static KainoteRegistryStore store = kainote_load_registry_store(); return store; }
inline void kainote_save_registry_store() {
    std::ofstream output(kainote_registry_store_path(), std::ios::trunc);
    for (const auto& [key, values] : kainote_registry_store()) {
        for (const auto& [name, value] : values) {
            output << kainote_registry_encode(key) << '\t' << kainote_registry_encode(name) << '\t' << kainote_registry_encode(value) << '\n';
        }
        if (values.empty()) output << kainote_registry_encode(key) << '\t' << '\t' << '\n';
    }
}
inline std::wstring kainote_registry_normalize_subkey(LPCWSTR subkey) {
    std::wstring value = subkey ? std::wstring(subkey) : std::wstring();
    std::replace(value.begin(), value.end(), L'/', L'\\');
    while (!value.empty() && value.front() == L'\\') value.erase(value.begin());
    while (!value.empty() && value.back() == L'\\') value.pop_back();
    return value;
}
inline std::wstring kainote_registry_base_path(HKEY key) {
    if (key == HKEY_CURRENT_USER) return L"HKEY_CURRENT_USER";
    auto* handle = reinterpret_cast<KainoteRegistryHandle*>(key);
    return handle ? handle->path : std::wstring();
}
inline std::wstring kainote_registry_join(HKEY key, LPCWSTR subkey) {
    std::wstring base = kainote_registry_base_path(key);
    std::wstring sub = kainote_registry_normalize_subkey(subkey);
    if (base.empty()) return sub;
    if (sub.empty()) return base;
    return base + L"\\" + sub;
}
inline long RegOpenKeyEx(HKEY hKey, LPCWSTR subKey, DWORD, DWORD, HKEY* out) {
    if (!out) return ERROR_INVALID_PARAMETER;
    *out = nullptr;
    std::wstring path = kainote_registry_join(hKey, subKey);
    if (path.empty()) return ERROR_INVALID_PARAMETER;
    std::lock_guard<std::mutex> lock(kainote_registry_mutex());
    auto& store = kainote_registry_store();
    if (store.find(path) == store.end()) return ERROR_FILE_NOT_FOUND;
    *out = reinterpret_cast<HKEY>(new KainoteRegistryHandle(path));
    return ERROR_SUCCESS;
}
inline long RegCreateKeyEx(HKEY hKey, LPCWSTR subKey, DWORD, LPWSTR, DWORD, DWORD, void*, HKEY* out, DWORD*) {
    if (!out) return ERROR_INVALID_PARAMETER;
    *out = nullptr;
    std::wstring path = kainote_registry_join(hKey, subKey);
    if (path.empty()) return ERROR_INVALID_PARAMETER;
    std::lock_guard<std::mutex> lock(kainote_registry_mutex());
    kainote_registry_store().try_emplace(path);
    kainote_save_registry_store();
    *out = reinterpret_cast<HKEY>(new KainoteRegistryHandle(path));
    return ERROR_SUCCESS;
}
inline long RegCloseKey(HKEY h) {
    if (!h || h == HKEY_CURRENT_USER) return ERROR_SUCCESS;
    delete reinterpret_cast<KainoteRegistryHandle*>(h);
    return ERROR_SUCCESS;
}
inline long RegSetValueExW(HKEY hKey, LPCWSTR valueName, DWORD, DWORD type, const BYTE* data, DWORD) {
    if (type != REG_SZ) return ERROR_INVALID_PARAMETER;
    std::wstring path = kainote_registry_base_path(hKey);
    if (path.empty() || hKey == HKEY_CURRENT_USER) return ERROR_INVALID_PARAMETER;
    std::wstring name = valueName ? std::wstring(valueName) : std::wstring();
    std::wstring value = data ? std::wstring(reinterpret_cast<const wchar_t*>(data)) : std::wstring();
    std::lock_guard<std::mutex> lock(kainote_registry_mutex());
    kainote_registry_store()[path][name] = value;
    kainote_save_registry_store();
    return ERROR_SUCCESS;
}
template <typename TypePtr, typename SizePtr>
inline long RegQueryValueExW(HKEY hKey, LPCWSTR valueName, DWORD*, TypePtr type, LPBYTE data, SizePtr size) {
    std::wstring path = kainote_registry_base_path(hKey);
    if (path.empty() || hKey == HKEY_CURRENT_USER) return ERROR_INVALID_PARAMETER;
    std::wstring name = valueName ? std::wstring(valueName) : std::wstring();
    std::lock_guard<std::mutex> lock(kainote_registry_mutex());
    auto keyIt = kainote_registry_store().find(path);
    if (keyIt == kainote_registry_store().end()) return ERROR_FILE_NOT_FOUND;
    auto valueIt = keyIt->second.find(name);
    if (valueIt == keyIt->second.end()) return ERROR_FILE_NOT_FOUND;
    if (type) *type = REG_SZ;
    const std::wstring& value = valueIt->second;
    const std::size_t required = (value.size() + 1) * sizeof(wchar_t);
    if (size && static_cast<std::size_t>(*size) < required) {
        *size = required;
        return ERROR_MORE_DATA;
    }
    if (data) std::wmemcpy(reinterpret_cast<wchar_t*>(data), value.c_str(), value.size() + 1);
    if (size) *size = required;
    return ERROR_SUCCESS;
}
inline long RegDeleteTree(HKEY hKey, LPCWSTR subKey) {
    std::wstring path = kainote_registry_join(hKey, subKey);
    if (path.empty()) return ERROR_INVALID_PARAMETER;
    std::lock_guard<std::mutex> lock(kainote_registry_mutex());
    auto& store = kainote_registry_store();
    bool removed = false;
    for (auto it = store.begin(); it != store.end();) {
        if (it->first == path || it->first.rfind(path + L"\\", 0) == 0) {
            it = store.erase(it);
            removed = true;
        } else {
            ++it;
        }
    }
    if (removed) kainote_save_registry_store();
    return removed ? ERROR_SUCCESS : ERROR_FILE_NOT_FOUND;
}
constexpr long SHCNE_ASSOCCHANGED = 0x08000000L;
constexpr unsigned SHCNF_IDLIST = 0;
inline void SHChangeNotify(long, unsigned, const void*, const void*) {}
inline UINT RegisterWindowMessage(LPCWSTR) { static UINT next = 0xC000; return next++; }
struct STARTUPINFO { DWORD cb{}; };
struct PROCESS_INFORMATION { HANDLE hProcess{}; HANDLE hThread{}; DWORD dwProcessId{}; DWORD dwThreadId{}; };
inline std::vector<std::wstring> kainote_parse_windows_command_line(LPCWSTR commandLine) {
    std::vector<std::wstring> arguments;
    if (!commandLine) return arguments;
    const wchar_t* cursor = commandLine;
    while (*cursor) {
        while (*cursor && std::iswspace(*cursor)) ++cursor;
        if (!*cursor) break;
        std::wstring argument;
        bool quoted = false;
        while (*cursor && (quoted || !std::iswspace(*cursor))) {
            if (*cursor == L'\\') {
                std::size_t slashCount = 0;
                while (*cursor == L'\\') {
                    ++slashCount;
                    ++cursor;
                }
                if (*cursor == L'"') {
                    argument.append(slashCount / 2, L'\\');
                    if (slashCount % 2) {
                        argument.push_back(L'"');
                        ++cursor;
                    } else {
                        quoted = !quoted;
                        ++cursor;
                    }
                } else {
                    argument.append(slashCount, L'\\');
                }
            } else if (*cursor == L'"') {
                quoted = !quoted;
                ++cursor;
            } else {
                argument.push_back(*cursor++);
            }
        }
        arguments.push_back(std::move(argument));
    }
    return arguments;
}
inline BOOL CreateProcessW(LPCWSTR applicationName, LPWSTR commandLine, void*, void*, BOOL, DWORD, void*, LPCWSTR, STARTUPINFO*, PROCESS_INFORMATION* processInfo) {
    auto arguments = kainote_parse_windows_command_line(commandLine);
    if (arguments.empty() && applicationName) arguments.emplace_back(applicationName);
    if (arguments.empty()) return FALSE;
    std::vector<const wchar_t*> argv;
    argv.reserve(arguments.size() + 1);
    for (const auto& argument : arguments) argv.push_back(argument.c_str());
    argv.push_back(nullptr);
    const long pid = wxExecute(argv.data(), wxEXEC_ASYNC);
    if (pid == 0) return FALSE;
    if (processInfo) {
        *processInfo = PROCESS_INFORMATION{};
        processInfo->dwProcessId = static_cast<DWORD>(pid);
    }
    return TRUE;
}
constexpr const wchar_t* RT_RCDATA = L"RCDATA";
inline HRSRC FindResource(void*, const wchar_t*, const wchar_t*) { return nullptr; }
inline HGLOBAL LoadResource(void*, HRSRC) { return nullptr; }
inline void* LockResource(HGLOBAL) { return nullptr; }
inline DWORD SizeofResource(void*, HRSRC) { return 0; }
#define GetHwnd GetHandle
constexpr DWORD FILE_ATTRIBUTE_DIRECTORY = 0x10;
constexpr DWORD LOAD_WITH_ALTERED_SEARCH_PATH = 0x00000008;
inline HMODULE LoadLibraryExW(const wchar_t* filename, void*, DWORD) {
    std::string path = kainote_normalize_path(filename);
    return dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
}
inline FARPROC GetProcAddress(HMODULE module, const char* name) { return module ? dlsym(module, name) : nullptr; }
inline BOOL FreeLibrary(HMODULE module) { return module ? (dlclose(module) == 0) : FALSE; }
inline DWORD GetModuleFileNameW(HMODULE, wchar_t* buffer, DWORD size) {
    if (!buffer || size == 0) return 0;
    char path[MAX_PATH]{};
    ssize_t n = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (n < 0) return 0;
    path[n] = '\0';
    auto w = kainote_utf8_to_wstring(path);
    std::wcsncpy(buffer, w.c_str(), size - 1);
    buffer[size - 1] = L'\0';
    return static_cast<DWORD>(std::wcslen(buffer));
}
inline DWORD GetFileAttributesW(const wchar_t* path) { std::string p = kainote_normalize_path(path); struct stat st{}; if (stat(p.c_str(), &st) != 0) return INVALID_FILE_ATTRIBUTES; DWORD attr = FILE_ATTRIBUTE_NORMAL; if (S_ISDIR(st.st_mode)) attr |= FILE_ATTRIBUTE_DIRECTORY; if (access(p.c_str(), W_OK) != 0) attr |= FILE_ATTRIBUTE_READONLY; return attr; }
#ifndef GetHWND
#define GetHWND GetHandle
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef S_OK
#define S_OK 0
#define E_FAIL 0x80004005L
#endif

#ifndef __stdcall
#define __stdcall
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#define FAILED(hr) ((HRESULT)(hr) < 0)
#define SUCCEEDED(hr) ((HRESULT)(hr) >= 0)

struct tagRECT { long left, top, right, bottom; };
using RECT = tagRECT;
using LPRECT = RECT*;
struct MONITORINFO { DWORD cbSize{sizeof(MONITORINFO)}; RECT rcMonitor{0,0,1920,1080}; RECT rcWork{0,0,1920,1040}; DWORD dwFlags{1}; };
constexpr DWORD MONITORINFOF_PRIMARY = 1;
using MONITORENUMPROC = int (CALLBACK *)(HMONITOR, HDC, LPRECT, LPARAM);
inline BOOL GetMonitorInfo(HMONITOR, MONITORINFO* info) { if (info) { info->rcMonitor = {0,0,1920,1080}; info->rcWork = {0,0,1920,1040}; info->dwFlags = MONITORINFOF_PRIMARY; } return TRUE; }
inline BOOL EnumDisplayMonitors(HDC, const RECT*, MONITORENUMPROC proc, LPARAM data) { if (!proc) return FALSE; RECT r{0,0,1920,1080}; return proc(nullptr, nullptr, &r, data); }

#endif // !_WIN32
