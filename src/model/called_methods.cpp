// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file called_methods.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/called_methods.hpp"
#include "model/qualified_name.hpp"
#include "model/written_type.hpp"
#include "archive/source_language.hpp"

#include <algorithm>
#include <cctype>
#include <deque>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace {
    // Gets the name of the method that a call on a data member names after its last access operator, where the symbols of an operator
    // function belong to its name, as in operator>> of out.operator>>
    //
    std::string calledMethodName(const std::string& callName) {
        constexpr std::string_view keyword = ".operator";
        const std::size_t start  = callName.rfind(keyword);
        const std::size_t symbol = start == std::string::npos ? start : start + keyword.size();
        if (symbol < callName.size() && !std::isalnum(static_cast<unsigned char>(callName[symbol])) && callName[symbol] != '_') return callName.substr(start + 1);
        return callName.substr(callName.find_last_of(".>") + 1);
    }
}

// Indexes the methods of the types by name, other than constructors, destructors, and the accessors of C# properties, which no call names,
// given the types of the archive that each type names as parents, the type of the enclosing instance of each inner class, the lookup of
// the types of fields, and the static methods of the types
//
calledMethods::calledMethods(const std::vector<typeModel>& types, std::vector<std::vector<std::size_t>> parents_,
                             std::vector<std::optional<std::size_t>> enclosing_, typeLookup findTypes_, const staticOverloads& statics,
                             extensionMethods extensions_) :
                             parents{std::move(parents_)}, enclosing{std::move(enclosing_)}, findTypes{std::move(findTypes_)}, extensions{std::move(extensions_)} {
    std::size_t position = 0;
    for (const typeModel& type : types) {
        names.push_back(type.getName()[3]);
        languages.push_back(type.getUnitLanguage());
        genericNames.push_back(type.getName()[2]);
        const auto ofType = statics.find(type.getUnitLanguage() + ' ' + type.getName()[3]);
        staticMethods.push_back(ofType == statics.end() ? argumentsByName{} : ofType->second);
        undefinedMethods.push_back(type.undefinedMethods());
        implicitAccessors.push_back(type.implicitAccessors());
        unknownParents.push_back(type.hasUnknownParent() && languageOf(type.getUnitLanguage()).overloads);
        parentAliases.push_back(type.getParentAliases());
        std::unordered_map<std::string, std::vector<namedMethod>>& byName = methods.emplace_back();
        for (const functionModel& method : type.getMethods()) {
            if (method.getConstructorOrDestructor().empty() && method.getPropertyName().empty())
                byName[method.getName()[3]].push_back({{position, type.argumentsOf(method)}, withoutQualifiers(method.getSignatureTypes()), method.isInConstantBody()});
            ++position;
        }
    }
}

// Tells whether a call of a method of a type to the object itself calls a static method, which is a free function: no method of its name
// in the type or in an ancestor takes its number of arguments, while a static method of the name does
//
bool calledMethods::callsStaticMethod(std::size_t type, const callModel& call) const {
    const std::string& written = call.getName()[3];
    const std::size_t nameStart = written.find_last_of(".>:") + 1;
    const std::string name = written.substr(nameStart);
    const int arguments = call.getArgumentCount();

    bool fitsStatic = false;
    std::unordered_set<std::size_t> visited;
    for (std::deque<std::size_t> waiting{nameStart == 0 ? holderOf(type, name) : type}; !waiting.empty(); waiting.pop_front()) {
        const std::size_t current = waiting.front();
        if (!visited.insert(current).second) continue;
        waiting.insert(waiting.end(), parents[current].begin(), parents[current].end());

        const auto named = methods[current].find(name);
        if (named != methods[current].end())
            for (const namedMethod& method : named->second)
                if (takes(method.callable.arguments, arguments)) return false;
        const auto declared = undefinedMethods[current].find(name);
        if (declared != undefinedMethods[current].end() &&
            std::any_of(declared->second.begin(), declared->second.end(), [arguments](const declaredMethod& method) { return takes(method.arguments, arguments); }))
            return false;
        fitsStatic = fitsStatic || takes(staticMethods[current], name, arguments);
    }
    return fitsStatic;
}

