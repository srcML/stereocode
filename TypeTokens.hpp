// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file TypeTokens.hpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef TYPETOKENS_HPP
#define TYPETOKENS_HPP

#include <string>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <cctype>
#include <iostream>

class typeTokens {
public:
    const std::string&   getTypeTokens      (const std::string&) const;
   
    void                 addTypeToken       (const std::string&);
    void                 createTokenList    ();
    void                 outputTokens       ();
    friend std::istream& operator>>         (std::istream&, typeTokens&);

private:
    std::unordered_map<std::string, std::unordered_set<std::string>>       ttypes;          // List of type tokens
    std::unordered_set<std::string>                                        userTtypes;      // List of user defined type tokens
    std::unordered_map<std::string, std::string>                           patterns;                                 
};

#endif
