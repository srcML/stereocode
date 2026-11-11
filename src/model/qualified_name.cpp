// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file qualified_name.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/qualified_name.hpp"

#include <cctype>

namespace {
    constexpr std::string_view operatorKeyword = "operator";

    // Tells whether a character can be part of an identifier
    //
    bool isIdentifierCharacter(char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    }

    // Tells whether the angle bracket at a position belongs to an operator name, such as operator< or operator>>=
    //
    bool isOperatorSymbol(std::string_view name, std::size_t position) {
        std::size_t start = position;
        while (start > 0 && std::string_view("<>=!+-*/%^&|~").find(name[start - 1]) != std::string_view::npos) --start;
        if (start < operatorKeyword.size() || name.compare(start - operatorKeyword.size(), operatorKeyword.size(), operatorKeyword) != 0) return false;

        const std::size_t keywordStart = start - operatorKeyword.size();
        return keywordStart == 0 || !isIdentifierCharacter(name[keywordStart - 1]);
    }

    // Calls a function for each character of a name with the depth of the generic argument lists around it,
    // which is zero for the angle brackets of a list and for every character outside the lists
    //
    template <typename visitor>
    void visitDepths(std::string_view name, visitor visit) {
        int depth = 0;
        for (std::size_t i = 0; i < name.size(); ++i) {
            const char c = name[i];
            const bool bracket = (c == '<' || c == '>') && !isOperatorSymbol(name, i);
            if (bracket && c == '<') {
                visit(i, depth, true);
                ++depth;
            }
            else if (bracket && depth > 0) {
                --depth;
                visit(i, depth, true);
            }
            else {
                visit(i, depth, false);
            }
        }
    }
}

// Splits a name at the separators of its qualifiers outside generic argument lists, which are :: in C++ and . in C# and Java
//
std::vector<std::string> splitQualifiedName(std::string_view name, std::string_view language) {
    const std::string_view separator = (language == "C++" || language == "C") ? "::" : ".";

    std::vector<std::string> parts(1);
    std::size_t skipUntil = 0;
    visitDepths(name, [&](std::size_t i, int depth, bool) {
        if (i < skipUntil) return;
        if (depth == 0 && name.compare(i, separator.size(), separator) == 0) {
            parts.emplace_back();
            skipUntil = i + separator.size();
            return;
        }
        parts.back() += name[i];
    });

    // A leading separator, as in ::std::string, names the global scope
    if (parts.size() > 1 && parts.front().empty()) parts.erase(parts.begin());
    return parts;
}

// Removes the generic argument lists of a name, such as <int> from Box<int>::reset, and keeps the angle brackets of operators
//
std::string withoutGenericArguments(std::string_view name) {
    std::string bare;
    visitDepths(name, [&](std::size_t i, int depth, bool bracket) {
        if (depth == 0 && !bracket) bare += name[i];
    });
    return bare;
}

// Gets the generic argument lists of a name, such as <int> of Box<int>, or an empty string for a name without one
//
std::string genericArguments(std::string_view name) {
    std::string arguments;
    visitDepths(name, [&](std::size_t i, int depth, bool bracket) {
        if (depth > 0 || bracket) arguments += name[i];
    });
    return arguments;
}

// Gets the number of generic parameters in the last generic argument list of a name, such as 2 for Map<K,V>,
// or 0 for a name without a generic argument list
//
int genericArity(std::string_view name) {
    int arity = 0;
    visitDepths(name, [&](std::size_t i, int depth, bool bracket) {
        if (bracket && name[i] == '<' && depth == 0) arity = 1;
        else if (!bracket && depth == 1 && name[i] == ',') ++arity;
    });
    return arity;
}

// Reduces each generic argument list of a name to its angle brackets and its own commas, such as Map<,> for Map<K,List<V>>
//
std::string genericSkeleton(std::string_view name) {
    std::string skeleton;
    visitDepths(name, [&](std::size_t i, int depth, bool bracket) {
        if (depth == 0 || (depth == 1 && !bracket && name[i] == ',')) skeleton += name[i];
    });
    return skeleton;
}