// Gets the positions of the methods that a call of a method of a type to the object itself may reach: a method of the type or of an ancestor,
// an ancestor alone for a call through super or base, and the type or the ancestor that qualifies a C++ call, by its name or by a member alias,
// as Base of Base::draw() does
//
std::vector<std::size_t> calledMethods::ofCallToType(std::size_t type, std::size_t caller, const callModel& call) const {
    const std::string& name = call.getName()[3];
    const int arguments = call.getArgumentCount();
    for (std::string_view self : {"this.", "this->", "(*this)."})
        if (name.compare(0, self.size(), self) == 0) return reachedMethods({type}, name.substr(self.size()), arguments, caller);
    for (std::string_view parent : {"super.", "base."})
        if (name.compare(0, parent.size(), parent) == 0) return reachedMethods(parents[type], name.substr(parent.size()), arguments, caller);

    // Outer.this.m() names a method of the enclosing instance of the type Outer, and Parent.super.m() a method of the interface Parent
    const auto qualifierBefore = [&name](std::size_t end) { return name.substr(0, end).substr(name.rfind('.', end - 1) + 1); };
    constexpr std::string_view outerObject = ".this.";
    if (const std::size_t outerEnd = name.find(outerObject); outerEnd != std::string::npos && outerEnd > 0) {
        for (std::optional<std::size_t> holder = type; holder; holder = enclosing[*holder])
            if (names[*holder] == qualifierBefore(outerEnd)) return reachedMethods({*holder}, name.substr(outerEnd + outerObject.size()), arguments, caller);
        return {};
    }
    constexpr std::string_view parentObject = ".super.";
    if (const std::size_t parentEnd = name.find(parentObject); parentEnd != std::string::npos && parentEnd > 0)
        return reachedMethods(typesNamed(type, qualifierBefore(parentEnd)), name.substr(parentEnd + parentObject.size()), arguments, caller);

    const std::size_t qualifierEnd = name.rfind("::");
    if (qualifierEnd == std::string::npos) return reachedMethods({holderOf(type, name)}, name, arguments, caller);

    const std::string qualifier = qualifyingType(name);
    const auto alias = parentAliases[type].find(qualifier);
    return reachedMethods(typesNamed(type, alias == parentAliases[type].end() ? qualifier : alias->second), name.substr(qualifierEnd + 2), arguments, caller);
}

// Gets the positions of the methods that a call of a method of a type on a field may reach, which the type of the field defines or inherits,
// or none when the call reaches the method through more than the field or the type of the field is no type of the archive
// A type name that several types of the archive fit equally well, as the classes of one name for several platforms do, reaches the methods of each
//
std::vector<std::size_t> calledMethods::ofCallOnField(std::size_t type, std::size_t caller, const callModel& call) const {
    if (call.getReceiverType().empty()) return {};
    std::vector<std::size_t> reached;
    for (std::size_t fieldType : findTypes(type, call.getReceiverType(), call.getReceiverUnit(), call.isThroughPointer())) {
        // Each type starts a lookup of its own, since a method of one platform hides no method of another
        const std::vector<std::size_t> ofType = reachedMethods({fieldType}, calledMethodName(call.getName()[3]), call.getArgumentCount(), caller);
        reached.insert(reached.end(), ofType.begin(), ofType.end());
    }
    return reached;
}

// Tells whether a call of a method of a type on a field calls the accessor that the language gives a component of a record: the call
// takes no argument and names such a component in each type that the type of the field names
//
bool calledMethods::callsImplicitAccessor(std::size_t type, const callModel& call) const {
    if (call.getReceiverType().empty() || call.getArgumentCount() != 0) return false;
    const std::vector<std::size_t> fieldTypes = findTypes(type, call.getReceiverType(), call.getReceiverUnit(), call.isThroughPointer());
    const std::string name = calledMethodName(call.getName()[3]);
    return !fieldTypes.empty() && std::all_of(fieldTypes.begin(), fieldTypes.end(), [&](std::size_t fieldType) { return implicitAccessors[fieldType].count(name) > 0; });
}

// Tells whether a call of a method of a type on a field calls a C# extension method of the archive with the field as its first argument: the
// method takes the number of arguments of the call and extends the type of the field or, for a type of the archive, one of its ancestors,
// where the generic parameters tell WeakReference from WeakReference<T>
// The caller tells first that no method of the type of the field fits the call, since such a method hides an extension method
//
bool calledMethods::callsExtensionMethod(std::size_t type, const callModel& call) const {
    const auto named = extensions.find(languages[type] + ' ' + calledMethodName(call.getName()[3]));
    if (named == extensions.end() || call.getReceiverType().empty()) return false;

    std::unordered_set<std::string> extendable{genericSkeleton(splitQualifiedName(call.getReceiverType(), languages[type]).back())};
    std::unordered_set<std::size_t> visited;
    const std::vector<std::size_t> fieldTypes = findTypes(type, call.getReceiverType(), call.getReceiverUnit(), call.isThroughPointer());
    for (std::deque<std::size_t> waiting(fieldTypes.begin(), fieldTypes.end()); !waiting.empty(); waiting.pop_front()) {
        const std::size_t current = waiting.front();
        if (!visited.insert(current).second) continue;
        extendable.insert(genericNames[current]);
        waiting.insert(waiting.end(), parents[current].begin(), parents[current].end());
    }
    return std::any_of(named->second.begin(), named->second.end(), [&](const extensionMethod& method) {
        return extendable.count(method.extendedType) > 0 && takes(method.arguments, call.getArgumentCount());
    });
}

