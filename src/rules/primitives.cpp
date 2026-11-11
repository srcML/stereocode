// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file primitives.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "rules/primitives.hpp"
#include "rules/specifiers.hpp"

#include <iostream>

// Checks if a variable has a non-primitive type, which is external unless it is the type of the variable's own class
// The first name in the type that is no primitive decides, where a template keeps the name before its arguments, such as Node for Node<T>
//
bool primitives::isNonPrimitive(variableModel& variable, const std::string& unitLanguage, const std::string& dataOrReturnTypeParent,
                                const specifiers& typeSpecifiers) const {
    for (const std::string& name : typeSpecifiers.typeNames(variable.getType(), unitLanguage)) {
        if (isPrimitive(name, unitLanguage)) continue;

        variable.setNonPrimitive(true);

        // If parent is empty, then the name is always external (e.g., free function return type)
        if (name != dataOrReturnTypeParent) variable.setNonPrimitiveExternal(true);
        return true;
    }
    return false;
}

// Checks if ( type ) is a primitive
// A user-defined primitive holds for all languages or for the language of its section
//
bool primitives::isPrimitive(const std::string& type, const std::string& unitLanguage) const {
    return (primitivesList.at(unitLanguage).find(type) != primitivesList.at(unitLanguage).end() || userPrimitivesList.contains(type, unitLanguage));
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
    primitivesList.insert({"C#", {"bool","byte","sbyte","char","double","float","int","uint","long","ulong","short","ushort","decimal","string","void","Boolean","Byte","SByte","Char","Double","Single","Int32","UInt32","Int64","UInt64","Int16","IntPtr","UIntPtr","UInt16","Decimal","String","Void"}});
    primitivesList.insert({"Java", {"boolean","byte","char","short","int","long","float","double", "void", "Boolean", "Byte", "Character", "Short", "Integer", "Long", "Float", "Double", "String", "Void"}});
}