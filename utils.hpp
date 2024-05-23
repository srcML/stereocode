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
#include <iostream>
#include <string>
#include <string_view>
#include "PrimitiveTypes.hpp"
 

bool                            isNonPrimitiveType            (const std::string& type, bool& externalNonPrimitive, 
                                                               const std::string& unitLanguage, const std::string& className);
bool                            isPrimitiveType               (const std::string&);
void                            createSpecifierList           ();
void                            removeSpecifiers              (std::string&, std::string);
void                            removeContainers              (std::string&, std::string);
void                            trimWhitespace                (std::string&);
void                            Ltrim                         (std::string&);
void                            Rtrim                         (std::string&);
void                            removeNamespace               (std::string&, bool, std::string_view);
void                            WStoBlank                     (std::string&);
void                            removeBetweenComma            (std::string& s, bool);

#endif
