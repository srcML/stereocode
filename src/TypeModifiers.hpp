// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file TypeModifiers.hpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef TYPEMODIFIERS_HPP
#define TYPEMODIFIERS_HPP

#include <string>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <cctype>
#include <iostream>

class typeModifiers {
public:
    const std::string&   getTypeModifiers   (const std::string&) const;
   
    void                 addTypeModifier       (const std::string&);
    void                 createModifierList    ();
    void                 outputModifiers       ();
    friend std::istream& operator>>         (std::istream&, typeModifiers&);

private:
    std::unordered_map<std::string, std::unordered_set<std::string>>       mtypes;          // List of type modifiers
    std::unordered_set<std::string>                                        userMtypes;      // List of user defined type modifiers
    std::unordered_map<std::string, std::string>                           patterns;                                 
};

#endif
