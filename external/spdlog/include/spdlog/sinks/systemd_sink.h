// Copyright(c) 2019 ZVYAGIN.Alexander@gmail.com
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>

#include <array>
#ifndef SD_JOURNAL_SUPPRESS_LOCATION
#define SD_JOURNAL_SUPPRESS_LOCATION
#endif
#include <systemd/sd-journal.h>

namespace spdlog {
namespace sinks {

/**
 * Sink that write to systemd journal using the `sd_journal_send()` library call.
 *
 * Locking is not needed, as `sd_journal_send()` itself is thread-safe.
 */
template<typename Mutex>
class systemd_sink : public base_sink<Mutex>
{
public:
    //
    systemd_sink()
        : syslog_levels_{{/* spdlog::level::trace      */ LOG_DEBUG,
              /* spdlog::level::debug      */ LOG_DEBUG,
              /* spdlog::level::info       */ LOG_INFO,
              /* spdlog::level::warn       */ LOG_WARNING,
              /* spdlog::level::err        */ LOG_ERR,
              /* spdlog::level::critical   */ LOG_CRIT,
              /* spdlog::level::off        */ LOG_INFO}}
    {}

    ~systemd_sink() override {}

    systemd_sink(const systemd_sink &) = delete;
    systemd_sink &operator=(const systemd_sink &) = delete;

protected:
    using levels_array = std::array<int, 7>;
    levels_array syslog_levels_;

    void sink_it_(const details::log_msg &msg) override
    {
        int err;

        size_t length = msg.payload.size();
        // limit to max int
        if (length > static_cast<size_t>(std::numeric_limits<int>::max()))
        {
            length = static_cast<size_t>(std::numeric_limits<int>::max());
        }

        // Do not send source location if not available
        if (msg.source.empty())
        {
            // Note: function call inside '()' to avoid macro expansion
            err = (sd_journal_send)("MESSAGE=%.*s", static_cast<int>(length), msg.payload.data(), "PRIORITY=%d", syslog_level(msg.level),
                "SYSLOG_IDENTIFIER=%.*s", static_cast<int>(msg.logger_name.size()), msg.logger_name.data(), nullptr);
        }
        else
        {
            err = (sd_journal_send)("MESSAGE=%.*s", static_cast<int>(length), msg.payload.data(), "PRIORITY=%d", syslog_level(msg.level),
                "SYSLOG_IDENTIFIER=%.*s", static_cast<int>(msg.logger_name.size()), msg.logger_name.data(), "CODE_FILE=%s",
                msg.source.filename, "CODE_LINE=%d", msg.source.line, "CODE_FUNC=%s", msg.source.funcname, nullptr);
        }

        if (err)
        {
            throw_spdlog_ex("Failed writing to systemd", errno);
        }
    }

    int syslog_level(level::level_enum l)
    {
        return syslog_levels_.at(static_cast<levels_array::size_type>(l));
    }

    void flush_() override {}
};

using systemd_sink_mt = systemd_sink<std::mutex>;
using systemd_sink_st = systemd_sink<details::null_mutex>;
} // namespace sinks

// Create and register a syslog logger
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> systemd_logger_mt(const std::string &logger_name)
{
    return Factory::template create<sinks::systemd_sink_mt>(logger_name);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> systemd_logger_st(const std::string &logger_name)
{
    return Factory::template create<sinks::systemd_sink_st>(logger_name);
}
} // namespace spdlog
