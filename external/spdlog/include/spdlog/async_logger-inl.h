// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/async_logger.h>
#endif

#include <spdlog/sinks/sink.h>
#include <spdlog/details/thread_pool.h>

#include <memory>
#include <string>

SPDLOG_INLINE spdlog::async_logger::async_logger(
    std::string logger_name, sinks_init_list sinks_list, std::weak_ptr<details::thread_pool> tp, async_overflow_policy overflow_policy)
    : async_logger(std::move(logger_name), sinks_list.begin(), sinks_list.end(), std::move(tp), overflow_policy)
{}

SPDLOG_INLINE spdlog::async_logger::async_logger(
    std::string logger_name, sink_ptr single_sink, std::weak_ptr<details::thread_pool> tp, async_overflow_policy overflow_policy)
    : async_logger(std::move(logger_name), {std::move(single_sink)}, std::move(tp), overflow_policy)
{}

// send the log message to the thread pool
SPDLOG_INLINE void spdlog::async_logger::sink_it_(const details::log_msg &msg)
{
    if (auto pool_ptr = thread_pool_.lock())
    {
        pool_ptr->post_log(shared_from_this(), msg, overflow_policy_);
    }
    else
    {
        throw_spdlog_ex("async log: thread pool doesn't exist anymore");
    }
}

// send flush request to the thread pool
SPDLOG_INLINE void spdlog::async_logger::flush_()
{
    if (auto pool_ptr = thread_pool_.lock())
    {
        pool_ptr->post_flush(shared_from_this(), overflow_policy_);
    }
    else
    {
        throw_spdlog_ex("async flush: thread pool doesn't exist anymore");
    }
}

//
// backend functions - called from the thread pool to do the actual job
//
SPDLOG_INLINE void spdlog::async_logger::backend_sink_it_(const details::log_msg &msg)
{
    for (auto &sink : sinks_)
    {
        if (sink->should_log(msg.level))
        {
            SPDLOG_TRY
            {
                sink->log(msg);
            }
            SPDLOG_LOGGER_CATCH()
        }
    }

    if (should_flush_(msg))
    {
        backend_flush_();
    }
}

SPDLOG_INLINE void spdlog::async_logger::backend_flush_()
{
    for (auto &sink : sinks_)
    {
        SPDLOG_TRY
        {
            sink->flush();
        }
        SPDLOG_LOGGER_CATCH()
    }
}

SPDLOG_INLINE std::shared_ptr<spdlog::logger> spdlog::async_logger::clone(std::string new_name)
{
    auto cloned = std::make_shared<spdlog::async_logger>(*this);
    cloned->name_ = std::move(new_name);
    return cloned;
}
