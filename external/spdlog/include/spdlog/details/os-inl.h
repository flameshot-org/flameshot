// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/details/os.h>
#endif

#include <spdlog/common.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <thread>
#include <array>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32

#include <io.h>      // _get_osfhandle and _isatty support
#include <process.h> //  _get_pid support
#include <spdlog/details/windows_include.h>

#ifdef __MINGW32__
#include <share.h>
#endif

#if defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT) || defined(SPDLOG_WCHAR_FILENAMES)
#include <limits>
#endif

#include <direct.h> // for _mkdir/_wmkdir

#else // unix

#include <fcntl.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/syscall.h> //Use gettid() syscall under linux to get thread id

#elif defined(_AIX)
#include <pthread.h> // for pthread_getthreadid_np

#elif defined(__DragonFly__) || defined(__FreeBSD__)
#include <pthread_np.h> // for pthread_getthreadid_np

#elif defined(__NetBSD__)
#include <lwp.h> // for _lwp_self

#elif defined(__sun)
#include <thread.h> // for thr_self
#endif

#endif // unix

#ifndef __has_feature      // Clang - feature checking macros.
#define __has_feature(x) 0 // Compatibility with non-clang compilers.
#endif

namespace spdlog {
namespace details {
namespace os {

SPDLOG_INLINE spdlog::log_clock::time_point now() SPDLOG_NOEXCEPT
{

#if defined __linux__ && defined SPDLOG_CLOCK_COARSE
    timespec ts;
    ::clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    return std::chrono::time_point<log_clock, typename log_clock::duration>(
        std::chrono::duration_cast<typename log_clock::duration>(std::chrono::seconds(ts.tv_sec) + std::chrono::nanoseconds(ts.tv_nsec)));

#else
    return log_clock::now();
#endif
}
SPDLOG_INLINE std::tm localtime(const std::time_t &time_tt) SPDLOG_NOEXCEPT
{

#ifdef _WIN32
    std::tm tm;
    ::localtime_s(&tm, &time_tt);
#else
    std::tm tm;
    ::localtime_r(&time_tt, &tm);
#endif
    return tm;
}

SPDLOG_INLINE std::tm localtime() SPDLOG_NOEXCEPT
{
    std::time_t now_t = ::time(nullptr);
    return localtime(now_t);
}

SPDLOG_INLINE std::tm gmtime(const std::time_t &time_tt) SPDLOG_NOEXCEPT
{

#ifdef _WIN32
    std::tm tm;
    ::gmtime_s(&tm, &time_tt);
#else
    std::tm tm;
    ::gmtime_r(&time_tt, &tm);
#endif
    return tm;
}

SPDLOG_INLINE std::tm gmtime() SPDLOG_NOEXCEPT
{
    std::time_t now_t = ::time(nullptr);
    return gmtime(now_t);
}

// fopen_s on non windows for writing
SPDLOG_INLINE bool fopen_s(FILE **fp, const filename_t &filename, const filename_t &mode)
{
#ifdef _WIN32
#ifdef SPDLOG_WCHAR_FILENAMES
    *fp = ::_wfsopen((filename.c_str()), mode.c_str(), _SH_DENYNO);
#else
    *fp = ::_fsopen((filename.c_str()), mode.c_str(), _SH_DENYNO);
#endif
#if defined(SPDLOG_PREVENT_CHILD_FD)
    if (*fp != nullptr)
    {
        auto file_handle = reinterpret_cast<HANDLE>(_get_osfhandle(::_fileno(*fp)));
        if (!::SetHandleInformation(file_handle, HANDLE_FLAG_INHERIT, 0))
        {
            ::fclose(*fp);
            *fp = nullptr;
        }
    }
#endif
#else // unix
#if defined(SPDLOG_PREVENT_CHILD_FD)
    const int mode_flag = mode == SPDLOG_FILENAME_T("ab") ? O_APPEND : O_TRUNC;
    const int fd = ::open((filename.c_str()), O_CREAT | O_WRONLY | O_CLOEXEC | mode_flag, mode_t(0644));
    if (fd == -1)
    {
        return false;
    }
    *fp = ::fdopen(fd, mode.c_str());
    if (*fp == nullptr)
    {
        ::close(fd);
    }
#else
    *fp = ::fopen((filename.c_str()), mode.c_str());
#endif
#endif

    return *fp == nullptr;
}

SPDLOG_INLINE int remove(const filename_t &filename) SPDLOG_NOEXCEPT
{
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
    return ::_wremove(filename.c_str());
#else
    return std::remove(filename.c_str());
#endif
}

SPDLOG_INLINE int remove_if_exists(const filename_t &filename) SPDLOG_NOEXCEPT
{
    return path_exists(filename) ? remove(filename) : 0;
}

SPDLOG_INLINE int rename(const filename_t &filename1, const filename_t &filename2) SPDLOG_NOEXCEPT
{
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
    return ::_wrename(filename1.c_str(), filename2.c_str());
#else
    return std::rename(filename1.c_str(), filename2.c_str());
#endif
}

// Return true if path exists (file or directory)
SPDLOG_INLINE bool path_exists(const filename_t &filename) SPDLOG_NOEXCEPT
{
#ifdef _WIN32
#ifdef SPDLOG_WCHAR_FILENAMES
    auto attribs = ::GetFileAttributesW(filename.c_str());
#else
    auto attribs = ::GetFileAttributesA(filename.c_str());
#endif
    return attribs != INVALID_FILE_ATTRIBUTES;
#else // common linux/unix all have the stat system call
    struct stat buffer;
    return (::stat(filename.c_str(), &buffer) == 0);
#endif
}

// Return file size according to open FILE* object
SPDLOG_INLINE size_t filesize(FILE *f)
{
    if (f == nullptr)
    {
        throw_spdlog_ex("Failed getting file size. fd is null");
    }
#if defined(_WIN32) && !defined(__CYGWIN__)
    int fd = ::_fileno(f);
#if _WIN64 // 64 bits
    __int64 ret = ::_filelengthi64(fd);
    if (ret >= 0)
    {
        return static_cast<size_t>(ret);
    }

#else // windows 32 bits
    long ret = ::_filelength(fd);
    if (ret >= 0)
    {
        return static_cast<size_t>(ret);
    }
#endif

#else // unix
// OpenBSD doesn't compile with :: before the fileno(..)
#if defined(__OpenBSD__)
    int fd = fileno(f);
#else
    int fd = ::fileno(f);
#endif
// 64 bits(but not in osx or cygwin, where fstat64 is deprecated)
#if (defined(__linux__) || defined(__sun) || defined(_AIX)) && (defined(__LP64__) || defined(_LP64))
    struct stat64 st;
    if (::fstat64(fd, &st) == 0)
    {
        return static_cast<size_t>(st.st_size);
    }
#else // other unix or linux 32 bits or cygwin
    struct stat st;
    if (::fstat(fd, &st) == 0)
    {
        return static_cast<size_t>(st.st_size);
    }
#endif
#endif
    throw_spdlog_ex("Failed getting file size from fd", errno);
    return 0; // will not be reached.
}

// Return utc offset in minutes or throw spdlog_ex on failure
SPDLOG_INLINE int utc_minutes_offset(const std::tm &tm)
{

#ifdef _WIN32
#if _WIN32_WINNT < _WIN32_WINNT_WS08
    TIME_ZONE_INFORMATION tzinfo;
    auto rv = ::GetTimeZoneInformation(&tzinfo);
#else
    DYNAMIC_TIME_ZONE_INFORMATION tzinfo;
    auto rv = ::GetDynamicTimeZoneInformation(&tzinfo);
#endif
    if (rv == TIME_ZONE_ID_INVALID)
        throw_spdlog_ex("Failed getting timezone info. ", errno);

    int offset = -tzinfo.Bias;
    if (tm.tm_isdst)
    {
        offset -= tzinfo.DaylightBias;
    }
    else
    {
        offset -= tzinfo.StandardBias;
    }
    return offset;
#else

#if defined(sun) || defined(__sun) || defined(_AIX) || (!defined(_BSD_SOURCE) && !defined(_GNU_SOURCE))
    // 'tm_gmtoff' field is BSD extension and it's missing on SunOS/Solaris
    struct helper
    {
        static long int calculate_gmt_offset(const std::tm &localtm = details::os::localtime(), const std::tm &gmtm = details::os::gmtime())
        {
            int local_year = localtm.tm_year + (1900 - 1);
            int gmt_year = gmtm.tm_year + (1900 - 1);

            long int days = (
                // difference in day of year
                localtm.tm_yday -
                gmtm.tm_yday

                // + intervening leap days
                + ((local_year >> 2) - (gmt_year >> 2)) - (local_year / 100 - gmt_year / 100) +
                ((local_year / 100 >> 2) - (gmt_year / 100 >> 2))

                // + difference in years * 365 */
                + (long int)(local_year - gmt_year) * 365);

            long int hours = (24 * days) + (localtm.tm_hour - gmtm.tm_hour);
            long int mins = (60 * hours) + (localtm.tm_min - gmtm.tm_min);
            long int secs = (60 * mins) + (localtm.tm_sec - gmtm.tm_sec);

            return secs;
        }
    };

    auto offset_seconds = helper::calculate_gmt_offset(tm);
#else
    auto offset_seconds = tm.tm_gmtoff;
#endif

    return static_cast<int>(offset_seconds / 60);
#endif
}

// Return current thread id as size_t
// It exists because the std::this_thread::get_id() is much slower(especially
// under VS 2013)
SPDLOG_INLINE size_t _thread_id() SPDLOG_NOEXCEPT
{
#ifdef _WIN32
    return static_cast<size_t>(::GetCurrentThreadId());
#elif defined(__linux__)
#if defined(__ANDROID__) && defined(__ANDROID_API__) && (__ANDROID_API__ < 21)
#define SYS_gettid __NR_gettid
#endif
    return static_cast<size_t>(::syscall(SYS_gettid));
#elif defined(_AIX) || defined(__DragonFly__) || defined(__FreeBSD__)
    return static_cast<size_t>(::pthread_getthreadid_np());
#elif defined(__NetBSD__)
    return static_cast<size_t>(::_lwp_self());
#elif defined(__OpenBSD__)
    return static_cast<size_t>(::getthrid());
#elif defined(__sun)
    return static_cast<size_t>(::thr_self());
#elif __APPLE__
    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    return static_cast<size_t>(tid);
#else // Default to standard C++11 (other Unix)
    return static_cast<size_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
#endif
}

// Return current thread id as size_t (from thread local storage)
SPDLOG_INLINE size_t thread_id() SPDLOG_NOEXCEPT
{
#if defined(SPDLOG_NO_TLS)
    return _thread_id();
#else // cache thread id in tls
    static thread_local const size_t tid = _thread_id();
    return tid;
#endif
}

// This is avoid msvc issue in sleep_for that happens if the clock changes.
// See https://github.com/gabime/spdlog/issues/609
SPDLOG_INLINE void sleep_for_millis(int milliseconds) SPDLOG_NOEXCEPT
{
#if defined(_WIN32)
    ::Sleep(milliseconds);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
#endif
}

// wchar support for windows file names (SPDLOG_WCHAR_FILENAMES must be defined)
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
SPDLOG_INLINE std::string filename_to_str(const filename_t &filename)
{
    memory_buf_t buf;
    wstr_to_utf8buf(filename, buf);
    return fmt::to_string(buf);
}
#else
SPDLOG_INLINE std::string filename_to_str(const filename_t &filename)
{
    return filename;
}
#endif

SPDLOG_INLINE int pid() SPDLOG_NOEXCEPT
{

#ifdef _WIN32
    return static_cast<int>(::GetCurrentProcessId());
#else
    return static_cast<int>(::getpid());
#endif
}

// Determine if the terminal supports colors
// Based on: https://github.com/agauniyal/rang/
SPDLOG_INLINE bool is_color_terminal() SPDLOG_NOEXCEPT
{
#ifdef _WIN32
    return true;
#else

    static const bool result = []() {
        const char *env_colorterm_p = std::getenv("COLORTERM");
        if (env_colorterm_p != nullptr)
        {
            return true;
        }

        static constexpr std::array<const char *, 15> terms = {{"ansi", "color", "console", "cygwin", "gnome", "konsole", "kterm", "linux",
            "msys", "putty", "rxvt", "screen", "vt100", "xterm", "alacritty"}};

        const char *env_term_p = std::getenv("TERM");
        if (env_term_p == nullptr)
        {
            return false;
        }

        return std::any_of(terms.begin(), terms.end(), [&](const char *term) { return std::strstr(env_term_p, term) != nullptr; });
    }();

    return result;
#endif
}

// Determine if the terminal attached
// Source: https://github.com/agauniyal/rang/
SPDLOG_INLINE bool in_terminal(FILE *file) SPDLOG_NOEXCEPT
{

#ifdef _WIN32
    return ::_isatty(_fileno(file)) != 0;
#else
    return ::isatty(fileno(file)) != 0;
#endif
}

#if (defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT) || defined(SPDLOG_WCHAR_FILENAMES)) && defined(_WIN32)
SPDLOG_INLINE void wstr_to_utf8buf(wstring_view_t wstr, memory_buf_t &target)
{
    if (wstr.size() > static_cast<size_t>((std::numeric_limits<int>::max)()))
    {
        throw_spdlog_ex("UTF-16 string is too big to be converted to UTF-8");
    }

    int wstr_size = static_cast<int>(wstr.size());
    if (wstr_size == 0)
    {
        target.resize(0);
        return;
    }

    int result_size = static_cast<int>(target.capacity());
    if ((wstr_size + 1) * 2 > result_size)
    {
        result_size = ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr_size, NULL, 0, NULL, NULL);
    }

