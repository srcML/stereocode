// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file utils.cpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "utils.hpp"

std::unordered_map<std::string, std::string> specifierPattern; // Used to remove specifiers
std::unordered_map<std::string, std::string> containerPattern; // Used to remove containers
extern primitiveTypes                        PRIMITIVES;   
extern std::vector<std::string>              LANGUAGE;

bool isNonPrimitiveType(const std::string& type, bool& externalNonPrimitive, 
                        const std::string& unitLanguage, const std::string& className) {
    std::string typeParsed = type;
    removeSpecifiers(typeParsed, unitLanguage); // Can take full type as is
    size_t listOpen = typeParsed.find("<");
    if (listOpen != std::string::npos) {
        std::string typeLeft = typeParsed.substr(0, listOpen);
        std::string typeRight = typeParsed.substr(listOpen, typeParsed.size() - listOpen);
        // removeNamespace() can mess up the string when there is a namespace inside <>
        // For example: Factory <hippodraw::DataRep> --> removeNamespace() --> DataRep>
        // This is why we need to separate them
        //
        removeNamespace(typeLeft, true, unitLanguage); // No generics and no comma separated list
        typeParsed = typeLeft + typeRight;
    }

    bool isNonPrimitive = false; 
    trimWhitespace(typeParsed);  // Can take full type as is
    removeContainers(typeParsed, unitLanguage); // Can take full type as is. Needs type without spaces first.
    
    size_t start = 0;
    size_t end = typeParsed.find(",");
    std::string subType;
    while (end != std::string::npos) {
        subType = typeParsed.substr(start, end - start);   
        removeNamespace(subType, true, unitLanguage); 

        if (!isPrimitiveType(subType)) {
            isNonPrimitive = true;
            if (subType.find(className) == std::string::npos)
                externalNonPrimitive = true;
        }
        
        start = end + 1;
        end = typeParsed.find(",", start);
    }

    subType = typeParsed.substr(start, typeParsed.size() - start);
    removeNamespace(subType, true, unitLanguage);
    if (!isPrimitiveType(subType)) {
        isNonPrimitive = true;
        
        if (subType.find(className) == std::string::npos)
            externalNonPrimitive = true;
    }

    return isNonPrimitive;
}

// Checks if a type is primitive.  
//
bool isPrimitiveType(const std::string& type) {
    std::istringstream subType(type);
    std::string token;
    while (std::getline(subType, token, ','))
        if (!PRIMITIVES.isPrimitive(token)) return false;
    return true;
}

// Creates list of specifiers
//
void createSpecifierList() {
    std::vector<std::string> specifier;
    std::string container;
    for (const auto& l : LANGUAGE) {
        if (l == "C++") {
            // \\[.*\\] match any square brackets [] with any single character inside
            //   (if any) . with zero or more occurrence * (including empty)
            //   such as int[] or int[,] and so on
            specifier = { "const", "volatile", "inline", "virtual", "friend", "extern", "&", "&&", "\\*", "public", "private", "protected",
                          "mutable", "static", "thread_local", "register", "constexpr", "explicit", "signed", "unsigned" };

            container =  "<|>|vector|list|set|map|unordered_map|array|multimap|unordered_multimap|::iterator|::_iterator|";
            container += "forward_list|stack|queue|priority_queue|deque|multiset|unordered_set|unordered_multiset|pair";
        }
        else if (l == "C#") {
            specifier = { "readonly", "ref", "out", "in", "unsafe", "internal", "params",
                          "public", "private", "protected", "static", "virtual", "\\*", "volatile", "\\[.*\\]",
                          "this",  "override", "abstract",  "extern", "async", "partial", "explicit", "implicit"
                          "new", "sealed", "event", "const", "\\?" };

            container =  "<|>|List|Dictionary|HashSet|Queue|Stack|SortedList|LinkedList|BitArray|";
            container += "KeyedCollection|SortedSet|BlockingCollection|ConcurrentQueue|";
            container += "ConcurrentStack|ConcurrentDictionary|ConcurrentBag|";
            container += "ReadOnlyCollection|ReadOnlyDictionary|Tuple|ValueTuple|NameValueCollection|";
            container += "StringCollection|StringDictionary|HybridDictionary|OrderedDictionary";
        }
        else if (l == "Java") {
            specifier = { "public", "private", "protected", "static", "final", "transient",  "\\[.*\\]", "\\?", "@\\w+",
                          "volatile", "synchronized", "native", "strictfp", "abstract", "default", "super", "extends", "\\.\\.\\." };

            container =  "<|>|List|ArrayList|LinkedList|Set|HashSet|LinkedHashSet|SortedSet|TreeSet|";
            container += "Map|HashMap|Hashtable|LinkedHashMap|SortedMap|TreeMap|Deque|ArrayDeque|Queue|";
            container += "PriorityQueue|Vector|Stack|EnumSet|EnumMap|Iterator";
        }

        // Create a regex pattern of specifiers joined by the '|' (or) operator.
        // \bword\b ensures that specifiers are matched only when they appear as 
        //   whole words (except for certain specifiers), that is, words between non-word characters.
        //   For example, a class called staticClass, the static in this name will be kept when 
        //   removing the specifiers from the datatypes of its instances.
        std::string pattern;
        for (const auto& s : specifier) {
            if (pattern.size() > 0) pattern += "|";
            
            // If specifier is one of the special characters (like * or & or []), match them without word boundaries.
            if (s == "\\*" || s == "&" || s == "&&" || s == "\\[.*\\]" || s == "\\?" || s == "\\.\\.\\." || "@\\w+") 
                pattern += s;     
            else 
                pattern += "\\b::" + s + "\\b";      
        }
        pattern = "(" + pattern + ")";
        specifierPattern.insert({l, pattern});

        containerPattern.insert({l, container});
    }
}

