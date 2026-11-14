// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file hole_names.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef HOLE_NAMES_HPP
#define HOLE_NAMES_HPP

#include <cstddef>
#include <string>
#include <unordered_set>
#include <vector>

// The names that the code in the holes of an interpolated string uses, where srcML marks the whole string as one literal, as it marks
// an f-string of Python and a template literal of JavaScript

std::size_t     quotedEnd       (const std::string&, std::size_t);
void            addHoleNames    (const std::string&, const std::unordered_set<std::string>&, std::vector<std::string>&);

#endif
