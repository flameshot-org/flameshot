# Copyright(c) 2019 spdlog authors Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.10)
project(spdlog_bench CXX)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog CONFIG REQUIRED)
endif()

find_package(Threads REQUIRED)
find_package(benchmark CONFIG)
if (NOT benchmark_FOUND)
    message(STATUS "Using CMake Version ${CMAKE_VERSION}")
    if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.11.0")
        # User can fetch googlebenchmark 			
        message(STATUS "Downloading GoogleBenchmark")
        include(FetchContent)
        set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE INTERNAL "")
        # Do not build and run googlebenchmark tests
        FetchContent_Declare(googlebenchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.5.2)

        FetchContent_MakeAvailable(googlebenchmark)
    else()
        message(FATAL_ERROR "GoogleBenchmark is missing. Use CMake >= 3.11 or download it")
    endif()
endif()

add_executable(bench bench.cpp)
spdlog_enable_warnings(bench)
target_link_libraries(bench PRIVATE spdlog::spdlog)

add_executable(async_bench async_bench.cpp)
target_link_libraries(async_bench PRIVATE spdlog::spdlog)

add_executable(latency latency.cpp)
target_link_libraries(latency PRIVATE benchmark::benchmark spdlog::spdlog)

add_executable(formatter-bench formatter-bench.cpp)
target_link_libraries(formatter-bench PRIVATE benchmark::benchmark spdlog::spdlog)
