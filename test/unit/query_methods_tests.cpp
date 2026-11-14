// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file query_methods_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "rules/query_methods.hpp"

#include <sstream>

namespace {
    // Gets the default query methods with the entries of a user list file held in a string
    //
    queryMethods queryMethodsWith(const std::string& userFile) {
        queryMethods methods;
        std::istringstream in(userFile);
        in >> methods;
        methods.initializeQueryMethods();
        return methods;
    }
}

TEST_CASE("each language has its own default query methods") {
    const queryMethods methods = queryMethodsWith("");

    CHECK(methods.isQuery("m_items.size", "C++"));
    CHECK(methods.isQuery("m_name.c_str", "C++"));
    CHECK(methods.isQuery("items.Count", "C#"));
    CHECK(methods.isQuery("prices.TryGetValue", "C#"));
    CHECK(methods.isQuery("items.isEmpty", "Java"));
    CHECK(methods.isQuery("map.containsKey", "Java"));

    CHECK_FALSE(methods.isQuery("items.Count", "Java"));
    CHECK_FALSE(methods.isQuery("items.isEmpty", "C++"));
    CHECK_FALSE(methods.isQuery("m_items.push_back", "C++"));
    CHECK_FALSE(methods.isQuery("items.add", "Java"));
}

TEST_CASE("GetValue of C# is a query method with one argument, since a ConditionalWeakTable adds a missing key through GetValue with a callback") {
    const queryMethods methods = queryMethodsWith("");

    CHECK(methods.isQuery("IsOpenProperty.GetValue", 1, "C#"));
    CHECK(methods.isQuery("items.Count", 0, "C#"));
    CHECK_FALSE(methods.isQuery("s_trackers.GetValue", 2, "C#"));
    CHECK_FALSE(methods.isQuery("values.GetValue", 1, "Java"));
    CHECK_FALSE(methods.isQuery("values.GetValue", "C#"));
}

TEST_CASE("every object of C# and Java inherits the accessors of the root class, which their numbers of arguments tell from other methods") {
    const queryMethods methods = queryMethodsWith("");

    for (const char* accessor : {"hashCode", "toString", "getClass", "clone"}) CHECK(methods.isRootAccessor(accessor, 0, "Java"));
    CHECK(methods.isRootAccessor("equals", 1, "Java"));
    for (const char* accessor : {"GetHashCode", "ToString", "GetType", "MemberwiseClone"}) CHECK(methods.isRootAccessor(accessor, 0, "C#"));
    CHECK(methods.isRootAccessor("Equals", 1, "C#"));

    SUBCASE("every Python object inherits the accessors of the class object, unlike the methods that set an attribute") {
        CHECK(methods.isRootAccessor("__repr__", 0, "Python"));
        CHECK(methods.isRootAccessor("__getattribute__", 1, "Python"));
        CHECK_FALSE(methods.isRootAccessor("__setattr__", 2, "Python"));
        CHECK_FALSE(methods.isRootAccessor("__init__", 0, "Python"));
    }
    SUBCASE("the static Equals of the C# object takes two arguments") {
        CHECK_FALSE(methods.isRootAccessor("Equals", 2, "C#"));
    }
    SUBCASE("the static methods of the C# object are functions, which use no object") {
        CHECK(methods.isRootFunction("Equals", 2, "C#"));
        CHECK(methods.isRootFunction("ReferenceEquals", 2, "C#"));
        CHECK_FALSE(methods.isRootFunction("Equals", 1, "C#"));
        CHECK_FALSE(methods.isRootFunction("ReferenceEquals", 2, "Java"));
    }
    SUBCASE("a method of a container, a method that may change the object, and a method of another language are no such accessors") {
        CHECK_FALSE(methods.isRootAccessor("size", 0, "Java"));
        CHECK_FALSE(methods.isRootAccessor("notify", 0, "Java"));
        CHECK_FALSE(methods.isRootAccessor("toString", 0, "C#"));
        CHECK_FALSE(methods.isRootAccessor("ToString", 0, "C++"));
    }
    SUBCASE("the accessors that return the class of the object are query methods on a data member as well") {
        CHECK(methods.isQuery("item.getClass", "Java"));
        CHECK(methods.isQuery("item.GetType", "C#"));
    }
}

