// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file PrimitiveTypes.cpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "PrimitiveTypes.hpp"

extern std::vector<std::string> LANGUAGE;

// Checks if 'type' is a primitive
// User-defined primitives are checked for all languages
//
bool primitiveTypes::isPrimitive(const std::string& type, const std::string& unitLanguage) {
    return (ptypes.at(unitLanguage).find(type) != ptypes.at(unitLanguage).end() || 
           userTypes.find(type) != userTypes.end());
}

// Reads a set of user-defined primitive types
// File should list one type per line
//
std::istream& operator>>(std::istream& in, primitiveTypes& primitives) {
    std::string utype;
    
    while(std::getline(in, utype))
        primitives.addPrimitive(utype);

    return in;
}

// Adds "userType" to user-defined primitives if not already present
// Apply to all languages
//
void primitiveTypes::addPrimitive(const std::string& userType) {
    userTypes.insert(userType);
}

void primitiveTypes::outputPrimitives() {
    std::cerr<<"---Primitives---";
    for (const auto& pair : ptypes) {
        std::cerr<<"\n[" << pair.first << "]:" ;
        for (const std::string& primit : pair.second) 
            std::cerr << ' ' << primit;
    }

    if (userTypes.size() > 0) {
        std::cerr<<"\n[User-Defined]:";
        for (const std::string& primit : userTypes) 
            std::cerr << ' ' << primit;
    }
    std::cerr << "\n\n";
}

// Specific primitives are used based on the language of the unit (i.e., source file)
// Generic types (e.g., T), auto (C++), and var (C# and Java) 
//  are considered as non-primitive unless added by user
//
void primitiveTypes::createPrimitiveList() {
    for (const auto& l : LANGUAGE) {
        if (l == "C++") {
            ptypes.insert({l, {
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
            }});
        }
        else if (l == "C#") {  
            ptypes.insert({l, {
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
            }});
        }
        else if (l == "Java") {
            ptypes.insert({l, {
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
            }});
        }
    }
}
