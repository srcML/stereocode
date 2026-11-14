// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file written_type.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/written_type.hpp"

#include <cctype>

// Adds text to the end of a type, as a part of its own after a name and otherwise to the text of the last part
//
void addText(writtenType& type, std::string_view text) {
    if (text.empty()) return;
    if (type.empty() || type.back().isName) type.push_back({std::string(text), false});
    else type.back().text += text;
}

// Gets the text of a type, its parts joined, such as constCount& of const Count&
//
std::string typeText(const writtenType& type) {
    std::string text;
    for (const typePart& part : type) text += part.text;
    return text;
}

// Gets the text of the types of a parameter list, such as (constCount&,int), which tells overloads apart
//
std::string typeListText(const std::vector<writtenType>& types) {
    std::string text = "(";
    for (const writtenType& type : types) {
        if (text.size() > 1) text += ',';
        text += typeText(type);
    }
    return text + ")";
}

// Gets the types of a parameter list without the names that qualify them, such as (string&,int) of (std::string&,int) and (List<String>)
// of (java.util.List<java.lang.String>), since a declaration and a definition may qualify a type apart, as after using namespace std;
//
std::string withoutQualifiers(const std::string& types) {
    const auto inName = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; };
    std::string bare;
    std::size_t nameStart = 0;
    for (std::size_t i = 0; i < types.size(); ++i) {
        // A dot qualifies between two names, unlike the dots of a variadic parameter, as of String...
        const bool scope = types.compare(i, 2, "::") == 0;
        const bool dot   = types[i] == '.' && i > 0 && inName(types[i - 1]) && i + 1 < types.size() && inName(types[i + 1]) &&
                           !std::isdigit(static_cast<unsigned char>(types[i + 1]));
        if (scope || dot) {
            bare.erase(nameStart);
            if (scope) ++i;
            continue;
        }
        if (!inName(types[i])) nameStart = bare.size() + 1;
        bare += types[i];
    }
    return bare;
}
