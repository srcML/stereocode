// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file utils.hpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */


#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <regex>
#include <sstream>

#include "PrimitiveTypes.hpp"
#include "AttributeInfo.hpp"

extern       primitiveTypes PRIMITIVES;

const        std::vector<std::string> ASSIGNMENT_OPERATOR = {"=", "+=", "-=", "*=", "/=", "%=", ">>=", "<<=", "&=", "^=", "|=", "\\?\\?=", ">>>=" };

bool         isPrimitiveContainer (const std::string&);
bool         isAttribute          (std::vector<AttributeInfo>&, const std::vector<std::string>&, 
                                   const std::vector<std::string>&, const std::string&, bool, int&, int&);

std::string  removeSpecifiers     (const std::string&);
std::string  trimWhitespace       (const std::string&);
std::string  Ltrim                (const std::string&);
std::string  Rtrim                (const std::string&);
std::string  removeNamespace      (const std::string&);
std::string  WStoBlank            (const std::string&);

#endif
