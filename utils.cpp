// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file utils.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "utils.hpp"

// Checks if file is header file using its extension
//
bool isHeaderFile(const std::string& fname) {
    for (const std::string& ext : HEADER_FILE_EXTENSION)
        if (fname.find(ext) != std::string::npos) return true;
    return false;
}

// Checks if method is const
//
bool checkConst(const std::string& srcml) {
    size_t end = srcml.find("{");
    std::string functionSrcml = trimWhitespace(srcml.substr(0, end));
    if (functionSrcml.find("<specifier>const</specifier><block>") != std::string::npos) return true;
    else if (functionSrcml.find("</parameter_list><specifier>const</specifier>") != std::string::npos) return true; // Useful for methods such as void foo() const try {}
    return false;
}

// Checks if a name could be an inherited attribute
//
// REQUIRES: parentClass.size() > 0 - the class inherits from another class
//
bool isInheritedAttribute(const std::vector<std::string>& parameter_names,
                                      const std::vector<std::string>& local_var_names,
                                      const std::string& expr) {
    bool is_inherited = true;
    
    // Checks for literal return expression
    if (expr == "#") is_inherited = false;
    if (expr == "true" || expr == "false" || expr == "TRUE" || expr == "FALSE") is_inherited = false;

    // expr is a keyword that is not a attribute
    if (expr == "this" || expr == "cout" || expr == "endl") is_inherited = false;

    for (int k = 0; k < expr.size(); ++k) {  // expr is not inherited if it contains an operator
        if (expr[k] == '+' || expr[k] == '-' || expr[k] == '*' || expr[k] == '/'
            || expr[k] == '%' || expr[k] == '(' || expr[k] == '!' || expr[k] == '&'
            || expr[k] == '|' || expr[k] == '=' || expr[k] == '>' || expr[k] == '<'
            || expr[k] == '.' || expr[k] == '?' || expr[k] == ':' || expr[k] == '"') {
            is_inherited = false;
        }
    }

    return is_inherited;
}

// Checks if a primitive type
//  Examples: int, bool, char, double, cont int, inline int
//            vector<int>, map<int,int>, int[], int*, int&
//
bool isPrimitiveContainer(const std::string& str) {
    std::string s = removeSpecifiers(str);

    std::vector<std::string> containers = {"std::vector", "std::list", "std::set", "std::map", 
                                           "std::unordered_map", "std::forward_list", "std::array", "std::stack", 
                                           "std::queue", "vector", "list", "set", "map", "unordered_map", "array",
                                           "forward_list", "stack", "queue"};
    for (const std::string& container : containers) {
        size_t pos = s.find(container);
        if (pos != std::string::npos) {
            s.erase(pos, container.size());
        }
    }

    std::replace_if(s.begin(), s.end(), [](char c) { return c == '<' || c == '>'; }, ' ');
    s = trimWhitespace(s);
    std::replace_if(s.begin(), s.end(), [](char c) { return c == ','; }, ' ');

    if (s.find(" ") != std::string::npos) {  // Multiple type names
        size_t start = 0;
        size_t end = s.find(" ");
        while (end != std::string::npos) {
            if (!PRIMITIVES.isPrimitive(s.substr(start, end - start))) return false;
            start = end + 1;
            end = s.find(" ", start);
        }
        return PRIMITIVES.isPrimitive(s.substr(start, s.size() - start));
    }
    return PRIMITIVES.isPrimitive(s);  // One type name
}

// Checks if possible_Attr is an attribute
//
bool isAttribute(const std::vector<AttributeInfo>& attribute, const std::vector<std::string>& parameter_names,
                 const std::vector<std::string>& local_var_names, const std::string& possible_attr) {
    std::string name = trimWhitespace(possible_attr);
    size_t left_sq_bracket = name.find("[");    // remove [] if the name is an array
    if (left_sq_bracket != std::string::npos) {
        name = name.substr(0, left_sq_bracket);
    }
    for (int i = 0; i < parameter_names.size(); ++i) { 
        if (name == parameter_names[i]) return false;
    }
    for (int i = 0; i < local_var_names.size(); ++i) { 
        if (name == local_var_names[i]) return false;
    }
    for (int i = 0; i < attribute.size(); ++i) {
        if (name == attribute[i].getName()) return true; 
    }
    return false;
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
    std::string result;
    result.reserve(s.size()); // Preallocate memory for efficiency
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
    std::string result;
    result.reserve(s.size()); // Preallocate memory for efficiency
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
    std::string result;
    result.reserve(s.size()); // Preallocate memory for efficiency
    for (const char& c : s) {
        result += isspace(c) ? ' ' : c;
    }
    return result;
}