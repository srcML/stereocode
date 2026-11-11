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

// The numbers of arguments that a parameter list takes
//
struct argumentRange {
    int         minimum{0};             // Parameters without a default value, the variadic one excluded
    int         maximum{0};             // All parameters
    bool        variadic{false};        // A parameter takes any number of arguments, as ... of C and C++, params of C#, and int... of Java do
};

#endif
