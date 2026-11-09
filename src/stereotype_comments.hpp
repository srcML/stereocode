// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_comments.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef STEREOTYPE_COMMENTS_HPP
#define STEREOTYPE_COMMENTS_HPP

#include <string>
#include <string_view>

// The line comments "// @stereotype <labels>" of the ICSME 2024 paper, before each labeled element of a srcML unit

std::string withStereotypeComments (std::string_view);

#endif
