#if _WIN32

#include "includes.h"
#include "test_sink.h"

#include "spdlog/sinks/win_eventlog_sink.h"

static const LPCSTR TEST_SOURCE = "spdlog_test";

static void test_single_print(std::function<void(std::string const &)> do_log, std::string const &expected_contents, WORD expected_ev_type)
{
    using namespace std::chrono;
    do_log(expected_contents);
    const auto expected_time_generated = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();

    struct handle_t
    {
        HANDLE handle_;

        ~handle_t()
        {
            if (handle_)
            {
                REQUIRE(CloseEventLog(handle_));
            }
        }
    } event_log{::OpenEventLog(nullptr, TEST_SOURCE)};

    REQUIRE(event_log.handle_);

    DWORD read_bytes{}, size_needed{};
    auto ok =
        ::ReadEventLog(event_log.handle_, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ, 0, &read_bytes, 0, &read_bytes, &size_needed);
    REQUIRE(!ok);
    REQUIRE(::GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    std::vector<char> record_buffer(size_needed);
    PEVENTLOGRECORD record = (PEVENTLOGRECORD)record_buffer.data();

    ok = ::ReadEventLog(
        event_log.handle_, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ, 0, record, size_needed, &read_bytes, &size_needed);
    REQUIRE(ok);

    REQUIRE(record->NumStrings == 1);
    REQUIRE(record->EventType == expected_ev_type);
    REQUIRE((expected_time_generated - record->TimeGenerated) <= 3u);

    std::string message_in_log(((char *)record + record->StringOffset));
    REQUIRE(message_in_log == expected_contents + spdlog::details::os::default_eol);
}

TEST_CASE("eventlog", "[eventlog]")
{
    using namespace spdlog;

    auto test_sink = std::make_shared<sinks::win_eventlog_sink_mt>(TEST_SOURCE);

    spdlog::logger test_logger("eventlog", test_sink);
    test_logger.set_level(level::trace);

    test_sink->set_pattern("%v");

    test_single_print([&test_logger](std::string const &msg) { test_logger.trace(msg); }, "my trace message", EVENTLOG_SUCCESS);
    test_single_print([&test_logger](std::string const &msg) { test_logger.debug(msg); }, "my debug message", EVENTLOG_SUCCESS);
    test_single_print([&test_logger](std::string const &msg) { test_logger.info(msg); }, "my info message", EVENTLOG_INFORMATION_TYPE);
    test_single_print([&test_logger](std::string const &msg) { test_logger.warn(msg); }, "my warn message", EVENTLOG_WARNING_TYPE);
    test_single_print([&test_logger](std::string const &msg) { test_logger.error(msg); }, "my error message", EVENTLOG_ERROR_TYPE);
    test_single_print([&test_logger](std::string const &msg) { test_logger.critical(msg); }, "my critical message", EVENTLOG_ERROR_TYPE);
}

#endif //_WIN32