// Gets the type whose method a call without an object in a type names: the nearest type that has a method of the name among the
// type with its ancestors and the types of the enclosing instances of a Java inner class, or the outermost of these types
//
std::size_t calledMethods::holderOf(std::size_t type, const std::string& name) const {
    std::size_t holder = type;
    while (enclosing[holder] && !hasMethodNamed(holder, name)) holder = *enclosing[holder];
    return holder;
}

// Tells whether a type or one of its ancestors defines or declares a method of a name
//
bool calledMethods::hasMethodNamed(std::size_t type, const std::string& name) const {
    std::unordered_set<std::size_t> visited;
    for (std::deque<std::size_t> waiting{type}; !waiting.empty(); waiting.pop_front()) {
        const std::size_t current = waiting.front();
        if (!visited.insert(current).second) continue;
        if (methods[current].count(name) > 0 || undefinedMethods[current].count(name) > 0) return true;
        waiting.insert(waiting.end(), parents[current].begin(), parents[current].end());
    }
    return false;
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

// Gets the positions of the methods of a name that a call of a method with a number of arguments may reach in the types where the lookup
// starts and in their ancestors, where a method of a nearer type hides the methods of farther types with the same parameter types
// The call reaches no method with stereotypes where it may name a method without a definition: a declared method that takes its arguments,
// as an abstract method, or a method of a parent outside the archive, where no method takes the arguments or the call reaches the calling
// method alone, from which the named method may differ in the types of its parameters only
//
std::vector<std::size_t> calledMethods::reachedMethods(const std::vector<std::size_t>& start, const std::string& name, int arguments,
                                                       std::size_t caller) const {
    static const std::vector<namedMethod> none;
    static const std::vector<declaredMethod> undeclared;
    std::vector<overload> reached;
    std::unordered_set<std::string> hidden;
    std::unordered_set<std::size_t> visited;
    std::vector<std::string> declaredTypes;
    const std::string* callerTypes = nullptr;
    bool unknownParent = false;
    for (std::deque<std::size_t> waiting(start.begin(), start.end()); !waiting.empty(); waiting.pop_front()) {
        const std::size_t current = waiting.front();
        if (!visited.insert(current).second) continue;
        waiting.insert(waiting.end(), parents[current].begin(), parents[current].end());
        unknownParent = unknownParent || unknownParents[current];

        // The definitions of a method in several #if alternatives of one type all count, as those in the bodies of several enum constants do
        const auto named = methods[current].find(name);
        const std::vector<namedMethod>& ofName = named == methods[current].end() ? none : named->second;
        const auto reach = [&](bool inConstantBody) {
            std::vector<std::string> defined;
            for (const namedMethod& method : ofName) {
                if (method.inConstantBody != inConstantBody || hidden.count(method.parameterTypes) > 0) continue;
                reached.push_back(method.callable);
                defined.push_back(method.parameterTypes);
                if (method.callable.position == caller) callerTypes = &method.parameterTypes;
            }
            hidden.insert(defined.begin(), defined.end());
        };
        reach(false);

        // A method with as many parameters may define the declared method, since an override may write the types in another way
        const auto declared = undefinedMethods[current].find(name);
        for (const declaredMethod& method : declared == undefinedMethods[current].end() ? undeclared : declared->second) {
            if (!takes(method.arguments, arguments)) continue;
            const bool overridden = std::any_of(reached.begin(), reached.end(), [&method](const overload& other) {
                return other.arguments.maximum == method.arguments.maximum && other.arguments.variadic == method.arguments.variadic;
            });
            if (!overridden) return {};
            declaredTypes.push_back(method.parameterTypes);
        }
        reach(true);
    }

    const bool taken = std::any_of(reached.begin(), reached.end(), [arguments](const overload& method) { return takes(method.arguments, arguments); });
    if (!taken && unknownParent) return {};

    // A call that reaches the calling method alone may name a declared method with other parameter types or a method of such a parent
    const std::vector<std::size_t> taking = overloadsTaking(reached, arguments);
    if (!callerTypes || taking.size() != 1 || taking.front() != caller) return taking;
    const bool otherDeclared = std::any_of(declaredTypes.begin(), declaredTypes.end(), [callerTypes](const std::string& types) { return types != *callerTypes; });
    return otherDeclared || unknownParent ? std::vector<std::size_t>{} : taking;
}
