// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file query_methods.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef QUERY_METHODS_HPP
#define QUERY_METHODS_HPP

#include "rules/user_lists.hpp"

#include <istream>
#include <string>
#include <unordered_map>
#include <unordered_set>

// The query methods, such as size or ToString, whose calls on a data member read the data member without changing it
//
class queryMethods {
public:
    bool                    isQuery                 (const std::string&, const std::string&) const;
    void                    initializeQueryMethods  ();
    void                    outputQueryMethods      () const;

    friend std::istream&    operator>>              (std::istream&, queryMethods&);
private:
    std::unordered_map<std::string, std::unordered_set<std::string>>   defaultMethods;     // Query methods of each language
    userEntries                                                        userMethods;        // Query methods of the user, for every language or for the language of a section
};

#endif
