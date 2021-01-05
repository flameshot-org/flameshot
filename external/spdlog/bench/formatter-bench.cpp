//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "benchmark/benchmark.h"

#include "spdlog/spdlog.h"
#include "spdlog/pattern_formatter.h"

void bench_formatter(benchmark::State &state, std::string pattern)
{
    auto formatter = spdlog::details::make_unique<spdlog::pattern_formatter>(pattern);
    spdlog::memory_buf_t dest;
    std::string logger_name = "logger-name";
    const char *text = "Hello. This is some message with length of 80                                   ";

    spdlog::source_loc source_loc{"a/b/c/d/myfile.cpp", 123, "some_func()"};
    spdlog::details::log_msg msg(source_loc, logger_name, spdlog::level::info, text);

    for (auto _ : state)
    {
        dest.clear();
        formatter->format(msg, dest);
        benchmark::DoNotOptimize(dest);
    }
}

void bench_formatters()
{
    // basic patterns(single flag)
    std::string all_flags = "+vtPnlLaAbBcCYDmdHIMSefFprRTXzEisg@luioO%";
    std::vector<std::string> basic_patterns;
    for (auto &flag : all_flags)
    {
        auto pattern = std::string("%") + flag;
        benchmark::RegisterBenchmark(pattern.c_str(), bench_formatter, pattern);

        //        pattern = std::string("%16") + flag;
        //        benchmark::RegisterBenchmark(pattern.c_str(), bench_formatter, pattern);
        //
        //        // bench center padding
        //        pattern = std::string("%=16") + flag;
        //        benchmark::RegisterBenchmark(pattern.c_str(), bench_formatter, pattern);
    }

    // complex patterns
    std::vector<std::string> patterns = {
        "[%D %X] [%l] [%n] %v",
        "[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v",
        "[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] [%t] %v",
    };
    for (auto &pattern : patterns)
    {
        benchmark::RegisterBenchmark(pattern.c_str(), bench_formatter, pattern)->Iterations(2500000);
    }
}

int main(int argc, char *argv[])
{

    spdlog::set_pattern("[%^%l%$] %v");
    if (argc != 2)
    {
        spdlog::error("Usage: {} <pattern> (or \"all\" to bench all)", argv[0]);
        exit(1);
    }

    std::string pattern = argv[1];
    if (pattern == "all")
    {
        bench_formatters();
    }
    else
    {
        benchmark::RegisterBenchmark(pattern.c_str(), bench_formatter, pattern);
    }
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}
