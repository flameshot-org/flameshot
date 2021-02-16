#include "includes.h"
#include "spdlog/sinks/systemd_sink.h"

TEST_CASE("systemd", "[all]")
{
    auto systemd_sink = std::make_shared<spdlog::sinks::systemd_sink_st>();
    spdlog::logger logger("spdlog_systemd_test", systemd_sink);
    logger.set_level(spdlog::level::trace);
    logger.trace("test spdlog trace");
    logger.debug("test spdlog debug");
    SPDLOG_LOGGER_INFO((&logger), "test spdlog info");
    SPDLOG_LOGGER_WARN((&logger), "test spdlog warn");
    SPDLOG_LOGGER_ERROR((&logger), "test spdlog error");
    SPDLOG_LOGGER_CRITICAL((&logger), "test spdlog critical");
}
