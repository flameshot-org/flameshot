/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

using spdlog::details::os::create_dir;
using spdlog::details::os::path_exists;

bool try_create_dir(const char *path, const char *normalized_path)
{
    auto rv = create_dir(path);
    REQUIRE(rv == true);
    return path_exists(normalized_path);
}

TEST_CASE("create_dir", "[create_dir]")
{
    prepare_logdir();

    REQUIRE(try_create_dir("test_logs/dir1/dir1", "test_logs/dir1/dir1"));
    REQUIRE(try_create_dir("test_logs/dir1/dir1", "test_logs/dir1/dir1")); // test existing
    REQUIRE(try_create_dir("test_logs/dir1///dir2//", "test_logs/dir1/dir2"));
    REQUIRE(try_create_dir("./test_logs/dir1/dir3", "test_logs/dir1/dir3"));
    REQUIRE(try_create_dir("test_logs/../test_logs/dir1/dir4", "test_logs/dir1/dir4"));

#ifdef WIN32
    // test backslash folder separator
    REQUIRE(try_create_dir("test_logs\\dir1\\dir222", "test_logs\\dir1\\dir222"));
    REQUIRE(try_create_dir("test_logs\\dir1\\dir223\\", "test_logs\\dir1\\dir223\\"));
    REQUIRE(try_create_dir(".\\test_logs\\dir1\\dir2\\dir99\\..\\dir23", "test_logs\\dir1\\dir2\\dir23"));
    REQUIRE(try_create_dir("test_logs\\..\\test_logs\\dir1\\dir5", "test_logs\\dir1\\dir5"));
#endif
}

TEST_CASE("create_invalid_dir", "[create_dir]")
{
    REQUIRE(create_dir("") == false);
    REQUIRE(create_dir(spdlog::filename_t{}) == false);
#ifdef __linux__
    REQUIRE(create_dir("/proc/spdlog-utest") == false);
#endif
}

TEST_CASE("dir_name", "[create_dir]")
{
    using spdlog::details::os::dir_name;
    REQUIRE(dir_name("").empty());
    REQUIRE(dir_name("dir").empty());

#ifdef WIN32
    REQUIRE(dir_name(R"(dir\)") == "dir");
    REQUIRE(dir_name(R"(dir\\\)") == R"(dir\\)");
    REQUIRE(dir_name(R"(dir\file)") == "dir");
    REQUIRE(dir_name(R"(dir/file)") == "dir");
    REQUIRE(dir_name(R"(dir\file.txt)") == "dir");
    REQUIRE(dir_name(R"(dir/file)") == "dir");
    REQUIRE(dir_name(R"(dir\file.txt\)") == R"(dir\file.txt)");
    REQUIRE(dir_name(R"(dir/file.txt/)") == R"(dir\file.txt)");
    REQUIRE(dir_name(R"(\dir\file.txt)") == R"(\dir)");
    REQUIRE(dir_name(R"(/dir/file.txt)") == R"(\dir)");
    REQUIRE(dir_name(R"(\\dir\file.txt)") == R"(\\dir)");
    REQUIRE(dir_name(R"(//dir/file.txt)") == R"(\\dir)");
    REQUIRE(dir_name(R"(..\file.txt)") == "..");
    REQUIRE(dir_name(R"(../file.txt)") == "..");
    REQUIRE(dir_name(R"(.\file.txt)") == ".");
    REQUIRE(dir_name(R"(./file.txt)") == ".");
    REQUIRE(dir_name(R"(c:\\a\b\c\d\file.txt)") == R"(c:\\a\b\c\d)");
    REQUIRE(dir_name(R"(c://a/b/c/d/file.txt)") == R"(c:\\a\b\c\d)");
#else
    REQUIRE(dir_name("dir/") == "dir");
    REQUIRE(dir_name("dir///") == "dir//");
    REQUIRE(dir_name("dir/file") == "dir");
    REQUIRE(dir_name("dir/file.txt") == "dir");
    REQUIRE(dir_name("dir/file.txt/") == "dir/file.txt");
    REQUIRE(dir_name("/dir/file.txt") == "/dir");
    REQUIRE(dir_name("//dir/file.txt") == "//dir");
    REQUIRE(dir_name("../file.txt") == "..");
    REQUIRE(dir_name("./file.txt") == ".");
#endif
}
