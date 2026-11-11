// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file user_lists.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef USER_LISTS_HPP
#define USER_LISTS_HPP

#include <istream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// The files of user primitives, calls, specifiers, and query methods, which hold one entry per line
// A section header, such as [C#], gives the entries after it to the units of its language, while the entries before the first header hold for every language

// An entry of a list file with the language of its section, which is empty for an entry before the first section header
//
struct listEntry {
    std::string language;
    std::string text;
};

std::vector<listEntry>      readListEntries     (std::istream&);
std::vector<std::string>    headersInOtherCase  (std::istream&);

// The entries of a user list file, each for every language or for the language of its section
//
class userEntries {
public:
    bool                        contains    (const std::string&, const std::string&) const;
    std::vector<std::string>    entriesOf   (const std::string&) const;
    void                        print       (std::ostream&) const;

    friend std::istream&        operator>>  (std::istream&, userEntries&);
private:
    std::unordered_map<std::string, std::unordered_set<std::string>> entries;  // Entries of each language, where the empty language stands for every language
};

#endif
