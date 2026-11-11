// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file written_type.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/written_type.hpp"

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
