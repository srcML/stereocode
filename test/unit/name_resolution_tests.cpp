// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file name_resolution_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "name_resolution.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace {
    // The names of the globals that each type or namespace holds, where the global namespace has an empty name
    using globalOwners = std::unordered_map<std::string, std::unordered_set<std::string>>;

    // Gets scopes that hold the given names of locals, parameters, fields, and globals
    //
    variableScopes scopesOf(std::unordered_set<std::string> locals, std::unordered_set<std::string> parameters, std::unordered_set<std::string> fields,
                            std::unordered_set<std::string> earlierLocals = {}, globalOwners globals = {}) {
        return {
            [locals](const std::string& name) { return locals.count(name) > 0; },
            [parameters](const std::string& name) { return parameters.count(name) > 0; },
            [fields](const std::string& name) { return fields.count(name) > 0; },
            [earlierLocals](const std::string& name) { return earlierLocals.count(name) > 0; },
            [globals](const std::string& name) {
                for (const auto& [owner, names] : globals)
                    if (names.count(name) > 0) return true;
                return false;
            },
            [globals](const std::string& owner, const std::string& name) {
                const auto found = globals.find(owner);
                return found != globals.end() && found->second.count(name) > 0;
            }
        };
    }

    // Resolves a name in the body of a method of the type Foo
    //
    nameResolution resolve(const std::string& expression, const std::string& language, const variableScopes& scopes, bool wholeExpression = false) {
        return resolveName(expression, language, "Foo", wholeExpression, scopes);
    }
}

TEST_CASE("a local hides a parameter, and both hide a field of the same name") {
    const variableScopes scopes = scopesOf({"a"}, {"a", "b"}, {"a", "b", "c"});

    CHECK(resolve("a", "C++", scopes).kind == variableKind::local);
    CHECK(resolve("b", "C++", scopes).kind == variableKind::parameter);
    CHECK(resolve("c", "C++", scopes).kind == variableKind::field);

    const nameResolution unknown = resolve("d", "C++", scopes);
    CHECK(unknown.kind == variableKind::unknown);
    CHECK(unknown.variable == "d");
}

TEST_CASE("a local declared before a name outside its scope matches only a name that no variable in scope matches") {
    const variableScopes scopes = scopesOf({}, {"p"}, {"x"}, {"x", "p", "n"});

    CHECK(resolve("x", "C++", scopes).kind == variableKind::field);
    CHECK(resolve("p", "C++", scopes).kind == variableKind::parameter);
    CHECK(resolve("n", "C++", scopes).kind == variableKind::local);
    CHECK(resolve("this->n", "C++", scopes).kind == variableKind::unknown);
}

TEST_CASE("the object itself qualifies a field, which no local or parameter hides") {
    const variableScopes scopes = scopesOf({"x"}, {"x"}, {"x"});

    for (const char* expression : {"this->x", "(*this).x", "this->x.y"}) {
        const nameResolution resolution = resolve(expression, "C++", scopes);
        CHECK(resolution.kind == variableKind::field);
        CHECK(resolution.variable == "x");
        CHECK(resolution.throughMember);
    }
    CHECK(resolve("this.x", "Java", scopes).kind == variableKind::field);
    CHECK(resolve("super.x", "Java", scopes).kind == variableKind::field);
    CHECK(resolve("base.x", "C#", scopes).kind == variableKind::field);
}

TEST_CASE("an expression that starts with a variable resolves to the variable") {
    const variableScopes scopes = scopesOf({"list"}, {"p"}, {"items"});

    const nameResolution member = resolve("p->next", "C++", scopes);
    CHECK(member.kind == variableKind::parameter);
    CHECK(member.variable == "p");
    CHECK(member.throughMember);

    CHECK(resolve("list.add", "Java", scopes).kind == variableKind::local);
    CHECK(resolve("items?.Count", "C#", scopes).kind == variableKind::field);
}

TEST_CASE("the name of the own type qualifies a field") {
    const variableScopes scopes = scopesOf({}, {}, {"count"});

    CHECK(resolve("Foo::count", "C++", scopes).kind == variableKind::field);
    CHECK(resolve("Foo.count", "C#", scopes).kind == variableKind::field);
    CHECK(resolve("Bar::count", "C++", scopes).kind == variableKind::unknown);
}

TEST_CASE("an index or a dereference marks how a name reaches its variable") {
    const variableScopes scopes = scopesOf({}, {"a", "p"}, {});

    const nameResolution indexed = resolve("a[i]", "C++", scopes);
    CHECK(indexed.kind == variableKind::parameter);
    CHECK(indexed.throughIndex);
    CHECK(writesThroughVariable(indexed));

    const nameResolution dereferenced = resolve("*p", "C++", scopes, true);
    CHECK(dereferenced.kind == variableKind::parameter);
    CHECK(dereferenced.dereferenced);

    const nameResolution itself = resolve("p", "C++", scopes);
    CHECK_FALSE(writesThroughVariable(itself));
}

