// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file TypeModifiers.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "TypeModifiers.hpp"

extern std::vector<std::string> LANGUAGE;

const std::string& typeModifiers::getTypeModifiers (const std::string& unitLang) const {
    return patterns.at(unitLang);
}

// Adds "type" to user-defined type modifiers if not already present
//
void typeModifiers::addTypeModifier  (const std::string& type) {
    userMtypes.insert(type);
}

// Reads a set of user-defined type Modifiers
// File should list one type per line
//
std::istream& operator>>(std::istream& in, typeModifiers& tmodifiers) {
    std::string name;
    while(std::getline(in, name))
        tmodifiers.addTypeModifier  (name);
    return in;
}

void typeModifiers::outputModifiers() {
    std::cerr<<"---Type Modifiers---";
    for (const auto& pair : mtypes) {
        std::cerr<<"\n[" << pair.first << "]:" ;
        for (const std::string& type : pair.second) 
            std::cerr << ' ' << type;
    }
    if (userMtypes.size() > 0) {
        std::cerr<<"\n[User-Defined]:";
        for (const std::string& type : userMtypes) 
            std::cerr << ' ' << type;
    }
    std::cerr << "\n\n";
}

// Specific type Modifiers are used based on unit language
//
void typeModifiers::createModifierList() {
    for (const auto& l : LANGUAGE) {
        if (l == "C++") {
            mtypes.insert({l, { "const", "volatile", "inline", "virtual", "friend", "extern", "&", "&&", "\\*", "public", "private", "protected",
                        "mutable", "static", "thread_local", "register", "constexpr", "explicit", "signed", "unsigned",
                        "<", ">", "vector", "list", "set", "map", "unordered_map", "array", "multimap", "unordered_multimap", 
                        "::iterator", "::const_iterator", "forward_list", "stack", "queue", "priority_queue", "deque", "multiset", 
                        "unordered_set", "unordered_multiset", "pair"}});
        }
        
        // \\[.*\\] match any square brackets [] with any single character inside
        //   (if any) . with zero or more occurrence * (including empty)
        //   such as int[] or int[,] and so on
        else if (l == "C#") {
             mtypes.insert({l, { "readonly", "ref", "out", "in", "unsafe", "internal", "params",
                        "public", "private", "protected", "static", "virtual", "\\*", "volatile", "\\[.*\\]",
                        "this",  "override", "abstract",  "extern", "async", "partial", "explicit", "implicit"
                        "new", "sealed", "event", "const", "\\?", "<", ">", "List", "Dictionary", "HashSet", "Queue", "Stack", "SortedList", "LinkedList", 
                        "BitArray", "KeyedCollection", "SortedSet", "BlockingCollection", "ConcurrentQueue", "ConcurrentStack", 
                        "ConcurrentDictionary", "ConcurrentBag", "ReadOnlyCollection", "ReadOnlyDictionary", "Tuple", "ValueTuple", 
                        "NameValueCollection", "StringCollection", "StringDictionary", "HybridDictionary", "OrderedDictionary"}});
        }
        else if (l == "Java") {
            mtypes.insert({l, { "public", "private", "protected", "static", "final", "transient",  "\\[.*\\]", "\\?", "@\\w+",
                        "volatile", "synchronized", "native", "strictfp", "abstract", "default", "super", "extends", "\\.\\.\\.",
                        "<", ">", "List", "ArrayList", "LinkedList", "Set", "HashSet", "LinkedHashSet", "SortedSet", "TreeSet", "Map", 
                        "HashMap", "Hashtable", "LinkedHashMap", "SortedMap", "TreeMap", "Deque", "ArrayDeque", "Queue", "PriorityQueue", 
                        "Vector", "Stack", "EnumSet", "EnumMap", "Iterator"}});
        }

        // Create a regex pattern of specifiers joined by the '|' (or) operator
        // \b pattern \b ensures that 'pattern' is only matched when it is surrounded by non-word characters
        // For example, a type called staticClass, the static in this name will be kept when removing the specifiers
        // Non-word characters is anything outside of [A-Za-z0-9]
        //  For instance, \n or \r or ' ' or beginning or end of strings are considered as non-word characters
        std::string pattern;
        for (const auto& s : mtypes.at(l)) {
            if (pattern.size() > 0) pattern += "|";
            
            // If specifier is one of the special characters (like * or & or []), match them without word boundaries.
            bool isAlphaNumeric = true;
            for (char c : s) {
                if (!std::isalnum(c) && c != '_')
                    isAlphaNumeric = false;
            }

            if (!isAlphaNumeric)
                pattern += s;     
            else 
                pattern += "\\b" + s + "\\b";      
        }
        
        // User-defined modifer types apply to all languages
        for (const auto& s : userMtypes) {
            pattern += "|";
            
            bool isAlphaNumeric = true;
            for (char c : s) {
                if (!std::isalnum(c) && c != '_')
                    isAlphaNumeric = false;
            }

            if (!isAlphaNumeric)
                pattern += s;     
            else 
                pattern += "\\b" + s + "\\b";      
        }

        pattern = "(" + pattern + ")";
        patterns.insert({l, pattern});
    }
}
