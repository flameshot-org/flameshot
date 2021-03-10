// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Jeremy Borgman

#include <algorithm>
#include <ctime>
#include <map>
#include <sstream>
#include <vector>

namespace strfparse {
std::vector<std::string> split(std::string const& s, char delimiter);

std::vector<char> create_specifier_list();

std::string replace_all(std::string input,
                        std::string const& to_find,
                        std::string const& to_replace);

std::vector<char> match_specifiers(std::string const& specifier,
                                   std::vector<char> allowed_specifier);

std::string format_time_string(std::string const& specifier);
}
