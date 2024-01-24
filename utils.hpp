// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file utils.hpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <regex>
#include <sstream>
#include <string_view>

#include "PrimitiveTypes.hpp"
#include "Variable.hpp"

extern       primitiveTypes PRIMITIVES;

bool                            isPrimitiveType      (const std::string&);
void                            removeSpecifiers     (std::string&, std::string_view);
void                            removeContainers     (std::string&, std::string_view);
void                            trimWhitespace       (std::string&);
void                            Ltrim                (std::string&);
void                            Rtrim                (std::string&);
void                            removeNamespace      (std::string&, bool, std::string_view);
void                            WStoBlank            (std::string&);
void                            commaConversion      (std::string& s);
void                            removeBetweenComma   (std::string& s);

#endif
