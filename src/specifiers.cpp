// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file specifiers.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "specifiers.hpp"

#include <regex>
#include <iostream>
#include <vector>

// Removes specifiers from a type string based on unit language
//
void specifiers::removeSpecifiers(std::string& type, std::string unitLanguage) {
    static const std::regex cppPattern(specifiersPatterns.at("C++"));
    static const std::regex javaPattern(specifiersPatterns.at("Java"));
    static const std::regex csharpPattern(specifiersPatterns.at("C#"));

    const std::regex* currentPattern = nullptr;
    if (unitLanguage == "C++") currentPattern = &cppPattern;
    else if (unitLanguage == "Java") currentPattern = &javaPattern;
    else if (unitLanguage == "C#") currentPattern = &csharpPattern;
    
    if (currentPattern) type = std::regex_replace(type, *currentPattern, " ");
}

// Reads a set of user-defined type specifiers
// File should list one type per line
//
std::istream& operator>>(std::istream& in, specifiers& specifiers) {
    std::string name;
    while(std::getline(in, name)) specifiers.userSpecifiersList.insert(name);
    return in;
}

// Outputs the list of specifiers to standard error for debugging purposes
//
void specifiers::outputSpecifiers() {
    std::cerr<<"---Specifiers---";
    for (const auto& pair : specifiersList) {
        std::cerr<<"\n[" << pair.first << "]:" ;
        for (const std::string& type : pair.second) 
            std::cerr << ' ' << type;
    }
    if (userSpecifiersList.size() > 0) {
        std::cerr<<"\n[User-Defined]:";
        for (const std::string& type : userSpecifiersList) 
            std::cerr << ' ' << type;
    }
    std::cerr << "\n\n";
}

// Specific type specifiers are used based on unit language
//
void specifiers::initializeSpecifiersList() {
    specifiersList.insert({"C++", { "const", "volatile", "inline", "virtual", "friend", "extern", "&", "&&", "\\*", "public", "private", "protected",
                "mutable", "static", "thread_local", "register", "constexpr", "explicit", "signed", "unsigned",
                "<", ">", "vector", "list", "set", "map", "unordered_map", "array", "multimap", "unordered_multimap", 
                "::iterator", "::const_iterator", "forward_list", "stack", "queue", "priority_queue", "deque", "multiset", 
                "unordered_set", "unordered_multiset", "pair", "restrict", "_Noreturn", "_Thread_local"}});

    specifiersList.insert({"C#", { "readonly", "ref", "out", "in", "unsafe", "internal", "params",
                "public", "private", "protected", "static", "virtual", "\\*", "volatile",
                "this",  "override", "abstract",  "extern", "async", "partial", "explicit", "implicit",
                "new", "sealed", "event", "const", "\\?", "<", ">", "List", "Dictionary", "HashSet", "Queue", "Stack", "SortedList", "LinkedList", 
                "BitArray", "KeyedCollection", "SortedSet", "BlockingCollection", "ConcurrentQueue", "ConcurrentStack", 
                "ConcurrentDictionary", "ConcurrentBag", "ReadOnlyCollection", "ReadOnlyDictionary", "Tuple", "ValueTuple", 
                "NameValueCollection", "StringCollection", "StringDictionary", "HybridDictionary", "OrderedDictionary"}});

    specifiersList.insert({"Java", { "public", "private", "protected", "static", "final", "transient", "\\?", "@\\w+",
                "volatile", "synchronized", "native", "strictfp", "abstract", "default", "super", "extends", "\\.\\.\\.",
                "<", ">", "List", "ArrayList", "LinkedList", "Set", "HashSet", "LinkedHashSet", "SortedSet", "TreeSet", "Map", 
                "HashMap", "Hashtable", "LinkedHashMap", "SortedMap", "TreeMap", "Deque", "ArrayDeque", "Queue", "PriorityQueue", 
                "Vector", "Stack", "EnumSet", "EnumMap", "Iterator"}});

    // Create a regex pattern of specifiers joined by the '|' (or) operator
    // \b pattern \b ensures that 'pattern' is only matched when it is surrounded by non-word characters
    // For example, a type called staticClass, the static in this name will be kept when removing the specifiers
    // Non-word characters is anything outside of [A-Za-z0-9]
    //  For instance, \n or \r or ' ' or beginning or end of strings are considered as non-word characters
    for (const auto& specifiers: specifiersList) {
        std::string pattern;
        for (const auto& specifier : specifiers.second) {
            if (pattern.size() > 0) pattern += "|";
            
            // If specifier is one of the special characters (like * or & or []), match them without word boundaries.
            bool isAlphaNumeric = true;
            for (char c : specifier) {
                if (!std::isalnum(c) && c != '_')
                    isAlphaNumeric = false;
            }

            if (!isAlphaNumeric)
                pattern += specifier;     
            else 
                pattern += "\\b" + specifier + "\\b";      
        }
        
        // User-defined specifier types apply to all languages
        for (const auto& userSpecifier : userSpecifiersList) {
            pattern += "|";
            
            bool isAlphaNumeric = true;
            for (char c : userSpecifier) {
                if (!std::isalnum(c) && c != '_')
                    isAlphaNumeric = false;
            }

            if (!isAlphaNumeric)
                pattern += userSpecifier;     
            else 
                pattern += "\\b" + userSpecifier + "\\b";      
        }

        pattern = "(" + pattern + ")";
        specifiersPatterns.insert({specifiers.first, pattern});
    }
}
