// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file specifiers.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef SPECIFIERS_HPP
#define SPECIFIERS_HPP

#include "rules/specifier_pattern.hpp"
#include "rules/user_lists.hpp"

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

class specifiers {
public:
    void                 removeSpecifiers         (std::string&, const std::string&) const;
    std::vector<std::string> typeNames            (const std::string&, const std::string&) const;
    void                 initializeSpecifiersList ();
    void                 outputSpecifiers         () const;
    friend std::istream& operator>>               (std::istream&, specifiers&);
private:
    std::unordered_map<std::string, std::unordered_set<std::string>>       specifiersList;          // List of specifiers
    userEntries                                                            userSpecifiersList;      // List of user defined specifiers
    std::unordered_map<std::string, specifierPattern>                      specifiersPatterns;      // Pattern of the specifiers of each language
};

#endif
