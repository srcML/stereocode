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
#include "PrimitiveTypes.hpp"
#include "AttributeInfo.hpp"

extern primitiveTypes PRIMITIVES;

const std::string     NO_STEREOTYPE = "";
const std::vector<std::string> ASSIGNMENT_OPERATOR = {"=", "+=", "-=", "*=", "/=", "%=", ">>=", "<<=", "&=", "^=", "|=", "<<"};
const std::vector<std::string> HEADER_FILE_EXTENSION = {".hpp", ".h", ".HPP", ".H", ".hxx", ".hh", ".h++", ".i", ".ii", ".tcc"};

bool         isHeaderFile         (const std::string&);
bool         checkConst           (const std::string&);
bool         isInheritedAttribute (const std::vector<std::string>&, const std::vector<std::string>&, const std::string&);
bool         isPrimitiveContainer (const std::string&);
bool         isAttribute          (const std::vector<AttributeInfo>&, const std::vector<std::string>&, 
                                   const std::vector<std::string>&, const std::string&);
bool         isDuplicateAttribute (const std::vector<AttributeInfo>&, const std::string&);

std::string  removeSpecifiers     (const std::string&);
std::string  separateTypeName     (const std::string&);
std::string  trimWhitespace       (const std::string&);
std::string  Ltrim                (const std::string&);
std::string  Rtrim                (const std::string&);
std::string  multiBlanksToBlank   (const std::string&);
std::string  WStoBlank            (const std::string&);

#endif