// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file member_access.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/member_access.hpp"

#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

namespace {
    // The access syntax of a language
    //
    struct accessSyntax {
        std::vector<std::string_view>   selfObjects;            // Names of the object itself, such as this
        bool                            selfOperatorIncluded;   // The names of the object itself end with their access operator
        std::string_view                objectEnd;              // Characters that end the name of another object
        std::vector<std::string_view>   operators;              // Access operators
        std::string_view                memberEnd;              // Characters that end the member
        std::string_view                wholeMemberExcluded;    // Characters that a member at the end of a whole expression excludes
    };

    // The splits try the alternatives in the backtracking order of a regular expression such as
    // ^(?:\(\*this\)\.|this->|([^.>-]*)(?:::|\.|->))([^.>-]*) for C++ and ^(?:super|this|([^.]*))\.([^.]*) for Java
    // The model of a Python function writes this for the name of the object, so Python takes the syntax of Java, as JavaScript does
    const accessSyntax cppSyntax    {{"(*this).", "this->"}, true,  ".>-", {"::", ".", "->"}, ".>-", ".>(){}-"};
    const accessSyntax javaSyntax   {{"super", "this"},      false, ".",   {"."},             ".",   ".()"};
    const accessSyntax csharpSyntax {{"base", "this"},       false, ".>-", {".", "->"},       ".>-", ".>()-"};

    // Gets the access whose member starts at a position, where the member of a whole expression must reach its end
    //
    std::optional<memberAccess> memberFrom(const std::string& expression, std::size_t start, std::string object, const accessSyntax& syntax, bool wholeExpression) {
        if (wholeExpression) {
            if (expression.find_first_of(syntax.wholeMemberExcluded, start) != std::string::npos) return std::nullopt;
            return memberAccess{std::move(object), expression.substr(start)};
        }

        const std::size_t end = std::min(expression.find_first_of(syntax.memberEnd, start), expression.size());
        return memberAccess{std::move(object), expression.substr(start, end - start)};
    }
}

// Splits an expression into the object and the member that it accesses first, or gives nothing for an expression without an access
// A whole expression, such as the expression of a return, has no further access or call after the member
//
std::optional<memberAccess> splitMemberAccess(const std::string& expression, const std::string& unitLanguage, bool wholeExpression) {
    const accessSyntax* syntax = nullptr;
    if      (unitLanguage == "C++")                             syntax = &cppSyntax;
    else if (unitLanguage == "Java" || unitLanguage == "Python" || unitLanguage == "JavaScript") syntax = &javaSyntax;
    else if (unitLanguage == "C#")                              syntax = &csharpSyntax;
    else return std::nullopt;

    for (std::string_view self : syntax->selfObjects) {
        if (expression.compare(0, self.size(), self) != 0) continue;

        if (syntax->selfOperatorIncluded) {
            if (auto access = memberFrom(expression, self.size(), "", *syntax, wholeExpression)) return access;
            continue;
        }
        for (std::string_view accessOperator : syntax->operators) {
            if (expression.compare(self.size(), accessOperator.size(), accessOperator) != 0) continue;
            if (auto access = memberFrom(expression, self.size() + accessOperator.size(), "", *syntax, wholeExpression)) return access;
        }
    }

    // The name of another object is the longest start of the expression that an access operator follows
    const std::size_t longestObject = std::min(expression.find_first_of(syntax->objectEnd), expression.size());
    for (std::size_t objectSize = longestObject + 1; objectSize-- > 0;) {
        for (std::string_view accessOperator : syntax->operators) {
            if (expression.compare(objectSize, accessOperator.size(), accessOperator) != 0) continue;
            if (auto access = memberFrom(expression, objectSize + accessOperator.size(), expression.substr(0, objectSize), *syntax, wholeExpression)) return access;
        }
    }
    return std::nullopt;
}
