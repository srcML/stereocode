// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file return_types.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef RETURN_TYPES_HPP
#define RETURN_TYPES_HPP

#include <string>

bool isBooleanType     (const std::string&, const std::string&);
bool isVoidType        (const std::string&, const std::string&);
bool isVoidPointerType (const std::string&, const std::string&);
bool isNonVoidType     (const std::string&, const std::string&, const std::string&);
bool isSpelledAsMacro  (const std::string&);

#endif
