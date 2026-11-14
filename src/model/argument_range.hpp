// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file argument_range.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef ARGUMENT_RANGE_HPP
#define ARGUMENT_RANGE_HPP

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

// The numbers of arguments that a parameter list takes
//
struct argumentRange {
    int         minimum{0};             // Parameters without a default value, the variadic one excluded
    int         maximum{0};             // All parameters
    bool        variadic{false};        // A parameter takes any number of arguments, as ... of C and C++, params of C#, and int... of Java do
};

// The numbers of arguments that the methods of each name take
//
using argumentsByName = std::unordered_map<std::string, std::vector<argumentRange>>;

// Tells whether a parameter list takes a number of arguments
//
inline bool takes(const argumentRange& range, int arguments) {
    return range.minimum <= arguments && (range.variadic || arguments <= range.maximum);
}

// Tells whether a method of a name takes a number of arguments
//
inline bool takes(const argumentsByName& methods, const std::string& name, int arguments) {
    const auto named = methods.find(name);
    return named != methods.end() &&
           std::any_of(named->second.begin(), named->second.end(), [arguments](const argumentRange& range) { return takes(range, arguments); });
}

#endif
