// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file return_types.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "return_types.hpp"

// Checks whether a return type, without specifiers and whitespace, is Boolean in the unit language
//
bool isBooleanType(const std::string& language, const std::string& type) {
    if (language == "C++")  return type == "bool" || type == "_Bool"; // C units run as C++, which brings the _Bool of C
    if (language == "C#")   return type == "bool" || type == "Boolean" || type == "System.Boolean";
    if (language == "Java") return type == "boolean" || type == "Boolean" || type == "java.lang.Boolean";
    return false;
}

// Checks whether a return type, without specifiers and whitespace, is void in the unit language
//
bool isVoidType(const std::string& language, const std::string& type) {
    if (language == "C++")                      return type == "void";
    if (language == "C#" || language == "Java") return type == "void" || type == "Void";
    return false;
}

// Checks whether a declared return type, without whitespace, is a pointer to void (C++ and C#)
//
bool isVoidPointerType(const std::string& language, const std::string& declaredType) {
    return language != "Java" && declaredType.find("void*") != std::string::npos;
}

// Checks whether a function returns a value, given its return type without specifiers and whitespace and its declared type
// The specifiers include *, so void* becomes void and only the declared type tells the two apart
//
bool isNonVoidType(const std::string& language, const std::string& type, const std::string& declaredType) {
    return !isVoidType(language, type) || isVoidPointerType(language, declaredType);
}
