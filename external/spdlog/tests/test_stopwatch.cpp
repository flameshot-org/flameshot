#include "includes.h"
#include "test_sink.h"
#include "spdlog/stopwatch.h"

TEST_CASE("stopwatch1", "[stopwatch]")
{
    using std::chrono::milliseconds;
    milliseconds wait_ms(250);
    milliseconds tolerance_ms(250);

    spdlog::stopwatch sw;
    std::this_thread::sleep_for(wait_ms);
    REQUIRE(sw.elapsed() >= wait_ms);
    REQUIRE(sw.elapsed() <= wait_ms + tolerance_ms);
}

TEST_CASE("stopwatch2", "[stopwatch]")
{
    using spdlog::sinks::test_sink_st;

    std::chrono::duration<double> wait_duration(0.250);
    std::chrono::duration<double> tolerance_duration(0.250);

    auto test_sink = std::make_shared<test_sink_st>();

    spdlog::stopwatch sw;
    spdlog::logger logger("test-stopwatch", test_sink);
    logger.set_pattern("%v");
    std::this_thread::sleep_for(wait_duration);
    logger.info("{}", sw);
    auto val = std::stod(test_sink->lines()[0]);

    REQUIRE(val >= wait_duration.count());
    REQUIRE(val <= (wait_duration + tolerance_duration).count());
}
