// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef SPDLOG_COMPILED_LIB
#error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#include <spdlog/async.h>
#include <spdlog/async_logger-inl.h>
#include <spdlog/details/periodic_worker-inl.h>
#include <spdlog/details/thread_pool-inl.h>

template class SPDLOG_API spdlog::details::mpmc_blocking_queue<spdlog::details::async_msg>;