// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file primitives.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "rules/primitives.hpp"
#include "helper_functions.hpp"
#include "rules/specifiers.hpp"

#include <iostream>

namespace {
    // Tells whether a written name of a type names a type, where each of the two names may carry the names that qualify it: one name ends
    // with the other after a qualifier, as Request and http::Request do, unlike wire::Request and http::Request
    //
    bool namesType(const std::string& written, const std::string& type) {
        const std::string& longer  = written.size() >= type.size() ? written : type;
        const std::string& shorter = written.size() >= type.size() ? type : written;
        if (shorter.empty() || longer.compare(longer.size() - shorter.size(), shorter.size(), shorter) != 0) return false;
        if (longer.size() == shorter.size()) return true;
        const char before = longer[longer.size() - shorter.size() - 1];
        return before == '.' || before == ':';
    }
}

// Checks if a variable has a non-primitive type, which is external unless it is the type of the variable's own class
// The first name in the type that is no primitive decides, where a template keeps the name before its arguments, such as Node for Node<T>
// The own class is given by its name, with the namespaces and the types around it where they are known, or by nothing for a free function
//
bool primitives::isNonPrimitive(variableModel& variable, const std::string& unitLanguage, const std::string& ownType,
                                const specifiers& typeSpecifiers) const {
    for (const std::string& written : typeSpecifiers.typeNames(variable.getType(), unitLanguage, true)) {
        std::string name = written;
        HELPERS::removeNamespace(name, unitLanguage, true);
        if (isPrimitive(name, unitLanguage)) continue;

        variable.setNonPrimitive(true);
        if (!namesType(written, ownType)) variable.setNonPrimitiveExternal(true);
        return true;
    }
    return false;
}

// Checks if ( type ) is a primitive
// A user-defined primitive holds for all languages or for the language of its section
//
bool primitives::isPrimitive(const std::string& type, const std::string& unitLanguage) const {
    const auto defaults = primitivesList.find(unitLanguage);
    return (defaults != primitivesList.end() && defaults->second.count(type) > 0) || userPrimitivesList.contains(type, unitLanguage);
}

// Reads a set of user-defined primitives
// File should list one primitive per line
//
std::istream& operator>>(std::istream& in, primitives& primitives) {
    return in >> primitives.userPrimitivesList;
}

// Outputs the list of primitives to standard error for debugging purposes
//
void primitives::outputPrimitives() const {
    std::cerr<<"---Primitives---";
    for (const auto& pair : primitivesList) {
        std::cerr<<"\n[" << pair.first << "]:" ;
        for (const std::string& primitive : pair.second) 
            std::cerr << ' ' << primitive;
    }

    userPrimitivesList.print(std::cerr);
    std::cerr << "\n\n";
}

// Generic types (e.g., T), auto (C++), and var (C# and Java) are considered as non-primitive unless added by user
//
void primitives::initializePrimitivesList() {
    primitivesList.insert({"C++", {"short","shortint","int","int8_t","int16_t","int32_t","int64_t","uint8_t","uint16_t","uint32_t","uint64_t","long","longint","longlong","longlongint","float","double","longdouble","char","byte","string","size_type","size_t","wchar_t","char8_t","char16_t","char32_t","bool","ptrdiff_t","void",
                                   "wstring","u8string","u16string","u32string","string_view","wstring_view"}});
    primitivesList.insert({"C#", {"bool","byte","sbyte","char","double","float","int","uint","long","ulong","short","ushort","decimal","string","void","Boolean","Byte","SByte","Char","Double","Single","Int32","UInt32","Int64","UInt64","Int16","IntPtr","UIntPtr","UInt16","Decimal","String","Void",
                                  "nint","nuint","Int128","UInt128","Half"}});
    primitivesList.insert({"Java", {"boolean","byte","char","short","int","long","float","double", "void", "Boolean", "Byte", "Character", "Short", "Integer", "Long", "Float", "Double", "String", "Void"}});
    // The value of a Literal is no type, and a name without an annotation has no type, which counts as primitive
    primitivesList.insert({"Python", {"int", "float", "complex", "bool", "str", "bytes", "bytearray", "None", "NoneType", "Any", "object", "Literal"}});
    // A variable without an annotation has no type, which counts as primitive, and a function type, which the model writes as Function, holds code
    primitivesList.insert({"JavaScript", {"number", "string", "boolean", "bigint", "symbol", "void", "undefined", "null", "never", "any", "unknown", "object",
                                          "Number", "String", "Boolean", "BigInt", "Symbol", "Function"}});
}