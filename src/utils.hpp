// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file utils.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <string_view>
#include "variable.hpp"

void                            checkNonPrimitiveType         (const std::string& type, variable&, const std::string& unitLanguage, const std::string& className);
bool                            isPrimitiveType               (const std::string&, const std::string& unitLanguage);
bool                            matchSubstringAtBeginning     (const std::string&, const std::string&);
void                            createSpecifierList           ();
void                            removeTypeSpecifiers          (std::string&, std::string);
void                            removeBracketSuffix           (std::string&);
void                            removeLeadingAsterisks        (std::string&);
void                            trimWhitespace                (std::string&);
void                            Ltrim                         (std::string&);
void                            Rtrim                         (std::string&);
void                            removeNamespace               (std::string&, std::string_view, bool);
void                            removeBetweenComma            (std::string& s, bool);

#endif
