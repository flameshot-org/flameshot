#include "includes.h"
#include "test_sink.h"
#include "spdlog/async.h"

TEST_CASE("time_point1", "[time_point log_msg]")
{
    std::shared_ptr<spdlog::sinks::test_sink_st> test_sink(new spdlog::sinks::test_sink_st);
    spdlog::logger logger("test-time_point", test_sink);

    spdlog::source_loc source{};
    std::chrono::system_clock::time_point tp{std::chrono::system_clock::now()};
    test_sink->set_pattern("%T.%F"); // interested in the time_point

    // all the following should have the same time
    test_sink->set_delay(std::chrono::milliseconds(10));
    for (int i = 0; i < 5; i++)
    {
        spdlog::details::log_msg msg{tp, source, "test_logger", spdlog::level::info, "message"};
        test_sink->log(msg);
    }

    logger.log(tp, source, spdlog::level::info, "formatted message");
    logger.log(tp, source, spdlog::level::info, "formatted message");
    logger.log(tp, source, spdlog::level::info, "formatted message");
    logger.log(tp, source, spdlog::level::info, "formatted message");
    logger.log(source, spdlog::level::info, "formatted message"); // last line has different time_point

    // now the real test... that the times are the same.
    std::vector<std::string> lines = test_sink->lines();
    REQUIRE(lines[0] == lines[1]);
    REQUIRE(lines[2] == lines[3]);
    REQUIRE(lines[4] == lines[5]);
    REQUIRE(lines[6] == lines[7]);
    REQUIRE(lines[8] != lines[9]);
    spdlog::drop_all();
}
