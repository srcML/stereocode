// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file utils.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "utils.hpp"

// Checks if method is const
//
bool checkConst(const std::string& srcml) {
    size_t end = srcml.find("{");
    std::string functionSrcml = trimWhitespace(srcml.substr(0, end));
    if (functionSrcml.find("<specifier>const</specifier><block>") != std::string::npos) return true;
    else if (functionSrcml.find("</parameter_list><specifier>const</specifier>") != std::string::npos) return true; // Useful for methods such as void foo() const try {}
    return false;
}

// Checks if a primitive type
//  Examples: int, bool, char, double, cont int, inline int
//            vector<int>, map<int,int>, int[], int*, int&
//
bool isPrimitiveContainer(const std::string& str) {
    std::string s = removeSpecifiers(str);

    std::vector<std::string> containers = {"std::vector", "std::list", "std::set", "std::map", "std::unordered_multimap", "std::unordered_multiset"
                                           "std::unordered_map", "std::forward_list", "std::array", "std::stack", 
                                           "std::queue", "std::priority_queue", "std::deque", "std::multiset", "std::unordered_set"
                                           "std::multimap", "vector", "list", "set", "map", "unordered_map", "array", "multimap", "unordered_multimap"
                                           "forward_list", "stack", "queue", "priority_queue", "deque", "multiset", "unordered_set", "unordered_multiset"};
    for (const std::string& container : containers) {
        size_t pos = s.find(container);
        if (pos != std::string::npos) {
            s.erase(pos, container.size());
        }
    }

    std::replace_if(s.begin(), s.end(), [](char c) { return c == '<' || c == '>'; }, ' ');
    s = trimWhitespace(s);

    if (s.find(",") != std::string::npos) {  // Multiple type names
        size_t start = 0;
        size_t end = s.find(",");
        while (end != std::string::npos) {
            if (!PRIMITIVES.isPrimitive(s.substr(start, end - start))) return false;
            start = end + 1;
            end = s.find(",", start);
        }
        return PRIMITIVES.isPrimitive(s.substr(start, end - start));
    }
    return PRIMITIVES.isPrimitive(s);  // One type name
}

// Checks if possibleAttribute is an attribute
//
bool isAttribute(std::vector<AttributeInfo>& attribute, const std::vector<std::string>& parameterName,
                 const std::vector<std::string>& localVariableName, const std::string& possibleAttribute, bool modified, int& attributeIndex, int& parameterIndex) {
    
    // A local variable or a parameter could overshadow an attribute if it has the same name. 
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

// Check if an attribute already exists in the list of attributes
//
bool isDuplicateAttribute (const std::vector<AttributeInfo>& attribute, const std::string& s){
    for (const auto& attr: attribute){
        if(attr.getName() == s) return true;
    }
    return false;
}

// Removes specifiers from type name
//
std::string removeSpecifiers(const std::string& type) {
    std::string result = type;
    std::vector<std::string> specifiers = {"static", "mutable", "inline", "virtual", "const", "friend", "*", "&&", "&", "[]"};
    
    for (const std::string& specifier : specifiers) {
        size_t pos = result.find(specifier);
        if (pos != std::string::npos) {
            result.erase(pos, specifier.size());
        }
    }

    return result;
}

// Removes WS, specifiers, *, & from type name
//
std::string separateTypeName(const std::string& type) {
    std::string result = trimWhitespace(type);
    result = removeSpecifiers(result);
    return result;
}

// Removes all whitespace from string
//
std::string trimWhitespace(const std::string& s) {
    std::string result = "";
    for (const char& c : s) {
        if (!isspace(c)) result += c;    
    }
    return result;
}

// Trim blanks off the left of string
//
std::string Ltrim(const std::string& s) {
    std::string result = s;
    size_t firstNonSpace = result.find_first_not_of(' ');
    if (firstNonSpace != std::string::npos) {
        return s.substr(firstNonSpace);
    }
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

// Normalize multiple blanks to one blank
//
std::string multiBlanksToBlank(const std::string& s) {
    std::string result = "";
    bool prevSpace = false;
    for (const char& c : s) {
        if (isspace(c)) {
            if (!prevSpace) {
                result += ' ';
                prevSpace = true;
            }
        } 
        else {
            result += c;
            prevSpace = false;
        }
    }
    return result;
}

// Converts all whitespace to blanks
//
std::string WStoBlank(const std::string& s) {
    std::string result = "";
    for (const char& c : s) {
        result += isspace(c) ? ' ' : c;
    }
    return result;
}


// Removes namespaces from method names
//
std::string removeNamespace(const std::string& methodName) {
    std::string result = methodName;

    // Find the last occurrence of '::' to extract the method name and its preceding parts
    size_t lastDoubleColon = methodName.rfind("::");
    if (lastDoubleColon != std::string::npos) {
        // Find the second last occurrence of '::' to extract the class name and the remaining part
        size_t secondLastDoubleColon = methodName.rfind("::", lastDoubleColon - 1);
        if (secondLastDoubleColon != std::string::npos) {
            result = methodName.substr(secondLastDoubleColon + 2);  // Skip '::'
        } 
        else {
            result = methodName ;  // If '::' not found, return the entire input
        }
    }
    return result;
}