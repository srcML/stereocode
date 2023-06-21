// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file variable.hpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
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

const std::vector<std::string> HEADER_FILE_EXTENSION = {".hpp", ".h", ".HPP", ".H", ".hxx", ".hh", ".h++", ".i", ".ii", ".tcc"};

bool         isHeaderFile         (const std::string&);
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
