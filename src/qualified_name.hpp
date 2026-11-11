// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file qualified_name.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef QUALIFIED_NAME_HPP
#define QUALIFIED_NAME_HPP

#include <string>
#include <string_view>
#include <vector>

// The parts of a name written without whitespace, such as std, vector<int>, and size_type in std::vector<int>::size_type

std::vector<std::string>    splitQualifiedName      (std::string_view, std::string_view);
std::string                 withoutGenericArguments (std::string_view);
std::string                 genericArguments        (std::string_view);
int                         genericArity            (std::string_view);
std::string                 genericSkeleton         (std::string_view);

#endif
