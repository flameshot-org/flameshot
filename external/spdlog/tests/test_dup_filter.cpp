#include "includes.h"
#include "spdlog/sinks/dup_filter_sink.h"
#include "test_sink.h"

TEST_CASE("dup_filter_test1", "[dup_filter_sink]")
{
    using spdlog::sinks::dup_filter_sink_st;
    using spdlog::sinks::test_sink_mt;

    dup_filter_sink_st dup_sink{std::chrono::seconds{5}};
    auto test_sink = std::make_shared<test_sink_mt>();
    dup_sink.add_sink(test_sink);

    for (int i = 0; i < 10; i++)
    {
        dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
    }

    REQUIRE(test_sink->msg_counter() == 1);
}

TEST_CASE("dup_filter_test2", "[dup_filter_sink]")
{
    using spdlog::sinks::dup_filter_sink_st;
    using spdlog::sinks::test_sink_mt;

    dup_filter_sink_st dup_sink{std::chrono::seconds{0}};
    auto test_sink = std::make_shared<test_sink_mt>();
    dup_sink.add_sink(test_sink);

    for (int i = 0; i < 10; i++)
    {
        dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    REQUIRE(test_sink->msg_counter() == 10);
}

TEST_CASE("dup_filter_test3", "[dup_filter_sink]")
{
    using spdlog::sinks::dup_filter_sink_st;
    using spdlog::sinks::test_sink_mt;

    dup_filter_sink_st dup_sink{std::chrono::seconds{1}};
    auto test_sink = std::make_shared<test_sink_mt>();
    dup_sink.add_sink(test_sink);

    for (int i = 0; i < 10; i++)
    {
        dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
        dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message2"});
    }

    REQUIRE(test_sink->msg_counter() == 20);
}

TEST_CASE("dup_filter_test4", "[dup_filter_sink]")
{
    using spdlog::sinks::dup_filter_sink_mt;
    using spdlog::sinks::test_sink_mt;

    dup_filter_sink_mt dup_sink{std::chrono::milliseconds{10}};
    auto test_sink = std::make_shared<test_sink_mt>();
    dup_sink.add_sink(test_sink);

    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message"});
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message"});
    REQUIRE(test_sink->msg_counter() == 2);
}

TEST_CASE("dup_filter_test5", "[dup_filter_sink]")
{
    using spdlog::sinks::dup_filter_sink_mt;
    using spdlog::sinks::test_sink_mt;

    dup_filter_sink_mt dup_sink{std::chrono::seconds{5}};
    auto test_sink = std::make_shared<test_sink_mt>();
    dup_sink.add_sink(test_sink);

    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
    dup_sink.log(spdlog::details::log_msg{"test", spdlog::level::info, "message2"});

    REQUIRE(test_sink->msg_counter() == 3); // skip 2 messages but log the "skipped.." message before message2
}
