//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// latency.cpp : spdlog latency benchmarks
//

#include "benchmark/benchmark.h"

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

void bench_c_string(benchmark::State &state, std::shared_ptr<spdlog::logger> logger)
{
    const char *msg = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum pharetra metus cursus "
                      "lacus placerat congue. Nulla egestas, mauris a tincidunt tempus, enim lectus volutpat mi, eu consequat sem "
                      "libero nec massa. In dapibus ipsum a diam rhoncus gravida. Etiam non dapibus eros. Donec fringilla dui sed "
                      "augue pretium, nec scelerisque est maximus. Nullam convallis, sem nec blandit maximus, nisi turpis ornare "
                      "nisl, sit amet volutpat neque massa eu odio. Maecenas malesuada quam ex, posuere congue nibh turpis duis.";

    for (auto _ : state)
    {
        logger->info(msg);
    }
}

void bench_logger(benchmark::State &state, std::shared_ptr<spdlog::logger> logger)
{
    int i = 0;
    for (auto _ : state)
    {
        logger->info("Hello logger: msg number {}...............", ++i);
    }
}

void bench_logger_fmt_string(benchmark::State &state, std::shared_ptr<spdlog::logger> logger)
{
    int i = 0;
    for (auto _ : state)
    {
        logger->info(FMT_STRING("Hello logger: msg number {}..............."), ++i);
        ;
    }
}

void bench_disabled_macro(benchmark::State &state, std::shared_ptr<spdlog::logger> logger)
{
    int i = 0;
    benchmark::DoNotOptimize(i);      // prevent unused warnings
    benchmark::DoNotOptimize(logger); // prevent unused warnings
    for (auto _ : state)
    {
        SPDLOG_LOGGER_DEBUG(logger, "Hello logger: msg number {}...............", i++);
        SPDLOG_DEBUG("Hello logger: msg number {}...............", i++);
    }
}

#ifdef __linux__
void bench_dev_null()
{
    auto dev_null_st = spdlog::basic_logger_st("/dev/null_st", "/dev/null");
    benchmark::RegisterBenchmark("/dev/null_st", bench_logger, std::move(dev_null_st))->UseRealTime();
    spdlog::drop("/dev/null_st");

    auto dev_null_mt = spdlog::basic_logger_mt("/dev/null_mt", "/dev/null");
    benchmark::RegisterBenchmark("/dev/null_mt", bench_logger, std::move(dev_null_mt))->UseRealTime();
    spdlog::drop("/dev/null_mt");
}
#endif // __linux__

int main(int argc, char *argv[])
{
    using spdlog::sinks::null_sink_mt;
    using spdlog::sinks::null_sink_st;

    size_t file_size = 30 * 1024 * 1024;
    size_t rotating_files = 5;
    int n_threads = benchmark::CPUInfo::Get().num_cpus;

    auto full_bench = argc > 1 && std::string(argv[1]) == "full";

    // disabled loggers
    auto disabled_logger = std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_mt>());
    disabled_logger->set_level(spdlog::level::off);
    benchmark::RegisterBenchmark("disabled-at-compile-time", bench_disabled_macro, disabled_logger);
    benchmark::RegisterBenchmark("disabled-at-runtime", bench_logger, disabled_logger);
    // with backtrace of 64
    auto tracing_disabled_logger = std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_mt>());
    tracing_disabled_logger->enable_backtrace(64);
    benchmark::RegisterBenchmark("disabled-at-runtime/backtrace", bench_logger, tracing_disabled_logger);

    auto null_logger_st = std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_st>());
    benchmark::RegisterBenchmark("null_sink_st (500_bytes c_str)", bench_c_string, std::move(null_logger_st));
    benchmark::RegisterBenchmark("null_sink_st", bench_logger, null_logger_st);
    benchmark::RegisterBenchmark("null_sink_fmt_string", bench_logger_fmt_string, null_logger_st);
    // with backtrace of 64
    auto tracing_null_logger_st = std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_st>());
    tracing_null_logger_st->enable_backtrace(64);
    benchmark::RegisterBenchmark("null_sink_st/backtrace", bench_logger, tracing_null_logger_st);

#ifdef __linux
    bench_dev_null();
