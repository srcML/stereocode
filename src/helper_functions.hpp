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
    std::string                     escapeCSV                     (const std::string& data);
    bool                            isSubstringAtBeginning        (const std::string&, const std::string&);
    void                            removeBracketSuffix           (std::string&);
    void                            removeLeadingAsterisks        (std::string&);
    void                            removeNamespace               (std::string&, std::string_view, bool);
    void                            removeBetweenComma            (std::string& s, bool);
    void                            removeWhitespace              (std::string&);
};

#endif