TEST_CASE("a returned expression is a variable only when it consists of the variable and trailing indexes") {
    const variableScopes scopes = scopesOf({}, {"i"}, {"arr", "x"});

    CHECK(resolve("arr[i]", "C++", scopes, true).kind == variableKind::field);
    CHECK(resolve("arr[i][j]", "Java", scopes, true).kind == variableKind::field);
    CHECK(resolve("(arr[i])", "C#", scopes, true).kind == variableKind::field);
    CHECK(resolve("arr[i] + x", "C++", scopes, true).kind == variableKind::unknown);
    CHECK(resolve("arr[i] == 0", "Java", scopes, true).kind == variableKind::unknown);
    CHECK(resolve("this->x", "C++", scopes, true).kind == variableKind::field);
    CHECK(resolve("(*this).x", "C++", scopes, true).kind == variableKind::field);
    CHECK(resolve("x.size()", "C++", scopes, true).kind == variableKind::unknown);
}

TEST_CASE("an expression without a name resolves to no variable") {
    const nameResolution resolution = resolve("()", "C++", scopesOf({}, {}, {}));
    CHECK(resolution.kind == variableKind::unknown);
    CHECK(resolution.variable.empty());
}

TEST_CASE("a Java name qualified by an enclosing instance resolves like a name qualified by this") {
    const variableScopes scopes = scopesOf({"count"}, {}, {"count"});

    const nameResolution resolution = resolve("Counter.this.count", "Java", scopes);
    CHECK(resolution.kind == variableKind::field);
    CHECK(resolution.variable == "count");
    CHECK(resolve("outer.Counter.this.count", "Java", scopes).kind == variableKind::field);
}

TEST_CASE("the object itself is removed from a qualified name") {
    CHECK(withoutSelfAccess("this.cache") == "cache");
    CHECK(withoutSelfAccess("this->cache") == "cache");
    CHECK(withoutSelfAccess("(*this).cache") == "cache");
    CHECK(withoutSelfAccess("base.cache") == "cache");
    CHECK(withoutSelfAccess("super.cache") == "cache");
    CHECK(withoutSelfAccess("other.cache") == "other.cache");
    CHECK(withoutSelfAccess("thisCache") == "thisCache");
}

TEST_CASE("a C++ call through the object itself or a type of its hierarchy calls a method of the object") {
    const std::unordered_set<std::string> ownTypes{"A", "Base"};

    CHECK(cppCallReceiver("this->g", ownTypes) == callReceiver::objectItself);
    CHECK(cppCallReceiver("(*this).g", ownTypes) == callReceiver::objectItself);
    CHECK(cppCallReceiver("A::g", ownTypes) == callReceiver::objectItself);
    CHECK(cppCallReceiver("Base::draw", ownTypes) == callReceiver::objectItself);
    CHECK(cppCallReceiver("Base<T>::draw", ownTypes) == callReceiver::objectItself);
    CHECK(cppCallReceiver("ns::Base::draw", ownTypes) == callReceiver::objectItself);
    CHECK(cppCallReceiver("this->Base::draw", ownTypes) == callReceiver::objectItself);
}

TEST_CASE("a C++ call qualified by a namespace or another type calls no object, and a call on a member calls another object") {
    const std::unordered_set<std::string> ownTypes{"A"};

    CHECK(cppCallReceiver("std::puts", ownTypes) == callReceiver::noObject);
    CHECK(cppCallReceiver("::puts", ownTypes) == callReceiver::noObject);
    CHECK(cppCallReceiver("Other::create", ownTypes) == callReceiver::noObject);
    CHECK(cppCallReceiver("a.f", ownTypes) == callReceiver::otherObject);
    CHECK(cppCallReceiver("p->f", ownTypes) == callReceiver::otherObject);
    CHECK(cppCallReceiver("this->member.f", ownTypes) == callReceiver::otherObject);
    CHECK(cppCallReceiver("this->member->f", ownTypes) == callReceiver::otherObject);
}

TEST_CASE("a name other than the object itself and the own type qualifies a member, which in C++ needs ::") {
    const variableScopes scopes = scopesOf({}, {}, {"x"});

    const nameResolution other = resolve("Other.count", "Java", scopes);
    CHECK(other.kind == variableKind::unknown);
    CHECK(other.variable == "Other");
    CHECK(other.otherQualifier);
    CHECK(resolve("Other.Count", "C#", scopes).otherQualifier);
    CHECK(resolve("Other::count", "C++", scopes).otherQualifier);

    CHECK_FALSE(resolve("view.visible", "C++", scopes).otherQualifier);
    CHECK_FALSE(resolve("view->visible", "C++", scopes).otherQualifier);
    CHECK_FALSE(resolve("this.count", "Java", scopes).otherQualifier);
    CHECK_FALSE(resolve("Foo.count", "Java", scopes).otherQualifier);
    CHECK_FALSE(resolve("Foo::count", "C++", scopes).otherQualifier);
    CHECK_FALSE(resolve("count", "Java", scopes).otherQualifier);
}

