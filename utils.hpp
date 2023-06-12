/**
 * @file util.hpp
 *
 * @copyright Copyright (C) 2010-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of Stereocode.
 * 
 * Stereocode is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Stereocode is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stereocode; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <cstring>
#include <vector>
#include <set>
#include <algorithm>
#include <cctype>
#include "PrimitiveTypes.hpp"

extern primitiveTypes PRIMITIVES;
extern bool           DEBUG;

const std::string     NO_STEREOTYPE = "unclassified";

const std::vector<std::string> METHOD_STEREOTYPE =
   {"get", "non-const-get", "set", "predicate",
    "property", "void-accessor", "collaborator", "command", "non-void-command",
    "controller", "factory", "empty", "stateless", "wrapper" };

const std::vector<std::string> CLASS_STEREOTYPE =
   {"entity", "minimal-entity", "data-provider", "command", "boundary",
    "control", "pure-control", "factory",  "large-class", "lazy-class",
    "degenerate", "data-class", "small-class" };

const std::vector<std::string> ASSIGNMENT_OPERATOR = {"=", "+=", "-=", "*=", "/=", "%=", ">>=", "<<=", "&=", "^=", "|=", "<<"};

bool         isGlobalConstFormat  (const std::string&);
bool         checkConst           (const std::string&);
bool         isInheritedAttribute (const std::vector<std::string>&, const std::vector<std::string>&, const std::string&);
int          countPureCalls       (const std::vector<std::string>&) ;
bool         isPrimitiveContainer (const std::string&);

std::string  removeSpecifiers     (const std::string&);
std::string  separateTypeName     (const std::string&);
std::string  trimWhitespace       (const std::string&);
std::string  WStoBlank            (const std::string&);
std::string  Ltrim                (const std::string&);
std::string  Rtrim                (const std::string&);
std::string  multiBlanksToBlank   (const std::string&);


#endif
