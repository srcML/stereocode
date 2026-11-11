// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file interpolated_strings.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef INTERPOLATED_STRINGS_HPP
#define INTERPOLATED_STRINGS_HPP

#include <libxml/tree.h>

#include <string>
#include <vector>

// The names that the holes of a C# interpolated string use, such as count and this.items of $"{count,5} of {this.items.Count:D}",
// read from the text of the string, which srcML marks as string literals

// The names of the holes of the interpolated string that a srcML element starts, such as the name $ before the literal of $"{count}"
//
struct interpolatedString {
    std::vector<std::string>    names;                  // Each name with the names and access operators of its chain, such as this.items
    const xmlNode*              closingNode{nullptr};   // Text node or element without children that holds the closing quote, or null when the text ends first
};

interpolatedString readInterpolatedString (const xmlNode*);

#endif
