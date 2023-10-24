// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file utils.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */


#include "utils.hpp"

// Removes specifiers from type name.
//
std::string removeSpecifiers(const std::string& type) {
    std::string result = type;

    // List of specifiers (you can expand this list as needed)
    // Some specifiers need to be escaped for both the string and the regex engine as.
    std::vector<std::string> specifiers;
    std::string lang = PRIMITIVES.getLanguage();
    if (lang == "C++"){
        specifiers = { "const", "volatile", "inline", "virtual", "friend", "&", "&&", "\\*",
                       "mutable", "static", "thread_local", "register", "constexpr", "explicit" };
    }
    else if (lang == "C#"){
        specifiers = { "readonly", "ref", "out", "in", "unsafe", "internal", "params",
                       "public", "private", "protected", "static", "virtual", "\\*", "volatile", "\\[.*?\\]",
                       "this",  "override", "abstract",  "extern", "async", "partial", "explicit", "implicit"
                       "new", "sealed", "event", "const" };
    }
    else if (lang == "Java"){
        specifiers = { "public", "private", "protected", "static", "final", "transient",  "\\[.*?\\]",
                       "volatile", "synchronized", "native", "strictfp", "abstract", "default" };
    }

    // Create a regex pattern of specifiers joined by the '|' (or) operator.
    // \bword\b ensures that specifiers are matched only when they appear as 
    //   whole words (except for certain specifiers), not as part of other words.
    //   For example, a class called staticClass, the static in this name will be kept when 
    //   removing the specifiers from the datatypes of its instances.
    std::string pattern = "";
    for (const auto& specifier : specifiers) {
        if (pattern.size() > 0) pattern += "|";
        
        // If specifier is one of the special characters (like * or & or []), match them without word boundaries.
        if (specifier == "\\*" || specifier == "&" || specifier == "\\[.*?\\]") 
            pattern += specifier;     
        else 
            pattern += "\\b" + specifier + "\\b";     
    }

    pattern = "(" + pattern + ")";
    std::regex regexPattern(pattern);

    // Use regex_replace to remove all specifiers from the input string
    result = std::regex_replace(result, regexPattern, "");

    return result;
}

// Remove containers
// For example: std::vector<std::string>, std::map<int, std::string>.
//
std::string removeContainers(const std::string& type){
    std::string pattern= "";
    if (PRIMITIVES.getLanguage() == "C++"){
        pattern = "\\w+(?=::)|::|<|>|vector|list|set|map|unordered_map|array|multimap|signed|unsigned|unordered_multimap|";
        pattern += "forward_list|stack|queue|priority_queue|deque|multiset|unordered_set|unordered_multiset";
    }
    else if (PRIMITIVES.getLanguage() == "C#"){
        pattern = "\\w+(?=\\.)|\\?|<|>|\\.|List|Dictionary|HashSet|Queue|Stack|SortedList|LinkedList|BitArray|";
        pattern += "KeyedCollection|SortedSet|BlockingCollection|ConcurrentQueue|";
        pattern += "ConcurrentStack|ConcurrentDictionary|ConcurrentBag|";
        pattern += "ReadOnlyCollection|ReadOnlyDictionary|Tuple|ValueTuple|NameValueCollection|";
        pattern += "StringCollection|StringDictionary|HybridDictionary|OrderedDictionary";
    }
    else if (PRIMITIVES.getLanguage() == "Java"){
        pattern = "\\w+(?=\\.)|<|>|\\.|List|ArrayList|LinkedList|Set|HashSet|LinkedHashSet|SortedSet|TreeSet|";
        pattern += "Map|HashMap|Hashtable|LinkedHashMap|SortedMap|TreeMap|Deque|ArrayDeque|Queue|";
        pattern += "PriorityQueue|Vector|Stack|EnumSet|EnumMap|Iterator";
    }
    pattern = "(" + pattern + ")";
    std::regex regexPattern(pattern);

    std::string result = std::regex_replace(type, regexPattern, "");
    return result;
}


// Checks if a type is primitive.  
//
bool isPrimitiveType(const std::string& type) {
    std::istringstream ss(type);
    std::string token = "";
    while (std::getline(ss, token, ','))
        if (!PRIMITIVES.isPrimitive(token)) return false;
    return true;
}


// Checks if possibleAttribute is an attribute (C++) or field (C# and Java)
//
bool isAttribute(std::vector<AttributeInfo>& attribute, const std::vector<std::string>& parameterName,
                 const std::vector<std::string>& localVariableName, const std::string& possibleAttribute, bool modified, int& attributeIndex, int& parameterIndex) {
    
    // A local variable or a parameter could overshadow an attribute, or a field, if it has the same name. 
    for (size_t i = 0; i < parameterName.size(); ++i) { 
        if (possibleAttribute == parameterName[i]) {
            parameterIndex = i;
            attributeIndex = -2; // Parameter or local is found
            return false;
        }
    }
    for (size_t i = 0; i < localVariableName.size(); ++i) { 
        if (possibleAttribute == localVariableName[i]) {
            attributeIndex = -2; // Parameter or local is found
            return false;
        }
    }
    for (size_t i = 0; i < attribute.size(); ++i) {
        if (possibleAttribute == attribute[i].getName()){
            if (modified && attribute[i].getModified()) return false;
            if (modified) attribute[i].setModified(true);
            attributeIndex = i;
            return true; 
        } 
    }
    return false; // Not an attribute, or local, or parameter
}



// Removes all whitespace from string
//
std::string trimWhitespace(const std::string& s) {
    std::string result = s;
    result.erase(std::remove_if(result.begin(), result.end(), [](unsigned char c) { return std::isspace(c); }), result.end());
    return result;
}

// Trim blanks of the right of string
//
std::string Rtrim(const std::string& s) {
    std::string result = s;
    size_t lastNonSpace = result.find_last_not_of(' ');
    if (lastNonSpace != std::string::npos) {
        return s.substr(0, lastNonSpace + 1);
    }
    return result;
}

// Removes namespaces from method names
// For example, given:
//  namespaceA::NamespaceB::ClassName::MethodName
//  becomes:
//  ClassName::MethodName
//
std::string removeNamespace(const std::string& methodName) {
    std::string result = methodName;

    // Find the last occurrence of '::' to extract the method name and its preceding parts
    size_t lastDoubleColon = result.rfind("::");
    if (lastDoubleColon != std::string::npos) {
        // Find the second last occurrence of '::' to extract the class name and the remaining part
        size_t secondLastDoubleColon = result.rfind("::", lastDoubleColon - 1);
        if (secondLastDoubleColon != std::string::npos) {
            result = result.substr(secondLastDoubleColon + 2);  // Skip '::'
        } 
    }
    return result;
}

// Converts all whitespace to blanks  ('\r' => ' ')
std::string WStoBlank(const std::string& s) {
    std::string result(s);
    std::replace_if(result.begin(),
                    result.end(),
                    [](char c) { return isspace(c); },
                    ' ');
    return result;
}
