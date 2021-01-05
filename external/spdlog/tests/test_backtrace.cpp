#include "includes.h"
#include "test_sink.h"
#include "spdlog/async.h"

TEST_CASE("bactrace1", "[bactrace]")
{

    using spdlog::sinks::test_sink_st;
    auto test_sink = std::make_shared<test_sink_st>();
    size_t backtrace_size = 5;

    spdlog::logger logger("test-backtrace", test_sink);
    logger.set_pattern("%v");
    logger.enable_backtrace(backtrace_size);

    logger.info("info message");
    for (int i = 0; i < 100; i++)
        logger.debug("debug message {}", i);

    REQUIRE(test_sink->lines().size() == 1);
    REQUIRE(test_sink->lines()[0] == "info message");

    logger.dump_backtrace();
    REQUIRE(test_sink->lines().size() == backtrace_size + 3);
    REQUIRE(test_sink->lines()[1] == "****************** Backtrace Start ******************");
    REQUIRE(test_sink->lines()[2] == "debug message 95");
    REQUIRE(test_sink->lines()[3] == "debug message 96");
    REQUIRE(test_sink->lines()[4] == "debug message 97");
    REQUIRE(test_sink->lines()[5] == "debug message 98");
    REQUIRE(test_sink->lines()[6] == "debug message 99");
    REQUIRE(test_sink->lines()[7] == "****************** Backtrace End ********************");
}

TEST_CASE("bactrace-async", "[bactrace]")
{
    using spdlog::sinks::test_sink_mt;
    auto test_sink = std::make_shared<test_sink_mt>();
    using spdlog::details::os::sleep_for_millis;

    size_t backtrace_size = 5;

    spdlog::init_thread_pool(120, 1);
    auto logger = std::make_shared<spdlog::async_logger>("test-bactrace-async", test_sink, spdlog::thread_pool());
    logger->set_pattern("%v");
    logger->enable_backtrace(backtrace_size);

    logger->info("info message");
    for (int i = 0; i < 100; i++)
        logger->debug("debug message {}", i);

    sleep_for_millis(10);
    REQUIRE(test_sink->lines().size() == 1);
    REQUIRE(test_sink->lines()[0] == "info message");

    logger->dump_backtrace();
    sleep_for_millis(100); //  give time for the async dump to complete
    REQUIRE(test_sink->lines().size() == backtrace_size + 3);
    REQUIRE(test_sink->lines()[1] == "****************** Backtrace Start ******************");
    REQUIRE(test_sink->lines()[2] == "debug message 95");
    REQUIRE(test_sink->lines()[3] == "debug message 96");
    REQUIRE(test_sink->lines()[4] == "debug message 97");
    REQUIRE(test_sink->lines()[5] == "debug message 98");
    REQUIRE(test_sink->lines()[6] == "debug message 99");
    REQUIRE(test_sink->lines()[7] == "****************** Backtrace End ********************");
}
