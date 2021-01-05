/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/stdout_color_sinks.h"
TEST_CASE("stdout_st", "[stdout]")
{
    auto l = spdlog::stdout_logger_st("test");
    l->set_pattern("%+");
    l->set_level(spdlog::level::trace);
    l->trace("Test stdout_st");
    spdlog::drop_all();
}

TEST_CASE("stdout_mt", "[stdout]")
{
    auto l = spdlog::stdout_logger_mt("test");
    l->set_pattern("%+");
    l->set_level(spdlog::level::debug);
    l->debug("Test stdout_mt");
    spdlog::drop_all();
}

TEST_CASE("stderr_st", "[stderr]")
{
    auto l = spdlog::stderr_logger_st("test");
    l->set_pattern("%+");
    l->info("Test stderr_st");
    spdlog::drop_all();
}

TEST_CASE("stderr_mt", "[stderr]")
{
    auto l = spdlog::stderr_logger_mt("test");
    l->set_pattern("%+");
    l->info("Test stderr_mt");
    l->warn("Test stderr_mt");
    l->error("Test stderr_mt");
    l->critical("Test stderr_mt");
    spdlog::drop_all();
}

// color loggers
TEST_CASE("stdout_color_st", "[stdout]")
{
    auto l = spdlog::stdout_color_st("test");
    l->set_pattern("%+");
    l->info("Test stdout_color_st");
    spdlog::drop_all();
}

TEST_CASE("stdout_color_mt", "[stdout]")
{
    auto l = spdlog::stdout_color_mt("test");
    l->set_pattern("%+");
    l->set_level(spdlog::level::trace);
    l->trace("Test stdout_color_mt");
    spdlog::drop_all();
}

TEST_CASE("stderr_color_st", "[stderr]")
{
    auto l = spdlog::stderr_color_st("test");
    l->set_pattern("%+");
    l->set_level(spdlog::level::debug);
    l->debug("Test stderr_color_st");
    spdlog::drop_all();
}

TEST_CASE("stderr_color_mt", "[stderr]")
{
    auto l = spdlog::stderr_color_mt("test");
    l->set_pattern("%+");
    l->info("Test stderr_color_mt");
    l->warn("Test stderr_color_mt");
    l->error("Test stderr_color_mt");
    l->critical("Test stderr_color_mt");
    spdlog::drop_all();
}

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT

TEST_CASE("wchar_api", "[stdout]")
{
    auto l = spdlog::stdout_logger_st("wchar_logger");
    l->set_pattern("%+");
    l->set_level(spdlog::level::trace);
    l->trace(L"Test wchar_api");
    l->trace(L"Test wchar_api {}", L"param");
    l->trace(L"Test wchar_api {}", 1);
    l->trace(L"Test wchar_api {}", std::wstring{L"wstring param"});
    l->trace(std::wstring{L"Test wchar_api wstring"});
    SPDLOG_LOGGER_DEBUG(l, L"Test SPDLOG_LOGGER_DEBUG {}", L"param");
    spdlog::drop_all();
}

#endif