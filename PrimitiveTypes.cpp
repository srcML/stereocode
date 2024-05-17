// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file PrimitiveTypes.cpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "PrimitiveTypes.hpp"

// Checks if "type" is a primitive
//
bool primitiveTypes::isPrimitive(const std::string& type) const {
    return (ptypes.find(type) != ptypes.end()) || (userTypes.find(type) != userTypes.end());
}

// Adds "type" to user-defined primitives if not already present
//
void primitiveTypes::addPrimitive(const std::string& type) {
    if (!isPrimitive(type)) userTypes.insert(type);
}

// Reads a set of user-defined primitive types
// File should list one type per line
//
std::istream& operator>>(std::istream& in, primitiveTypes& primitives) {
    std::string name;
    while(std::getline(in, name)) {
        trimWhitespace(name);
        primitives.addPrimitive(name);
    }
    return in;
}

// Specific primitives are used based on unit language
// Generic types (e.g., T), auto (C++), and var (C# and Java) 
//  are considered as non-primitive unless added by user
//
void primitiveTypes::setLanguage(const std::string& unitLang) {
    if (unitLanguage == unitLang) return;
    
    unitLanguage = unitLang;

    if (unitLanguage == "C++") {
        ptypes = {
            "short",
            "shortint",
            "int",
            "int8_t",
            "int16_t",
            "int32_t",
            "int64_t",
            "uint8_t",
            "uint16_t",
            "uint32_t",
            "uint64_t",
            "long",
            "longint",
            "longlong",
            "longlongint",
            "float",
            "double",
            "longdouble",
            "char",
            "byte",
            "string",
            "size_type",
            "size_t",
            "wchar_t",
            "char16_t",
            "char32_t",
            "bool",
            "ptrdiff_t",
            "void"
        };
    }
    else if (unitLanguage == "C#") {  
        ptypes = {
            "bool",
            "byte",
            "sbyte",
            "char",
            "double",
            "float",
            "int",
            "uint",
            "long",
            "ulong",
            "short",
            "ushort",
            "decimal",
            "string",
            "void",
            "Boolean",
            "Byte",
            "SByte",
            "Char",
            "Double",
            "Single",
            "Int32",
            "UInt32",
            "Int64",
            "UInt64",
            "Int16",
            "IntPtr",
            "UIntPtr",
            "UInt16",
            "Decimal",
            "String",
            "Void"
        };
    }
    else if (unitLanguage == "Java") {
        ptypes = {
            "boolean",
            "byte",
            "char",
            "short",
            "int",
            "long",
            "float",
            "double",
            "void",
            "Byte",
            "Character",
            "Short",
            "Integer",
            "Long",
            "Float",
            "Double",
            "String",
            "Void"
        };
    }
}
