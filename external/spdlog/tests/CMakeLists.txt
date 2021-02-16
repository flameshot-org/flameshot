cmake_minimum_required(VERSION 3.10)
project(spdlog_utests CXX)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

include(../cmake/utils.cmake)

find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(systemd libsystemd)
endif()

set(SPDLOG_UTESTS_SOURCES
    test_file_helper.cpp
    test_file_logging.cpp
    test_daily_logger.cpp
    test_misc.cpp
    test_eventlog.cpp
    test_pattern_formatter.cpp
    test_async.cpp
    test_registry.cpp
    test_macros.cpp
    utils.cpp
    main.cpp
    test_mpmc_q.cpp
    test_dup_filter.cpp
    test_fmt_helper.cpp
    test_stdout_api.cpp
    test_backtrace.cpp
    test_create_dir.cpp
    test_cfg.cpp
    test_time_point.cpp
    test_stopwatch.cpp)

if(NOT SPDLOG_NO_EXCEPTIONS)
    list(APPEND SPDLOG_UTESTS_SOURCES test_errors.cpp)
endif()

if(systemd_FOUND)
    list(APPEND SPDLOG_UTESTS_SOURCES test_systemd.cpp)
endif()

enable_testing()

function(spdlog_prepare_test test_target spdlog_lib)
    add_executable(${test_target} ${SPDLOG_UTESTS_SOURCES})
    spdlog_enable_warnings(${test_target})
    target_link_libraries(${test_target} PRIVATE ${spdlog_lib})
    if(systemd_FOUND)
        target_link_libraries(${test_target} PRIVATE ${systemd_LIBRARIES})
    endif()
    if(SPDLOG_SANITIZE_ADDRESS)
        spdlog_enable_sanitizer(${test_target})
    endif()
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES RUN_SERIAL ON)
endfunction()

# The compiled library tests
if(SPDLOG_BUILD_TESTS OR SPDLOG_BUILD_ALL)
    spdlog_prepare_test(spdlog-utests spdlog::spdlog)
endif()

# The header-only library version tests
if(SPDLOG_BUILD_TESTS_HO OR SPDLOG_BUILD_ALL)
    spdlog_prepare_test(spdlog-utests-ho spdlog::spdlog_header_only)
endif()
