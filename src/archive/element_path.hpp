// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file element_path.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef ELEMENT_PATH_HPP
#define ELEMENT_PATH_HPP

#include <vector>

// The place of an element in its unit, as the position of each element on the way down from the unit element,
// counted from 1 among the element children of its parent
//
using elementPath = std::vector<int>;

// An element of the input archive
//
struct elementLocation {
    int             unitNumber{0};      // Position of the unit in the archive, counted from 1
    elementPath     path;               // Place of the element in the unit
};

#endif
