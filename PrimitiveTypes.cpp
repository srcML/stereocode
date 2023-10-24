// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file PrimitiveTypes.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */


#include "PrimitiveTypes.hpp"

// RETVAL == true if s == (ptypes[i] or usertypes[i])
//
bool primitiveTypes::isPrimitive(const std::string& s) const {
    return (ptypes.find(s) != ptypes.end()) || (usertypes.find(s) != usertypes.end());
}

// Adds s to user specified primitives if not already present
//
void primitiveTypes::addPrimitive(const std::string& s) {
    if (!isPrimitive(s)) usertypes.insert(s);
}

// Reads a set of user defined primitive types to be added to a list
//
// REQUIRES: in.open(fname)
//  fname will list each type name one per line
//  No spaces in type name if compound long int => longint
//
std::istream& operator>>(std::istream& in, primitiveTypes& prims)  {
    std::string name;
    while(std::getline(in, name)) prims.addPrimitive(name);
    return in;
}

//Initially ptypes is empty or has user defined types.
//
// After language is determined then language specific primitive
//  types are added.
//
void primitiveTypes::setLanguage(const std::string& lang) {
    if (language == lang) return;        //Same language (done)
    language = lang;

    if (language == "C++") {
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
            "ptrdiff_t"
        };
    }
    if (language == "C#") {  
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
            "Boolean", // Same as bool, but used as System.Boolean
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
    if (language == "Java") {
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
