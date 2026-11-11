// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file name_resolution.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/name_resolution.hpp"
#include "helper_functions.hpp"
#include "model/member_access.hpp"
#include "model/qualified_name.hpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <vector>

namespace {
    // Removes the parentheses and braces around an expression, which may also remove those of a call
    // The parenthesis of (*this). stays, since it accesses a member of the object itself
    //
    void removeEnclosingBrackets(std::string& expr, const std::string& language) {
        if (language == "C++") {
            while (!expr.empty() && (expr.front() == '{' || expr.front() == '(') && expr.compare(0, 8, "(*this).") != 0) expr.erase(0, 1);
            while (!expr.empty() && (expr.back() == '}' || expr.back() == ')')) expr.pop_back();
        }
        else {
            while (!expr.empty() && expr.front() == '(') expr.erase(0, 1);
            while (!expr.empty() && expr.back() == ')') expr.pop_back();
        }
    }

    // Removes the balanced index groups at the end of an expression, such as [i][j] of a[i][j], and tells whether it removed one
    //
    bool removeTrailingIndex(std::string& expr) {
        bool removed = false;
        while (!expr.empty() && expr.back() == ']') {
            int depth = 0;
            std::size_t open = expr.size();
            while (open-- > 0) {
                if      (expr[open] == ']') ++depth;
                else if (expr[open] == '[' && --depth == 0) break;
            }
            if (open == std::string::npos) break;
            expr.erase(open);
            removed = true;
        }
        return removed;
    }

    // Tells whether a character can be part of a name, where Java allows $ and C# marks a keyword used as a name with @
    //
    bool isNameCharacter(char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '$' || c == '@';
    }

    // A name of an expression and whether the C++ scope operator :: follows it, which only a namespace or a type, never a variable, allows
    //
    struct accessedName {
        std::string name;
        bool        scope{false};
    };

    // Gets the names that access operators join in an expression, such as ns, g, and items of ns::g.items, where the global namespace
    // of ::g has an empty name, or nothing when the expression holds more than names, as g+1 does
    //
    std::vector<accessedName> accessedNames(const std::string& expr) {
        const std::string bare = withoutGenericArguments(expr);
        std::vector<accessedName> names(1);
        for (std::size_t i = 0; i < bare.size();) {
            const bool scope = bare.compare(i, 2, "::") == 0;
            const std::size_t operatorSize = scope || bare.compare(i, 2, "->") == 0 ? 2 : bare[i] == '.' ? 1 : 0;
            if (operatorSize > 0) {
                names.back().scope = scope;
                names.emplace_back();
                i += operatorSize;
            }
            else if (isNameCharacter(bare[i])) {
                names.back().name += bare[i++];
            }
            else {
                return {};
            }
        }
        return names;
    }

    // A global with the type or the namespace that qualifies its name, which is empty for a name without a qualifier
    //
    struct qualifiedGlobal {
        std::string name;
        std::string holder;
    };

    // Gets the global that an expression starts with, alone or after the types and namespaces that hold it,
    // such as g of g.items, level of settings::level or ::settings::level, and Count of Shop.Config.Count, or nothing
    //
    std::optional<qualifiedGlobal> globalOf(const std::string& expr, const variableScopes& scopes) {
        const std::vector<accessedName> names = accessedNames(expr);
        if (names.empty()) return std::nullopt;
        if (!names.front().name.empty() && !names.front().scope && scopes.isGlobal(names.front().name)) return qualifiedGlobal{names.front().name, ""};
        for (std::size_t i = 1; i < names.size(); ++i) {
            if (!names[i].name.empty() && !names[i].scope && scopes.holdsGlobal(names[i - 1].name, names[i].name)) return qualifiedGlobal{names[i].name, names[i - 1].name};
        }
        return std::nullopt;
    }
}

