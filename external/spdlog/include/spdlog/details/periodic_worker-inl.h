// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/details/periodic_worker.h>
#endif

namespace spdlog {
namespace details {

SPDLOG_INLINE periodic_worker::periodic_worker(const std::function<void()> &callback_fun, std::chrono::seconds interval)
{
    active_ = (interval > std::chrono::seconds::zero());
    if (!active_)
    {
        return;
    }

    worker_thread_ = std::thread([this, callback_fun, interval]() {
        for (;;)
        {
            std::unique_lock<std::mutex> lock(this->mutex_);
            if (this->cv_.wait_for(lock, interval, [this] { return !this->active_; }))
            {
                return; // active_ == false, so exit this thread
            }
            callback_fun();
        }
    });
}

// stop the worker thread and join it
SPDLOG_INLINE periodic_worker::~periodic_worker()
{
    if (worker_thread_.joinable())
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            active_ = false;
        }
        cv_.notify_one();
        worker_thread_.join();
    }
}

} // namespace details
} // namespace spdlog
