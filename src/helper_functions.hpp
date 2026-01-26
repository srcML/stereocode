// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file helper_functions.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef HELPER_FUNCTIONS_HPP
#define HELPER_FUNCTIONS_HPP

#include "variable_model.hpp"

#include <string>
#include <string_view>

class helperFunctions {
public:
    static std::string                     escapeCSV                     (const std::string& data);
    static bool                            isSubstringAtBeginning        (const std::string&, const std::string&);
    static void                            removeBracketSuffix           (std::string&);
    static void                            removeLeadingAsterisks        (std::string&);
    static void                            removeNamespace               (std::string&, std::string_view, bool);
    static void                            removeBetweenComma            (std::string& s, bool);
    static void                            removeWhitespace              (std::string&);
};

#endif
