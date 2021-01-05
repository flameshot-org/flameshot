// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/common.h>
#endif

namespace spdlog {
namespace level {
static string_view_t level_string_views[] SPDLOG_LEVEL_NAMES;

static const char *short_level_names[] SPDLOG_SHORT_LEVEL_NAMES;

SPDLOG_INLINE string_view_t &to_string_view(spdlog::level::level_enum l) SPDLOG_NOEXCEPT
{
    return level_string_views[l];
}

SPDLOG_INLINE const char *to_short_c_str(spdlog::level::level_enum l) SPDLOG_NOEXCEPT
{
    return short_level_names[l];
}

SPDLOG_INLINE spdlog::level::level_enum from_str(const std::string &name) SPDLOG_NOEXCEPT
{
    int level = 0;
    for (const auto &level_str : level_string_views)
    {
        if (level_str == name)
        {
            return static_cast<level::level_enum>(level);
        }
        level++;
    }
    // check also for "warn" and "err" before giving up..
    if (name == "warn")
    {
        return level::warn;
    }
    if (name == "err")
    {
        return level::err;
    }
    return level::off;
}
} // namespace level

SPDLOG_INLINE spdlog_ex::spdlog_ex(std::string msg)
    : msg_(std::move(msg))
{}

SPDLOG_INLINE spdlog_ex::spdlog_ex(const std::string &msg, int last_errno)
{
    memory_buf_t outbuf;
    fmt::format_system_error(outbuf, last_errno, msg);
    msg_ = fmt::to_string(outbuf);
}

SPDLOG_INLINE const char *spdlog_ex::what() const SPDLOG_NOEXCEPT
{
    return msg_.c_str();
}

SPDLOG_INLINE void throw_spdlog_ex(const std::string &msg, int last_errno)
{
    SPDLOG_THROW(spdlog_ex(msg, last_errno));
}

SPDLOG_INLINE void throw_spdlog_ex(std::string msg)
{
    SPDLOG_THROW(spdlog_ex(std::move(msg)));
}

} // namespace spdlog
