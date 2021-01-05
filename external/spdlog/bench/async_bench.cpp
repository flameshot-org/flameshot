//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// bench.cpp : spdlog benchmarks
//
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

#ifdef SPDLOG_FMT_EXTERNAL
#include <fmt/locale.h>
#else
#include "spdlog/fmt/bundled/locale.h"
#endif

#include "utils.h"
#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using namespace std;
using namespace std::chrono;
using namespace spdlog;
using namespace spdlog::sinks;
using namespace utils;

void bench_mt(int howmany, std::shared_ptr<spdlog::logger> log, int thread_count);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // disable fopen warning under msvc
#endif // _MSC_VER

int count_lines(const char *filename)
{
    int counter = 0;
    auto *infile = fopen(filename, "r");
    int ch;
    while (EOF != (ch = getc(infile)))
    {
        if ('\n' == ch)
            counter++;
    }
    fclose(infile);

    return counter;
}

void verify_file(const char *filename, int expected_count)
{
    spdlog::info("Verifying {} to contain {} line..", filename, expected_count);
    auto count = count_lines(filename);
    if (count != expected_count)
    {
        spdlog::error("Test failed. {} has {} lines instead of {}", filename, count, expected_count);
        exit(1);
    }
    spdlog::info("Line count OK ({})\n", count);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

int main(int argc, char *argv[])
{

    int howmany = 1000000;
    int queue_size = std::min(howmany + 2, 8192);
    int threads = 10;
    int iters = 3;

    try
    {
        spdlog::set_pattern("[%^%l%$] %v");
        if (argc == 1)
        {
            spdlog::info("Usage: {} <message_count> <threads> <q_size> <iterations>", argv[0]);
            return 0;
        }

        if (argc > 1)
            howmany = atoi(argv[1]);
        if (argc > 2)
            threads = atoi(argv[2]);
        if (argc > 3)
        {
            queue_size = atoi(argv[3]);
            if (queue_size > 500000)
            {
                spdlog::error("Max queue size allowed: 500,000");
                exit(1);
            }
        }

        if (argc > 4)
            iters = atoi(argv[4]);

        auto slot_size = sizeof(spdlog::details::async_msg);
        spdlog::info("-------------------------------------------------");
        spdlog::info(fmt::format(std::locale("en_US.UTF-8"), "Messages     : {:L}", howmany));
        spdlog::info(fmt::format(std::locale("en_US.UTF-8"), "Threads      : {:L}", threads));
        spdlog::info(fmt::format(std::locale("en_US.UTF-8"), "Queue        : {:L} slots", queue_size));
        spdlog::info(fmt::format(
            std::locale("en_US.UTF-8"), "Queue memory : {:L} x {:L} = {:L} KB ", queue_size, slot_size, (queue_size * slot_size) / 1024));
        spdlog::info(fmt::format(std::locale("en_US.UTF-8"), "Total iters  : {:L}", iters));
        spdlog::info("-------------------------------------------------");

        const char *filename = "logs/basic_async.log";
        spdlog::info("");
        spdlog::info("*********************************");
        spdlog::info("Queue Overflow Policy: block");
        spdlog::info("*********************************");
        for (int i = 0; i < iters; i++)
        {
            auto tp = std::make_shared<details::thread_pool>(queue_size, 1);
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
            auto logger = std::make_shared<async_logger>("async_logger", std::move(file_sink), std::move(tp), async_overflow_policy::block);
            bench_mt(howmany, std::move(logger), threads);
            // verify_file(filename, howmany);
        }

        spdlog::info("");
        spdlog::info("*********************************");
        spdlog::info("Queue Overflow Policy: overrun");
        spdlog::info("*********************************");
        // do same test but discard oldest if queue is full instead of blocking
        filename = "logs/basic_async-overrun.log";
        for (int i = 0; i < iters; i++)
        {
            auto tp = std::make_shared<details::thread_pool>(queue_size, 1);
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
            auto logger =
                std::make_shared<async_logger>("async_logger", std::move(file_sink), std::move(tp), async_overflow_policy::overrun_oldest);
            bench_mt(howmany, std::move(logger), threads);
        }
        spdlog::shutdown();
    }
    catch (std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        perror("Last error");
        return 1;
    }
    return 0;
}

void thread_fun(std::shared_ptr<spdlog::logger> logger, int howmany)
{
    for (int i = 0; i < howmany; i++)
    {
        logger->info("Hello logger: msg number {}", i);
    }
}

void bench_mt(int howmany, std::shared_ptr<spdlog::logger> logger, int thread_count)
{
    using std::chrono::high_resolution_clock;
    vector<thread> threads;
    auto start = high_resolution_clock::now();

    int msgs_per_thread = howmany / thread_count;
    int msgs_per_thread_mod = howmany % thread_count;
    for (int t = 0; t < thread_count; ++t)
    {
        if (t == 0 && msgs_per_thread_mod)
            threads.push_back(std::thread(thread_fun, logger, msgs_per_thread + msgs_per_thread_mod));
        else
            threads.push_back(std::thread(thread_fun, logger, msgs_per_thread));
    }

    for (auto &t : threads)
    {
        t.join();
    };

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    spdlog::info(fmt::format(std::locale("en_US.UTF-8"), "Elapsed: {} secs\t {:L}/sec", delta_d, int(howmany / delta_d)));
}
