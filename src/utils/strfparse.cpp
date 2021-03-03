// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Jeremy Borgman

#include "strfparse.h"

namespace strfparse {
std::vector<std::string> split(std::string const& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::vector<char> create_specifier_list()
{

    std::vector<char> allowed_specifier{ 'Y', 'H', 'a', 'A', 'b', 'B', 'c', 'C',
                                         'd', 'D', 'e', 'F', 'g', 'G', 'h', 'H',
                                         'I', 'j', 'm', 'M', 'n', 'p', 'r', 'R',
                                         'S', 't', 'T', 'u', 'U', 'V', 'w', 'W',
                                         'x', 'X', 'y', 'Y', 'z', 'Z' };
    return allowed_specifier;
}

std::string replace_all(std::string input,
                        std::string const& to_find,
                        std::string const& to_replace)
{
    size_t pos = 0;
    while ((pos = input.find(to_find, pos)) != std::string::npos) {
        input.replace(pos, to_find.length(), to_replace);
        pos += to_replace.length();
    }

    return input;
}

std::vector<char> match_specifiers(std::string const& specifier,
                                   std::vector<char> allowed_specifier)
{

    std::vector<char> spec_list;

    for (size_t i = 0; i < specifier.size() - 1; i++) {
        if (specifier[i] == '%') {
            spec_list.push_back(specifier[i + 1]);
        }
    }

    std::sort(spec_list.begin(), spec_list.end());
    std::sort(allowed_specifier.begin(), allowed_specifier.end());

    std::vector<char> overlap;
    std::set_intersection(spec_list.begin(),
                          spec_list.end(),
                          allowed_specifier.begin(),
                          allowed_specifier.end(),
                          back_inserter(overlap));

    return overlap;
}

std::string format_time_string(std::string const& specifier)
{

    if (specifier.empty()) {
        return "";
    }

    std::time_t t = std::time(nullptr);
    char buff[100];

    auto allowed_specifier = create_specifier_list();

    auto overlap = match_specifiers(specifier, allowed_specifier);

    // Create "Safe" string for strftime which is the specfiers delimited by *
    std::string lookup_string;
    for (auto const& e : overlap) {
        lookup_string.push_back('%');
        lookup_string.push_back(e);
        lookup_string.push_back('*');
    }

    std::strftime(
      buff, sizeof(buff), lookup_string.c_str(), std::localtime(&t));

    std::map<char, std::string> lookup_table;
    auto result = split(buff, '*');

    for (size_t i = 0; i < result.size(); i++) {
        lookup_table.emplace(std::make_pair(overlap[i], result[i]));
    }

    // Sub into original string
    std::string delim = "%";
    auto output_string = specifier;
    for (auto const& row : lookup_table) {
        auto to_find = delim + row.first;
        output_string = replace_all(output_string, to_find, row.second);
    }
    return output_string;
}
}
