// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file query_methods.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "rules/query_methods.hpp"

#include <algorithm>
#include <iostream>
#include <string_view>

// Tells whether a call, written without whitespace and generic arguments, such as m_items.size, calls a query method
// The method is the name after the last access operator
//
bool queryMethods::isQuery(const std::string& call, const std::string& unitLanguage) const {
    std::size_t start = 0;
    for (std::string_view separator : {".", "->", "::"}) {
        const std::size_t found = call.rfind(separator);
        if (found != std::string::npos) start = std::max(start, found + separator.size());
    }
    const std::string method = call.substr(start);

    const auto language = defaultMethods.find(unitLanguage);
    return (language != defaultMethods.end() && language->second.count(method) > 0) || userMethods.contains(method, unitLanguage);
}

// Reads the query methods of the user, one per line
//
std::istream& operator>>(std::istream& in, queryMethods& methods) {
    return in >> methods.userMethods;
}

// Outputs the lists of query methods to stderr for debugging purposes
//
void queryMethods::outputQueryMethods() const {
    std::cerr << "---Query Methods---";
    for (const auto& [language, methods] : defaultMethods) {
        std::cerr << "\n[" << language << "]:";
        for (const std::string& method : methods) std::cerr << ' ' << method;
    }
    userMethods.print(std::cerr);
    std::cerr << "\n\n";
}

// The methods of the strings, containers, and objects of each language that return a value about the object
// The C++ accessors that give write access, such as begin, at, or front, may change a data member through their result
//
void queryMethods::initializeQueryMethods() {
    defaultMethods.insert({"C++", {"size", "empty", "length", "count", "c_str", "capacity", "max_size"}});
    defaultMethods.insert({"C#", {"Count", "Contains", "ContainsKey", "TryGetValue", "ToString", "Equals", "GetHashCode"}});
    defaultMethods.insert({"Java", {"size", "isEmpty", "length", "get", "contains", "containsKey", "equals", "hashCode", "toString"}});
}
