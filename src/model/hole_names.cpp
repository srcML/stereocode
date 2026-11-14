// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file hole_names.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/hole_names.hpp"

#include <cctype>

namespace {
    // Tells whether a character can be part of a name, where a name of JavaScript may hold $ and start with #
    //
    bool isNameCharacter(char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '$' || c == '#';
    }
}

// Gets the end of the string that starts at its quote in the code of a hole, after the closing quote
//
std::size_t quotedEnd(const std::string& code, std::size_t position) {
    const char quote = code[position];
    for (++position; position < code.size(); ++position) {
        if (code[position] == '\\') ++position;
        else if (code[position] == quote) return position + 1;
    }
    return code.size();
}

// Adds the names that the code of a hole uses, other than the keywords of its language, each with the names and access operators of its chain,
// such as self.items or this.items
// A call uses only its object, as self.items of self.items.count(x), and a member of a result, as name of f().name, names no variable
//
void addHoleNames(const std::string& code, const std::unordered_set<std::string>& keywords, std::vector<std::string>& names) {
    for (std::size_t i = 0; i < code.size();) {
        const char c = code[i];
        if (c == '"' || c == '\'' || c == '`') {
            i = quotedEnd(code, i);
        }
        else if (std::isdigit(static_cast<unsigned char>(c))) {
            while (i < code.size() && (isNameCharacter(code[i]) || code[i] == '.')) ++i;
        }
        else if (isNameCharacter(c)) {
            const bool member = i > 0 && code[i - 1] == '.';
            std::size_t end = i;
            std::size_t lastPart = i;
            while (end < code.size() && (isNameCharacter(code[end]) || (code[end] == '.' && end + 1 < code.size() && isNameCharacter(code[end + 1])))) {
                if (code[end] == '.') lastPart = end;
                ++end;
            }

            std::string chain = code.substr(i, end - i);
            if (end < code.size() && code[end] == '(') chain = lastPart == i ? "" : code.substr(i, lastPart - i);
            if (!member && !chain.empty() && keywords.count(chain) == 0) names.push_back(std::move(chain));
            i = end;
        }
        else {
            ++i;
        }
    }
}