// Removes specifiers from type name
//
void removeSpecifiers(std::string& type, std::string unitLanguage) {
    std::regex regexPattern(specifierPattern[unitLanguage]);
    type = std::regex_replace(type, regexPattern, "");
}

// Remove containers from type name
//
void removeContainers(std::string& type, std::string unitLanguage){
    std::regex regexPattern(containerPattern[unitLanguage]);
    type = std::regex_replace(type, regexPattern, "");
}

// Removes all whitespace from string
//
void trimWhitespace(std::string& s) {
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); }), s.end());
}

// Trim blanks of the right of string
//
void Rtrim(std::string& s) {
    size_t lastNonSpace = s.find_last_not_of(' ');
    if (lastNonSpace != std::string::npos)
        s = s.substr(0, lastNonSpace + 1);   
}

// Removes namespaces from names
// all = false keeps the last :: or .
//
void removeNamespace(std::string& name, bool all, std::string_view unitLanguage) {
    size_t last, secondLast;
    if (unitLanguage == "C++") last = name.rfind("::");
    else last = name.rfind(".");
    if (last != std::string::npos) {
        if (all) {
            if (unitLanguage == "C++") name = name.substr(last + 2);
            else name = name.substr(last + 1);
        }
        else {
            if (unitLanguage == "C++") secondLast = name.rfind("::", last - 1);
            else secondLast = name.rfind(".", last - 1);
            if (secondLast != std::string::npos) {
                if (unitLanguage == "C++") name = name.substr(secondLast + 2); 
                else name = name.substr(secondLast + 1); 
            }                      
        }
    }
}

// Converts all whitespaces to blanks  ('\r' => ' ')
void WStoBlank(std::string& s) {
    std::replace_if(s.begin(),
                    s.end(),
                    [](char c) { return isspace(c); },
                    ' ');
}

// Removes all characters inside <> or () except for comma
// For example, myObject<int, std::pair<int, int>> becomes myObject<,>
//  and Foo(int, std::pair<int, int>, double) becomes Foo(,,)
//
void removeBetweenComma(std::string& s, bool isGeneric) {
    size_t opening;
    if (isGeneric)
      opening = s.find("<");
    else
      opening = s.find("(");

    if (opening != std::string::npos) {
        std::string name = s.substr(0, opening + 1);
        s = s.substr(opening + 1);
        
        // This could be nested inside () or <> for types
        // <[^>]*> --> starts at <, then matches everything except > and stops at > including the >
        std::string pattern = R"(<[^>]*>)";  
        std::regex regexPattern(pattern);
        s = std::regex_replace(s, regexPattern, "");  
        
        pattern = "";
        if (isGeneric)
          pattern = R"(([^,]*)(,|>))";
        else
          pattern = R"(([^,]*)(,|\)))";
        
        const std::regex regexPatternTwo (pattern);
        s = std::regex_replace(s, regexPatternTwo, "$2"); // $2 is used to replace the content with the second group
        s = name + s;
    }
}