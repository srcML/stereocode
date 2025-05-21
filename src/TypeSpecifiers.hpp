// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file TypeSpecifiers.hpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef TYPESPECIFIERS_HPP
#define TYPESPECIFIERS_HPP

#include <string>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <cctype>
#include <iostream>

class typeSpecifiers {
public:
    const std::string&   getTypeSpecifiers      (const std::string&) const;
   
    void                 addTypeSpecifier       (const std::string&);
    void                 createSpecifierList    ();
    void                 outputSpecifiers       ();
    friend std::istream& operator>>             (std::istream&, typeSpecifiers&);

private:
    std::unordered_map<std::string, std::unordered_set<std::string>>       specifiersTypes;          // List of type specifiers
    std::unordered_set<std::string>                                        userSpecifiersTypes;      // List of user defined type specifiers
    std::unordered_map<std::string, std::string>                           patterns;                                 
};

#endif
