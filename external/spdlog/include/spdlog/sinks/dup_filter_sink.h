// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "dist_sink.h"
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/log_msg.h>

#include <mutex>
#include <string>
#include <chrono>

// Duplicate message removal sink.
// Skip the message if previous one is identical and less than "max_skip_duration" have passed
//
// Example:
//
//     #include <spdlog/sinks/dup_filter_sink.h>
//
//     int main() {
//         auto dup_filter = std::make_shared<dup_filter_sink_st>(std::chrono::seconds(5));
//         dup_filter->add_sink(std::make_shared<stdout_color_sink_mt>());
//         spdlog::logger l("logger", dup_filter);
//         l.info("Hello");
//         l.info("Hello");
//         l.info("Hello");
//         l.info("Different Hello");
//     }
//
// Will produce:
//       [2019-06-25 17:50:56.511] [logger] [info] Hello
//       [2019-06-25 17:50:56.512] [logger] [info] Skipped 3 duplicate messages..
//       [2019-06-25 17:50:56.512] [logger] [info] Different Hello

namespace spdlog {
namespace sinks {
template<typename Mutex>
class dup_filter_sink : public dist_sink<Mutex>
{
public:
    template<class Rep, class Period>
    explicit dup_filter_sink(std::chrono::duration<Rep, Period> max_skip_duration)
        : max_skip_duration_{max_skip_duration}
    {}

protected:
    std::chrono::microseconds max_skip_duration_;
    log_clock::time_point last_msg_time_;
    std::string last_msg_payload_;
    size_t skip_counter_ = 0;

    void sink_it_(const details::log_msg &msg) override
    {
        bool filtered = filter_(msg);
        if (!filtered)
        {
            skip_counter_ += 1;
            return;
        }

        // log the "skipped.." message
        if (skip_counter_ > 0)
        {
            memory_buf_t buf;
            fmt::format_to(buf, "Skipped {} duplicate messages..", skip_counter_);
            details::log_msg skipped_msg{msg.logger_name, level::info, string_view_t{buf.data(), buf.size()}};
            dist_sink<Mutex>::sink_it_(skipped_msg);
        }

        // log current message
        dist_sink<Mutex>::sink_it_(msg);
        last_msg_time_ = msg.time;
        skip_counter_ = 0;
        last_msg_payload_.assign(msg.payload.data(), msg.payload.data() + msg.payload.size());
    }

    // return whether the log msg should be displayed (true) or skipped (false)
    bool filter_(const details::log_msg &msg)
    {
        auto filter_duration = msg.time - last_msg_time_;
        return (filter_duration > max_skip_duration_) || (msg.payload != last_msg_payload_);
    }
};

using dup_filter_sink_mt = dup_filter_sink<std::mutex>;
using dup_filter_sink_st = dup_filter_sink<details::null_mutex>;

} // namespace sinks
} // namespace spdlog
