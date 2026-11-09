// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file overloads.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef OVERLOADS_HPP
#define OVERLOADS_HPP

#include "argument_range.hpp"

#include <cstddef>
#include <vector>

// A method that the calls of its name may reach, with the numbers of arguments that it takes
//
struct overload {
    std::size_t     position{0};    // Position of the method among the methods of the run
    argumentRange   arguments;      // Numbers of arguments that the method takes
};

std::vector<std::size_t> overloadsTaking (const std::vector<overload>&, int);

#endif
