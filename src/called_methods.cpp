// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file called_methods.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "called_methods.hpp"

#include <deque>
#include <string_view>
#include <unordered_set>
#include <utility>

// Indexes the methods of the types by name, other than constructors, destructors, and the accessors of C# properties, which no call names,
// given the types of the archive that each type names as parents and the lookup of the types of fields
//
calledMethods::calledMethods(const std::vector<typeModel>& types, std::vector<std::vector<std::size_t>> parents_, typeLookup findType_) :
                             parents{std::move(parents_)}, findType{std::move(findType_)} {
    std::size_t position = 0;
    for (const typeModel& type : types) {
        names.push_back(type.getName()[3]);
        std::unordered_map<std::string, std::vector<namedMethod>>& byName = methods.emplace_back();
        for (const functionModel& method : type.getMethods()) {
            if (method.getConstructorOrDestructor().empty() && method.getPropertyName().empty())
                byName[method.getName()[3]].push_back({{position, type.argumentsOf(method)}, method.getSignatureTypes()});
            ++position;
        }
    }
}

// Gets the positions of the methods that a call of a method of a type to the object itself may reach: a method of the type or of an ancestor,
// an ancestor alone for a call through super or base, and the type or the ancestor that qualifies a C++ call, as Base of Base::draw() does
//
std::vector<std::size_t> calledMethods::ofCallToType(std::size_t type, const callModel& call) const {
    const std::string& name = call.getName()[3];
    for (std::string_view self : {"this.", "this->", "(*this)."})
        if (name.compare(0, self.size(), self) == 0) return reachedMethods({type}, name.substr(self.size()), call.getArgumentCount());
    for (std::string_view parent : {"super.", "base."})
        if (name.compare(0, parent.size(), parent) == 0) return reachedMethods(parents[type], name.substr(parent.size()), call.getArgumentCount());

    const std::size_t qualifierEnd = name.rfind("::");
    if (qualifierEnd == std::string::npos) return reachedMethods({type}, name, call.getArgumentCount());

    std::string qualifier = name.substr(0, qualifierEnd);
    const std::size_t namespaceEnd = qualifier.rfind("::");
    if (namespaceEnd != std::string::npos) qualifier.erase(0, namespaceEnd + 2);
    return reachedMethods(typesNamed(type, qualifier), name.substr(qualifierEnd + 2), call.getArgumentCount());
}

// Gets the positions of the methods that a call of a method of a type on a field may reach, which the type of the field defines or inherits,
// or none when the call reaches the method through more than the field or the type of the field is no type of the archive
//
std::vector<std::size_t> calledMethods::ofCallOnField(std::size_t type, const callModel& call) const {
    if (call.getReceiverType().empty()) return {};
    const std::optional<std::size_t> fieldType = findType(type, call.getReceiverType(), call.getReceiverUnit(), call.isThroughPointer());
    if (!fieldType) return {};

    const std::string& name = call.getName()[3];
    return reachedMethods({*fieldType}, name.substr(name.find_last_of(".>") + 1), call.getArgumentCount());
}

// Gets the type and the ancestors of a type that have a name, nearest first
//
std::vector<std::size_t> calledMethods::typesNamed(std::size_t type, const std::string& name) const {
    std::vector<std::size_t> named;
    std::unordered_set<std::size_t> visited;
    for (std::deque<std::size_t> waiting{type}; !waiting.empty(); waiting.pop_front()) {
        const std::size_t current = waiting.front();
        if (!visited.insert(current).second) continue;
        if (names[current] == name) named.push_back(current);
        waiting.insert(waiting.end(), parents[current].begin(), parents[current].end());
    }
    return named;
}

// Gets the positions of the methods of a name that a call with a number of arguments may reach in the types where the lookup starts
// and in their ancestors, where a method of a nearer type hides the methods of farther types with the same parameter types
//
std::vector<std::size_t> calledMethods::reachedMethods(const std::vector<std::size_t>& start, const std::string& name, int arguments) const {
    std::vector<overload> reached;
    std::unordered_set<std::string> hidden;
    std::unordered_set<std::size_t> visited;
    for (std::deque<std::size_t> waiting(start.begin(), start.end()); !waiting.empty(); waiting.pop_front()) {
        const std::size_t current = waiting.front();
        if (!visited.insert(current).second) continue;
        waiting.insert(waiting.end(), parents[current].begin(), parents[current].end());

        const auto named = methods[current].find(name);
        if (named == methods[current].end()) continue;

        // The definitions of a method in several #if alternatives of one type all count
        std::vector<std::string> defined;
        for (const namedMethod& method : named->second) {
            if (hidden.count(method.signatureTypes) > 0) continue;
            reached.push_back(method.callable);
            defined.push_back(method.signatureTypes);
        }
        hidden.insert(defined.begin(), defined.end());
    }
    return overloadsTaking(reached, arguments);
}
