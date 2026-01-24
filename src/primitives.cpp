// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file primitives.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "primitives.hpp"
#include "helper_functions.hpp"
#include "specifiers.hpp"

#include <iostream>
#include <vector>
#include <sstream>

extern thread_local helperFunctions   HELPERS;
thread_local extern specifiers        SPECIFIERS;

// Checks if ( dataOrReturnType ) is non-primitive
//
bool primitives::isNonPrimitive(variableModel& variable, const std::string& unitLanguage, const std::string& dataOrReturnTypeParent) {
    std::string dataOrReturnTypeParsed = variable.getType();

    std::size_t listOpen = dataOrReturnTypeParsed.find("<");
    if (listOpen != std::string::npos) {
        std::string left  = dataOrReturnTypeParsed.substr(0, listOpen);
        std::string right = dataOrReturnTypeParsed.substr(listOpen, dataOrReturnTypeParsed.size() - listOpen);

        // removeNamespace() can mess up the string when there is a namespace inside <>
        // For example: Factory <hippodraw::DataRep> --> removeNamespace() --> DataRep>
        // This is why we need to separate them
        HELPERS.removeNamespace(left, unitLanguage, true);

        dataOrReturnTypeParsed = left + right;
    }

    SPECIFIERS.removeSpecifiers(dataOrReturnTypeParsed, unitLanguage);
    HELPERS.removeWhitespace(dataOrReturnTypeParsed);
     
    std::stringstream dataOrReturnTypeParsedStream(dataOrReturnTypeParsed);
    std::string dataOrReturnTypeParsedSnippet;

    while (std::getline(dataOrReturnTypeParsedStream, dataOrReturnTypeParsedSnippet, ',')) {
        HELPERS.removeNamespace(dataOrReturnTypeParsedSnippet, unitLanguage, true);
        HELPERS.removeBracketSuffix(dataOrReturnTypeParsedSnippet);
        
        // getline might return empty strings for types like "List<,>" if not fully cleaned
        if (dataOrReturnTypeParsedSnippet.empty()) continue; 

        if (!isPrimitive(dataOrReturnTypeParsedSnippet, unitLanguage)) {
            variable.setNonPrimitive(true);
            
            // If parent is empty, then ( dataOrReturnTypeParsedSnippet ) is always external (e.g., free function return type)
            if (dataOrReturnTypeParsedSnippet != dataOrReturnTypeParent) {
                variable.setNonPrimitiveExternal(true);
            }
            return true; // Breaks on first non-primitive found
        }
    }
    return false;
}

// Checks if ( type ) is a primitive
// User-defined primitives are checked for all languages
//
bool primitives::isPrimitive(const std::string& type, const std::string& unitLanguage) {
    return (primitivesList.at(unitLanguage).find(type) != primitivesList.at(unitLanguage).end() || userPrimitivesList.find(type) != userPrimitivesList.end());
}

// Reads a set of user-defined primitives
// File should list one primitive per line
//
std::istream& operator>>(std::istream& in, primitives& primitives) {
    std::string utype;
    while(std::getline(in, utype))
        primitives.userPrimitivesList.insert(utype);

    return in;
}

// Outputs the list of primitives to standard error for debugging purposes
//
void primitives::outputPrimitives() {
    std::cerr<<"---Primitives---";
    for (const auto& pair : primitivesList) {
        std::cerr<<"\n[" << pair.first << "]:" ;
        for (const std::string& primitive : pair.second) 
            std::cerr << ' ' << primitive;
    }

    if (userPrimitivesList.size() > 0) {
        std::cerr<<"\n[User-Defined]:";
        for (const std::string& primitive : userPrimitivesList) 
            std::cerr << ' ' << primitive;
    }
    std::cerr << "\n\n";
}

// Generic types (e.g., T), auto (C++), and var (C# and Java) are considered as non-primitive unless added by user
//
void primitives::initializePrimitivesList() {
    primitivesList.insert({"C++", {"short","shortint","int","int8_t","int16_t","int32_t","int64_t","uint8_t","uint16_t","uint32_t","uint64_t","long","longint","longlong","longlongint","float","double","longdouble","char","byte","string","size_type","size_t","wchar_t","char16_t","char32_t","bool","ptrdiff_t","void"}});
    primitivesList.insert({"C#", {"bool","byte","sbyte","char","double","float","int","uint","long","ulong","short","ushort","decimal","string","void","Boolean","Byte","SByte","Char","Double","Single","Int32","UInt32","Int64","UInt64","Int16","IntPtr","UIntPtr","UInt16","Decimal","String","Void"}});
    primitivesList.insert({"Java", {"boolean","byte","char","short","int","long","float","double", "void", "Byte", "Character", "Short", "Integer", "Long", "Float", "Double", "String", "Void"}});
}