//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <cctype>

//
// Support for logging binary data as hex
// format flags, any combination of the followng:
// {:X} - print in uppercase.
// {:s} - don't separate each byte with space.
// {:p} - don't print the position on each line start.
// {:n} - don't split the output to lines.
// {:a} - show ASCII if :n is not set

//
// Examples:
//
// std::vector<char> v(200, 0x0b);
// logger->info("Some buffer {}", spdlog::to_hex(v));
// char buf[128];
// logger->info("Some buffer {:X}", spdlog::to_hex(std::begin(buf), std::end(buf)));
// logger->info("Some buffer {:X}", spdlog::to_hex(std::begin(buf), std::end(buf), 16));

namespace spdlog {
namespace details {

template<typename It>
class dump_info
{
public:
    dump_info(It range_begin, It range_end, size_t size_per_line)
        : begin_(range_begin)
        , end_(range_end)
        , size_per_line_(size_per_line)
    {}

    It begin() const
    {
        return begin_;
    }
    It end() const
    {
        return end_;
    }
    size_t size_per_line() const
    {
        return size_per_line_;
    }

private:
    It begin_, end_;
    size_t size_per_line_;
};
} // namespace details

// create a dump_info that wraps the given container
template<typename Container>
inline details::dump_info<typename Container::const_iterator> to_hex(const Container &container, size_t size_per_line = 32)
{
    static_assert(sizeof(typename Container::value_type) == 1, "sizeof(Container::value_type) != 1");
    using Iter = typename Container::const_iterator;
    return details::dump_info<Iter>(std::begin(container), std::end(container), size_per_line);
}

// create dump_info from ranges
template<typename It>
inline details::dump_info<It> to_hex(const It range_begin, const It range_end, size_t size_per_line = 32)
{
    return details::dump_info<It>(range_begin, range_end, size_per_line);
}

} // namespace spdlog

namespace fmt {

template<typename T>
struct formatter<spdlog::details::dump_info<T>>
{
    const char delimiter = ' ';
    bool put_newlines = true;
    bool put_delimiters = true;
    bool use_uppercase = false;
    bool put_positions = true; // position on start of each line
    bool show_ascii = false;

    // parse the format string flags
    template<typename ParseContext>
    auto parse(ParseContext &ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin();
        while (it != ctx.end() && *it != '}')
        {
            switch (*it)
            {
            case 'X':
                use_uppercase = true;
                break;
            case 's':
                put_delimiters = false;
                break;
            case 'p':
                put_positions = false;
                break;
            case 'n':
                put_newlines = false;
                show_ascii = false;
                break;
            case 'a':
                if (put_newlines)
                {
                    show_ascii = true;
                }
                break;
            }

            ++it;
        }
        return it;
    }

    // format the given bytes range as hex
    template<typename FormatContext, typename Container>
    auto format(const spdlog::details::dump_info<Container> &the_range, FormatContext &ctx) -> decltype(ctx.out())
    {
        SPDLOG_CONSTEXPR const char *hex_upper = "0123456789ABCDEF";
        SPDLOG_CONSTEXPR const char *hex_lower = "0123456789abcdef";
        const char *hex_chars = use_uppercase ? hex_upper : hex_lower;

#if FMT_VERSION < 60000
        auto inserter = ctx.begin();
#else
        auto inserter = ctx.out();
#endif

        int size_per_line = static_cast<int>(the_range.size_per_line());
        auto start_of_line = the_range.begin();
        for (auto i = the_range.begin(); i != the_range.end(); i++)
        {
            auto ch = static_cast<unsigned char>(*i);

            if (put_newlines && (i == the_range.begin() || i - start_of_line >= size_per_line))
            {
                if (show_ascii && i != the_range.begin())
                {
                    *inserter++ = delimiter;
                    *inserter++ = delimiter;
                    for (auto j = start_of_line; j < i; j++)
                    {
                        auto pc = static_cast<unsigned char>(*j);
                        *inserter++ = std::isprint(pc) ? static_cast<char>(*j) : '.';
                    }
                }

                put_newline(inserter, static_cast<size_t>(i - the_range.begin()));

                // put first byte without delimiter in front of it
                *inserter++ = hex_chars[(ch >> 4) & 0x0f];
                *inserter++ = hex_chars[ch & 0x0f];
                start_of_line = i;
                continue;
            }

            if (put_delimiters)
            {
                *inserter++ = delimiter;
            }

            *inserter++ = hex_chars[(ch >> 4) & 0x0f];
            *inserter++ = hex_chars[ch & 0x0f];
        }
        if (show_ascii) // add ascii to last line
        {
            if (the_range.end() - the_range.begin() > size_per_line)
            {
                auto blank_num = size_per_line - (the_range.end() - start_of_line);
                while (blank_num-- > 0)
                {
                    *inserter++ = delimiter;
                    *inserter++ = delimiter;
                    if (put_delimiters)
                    {
                        *inserter++ = delimiter;
                    }
                }
            }
            *inserter++ = delimiter;
            *inserter++ = delimiter;
            for (auto j = start_of_line; j != the_range.end(); j++)
            {
                auto pc = static_cast<unsigned char>(*j);
                *inserter++ = std::isprint(pc) ? static_cast<char>(*j) : '.';
            }
        }
        return inserter;
    }

    // put newline(and position header)
    template<typename It>
    void put_newline(It inserter, std::size_t pos)
    {
#ifdef _WIN32
        *inserter++ = '\r';
#endif
        *inserter++ = '\n';

        if (put_positions)
        {
            fmt::format_to(inserter, "{:<04X}: ", pos);
        }
    }
};
} // namespace fmt
