// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file utils.cpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "utils.hpp"

// Checks if a type is primitive.  
//
bool isPrimitiveType(const std::string& type) {
    std::istringstream subType(type);
    std::string token = "";
    while (std::getline(subType, token, ','))
        if (!PRIMITIVES.isPrimitive(token)) return false;
    return true;
}


// Removes specifiers from type name.
//
void removeSpecifiers(std::string& type, std::string_view unitLanguage) {
    // List of specifiers (you can expand this list as needed)
    // Some specifiers need to be escaped for both the string and the regex expression
    //  For example, \\* escapes \* in C++ which in turn escapes * for the regex 
    // \\[.*\\] match any square brackets [] with any single character inside (if any) . with zero or more occurrence * (including empty)
    //   such as int[] or int[,] and so on
    std::vector<std::string> specifiers;
    if (unitLanguage == "C++") {
        specifiers = { "const", "volatile", "inline", "virtual", "friend", "&", "&&", "\\*",
                       "mutable", "static", "thread_local", "register", "constexpr", "explicit" };
    }
    else if (unitLanguage == "C#") {
        specifiers = { "readonly", "ref", "out", "in", "unsafe", "internal", "params",
                       "public", "private", "protected", "static", "virtual", "\\*", "volatile", "\\[.*\\]",
                       "this",  "override", "abstract",  "extern", "async", "partial", "explicit", "implicit"
                       "new", "sealed", "event", "const", "\\?" };
    }
    else if (unitLanguage == "Java") {
        specifiers = { "public", "private", "protected", "static", "final", "transient",  "\\[.*\\]",
                       "volatile", "synchronized", "native", "strictfp", "abstract", "default", "\\?" };
    }

    // Create a regex pattern of specifiers joined by the '|' (or) operator.
    // \bword\b ensures that specifiers are matched only when they appear as 
    //   whole words (except for certain specifiers), and not as part of other words.
    //   For example, a class called staticClass, the static in this name will be kept when 
    //   removing the specifiers from the datatypes of its instances.
    std::string pattern = "";
    for (const auto& specifier : specifiers) {
        if (pattern.size() > 0) pattern += "|";
        
        // If specifier is one of the special characters (like * or & or []), match them without word boundaries.
        if (specifier == "\\*" || specifier == "&" || specifier == "\\[.*\\]" || specifier == "\\?") 
            pattern += specifier;     
        else 
            pattern += "\\b" + specifier + "\\b";     
    }

    pattern = "(" + pattern + ")";
    std::regex regexPattern(pattern);

    // Use regex_replace to remove all specifiers from the input string
    type = std::regex_replace(type, regexPattern, "");
}

// Remove containers
// For example: std::vector<std::string>, std::map<int, std::string>.
//
void removeContainers(std::string& type, std::string_view unitLanguage){
    std::string pattern= "";
    // .*:: match any single character . with zero or more occurrence * (including empty)
    //  until :: is found (e.g., abc::def::ghi.jkl::mno will match abc::def:: 
    // first leaving ghi.jkl::mno, then ghi.jkl:: is matched leaving only mno)
    //
    if (unitLanguage == "C++") {
        pattern = ".*::|<|>|vector|list|set|map|unordered_map|array|multimap|signed|unsigned|unordered_multimap|";
        pattern += "forward_list|stack|queue|priority_queue|deque|multiset|unordered_set|unordered_multiset|pair";
    }
    else if (unitLanguage == "C#") {
        pattern = ".*\\.|\\?|<|>|List|Dictionary|HashSet|Queue|Stack|SortedList|LinkedList|BitArray|";
        pattern += "KeyedCollection|SortedSet|BlockingCollection|ConcurrentQueue|";
        pattern += "ConcurrentStack|ConcurrentDictionary|ConcurrentBag|";
        pattern += "ReadOnlyCollection|ReadOnlyDictionary|Tuple|ValueTuple|NameValueCollection|";
        pattern += "StringCollection|StringDictionary|HybridDictionary|OrderedDictionary";
    }
    else if (unitLanguage == "Java") {
        pattern = ".*\\.|<|>|List|ArrayList|LinkedList|Set|HashSet|LinkedHashSet|SortedSet|TreeSet|";
        pattern += "Map|HashMap|Hashtable|LinkedHashMap|SortedMap|TreeMap|Deque|ArrayDeque|Queue|";
        pattern += "PriorityQueue|Vector|Stack|EnumSet|EnumMap|Iterator";
    }
    pattern = "(" + pattern + ")";
    std::regex regexPattern(pattern);

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

// Removes namespaces from method names
// all = false (C++ only) keeps the :: for the class name
//
void removeNamespace(std::string& methodName, bool all, std::string_view lang) {
    size_t last, secondLast;
    if (lang == "C++") last = methodName.rfind("::");
    else last = methodName.rfind(".");
    if (last != std::string::npos) {
        if (all) {
            if (lang == "C++") methodName = methodName.substr(last + 2);
            else methodName = methodName.substr(last + 1);
        }
        else if (!all && lang == "C++") {
            secondLast = methodName.rfind("::", last - 1);
            if (secondLast != std::string::npos) 
                methodName = methodName.substr(secondLast + 2);           
        }
    }
}

// Converts all whitespace to blanks  ('\r' => ' ')
void WStoBlank(std::string& s) {
    std::replace_if(s.begin(),
                    s.end(),
                    [](char c) { return isspace(c); },
                    ' ');
}

// Replaces , with |;
//
void commaConversion(std::string& s) {
    std::replace_if(s.begin(),
                    s.end(),
                    [](char c) { return c == ','; },
                    '|');
}

// Removes all characters inside <> except for ,
// myObject<int,double> becomes myObject<,>
//
void removeBetweenComma(std::string& s) {
    size_t genericOpening = s.find("<");
    if (genericOpening != std::string::npos) {
        std::string name = s.substr(0, s.find("<") + 1);
        s = s.substr(s.find("<") + 1);
        // match any character until , is encountered or until end of string is reached
        const std::regex pattern("([^,]*)(,|$)");
        s = std::regex_replace(s, pattern, "$2");
        s = name + s + ">";
    }
}
