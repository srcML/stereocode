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

namespace {
    // Gets the method of a call, the name after its last access operator
    //
    std::string methodOf(const std::string& call) {
        std::size_t start = 0;
        for (std::string_view separator : {".", "->", "::"}) {
            const std::size_t found = call.rfind(separator);
            if (found != std::string::npos) start = std::max(start, found + separator.size());
        }
        return call.substr(start);
    }

    // Tells whether the methods of a language hold a method with a number of arguments
    //
    bool holdsMethod(const std::unordered_map<std::string, std::unordered_map<std::string, int>>& methods, const std::string& method, int arguments,
                     const std::string& unitLanguage) {
        const auto language = methods.find(unitLanguage);
        if (language == methods.end()) return false;
        const auto found = language->second.find(method);
        return found != language->second.end() && found->second == arguments;
    }
}

// Tells whether a call, written without whitespace and generic arguments, such as m_items.size, calls a query method, whatever its arguments
//
bool queryMethods::isQuery(const std::string& call, const std::string& unitLanguage) const {
    const std::string method = methodOf(call);
    const auto language = defaultMethods.find(unitLanguage);
    return (language != defaultMethods.end() && language->second.count(method) > 0) || userMethods.contains(method, unitLanguage);
}

// Tells whether a call with a number of arguments calls a query method, where a method such as GetValue reads only with one number of arguments
//
bool queryMethods::isQuery(const std::string& call, int arguments, const std::string& unitLanguage) const {
    return isQuery(call, unitLanguage) || holdsMethod(countedMethods, methodOf(call), arguments, unitLanguage);
}

// Tells whether a method with a number of arguments is an accessor that every object inherits from the root class of its language,
// as toString of the Java class Object
//
bool queryMethods::isRootAccessor(const std::string& method, int arguments, const std::string& unitLanguage) const {
    return holdsMethod(rootAccessors, method, arguments, unitLanguage);
}

// Tells whether a method with a number of arguments is a static method of the root class of its language, as ReferenceEquals of the C# object,
// which a call without an object names in every type
//
bool queryMethods::isRootFunction(const std::string& method, int arguments, const std::string& unitLanguage) const {
    return holdsMethod(rootFunctions, method, arguments, unitLanguage);
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
        const auto counted = countedMethods.find(language);
        if (counted == countedMethods.end()) continue;
        for (const auto& [method, arguments] : counted->second) std::cerr << ' ' << method << '/' << arguments;
    }
    userMethods.print(std::cerr);
    std::cerr << "\n\n";
}

// The methods of the strings, containers, and objects of each language that return a value about the object
// The C++ accessors that give write access, such as begin, at, or front, may change a data member through their result
//
void queryMethods::initializeQueryMethods() {
    defaultMethods.insert({"C++", {"size", "empty", "length", "count", "c_str", "capacity", "max_size"}});
    defaultMethods.insert({"C#", {"Count", "Contains", "ContainsKey", "TryGetValue", "ToString", "Equals", "GetHashCode", "GetType"}});
    defaultMethods.insert({"Java", {"size", "isEmpty", "length", "get", "contains", "containsKey", "equals", "hashCode", "toString", "getClass"}});
    // The methods of the immutable types str, bytes, int, float, tuple, and frozenset, and those of list, dict, and set that change nothing
    defaultMethods.insert({"Python", {"capitalize", "casefold", "center", "count", "encode", "endswith", "expandtabs", "find", "format", "format_map", "index",
                "isalnum", "isalpha", "isascii", "isdecimal", "isdigit", "isidentifier", "islower", "isnumeric", "isprintable", "isspace", "istitle",
                "isupper", "join", "ljust", "lower", "lstrip", "partition", "removeprefix", "removesuffix", "replace", "rfind", "rindex", "rjust",
                "rpartition", "rsplit", "rstrip", "split", "splitlines", "startswith", "strip", "swapcase", "title", "translate", "upper", "zfill",
                "decode", "hex", "bit_length", "bit_count", "to_bytes", "as_integer_ratio", "is_integer", "conjugate",
                "copy", "get", "items", "keys", "values", "union", "intersection", "difference", "symmetric_difference", "isdisjoint", "issubset",
                "issuperset"}});
    // The methods of strings and those of arrays, maps, and sets that change nothing
    defaultMethods.insert({"JavaScript", {"charAt", "charCodeAt", "codePointAt", "endsWith", "includes", "indexOf", "lastIndexOf", "localeCompare", "match",
                "matchAll", "normalize", "padEnd", "padStart", "repeat", "replace", "replaceAll", "search", "slice", "split", "startsWith", "substring",
                "toLowerCase", "toUpperCase", "trim", "trimStart", "trimEnd", "toString", "valueOf", "at", "concat", "every", "filter", "find", "findIndex",
                "findLast", "findLastIndex", "flat", "flatMap", "forEach", "join", "keys", "values", "entries", "map", "reduce", "reduceRight", "some", "get",
                "has", "toSorted", "toReversed", "toSpliced"}});

    // GetValue of a dependency object, a registry key, or a member of a type reads, while GetValue of a ConditionalWeakTable takes a callback and adds a missing key
    countedMethods.insert({"C#", {{"GetValue", 1}}});

    rootAccessors.insert({"C#", {{"Equals", 1}, {"GetHashCode", 0}, {"ToString", 0}, {"GetType", 0}, {"MemberwiseClone", 0}}});
    rootAccessors.insert({"Java", {{"equals", 1}, {"hashCode", 0}, {"toString", 0}, {"getClass", 0}, {"clone", 0}}});
    rootAccessors.insert({"Python", {{"__repr__", 0}, {"__str__", 0}, {"__hash__", 0}, {"__eq__", 1}, {"__ne__", 1}, {"__lt__", 1}, {"__le__", 1},
                                     {"__gt__", 1}, {"__ge__", 1}, {"__format__", 1}, {"__sizeof__", 0}, {"__dir__", 0}, {"__getattribute__", 1},
                                     {"__reduce__", 0}, {"__reduce_ex__", 1}, {"__getstate__", 0}}});
    rootFunctions.insert({"C#", {{"Equals", 2}, {"ReferenceEquals", 2}}});
}
