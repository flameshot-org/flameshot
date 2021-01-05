#include "includes.h"
#include "test_sink.h"
#include "spdlog/fmt/bin_to_hex.h"

template<class T>
std::string log_info(const T &what, spdlog::level::level_enum logger_level = spdlog::level::info)
{

    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::logger oss_logger("oss", oss_sink);
    oss_logger.set_level(logger_level);
    oss_logger.set_pattern("%v");
    oss_logger.info(what);

    return oss.str().substr(0, oss.str().length() - strlen(spdlog::details::os::default_eol));
}

TEST_CASE("basic_logging ", "[basic_logging]")
{
    // const char
    REQUIRE(log_info("Hello") == "Hello");
    REQUIRE(log_info("").empty());

    // std::string
    REQUIRE(log_info(std::string("Hello")) == "Hello");
    REQUIRE(log_info(std::string()).empty());

    // Numbers
    REQUIRE(log_info(5) == "5");
    REQUIRE(log_info(5.6) == "5.6");

    // User defined class
    // REQUIRE(log_info(some_logged_class("some_val")) == "some_val");
}

TEST_CASE("log_levels", "[log_levels]")
{
    REQUIRE(log_info("Hello", spdlog::level::err).empty());
    REQUIRE(log_info("Hello", spdlog::level::critical).empty());
    REQUIRE(log_info("Hello", spdlog::level::info) == "Hello");
    REQUIRE(log_info("Hello", spdlog::level::debug) == "Hello");
    REQUIRE(log_info("Hello", spdlog::level::trace) == "Hello");
}

TEST_CASE("level_to_string_view", "[convert_to_string_view")
{
    REQUIRE(spdlog::level::to_string_view(spdlog::level::trace) == "trace");
    REQUIRE(spdlog::level::to_string_view(spdlog::level::debug) == "debug");
    REQUIRE(spdlog::level::to_string_view(spdlog::level::info) == "info");
    REQUIRE(spdlog::level::to_string_view(spdlog::level::warn) == "warning");
    REQUIRE(spdlog::level::to_string_view(spdlog::level::err) == "error");
    REQUIRE(spdlog::level::to_string_view(spdlog::level::critical) == "critical");
    REQUIRE(spdlog::level::to_string_view(spdlog::level::off) == "off");
}

TEST_CASE("to_short_c_str", "[convert_to_short_c_str]")
{
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::trace)) == "T");
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::debug)) == "D");
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::info)) == "I");
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::warn)) == "W");
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::err)) == "E");
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::critical)) == "C");
    REQUIRE(std::string(spdlog::level::to_short_c_str(spdlog::level::off)) == "O");
}

TEST_CASE("to_level_enum", "[convert_to_level_enum]")
{
    REQUIRE(spdlog::level::from_str("trace") == spdlog::level::trace);
    REQUIRE(spdlog::level::from_str("debug") == spdlog::level::debug);
    REQUIRE(spdlog::level::from_str("info") == spdlog::level::info);
    REQUIRE(spdlog::level::from_str("warning") == spdlog::level::warn);
    REQUIRE(spdlog::level::from_str("warn") == spdlog::level::warn);
    REQUIRE(spdlog::level::from_str("error") == spdlog::level::err);
    REQUIRE(spdlog::level::from_str("critical") == spdlog::level::critical);
    REQUIRE(spdlog::level::from_str("off") == spdlog::level::off);
    REQUIRE(spdlog::level::from_str("null") == spdlog::level::off);
}

TEST_CASE("periodic flush", "[periodic_flush]")
{
    using spdlog::sinks::test_sink_mt;
    auto logger = spdlog::create<test_sink_mt>("periodic_flush");
    auto test_sink = std::static_pointer_cast<test_sink_mt>(logger->sinks()[0]);

    spdlog::flush_every(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(1250));
    REQUIRE(test_sink->flush_counter() == 1);
    spdlog::flush_every(std::chrono::seconds(0));
    spdlog::drop_all();
}

TEST_CASE("clone-logger", "[clone]")
{
    using spdlog::sinks::test_sink_mt;
    auto test_sink = std::make_shared<test_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("orig", test_sink);
    logger->set_pattern("%v");
    auto cloned = logger->clone("clone");

    REQUIRE(cloned->name() == "clone");
    REQUIRE(logger->sinks() == cloned->sinks());
    REQUIRE(logger->level() == cloned->level());
    REQUIRE(logger->flush_level() == cloned->flush_level());
    logger->info("Some message 1");
    cloned->info("Some message 2");

    REQUIRE(test_sink->lines().size() == 2);
    REQUIRE(test_sink->lines()[0] == "Some message 1");
    REQUIRE(test_sink->lines()[1] == "Some message 2");

    spdlog::drop_all();
}

TEST_CASE("clone async", "[clone]")
{
    using spdlog::sinks::test_sink_st;
    spdlog::init_thread_pool(4, 1);
    auto test_sink = std::make_shared<test_sink_st>();
    auto logger = std::make_shared<spdlog::async_logger>("orig", test_sink, spdlog::thread_pool());
    logger->set_pattern("%v");
    auto cloned = logger->clone("clone");

    REQUIRE(cloned->name() == "clone");
    REQUIRE(logger->sinks() == cloned->sinks());
    REQUIRE(logger->level() == cloned->level());
    REQUIRE(logger->flush_level() == cloned->flush_level());

    logger->info("Some message 1");
    cloned->info("Some message 2");

    spdlog::details::os::sleep_for_millis(10);

    REQUIRE(test_sink->lines().size() == 2);
    REQUIRE(test_sink->lines()[0] == "Some message 1");
    REQUIRE(test_sink->lines()[1] == "Some message 2");

    spdlog::drop_all();
}

