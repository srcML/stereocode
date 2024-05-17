// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file IgnorableCalls.hpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef IGNORABLE_CALLS
#define IGNORABLE_CALLS

#include "utils.hpp"

class ignorableCalls {
public:
    bool           isCallIgnored    (const std::string&) const;
    
    void           addIgnoredCall   (const std::string&);
    void           setLanguage      (const std::string&);

    friend         std::istream& operator>>(std::istream&, ignorableCalls&);

private:
    std::string                           unitLanguage{};        // Language: "C++", "C#", or "Java"
    std::unordered_set<std::string>       ignoredCalls{};        // List of calls to ignore
    std::unordered_set<std::string>       userIgnoredCalls{};    // List of user-defined calls to ignore 
};

#endif