TEST_CASE("the default query methods of C++ give no write access to the data member") {
    const queryMethods methods = queryMethodsWith("");

    for (const char* call : {"m_items.size", "m_items.empty", "m_name.length", "m_keys.count", "m_name.c_str", "m_items.capacity", "m_items.max_size"})
        CHECK(methods.isQuery(call, "C++"));

    for (const char* call : {"m_items.begin", "m_items.end", "m_items.at", "m_items.data", "m_items.front", "m_items.back", "m_keys.find"})
        CHECK_FALSE(methods.isQuery(call, "C++"));

    SUBCASE("an accessor that gives write access is a query method only through the file of the user") {
        CHECK(queryMethodsWith("front\n").isQuery("m_items.front", "C++"));
    }
}

TEST_CASE("the method of a call is its name after the last access operator") {
    const queryMethods methods = queryMethodsWith("");

    CHECK(methods.isQuery("this->m_items.size", "C++"));
    CHECK(methods.isQuery("m_owner->m_items.empty", "C++"));
    CHECK(methods.isQuery("m_items.data->size", "C++"));
    CHECK(methods.isQuery("this.items.get", "Java"));

    CHECK_FALSE(methods.isQuery("m_items.sizeOf", "C++"));
    CHECK_FALSE(methods.isQuery("m_size.resize", "C++"));
}

TEST_CASE("the query methods of the user hold for every language") {
    const queryMethods methods = queryMethodsWith("# Qt\nisNull\n\n  measure \r\n");

    CHECK(methods.isQuery("m_text.isNull", "C++"));
    CHECK(methods.isQuery("text.measure", "C#"));
    CHECK(methods.isQuery("text.measure", "Java"));
    CHECK(methods.isQuery("m_items.size", "C++"));
    CHECK_FALSE(methods.isQuery("m_text.clear", "C++"));
}

TEST_CASE("a query method under a section header holds only for the units of its language") {
    const queryMethods methods = queryMethodsWith("isNull\n[C++]\nfront\n[Java]\npeek\n");

    CHECK(methods.isQuery("m_items.front", "C++"));
    CHECK_FALSE(methods.isQuery("items.front", "Java"));
    CHECK(methods.isQuery("items.peek", "Java"));
    CHECK_FALSE(methods.isQuery("m_items.peek", "C++"));
    CHECK(methods.isQuery("m_text.isNull", "C++"));
    CHECK(methods.isQuery("text.isNull", "C#"));
}

TEST_CASE("the default query methods of Python are those of its immutable types and those of its containers that change nothing") {
    const queryMethods methods = queryMethodsWith("");

    for (const char* call : {"this.name.startswith", "this.parts.count", "this.rows.get", "this.rows.keys", "text.encode"}) {
        CAPTURE(call);
        CHECK(methods.isQuery(call, "Python"));
    }
    for (const char* call : {"this.parts.append", "this.rows.update", "this.parts.sort", "this.rows.pop"}) {
        CAPTURE(call);
        CHECK_FALSE(methods.isQuery(call, "Python"));
    }
    CHECK_FALSE(methods.isQuery("name.startswith", "Java"));
}

TEST_CASE("the default query methods of JavaScript are those of its strings, arrays, maps, and sets that change nothing") {
    const queryMethods methods = queryMethodsWith("");

    for (const char* call : {"this.items.includes", "this.name.startsWith", "this.cache.get", "this.cache.has", "text.trim", "this.items.map"}) {
        CAPTURE(call);
        CHECK(methods.isQuery(call, "JavaScript"));
    }
    for (const char* call : {"this.items.push", "this.cache.set", "this.items.sort", "this.cache.delete", "this.items.splice"}) {
        CAPTURE(call);
        CHECK_FALSE(methods.isQuery(call, "JavaScript"));
    }
}
