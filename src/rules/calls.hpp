// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file calls.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef CALLS_HPP
#define CALLS_HPP

#include "rules/user_lists.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

class calls {
public:
    bool                    isIgnored                   (const std::string&, const std::string&) const;
    void                    initializeIgnorableCalls    ();
    void                    outputIgnorableCalls        () const;

    friend std::istream&    operator>>                  (std::istream&, calls&);
private:
    std::unordered_map<std::string, std::unordered_set<std::string>>       ignoredCallsList;        // List of calls to ignore
    userEntries                                                            userIgnoredCallsList;    // List of user-defined calls to ignore 
};

#endif
