// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file specifiers.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "rules/specifiers.hpp"
#include "helper_functions.hpp"

#include <cctype>
#include <iostream>
#include <sstream>
#include <vector>

// Removes specifiers from a type string based on unit language
//
void specifiers::removeSpecifiers(std::string& type, const std::string& unitLanguage) const {
    auto pattern = specifiersPatterns.find(unitLanguage);
    if (pattern != specifiersPatterns.end()) type = pattern->second.replace(type);
}

// Gets the names in a type once its specifiers are removed, such as Node and T in Node<T>*
// A specifier or a comma separates two names, while the whitespace inside a name goes, as in longint for long int
// A name keeps no namespace and no array brackets, and an integer argument, such as 3 in std::array<int, 3>, names no type
//
std::vector<std::string> specifiers::typeNames(const std::string& type, const std::string& unitLanguage) const {
    const auto pattern = specifiersPatterns.find(unitLanguage);
    const std::vector<std::string> pieces = pattern != specifiersPatterns.end() ? pattern->second.split(type) : std::vector<std::string>{type};

    std::vector<std::string> names;
    for (const std::string& piece : pieces) {
        std::stringstream stream(piece);
        for (std::string name; std::getline(stream, name, ',');) {
            HELPERS::removeWhitespace(name);
            HELPERS::removeNamespace(name, unitLanguage, true);
            HELPERS::removeBracketSuffix(name);
            if (!name.empty() && !std::isdigit(static_cast<unsigned char>(name.front()))) names.push_back(name);
        }
    }
    return names;
}

// Reads a set of user-defined type specifiers
// File should list one type per line
//
std::istream& operator>>(std::istream& in, specifiers& specifiers) {
    return in >> specifiers.userSpecifiersList;
}

// Outputs the list of specifiers to standard error for debugging purposes
//
void specifiers::outputSpecifiers() const {
    std::cerr<<"---Specifiers---";
    for (const auto& pair : specifiersList) {
        std::cerr<<"\n[" << pair.first << "]:" ;
        for (const std::string& type : pair.second) 
            std::cerr << ' ' << type;
    }
    userSpecifiersList.print(std::cerr);
    std::cerr << "\n\n";
}

// Specific type specifiers are used based on unit language
//
void specifiers::initializeSpecifiersList() {
    specifiersList.insert({"C++", { "const", "volatile", "inline", "virtual", "friend", "extern", "&", "&&", "\\*", "public", "private", "protected",
                "mutable", "static", "thread_local", "register", "constexpr", "explicit", "signed", "unsigned",
                "<", ">", "vector", "list", "set", "map", "unordered_map", "array", "multimap", "unordered_multimap", 
                "::iterator", "::const_iterator", "forward_list", "stack", "queue", "priority_queue", "deque", "multiset", 
                "unordered_set", "unordered_multiset", "pair", "span", "restrict", "_Noreturn", "_Thread_local",
                // srcML marks a calling convention of Windows as a name of the type
                "__stdcall", "__cdecl", "__fastcall", "__thiscall", "__vectorcall", "__clrcall", "WINAPI", "WINAPIV", "APIENTRY", "CALLBACK",
                "PASCAL", "STDMETHODCALLTYPE", "STDAPICALLTYPE"}});

    specifiersList.insert({"C#", { "readonly", "ref", "out", "in", "unsafe", "internal", "params",
                "public", "private", "protected", "static", "virtual", "\\*", "volatile",
                "this",  "override", "abstract",  "extern", "async", "partial", "explicit", "implicit",
                "new", "sealed", "event", "const", "\\?", "<", ">", "List", "Dictionary", "HashSet", "Queue", "Stack", "SortedList", "LinkedList", 
                "IEnumerable(?=<)", "IList(?=<)", "ICollection(?=<)", "IReadOnlyList", "IReadOnlyCollection", "ISet",
                "BitArray", "KeyedCollection", "SortedSet", "BlockingCollection", "ConcurrentQueue", "ConcurrentStack", 
                "ConcurrentDictionary", "ConcurrentBag", "ReadOnlyCollection", "ReadOnlyDictionary", "Tuple", "ValueTuple", 
                "NameValueCollection", "StringCollection", "StringDictionary", "HybridDictionary", "OrderedDictionary"}});

    specifiersList.insert({"Java", { "public", "private", "protected", "static", "final", "transient", "\\?", "@\\w+",
                "volatile", "synchronized", "native", "strictfp", "abstract", "default", "super", "extends", "\\.\\.\\.",
                "<", ">", "Collection", "Iterable", "List", "ArrayList", "LinkedList", "Set", "HashSet", "LinkedHashSet", "SortedSet", "TreeSet", "Map",
                "HashMap", "Hashtable", "LinkedHashMap", "SortedMap", "TreeMap", "Deque", "ArrayDeque", "Queue", "PriorityQueue", 
                "Vector", "Stack", "EnumSet", "EnumMap", "Iterator"}});

    // The pattern of each language joins its specifiers and then the user specifiers for all languages and for the language of a section
    for (const auto& languageSpecifiers : specifiersList) {
        const std::vector<std::string> defaults(languageSpecifiers.second.begin(), languageSpecifiers.second.end());
        specifiersPatterns.try_emplace(languageSpecifiers.first, defaults, userSpecifiersList.entriesOf(languageSpecifiers.first));
    }
}