// Resolves a name or an expression in the body of a function to the variable that it starts with
// Possible forms:
//  C++:  this->a; (*this).a; Foo::a; this->a.b; (*this).a.b; Foo::a.b; a.b; a
//  Java: super.a; this.a; Foo.a; super.a.b; this.a.b; Foo.a.b; a.b; a
//  C#:   base.a; this.a; Foo.a; base.a.b; this.a.b; Foo.a.b; a.b; a
// Foo is the type itself. A local hides a parameter, and both hide a field, unless the object itself qualifies the name
// A name that matches no variable in scope matches a global, which the types and namespaces that hold it may qualify, as in ns::g, ::g, or Config.Count,
//  and otherwise a local declared before it, since srcML can end a block early around preprocessor branches
// A whole expression, such as the expression of a return, matches a variable only when it consists of the variable
//
nameResolution resolveName(const std::string& expression, const std::string& language, const std::string& typeName,
                           bool wholeExpression, const variableScopes& scopes) {
    nameResolution resolution;

    std::string expr = expression;
    HELPERS::removeWhitespace(expr);

    // In Java, Outer.this.a reaches a data member of an enclosing instance, which counts as a data member of the inner class
    if (language == "Java") {
        const std::size_t qualifiedThis = expr.find(".this.");
        const auto isNamePart = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '$' || c == '.'; };
        if (qualifiedThis != std::string::npos && qualifiedThis > 0 && std::all_of(expr.begin(), expr.begin() + qualifiedThis, isNamePart))
            expr.erase(0, qualifiedThis + 1);
    }

    if (wholeExpression) {
        // An index inside the expression, such as arr[i] of arr[i] + x, makes it more than a variable
        removeEnclosingBrackets(expr, language);
        resolution.throughIndex = removeTrailingIndex(expr);
    }
    else {
        const std::size_t indexStart = expr.find('[');
        if (indexStart != std::string::npos) {
            expr.erase(indexStart);
            resolution.throughIndex = true;
        }
        removeEnclosingBrackets(expr, language);
    }

    // C# marks a null-conditional access, such as a?.b, with ? or ??
    if (language == "C#") expr.erase(std::remove(expr.begin(), expr.end(), '?'), expr.end());

    if (language != "Java") {
        const std::size_t asterisks = expr.find_first_not_of('*');
        resolution.dereferenced = asterisks != 0 && !expr.empty();
        expr.erase(0, asterisks == std::string::npos ? expr.size() : asterisks);
    }
    if (expr.empty()) return resolution;

    // In C++, :: before a name, as in ::g, names the global namespace, which holds no local, parameter, or field
    const bool globalNamespace = language == "C++" && expr.compare(0, 2, "::") == 0;

    // The first two names decide the variable, such as a.b for a.b.c()
    const std::optional<memberAccess> access = splitMemberAccess(expr, language, wholeExpression);
    resolution.throughMember = access.has_value();
    if (access) {
        resolution.object = access->object;
        resolution.member = access->member;
    }
    // In C++, only a type or a namespace comes before ::, while an object comes before . and ->
    resolution.otherQualifier = access && !access->object.empty() && access->object.substr(0, access->object.find('<')) != typeName &&
                                (language != "C++" || expr.compare(access->object.size(), 2, "::") == 0);

    std::vector<std::string> candidates{expr};
    bool ownMember = false;
    if (access) {
        if (access->object.empty()) {
            // After this, base, super, or (*this), only a field matches
            candidates.front() = access->member;
            ownMember = true;
        }
        else if (!wholeExpression) {
            candidates.front() = access->object;
        }

        // In C# and Java, the type name qualifies static fields; in C++, any field
        if (!access->object.empty() && !access->member.empty() && access->object.substr(0, access->object.find('<')) == typeName)
            candidates.push_back(access->member);
    }

    resolution.variable = candidates.front();
    if (globalNamespace) candidates.clear();
    for (const std::string& candidate : candidates) {
        resolution.variable = candidate;
        if (!ownMember) {
            if (scopes.isLocal(candidate)) {
                resolution.kind = variableKind::local;
                return resolution;
            }
            if (scopes.isParameter(candidate)) {
                resolution.kind = variableKind::parameter;
                return resolution;
            }
        }
        if (scopes.isField(candidate)) {
            resolution.kind = variableKind::field;
            return resolution;
        }
    }
    if (ownMember && !globalNamespace) return resolution;

    if (const std::optional<qualifiedGlobal> global = globalOf(expr, scopes)) {
        resolution.variable = global->name;
        resolution.holder   = global->holder;
        resolution.kind     = variableKind::global;
        return resolution;
    }
    for (const std::string& candidate : candidates) {
        if (scopes.isEarlierLocal(candidate)) {
            resolution.variable = candidate;
            resolution.kind     = variableKind::local;
            return resolution;
        }
    }
    return resolution;
}

// Gets a name without the object itself that qualifies it, such as cache for this.cache, this->cache, or (*this).cache
//
std::string withoutSelfAccess(const std::string& name) {
    for (const char* self : {"(*this).", "this->", "this.", "base.", "super."}) {
        const std::string prefix(self);
        if (name.compare(0, prefix.size(), prefix) == 0) return name.substr(prefix.size());
    }
    return name;
}

// Tells whether a write reaches the object that a variable refers to, through a member, an element, or the target of a pointer,
// such as p->b = v, a[i] = v, or *p = v, unlike a write to the variable itself
//
bool writesThroughVariable(const nameResolution& write) {
    return write.throughMember || write.throughIndex || write.dereferenced;
}

// Gets what a C++ call whose name has an operator calls a function on, given the names of the type of the method and of its parent types
// The name has no whitespace, such as this->f, Base<T>::f, std::f, or a.b
//
callReceiver cppCallReceiver(const std::string& callName, const std::unordered_set<std::string>& ownTypes) {
    std::string name = callName;
    for (const char* self : {"this->", "(*this)."}) {
        if (name.compare(0, std::char_traits<char>::length(self), self) == 0) {
            name.erase(0, std::char_traits<char>::length(self));
            return name.find('.') == std::string::npos && name.find("->") == std::string::npos ? callReceiver::objectItself : callReceiver::otherObject;
        }
    }

    const std::size_t qualifierEnd = name.rfind("::");
    if (qualifierEnd == std::string::npos || name.find('.') != std::string::npos || name.find("->") != std::string::npos) return callReceiver::otherObject;

    // The type that qualifies the function, without its namespaces and generic arguments
    std::string qualifier = name.substr(0, qualifierEnd);
    qualifier = qualifier.substr(0, qualifier.find('<'));
    const std::size_t namespaceEnd = qualifier.rfind("::");
    if (namespaceEnd != std::string::npos) qualifier.erase(0, namespaceEnd + 2);
    return ownTypes.find(qualifier) != ownTypes.end() ? callReceiver::objectItself : callReceiver::noObject;
}
