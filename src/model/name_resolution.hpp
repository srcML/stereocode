// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file name_resolution.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef NAME_RESOLUTION_HPP
#define NAME_RESOLUTION_HPP

#include <functional>
#include <string>
#include <unordered_set>

// The kinds of variables that a name in the body of a function refers to
//
enum class variableKind { local, parameter, field, global, unknown };

// The variable that a name or an expression refers to, and how the expression reaches the variable
//
struct nameResolution {
    variableKind    kind{variableKind::unknown};
    std::string     variable;               // Name of the variable, or the name that no scope holds (empty for an expression without a name)
    std::string     object;                 // Name before the first access operator, such as Other of Other.count, if any
    std::string     member;                 // Name after the first access operator, such as count of Other.count, if any
    std::string     holder;                 // Type or namespace that qualifies a global, such as Settings of Shop.Settings.Level, if any
    bool            throughMember{false};   // The expression accesses a member, such as a.b, p->b, or this->a
    bool            otherQualifier{false};  // A name other than the object itself and the own type qualifies the member, such as Other of Other.count
    bool            throughIndex{false};    // The expression indexes the variable, such as a[i]
    bool            dereferenced{false};    // The expression dereferences the variable, such as *p
};

// What a C++ call whose name has an operator calls a function on
//
enum class callReceiver {
    objectItself,   // The object itself, as in this->f(), (*this).f(), A::f() in A, or Base::f() in a type derived from Base
    noObject,       // No object, as in std::f() or ::f() for a free function or a static method of another type
    otherObject     // Another object, as in a.f() or p->f()
};

// The variables in scope where a name occurs
//
struct variableScopes {
    std::function<bool(const std::string&)>                     isLocal;            // A local whose block or statement holds the name
    std::function<bool(const std::string&)>                     isParameter;
    std::function<bool(const std::string&)>                     isField;            // A data member that a name without the object itself reaches
    std::function<bool(const std::string&)>                     isEarlierLocal;     // A local declared before the name, also outside the scope of the name
    std::function<bool(const std::string&)>                     isGlobal;           // A global of the name in any type or namespace
    std::function<bool(const std::string&, const std::string&)> holdsGlobal;        // A type or a namespace, or the global namespace for an empty name, that holds a global of the name
    std::function<bool(const std::string&)>                     isOwnMember;        // A data member that a name after the object itself reaches, as total of this.total
};

nameResolution  resolveName             (const std::string&, const std::string&, const std::string&, bool, const variableScopes&);
std::string     withoutSelfAccess       (const std::string&);
bool            writesThroughVariable   (const nameResolution&);
std::string     qualifyingType          (const std::string&);
callReceiver    cppCallReceiver         (const std::string&, const std::unordered_set<std::string>&);

#endif
