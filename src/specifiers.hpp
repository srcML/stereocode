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

#include <string>
#include <unordered_set>
#include <unordered_map>

class specifiers {
public:
    const std::string&   getSpecifiers            (const std::string&) const;
    void                 removeSpecifiers         (std::string&, std::string);
    void                 addSpecifiers            (const std::string&);
    void                 initializeSpecifiersList ();
    void                 outputSpecifiers         ();
    friend std::istream& operator>>               (std::istream&, specifiers&);
private:
    std::unordered_map<std::string, std::unordered_set<std::string>>       specifiersList;          // List of specifiers
    std::unordered_set<std::string>                                        userSpecifiersList;      // List of user defined specifiers
    std::unordered_map<std::string, std::string>                           specifiersPatterns;      // Regex patterns for each language               
};

#endif
