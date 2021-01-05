// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/null_mutex.h>
#include <mutex>

namespace spdlog {
namespace details {

struct console_mutex
{
    using mutex_t = std::mutex;
    static mutex_t &mutex()
    {
        static mutex_t s_mutex;
        return s_mutex;
    }
};

struct console_nullmutex
{
    using mutex_t = null_mutex;
    static mutex_t &mutex()
    {
        static mutex_t s_mutex;
        return s_mutex;
    }
};
} // namespace details
} // namespace spdlog
