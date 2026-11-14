// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file specifier_pattern.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "rules/specifier_pattern.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace {
    // Characters with a meaning in a regular expression outside of brackets
    constexpr std::string_view syntaxCharacters = "^$\\.*+?()[]{}|";

    // Tells whether \b and \w of a regular expression count a character as a word character in the classic locale
    //
    bool isWordCharacter(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
    }

    // Tells whether a specifier holds only letters, digits, and underscores
    //
    bool isWord(const std::string& specifier) {
        return std::all_of(specifier.begin(), specifier.end(), [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; });
    }
}

// Joins the default specifiers and then the user specifiers, in their order, into the alternatives of one pattern
// A word matches only between non-word characters, so that the specifier static keeps the type staticClass
//
specifierPattern::specifierPattern(const std::vector<std::string>& defaults, const std::vector<std::string>& userSpecifiers) {
    for (const std::string& specifier : defaults) add(readDefault(specifier));
    for (const std::string& specifier : userSpecifiers)
        if (!specifier.empty()) add(readUserSpecifier(specifier));
}

// Replaces each specifier in a type with a space, as std::regex_replace does with the alternatives as a regular expression
//
std::string specifierPattern::replace(const std::string& type) const {
    std::string replaced;
    for (std::size_t position = 0; position < type.size();) {
        const std::size_t end = matchEnd(type, position);
        if (end == position) {
            replaced += type[position++];
        }
        else {
            replaced += ' ';
            position = end;
        }
    }
    return replaced;
}

// Gets the texts of a type between its specifiers, such as Node and T of Node<T>*, some of which may be empty
//
std::vector<std::string> specifierPattern::split(const std::string& type) const {
    std::vector<std::string> pieces(1);
    for (std::size_t position = 0; position < type.size();) {
        const std::size_t end = matchEnd(type, position);
        if (end == position) {
            pieces.back() += type[position++];
        }
        else {
            pieces.emplace_back();
            position = end;
        }
    }
    return pieces;
}

// Tells whether a specifier of the pattern occurs in a type
//
bool specifierPattern::finds(const std::string& type) const {
    for (std::size_t position = 0; position < type.size(); ++position)
        if (matchEnd(type, position) != position) return true;
    return false;
}

// Reads a default specifier of literal characters and characters escaped with a backslash, optionally followed by \w+,
// or a word followed by (?=<), which names a container only where a generic argument list follows it
//
specifierPattern::literalSpecifier specifierPattern::readDefault(const std::string& specifier) {
    constexpr std::string_view genericSuffix = "(?=<)";

    literalSpecifier literal;
    if (isWord(specifier)) {
        literal.text      = specifier;
        literal.wholeWord = true;
        return literal;
    }
    if (specifier.size() > genericSuffix.size() && specifier.compare(specifier.size() - genericSuffix.size(), genericSuffix.size(), genericSuffix) == 0 &&
        isWord(specifier.substr(0, specifier.size() - genericSuffix.size()))) {
        literal.text        = specifier.substr(0, specifier.size() - genericSuffix.size());
        literal.wholeWord   = true;
        literal.genericOnly = true;
        return literal;
    }

    for (std::size_t i = 0; i < specifier.size(); ++i) {
        const char c = specifier[i];
        if (c == '\\' && specifier.compare(i, std::string::npos, "\\w+") == 0) {
            literal.wordRun = true;
            break;
        }
        if (c == '\\' && i + 1 < specifier.size() && syntaxCharacters.find(specifier[i + 1]) != std::string_view::npos) {
            literal.text += specifier[++i];
            continue;
        }
        if (syntaxCharacters.find(c) != std::string_view::npos) throw std::logic_error("the default specifier " + specifier + " is no literal");
        literal.text += c;
    }

    if (literal.text.empty()) throw std::logic_error("the default specifier " + specifier + " is empty");
    return literal;
}

// Reads a specifier of a user file, which matches as written and, for a word, only as a whole word
//
specifierPattern::literalSpecifier specifierPattern::readUserSpecifier(const std::string& specifier) {
    literalSpecifier literal;
    literal.text      = specifier;
    literal.wholeWord = isWord(specifier);
    return literal;
}

// Adds a specifier as the last alternative
//
void specifierPattern::add(literalSpecifier specifier) {
    byFirstCharacter[static_cast<unsigned char>(specifier.text.front())].push_back(specifiers.size());
    specifiers.push_back(std::move(specifier));
}

// Gets the end of the first specifier that matches at a position of a type, or the position itself when none matches
//
std::size_t specifierPattern::matchEnd(const std::string& type, std::size_t position) const {
    for (std::size_t index : byFirstCharacter[static_cast<unsigned char>(type[position])]) {
        const literalSpecifier& specifier = specifiers[index];
        if (type.compare(position, specifier.text.size(), specifier.text) != 0) continue;

        std::size_t end = position + specifier.text.size();
        if (specifier.wordRun) {
            const std::size_t runStart = end;
            while (end < type.size() && isWordCharacter(type[end])) ++end;
            if (end == runStart) continue;
        }

        const bool wordBefore = position > 0 && isWordCharacter(type[position - 1]);
        const bool wordAfter  = end < type.size() && isWordCharacter(type[end]);
        if (specifier.wholeWord && (wordBefore || wordAfter)) continue;
        if (specifier.genericOnly && (end == type.size() || type[end] != '<')) continue;
        return end;
    }
    return position;
}
