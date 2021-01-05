
#include "includes.h"
#include "test_sink.h"

#include <spdlog/cfg/env.h>
#include <spdlog/cfg/argv.h>

using spdlog::cfg::load_argv_levels;
using spdlog::cfg::load_env_levels;
using spdlog::sinks::test_sink_st;

TEST_CASE("env", "[cfg]")
{
    spdlog::drop("l1");
    auto l1 = spdlog::create<test_sink_st>("l1");
#ifdef CATCH_PLATFORM_WINDOWS
    _putenv_s("SPDLOG_LEVEL", "l1=warn");
#else
    setenv("SPDLOG_LEVEL", "l1=warn", 1);
#endif
    load_env_levels();
    REQUIRE(l1->level() == spdlog::level::warn);
    spdlog::set_default_logger(spdlog::create<test_sink_st>("cfg-default"));
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
}

TEST_CASE("argv1", "[cfg]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=warn"};
    load_argv_levels(2, argv);
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
}

TEST_CASE("argv2", "[cfg]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=warn,trace"};
    load_argv_levels(2, argv);
    auto l1 = spdlog::create<test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::trace);
}

TEST_CASE("argv3", "[cfg]")
{
    spdlog::set_level(spdlog::level::trace);

    spdlog::drop("l1");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=junk_name=warn"};
    load_argv_levels(2, argv);
    auto l1 = spdlog::create<test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::trace);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::trace);
}

TEST_CASE("argv4", "[cfg]")
{
    spdlog::set_level(spdlog::level::info);
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=junk"};
    load_argv_levels(2, argv);
    auto l1 = spdlog::create<test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::info);
}

TEST_CASE("argv5", "[cfg]")
{
    spdlog::set_level(spdlog::level::info);
    spdlog::drop("l1");
    const char *argv[] = {"ignore", "ignore", "SPDLOG_LEVEL=l1=warn,trace"};
    load_argv_levels(3, argv);
    auto l1 = spdlog::create<test_sink_st>("l1");
    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::trace);
    spdlog::set_level(spdlog::level::info);
}

TEST_CASE("argv6", "[cfg]")
{
    spdlog::set_level(spdlog::level::err);
    const char *argv[] = {""};
    load_argv_levels(1, argv);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::err);
    spdlog::set_level(spdlog::level::info);
}

TEST_CASE("argv7", "[cfg]")
{
    spdlog::set_level(spdlog::level::err);
    const char *argv[] = {""};
    load_argv_levels(0, argv);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::err);
    spdlog::set_level(spdlog::level::info);
}

TEST_CASE("level-not-set-test1", "[cfg]")
{
    spdlog::drop("l1");
    const char *argv[] = {"ignore", ""};
    load_argv_levels(2, argv);
    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    l1->set_level(spdlog::level::trace);
    REQUIRE(l1->level() == spdlog::level::trace);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
}

TEST_CASE("level-not-set-test2", "[cfg]")
{
    spdlog::drop("l1");
    spdlog::drop("l2");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=trace"};

    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    l1->set_level(spdlog::level::warn);
    auto l2 = spdlog::create<spdlog::sinks::test_sink_st>("l2");
    l2->set_level(spdlog::level::warn);

    load_argv_levels(2, argv);

    REQUIRE(l1->level() == spdlog::level::trace);
    REQUIRE(l2->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
}

TEST_CASE("level-not-set-test3", "[cfg]")
{
    spdlog::drop("l1");
    spdlog::drop("l2");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=trace"};

    load_argv_levels(2, argv);

    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    auto l2 = spdlog::create<spdlog::sinks::test_sink_st>("l2");

    REQUIRE(l1->level() == spdlog::level::trace);
    REQUIRE(l2->level() == spdlog::level::info);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
}

TEST_CASE("level-not-set-test4", "[cfg]")
{
    spdlog::drop("l1");
    spdlog::drop("l2");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=trace,warn"};

    load_argv_levels(2, argv);

    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    auto l2 = spdlog::create<spdlog::sinks::test_sink_st>("l2");

    REQUIRE(l1->level() == spdlog::level::trace);
    REQUIRE(l2->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::warn);
}

TEST_CASE("level-not-set-test5", "[cfg]")
{
    spdlog::drop("l1");
    spdlog::drop("l2");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=l1=junk,warn"};

    load_argv_levels(2, argv);

    auto l1 = spdlog::create<spdlog::sinks::test_sink_st>("l1");
    auto l2 = spdlog::create<spdlog::sinks::test_sink_st>("l2");

    REQUIRE(l1->level() == spdlog::level::warn);
    REQUIRE(l2->level() == spdlog::level::warn);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::warn);
}

TEST_CASE("restore-to-default", "[cfg]")
{
    spdlog::drop("l1");
    spdlog::drop("l2");
    const char *argv[] = {"ignore", "SPDLOG_LEVEL=info"};
    load_argv_levels(2, argv);
    REQUIRE(spdlog::default_logger()->level() == spdlog::level::info);
}
