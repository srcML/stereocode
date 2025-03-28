// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file IgnorableCalls.hpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef IGNORABLECALLS_HPP
#define IGNORABLECALLS_HPP

#include <string>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <iostream>

class ignorableCalls {
public:
    bool                 isIgnored                (const std::string&, const std::string&);
    void                 addCall                  (const std::string&);
    void                 createCallList           ();
    void                 outputCalls              ();
    friend std::istream& operator>>               (std::istream&, ignorableCalls&);

private:
    std::unordered_map<std::string, std::unordered_set<std::string>>       ignoredCalls;        // List of calls to ignore
    std::unordered_set<std::string>                                        userIgnoredCalls;    // List of user-defined calls to ignore 
};

#endif