TEST_CASE("a qualified name gives the member after its first access operator") {
    const variableScopes scopes = scopesOf({}, {}, {"x"});

    CHECK(resolve("Base::total", "C++", scopes).member == "total");
    CHECK(resolve("Base.total", "Java", scopes).member == "total");
    CHECK(resolve("this->x.y", "C++", scopes).member == "x");
    CHECK(resolve("view->visible", "C++", scopes).member == "visible");
    CHECK(resolve("count", "C#", scopes).member.empty());
}

TEST_CASE("a name that no variable in scope matches resolves to a global of that name") {
    const variableScopes scopes = scopesOf({}, {}, {}, {}, {{"", {"g", "table"}}});

    const nameResolution plain = resolve("g", "C++", scopes);
    CHECK(plain.kind == variableKind::global);
    CHECK(plain.variable == "g");

    const nameResolution member = resolve("g.items", "C++", scopes);
    CHECK(member.kind == variableKind::global);
    CHECK(member.variable == "g");
    CHECK(member.throughMember);
    CHECK(resolve("table[i]", "C++", scopes).kind == variableKind::global);
    CHECK(resolve("g", "Java", scopes, true).kind == variableKind::global);
    CHECK(resolve("g + 1", "C++", scopes, true).kind == variableKind::unknown);
    CHECK(resolve("other", "C++", scopes).kind == variableKind::unknown);
}

TEST_CASE("the types and namespaces that hold a global qualify its name") {
    const variableScopes scopes = scopesOf({}, {}, {}, {}, {{"", {"g"}}, {"settings", {"level"}}, {"Config", {"Count"}}, {"Foo", {"instances"}}});

    const nameResolution inNamespace = resolve("settings::level", "C++", scopes);
    CHECK(inNamespace.kind == variableKind::global);
    CHECK(inNamespace.variable == "level");
    CHECK(inNamespace.object == "settings");
    CHECK(resolve("::g", "C++", scopes).kind == variableKind::global);
    CHECK(resolve("::settings::level", "C++", scopes).kind == variableKind::global);
    CHECK(resolve("Config.Count", "C#", scopes).variable == "Count");
    CHECK(resolve("Shop.Config.Count", "C#", scopes).kind == variableKind::global);
    CHECK(resolve("Config.Count.Value", "Java", scopes).kind == variableKind::global);
    CHECK(resolve("Foo::instances", "C++", scopes).kind == variableKind::global);
    CHECK(resolve("Foo<T>::instances", "C++", scopes).kind == variableKind::global);

    CHECK(resolve("Shop.Config.Count", "C#", scopes).holder == "Config");
    CHECK(resolve("::settings::level", "C++", scopes).holder == "settings");
    CHECK(resolve("::g", "C++", scopes).holder.empty());
    CHECK(resolve("g", "C++", scopes).holder.empty());

    CHECK(resolve("Other.Count", "C#", scopes).kind == variableKind::unknown);
    CHECK(resolve("settings::g", "C++", scopes).kind == variableKind::unknown);
    CHECK(resolve("::level", "C++", scopes).kind == variableKind::unknown);
}

TEST_CASE("a local, a parameter, or a field hides a global of the same name, unless :: names the global namespace") {
    const variableScopes scopes = scopesOf({"a"}, {"b"}, {"c"}, {"d"}, {{"", {"a", "b", "c", "d", "e"}}});

    CHECK(resolve("a", "C++", scopes).kind == variableKind::local);
    CHECK(resolve("b", "C++", scopes).kind == variableKind::parameter);
    CHECK(resolve("c", "C++", scopes).kind == variableKind::field);
    CHECK(resolve("::a", "C++", scopes).kind == variableKind::global);
    CHECK(resolve("::c", "C++", scopes).kind == variableKind::global);

    // The object itself qualifies a member, which is a field or no known variable
    CHECK(resolve("this->e", "C++", scopes).kind == variableKind::unknown);
    CHECK(resolve("this.e", "Java", scopes).kind == variableKind::unknown);
}

TEST_CASE("a global hides a local whose scope has ended, which matches only a name that no global matches") {
    const variableScopes scopes = scopesOf({}, {}, {}, {"counter", "i"}, {{"", {"counter"}}, {"Foo", {"total"}}});

    CHECK(resolve("counter", "C++", scopes).kind == variableKind::global);
    CHECK(resolve("counter.value", "C++", scopes).kind == variableKind::global);
    CHECK(resolve("i", "C++", scopes).kind == variableKind::local);
}

TEST_CASE("a C++ name before :: names a namespace or a type, never a global") {
    const variableScopes scopes = scopesOf({}, {}, {}, {}, {{"", {"std", "Registry"}}, {"Registry", {"count"}}});

    CHECK(resolve("std::transform", "C++", scopes).kind == variableKind::unknown);
    CHECK(resolve("Registry::count", "C++", scopes).variable == "count");
    CHECK(resolve("std.length", "Java", scopes).kind == variableKind::global);
}
