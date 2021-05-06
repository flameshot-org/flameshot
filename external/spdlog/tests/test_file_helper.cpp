/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"

using spdlog::details::file_helper;

static void write_with_helper(file_helper &helper, size_t howmany)
{
    spdlog::memory_buf_t formatted;
    fmt::format_to(formatted, "{}", std::string(howmany, '1'));
    helper.write(formatted);
    helper.flush();
}

TEST_CASE("file_helper_filename", "[file_helper::filename()]]")
{
    prepare_logdir();

    file_helper helper;
    std::string target_filename = "test_logs/file_helper_test.txt";
    helper.open(target_filename);
    REQUIRE(helper.filename() == target_filename);
}

TEST_CASE("file_helper_size", "[file_helper::size()]]")
{
    prepare_logdir();
    std::string target_filename = "test_logs/file_helper_test.txt";
    size_t expected_size = 123;
    {
        file_helper helper;
        helper.open(target_filename);
        write_with_helper(helper, expected_size);
        REQUIRE(static_cast<size_t>(helper.size()) == expected_size);
    }
    REQUIRE(get_filesize(target_filename) == expected_size);
}

TEST_CASE("file_helper_reopen", "[file_helper::reopen()]]")
{
    prepare_logdir();
    std::string target_filename = "test_logs/file_helper_test.txt";
    file_helper helper;
    helper.open(target_filename);
    write_with_helper(helper, 12);
    REQUIRE(helper.size() == 12);
    helper.reopen(true);
    REQUIRE(helper.size() == 0);
}

TEST_CASE("file_helper_reopen2", "[file_helper::reopen(false)]]")
{
    prepare_logdir();
    std::string target_filename = "test_logs/file_helper_test.txt";
    size_t expected_size = 14;
    file_helper helper;
    helper.open(target_filename);
    write_with_helper(helper, expected_size);
    REQUIRE(helper.size() == expected_size);
    helper.reopen(false);
    REQUIRE(helper.size() == expected_size);
}

static void test_split_ext(const char *fname, const char *expect_base, const char *expect_ext)
{
    spdlog::filename_t filename(fname);
    spdlog::filename_t expected_base(expect_base);
    spdlog::filename_t expected_ext(expect_ext);

#ifdef _WIN32 // replace folder sep
    std::replace(filename.begin(), filename.end(), '/', '\\');
    std::replace(expected_base.begin(), expected_base.end(), '/', '\\');
#endif
    spdlog::filename_t basename;
    spdlog::filename_t ext;
    std::tie(basename, ext) = file_helper::split_by_extension(filename);
    REQUIRE(basename == expected_base);
    REQUIRE(ext == expected_ext);
}

TEST_CASE("file_helper_split_by_extension", "[file_helper::split_by_extension()]]")
{
    test_split_ext("mylog.txt", "mylog", ".txt");
    test_split_ext(".mylog.txt", ".mylog", ".txt");
    test_split_ext(".mylog", ".mylog", "");
    test_split_ext("/aaa/bb.d/mylog", "/aaa/bb.d/mylog", "");
    test_split_ext("/aaa/bb.d/mylog.txt", "/aaa/bb.d/mylog", ".txt");
    test_split_ext("aaa/bbb/ccc/mylog.txt", "aaa/bbb/ccc/mylog", ".txt");
    test_split_ext("aaa/bbb/ccc/mylog.", "aaa/bbb/ccc/mylog.", "");
    test_split_ext("aaa/bbb/ccc/.mylog.txt", "aaa/bbb/ccc/.mylog", ".txt");
    test_split_ext("/aaa/bbb/ccc/mylog.txt", "/aaa/bbb/ccc/mylog", ".txt");
    test_split_ext("/aaa/bbb/ccc/.mylog", "/aaa/bbb/ccc/.mylog", "");
    test_split_ext("../mylog.txt", "../mylog", ".txt");
    test_split_ext(".././mylog.txt", ".././mylog", ".txt");
    test_split_ext(".././mylog.txt/xxx", ".././mylog.txt/xxx", "");
    test_split_ext("/mylog.txt", "/mylog", ".txt");
    test_split_ext("//mylog.txt", "//mylog", ".txt");
    test_split_ext("", "", "");
    test_split_ext(".", ".", "");
    test_split_ext("..txt", ".", ".txt");
}