    if (result_size > 0)
    {
        target.resize(result_size);
        result_size = ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr_size, target.data(), result_size, NULL, NULL);

        if (result_size > 0)
        {
            target.resize(result_size);
            return;
        }
    }

    throw_spdlog_ex(fmt::format("WideCharToMultiByte failed. Last error: {}", ::GetLastError()));
}

SPDLOG_INLINE void utf8_to_wstrbuf(string_view_t str, memory_buf_t &target)
{
    if (str.size() > static_cast<size_t>((std::numeric_limits<int>::max)()))
    {
        throw_spdlog_ex("UTF-8 string is too big to be converted to UTF-16");
    }

    int str_size = static_cast<int>(str.size());
    if (str_size == 0)
    {
        target.resize(0);
        return;
    }

    int result_size = static_cast<int>(target.capacity());
    if ((str_size + 1) * 2 < result_size)
    {
        result_size = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.data(), str_size, NULL, 0);
    }

    if (result_size > 0)
    {
        target.resize(result_size);
        result_size = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.data(), str_size, (LPWSTR)target.data(), result_size);

        if (result_size > 0)
        {
            target.resize(result_size);
            return;
        }
    }

    throw_spdlog_ex(fmt::format("MultiByteToWideChar failed. Last error: {}", ::GetLastError()));
}
#endif // (defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT) || defined(SPDLOG_WCHAR_FILENAMES)) && defined(_WIN32)

