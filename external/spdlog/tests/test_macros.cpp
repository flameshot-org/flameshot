/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */

#include "includes.h"

#if SPDLOG_ACTIVE_LEVEL != SPDLOG_LEVEL_DEBUG
#error "Invalid SPDLOG_ACTIVE_LEVEL in test. Should be SPDLOG_LEVEL_DEBUG"
#endif

TEST_CASE("debug and trace w/o format string", "[macros]]")
{

    prepare_logdir();
    std::string filename = "test_logs/simple_log";

    auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("logger", filename);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);

    SPDLOG_LOGGER_TRACE(logger, "Test message 1");
    SPDLOG_LOGGER_DEBUG(logger, "Test message 2");
    logger->flush();

    using spdlog::details::os::default_eol;
    REQUIRE(ends_with(file_contents(filename), fmt::format("Test message 2{}", default_eol)));
    REQUIRE(count_lines(filename) == 1);

    spdlog::set_default_logger(logger);

    SPDLOG_TRACE("Test message 3");
    SPDLOG_DEBUG("Test message {}", 4);
    logger->flush();

    require_message_count(filename, 2);
    REQUIRE(ends_with(file_contents(filename), fmt::format("Test message 4{}", default_eol)));
}

TEST_CASE("disable param evaluation", "[macros]")
{
    SPDLOG_TRACE("Test message {}", throw std::runtime_error("Should not be evaluated"));
}

TEST_CASE("pass logger pointer", "[macros]")
{
    auto logger = spdlog::create<spdlog::sinks::null_sink_mt>("refmacro");
    auto &ref = *logger;
    SPDLOG_LOGGER_TRACE(&ref, "Test message 1");
    SPDLOG_LOGGER_DEBUG(&ref, "Test message 2");
}

// ensure that even if right macro level is on- don't evaluate if the logger's level is not high enough
// TEST_CASE("disable param evaluation2", "[macros]")
//{
//    auto logger = std::make_shared<spdlog::logger>("test-macro");
//    logger->set_level(spdlog::level::off);
//    int x = 0;
//    SPDLOG_LOGGER_DEBUG(logger, "Test message {}", ++x);
//    REQUIRE(x == 0);
//}
