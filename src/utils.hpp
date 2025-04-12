// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file utils.hpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <regex>
#include <sstream>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <map>
#include <cstddef>
#include "PrimitiveTypes.hpp"
#include "TypeModifiers.hpp"
#include "variable.hpp"
 

bool                            isNonPrimitiveType            (const std::string& type, variable&, 
                                                               const std::string& unitLanguage, const std::string& className);
bool                            isPrimitiveType               (const std::string&, const std::string& unitLanguage);
bool                            matchSubstring                (const std::string&, const std::string&);
void                            createSpecifierList           ();
void                            removeTypeModifiers           (std::string&, std::string);
void                            trimWhitespace                (std::string&);
void                            Ltrim                         (std::string&);
void                            Rtrim                         (std::string&);
void                            removeNamespace               (std::string&, bool, std::string_view);
void                            removeBetweenComma            (std::string& s, bool);
void                            srcmlBackwardCompatibility    (std::string&);

#endif