// return true on success
static SPDLOG_INLINE bool mkdir_(const filename_t &path)
{
#ifdef _WIN32
#ifdef SPDLOG_WCHAR_FILENAMES
    return ::_wmkdir(path.c_str()) == 0;
#else
    return ::_mkdir(path.c_str()) == 0;
#endif
#else
    return ::mkdir(path.c_str(), mode_t(0755)) == 0;
#endif
}

// create the given directory - and all directories leading to it
// return true on success or if the directory already exists
SPDLOG_INLINE bool create_dir(filename_t path)
{
    if (path_exists(path))
    {
        return true;
    }

    if (path.empty())
    {
        return false;
    }

#ifdef _WIN32
    // support forward slash in windows
    std::replace(path.begin(), path.end(), '/', folder_sep);
#endif

    size_t search_offset = 0;
    do
    {
        auto token_pos = path.find(folder_sep, search_offset);
        // treat the entire path as a folder if no folder separator not found
        if (token_pos == filename_t::npos)
        {
            token_pos = path.size();
        }

        auto subdir = path.substr(0, token_pos);

        if (!subdir.empty() && !path_exists(subdir) && !mkdir_(subdir))
        {
            return false; // return error if failed creating dir
        }
        search_offset = token_pos + 1;
    } while (search_offset < path.size());

    return true;
}

// Return directory name from given path or empty string
// "abc/file" => "abc"
// "abc/" => "abc"
// "abc" => ""
// "abc///" => "abc//"
SPDLOG_INLINE filename_t dir_name(filename_t path)
{
#ifdef _WIN32
    // support forward slash in windows
    std::replace(path.begin(), path.end(), '/', folder_sep);
#endif
    auto pos = path.find_last_of(folder_sep);
    return pos != filename_t::npos ? path.substr(0, pos) : filename_t{};
}

std::string SPDLOG_INLINE getenv(const char *field)
{

#if defined(_MSC_VER)
#if defined(__cplusplus_winrt)
    return std::string{}; // not supported under uwp
#else
    size_t len = 0;
    char buf[128];
    bool ok = ::getenv_s(&len, buf, sizeof(buf), field) == 0;
    return ok ? buf : std::string{};
#endif
#else // revert to getenv
    char *buf = ::getenv(field);
    return buf ? buf : std::string{};
#endif
}

} // namespace os
} // namespace details
} // namespace spdlog