TEST_CASE("to_hex", "[to_hex]")
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("oss", oss_sink);

    std::vector<unsigned char> v{9, 0xa, 0xb, 0xc, 0xff, 0xff};
    oss_logger.info("{}", spdlog::to_hex(v));

    auto output = oss.str();
    REQUIRE(ends_with(output, "0000: 09 0a 0b 0c ff ff" + std::string(spdlog::details::os::default_eol)));
}

TEST_CASE("to_hex_upper", "[to_hex]")
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("oss", oss_sink);

    std::vector<unsigned char> v{9, 0xa, 0xb, 0xc, 0xff, 0xff};
    oss_logger.info("{:X}", spdlog::to_hex(v));

    auto output = oss.str();
    REQUIRE(ends_with(output, "0000: 09 0A 0B 0C FF FF" + std::string(spdlog::details::os::default_eol)));
}

TEST_CASE("to_hex_no_delimiter", "[to_hex]")
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("oss", oss_sink);

    std::vector<unsigned char> v{9, 0xa, 0xb, 0xc, 0xff, 0xff};
    oss_logger.info("{:sX}", spdlog::to_hex(v));

    auto output = oss.str();
    REQUIRE(ends_with(output, "0000: 090A0B0CFFFF" + std::string(spdlog::details::os::default_eol)));
}

TEST_CASE("to_hex_show_ascii", "[to_hex]")
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("oss", oss_sink);

    std::vector<unsigned char> v{9, 0xa, 0xb, 0x41, 0xc, 0x4b, 0xff, 0xff};
    oss_logger.info("{:Xsa}", spdlog::to_hex(v, 8));

    REQUIRE(ends_with(oss.str(), "0000: 090A0B410C4BFFFF  ...A.K.." + std::string(spdlog::details::os::default_eol)));
}

TEST_CASE("to_hex_different_size_per_line", "[to_hex]")
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("oss", oss_sink);

    std::vector<unsigned char> v{9, 0xa, 0xb, 0x41, 0xc, 0x4b, 0xff, 0xff};

    oss_logger.info("{:Xsa}", spdlog::to_hex(v, 10));
    REQUIRE(ends_with(oss.str(), "0000: 090A0B410C4BFFFF  ...A.K.." + std::string(spdlog::details::os::default_eol)));

    oss_logger.info("{:Xs}", spdlog::to_hex(v, 10));
    REQUIRE(ends_with(oss.str(), "0000: 090A0B410C4BFFFF" + std::string(spdlog::details::os::default_eol)));

    oss_logger.info("{:Xsa}", spdlog::to_hex(v, 6));
    REQUIRE(ends_with(oss.str(), "0000: 090A0B410C4B  ...A.K" + std::string(spdlog::details::os::default_eol) + "0006: FFFF          .." +
                                     std::string(spdlog::details::os::default_eol)));

    oss_logger.info("{:Xs}", spdlog::to_hex(v, 6));
    REQUIRE(ends_with(oss.str(), "0000: 090A0B410C4B" + std::string(spdlog::details::os::default_eol) + "0006: FFFF" +
                                     std::string(spdlog::details::os::default_eol)));
}

TEST_CASE("to_hex_no_ascii", "[to_hex]")
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    spdlog::logger oss_logger("oss", oss_sink);

    std::vector<unsigned char> v{9, 0xa, 0xb, 0x41, 0xc, 0x4b, 0xff, 0xff};
    oss_logger.info("{:Xs}", spdlog::to_hex(v, 8));

    REQUIRE(ends_with(oss.str(), "0000: 090A0B410C4BFFFF" + std::string(spdlog::details::os::default_eol)));

    oss_logger.info("{:Xsna}", spdlog::to_hex(v, 8));

    REQUIRE(ends_with(oss.str(), "090A0B410C4BFFFF" + std::string(spdlog::details::os::default_eol)));
}

TEST_CASE("default logger API", "[default logger]")
{
    std::ostringstream oss;
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    spdlog::set_default_logger(std::make_shared<spdlog::logger>("oss", oss_sink));
    spdlog::set_pattern("*** %v");

    spdlog::default_logger()->set_level(spdlog::level::trace);
    spdlog::trace("hello trace");
    REQUIRE(oss.str() == "*** hello trace" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::debug("hello debug");
    REQUIRE(oss.str() == "*** hello debug" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::info("Hello");
    REQUIRE(oss.str() == "*** Hello" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::warn("Hello again {}", 2);
    REQUIRE(oss.str() == "*** Hello again 2" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::error(123);
    REQUIRE(oss.str() == "*** 123" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::critical(std::string("some string"));
    REQUIRE(oss.str() == "*** some string" + std::string(spdlog::details::os::default_eol));

    oss.str("");
    spdlog::set_level(spdlog::level::info);
    spdlog::debug("should not be logged");
    REQUIRE(oss.str().empty());
    spdlog::drop_all();
    spdlog::set_pattern("%v");
}