#endif // __linux__

    if (full_bench)
    {
        // basic_st
        auto basic_st = spdlog::basic_logger_st("basic_st", "latency_logs/basic_st.log", true);
        benchmark::RegisterBenchmark("basic_st", bench_logger, std::move(basic_st))->UseRealTime();
        spdlog::drop("basic_st");
        // with backtrace of 64
        auto tracing_basic_st = spdlog::basic_logger_st("tracing_basic_st", "latency_logs/tracing_basic_st.log", true);
        tracing_basic_st->enable_backtrace(64);
        benchmark::RegisterBenchmark("basic_st/backtrace", bench_logger, std::move(tracing_basic_st))->UseRealTime();
        spdlog::drop("tracing_basic_st");

        // rotating st
        auto rotating_st = spdlog::rotating_logger_st("rotating_st", "latency_logs/rotating_st.log", file_size, rotating_files);
        benchmark::RegisterBenchmark("rotating_st", bench_logger, std::move(rotating_st))->UseRealTime();
        spdlog::drop("rotating_st");
        // with backtrace of 64
        auto tracing_rotating_st =
            spdlog::rotating_logger_st("tracing_rotating_st", "latency_logs/tracing_rotating_st.log", file_size, rotating_files);
        benchmark::RegisterBenchmark("rotating_st/backtrace", bench_logger, std::move(tracing_rotating_st))->UseRealTime();
        spdlog::drop("tracing_rotating_st");

        // daily st
        auto daily_st = spdlog::daily_logger_mt("daily_st", "latency_logs/daily_st.log");
        benchmark::RegisterBenchmark("daily_st", bench_logger, std::move(daily_st))->UseRealTime();
        spdlog::drop("daily_st");
        auto tracing_daily_st = spdlog::daily_logger_mt("tracing_daily_st", "latency_logs/daily_st.log");
        benchmark::RegisterBenchmark("daily_st/backtrace", bench_logger, std::move(tracing_daily_st))->UseRealTime();
        spdlog::drop("tracing_daily_st");

        //
        // Multi threaded bench, 10 loggers using same logger concurrently
        //
        auto null_logger_mt = std::make_shared<spdlog::logger>("bench", std::make_shared<null_sink_mt>());
        benchmark::RegisterBenchmark("null_sink_mt", bench_logger, null_logger_mt)->Threads(n_threads)->UseRealTime();

        // basic_mt
        auto basic_mt = spdlog::basic_logger_mt("basic_mt", "latency_logs/basic_mt.log", true);
        benchmark::RegisterBenchmark("basic_mt", bench_logger, std::move(basic_mt))->Threads(n_threads)->UseRealTime();
        spdlog::drop("basic_mt");

        // rotating mt
        auto rotating_mt = spdlog::rotating_logger_mt("rotating_mt", "latency_logs/rotating_mt.log", file_size, rotating_files);
        benchmark::RegisterBenchmark("rotating_mt", bench_logger, std::move(rotating_mt))->Threads(n_threads)->UseRealTime();
        spdlog::drop("rotating_mt");

        // daily mt
        auto daily_mt = spdlog::daily_logger_mt("daily_mt", "latency_logs/daily_mt.log");
        benchmark::RegisterBenchmark("daily_mt", bench_logger, std::move(daily_mt))->Threads(n_threads)->UseRealTime();
        spdlog::drop("daily_mt");
    }

    // async
    auto queue_size = 1024 * 1024 * 3;
    auto tp = std::make_shared<spdlog::details::thread_pool>(queue_size, 1);
    auto async_logger = std::make_shared<spdlog::async_logger>(
        "async_logger", std::make_shared<null_sink_mt>(), std::move(tp), spdlog::async_overflow_policy::overrun_oldest);
    benchmark::RegisterBenchmark("async_logger", bench_logger, async_logger)->Threads(n_threads)->UseRealTime();

    auto async_logger_tracing = std::make_shared<spdlog::async_logger>(
        "async_logger_tracing", std::make_shared<null_sink_mt>(), std::move(tp), spdlog::async_overflow_policy::overrun_oldest);
    async_logger_tracing->enable_backtrace(32);
    benchmark::RegisterBenchmark("async_logger/tracing", bench_logger, async_logger_tracing)->Threads(n_threads)->UseRealTime();

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}
