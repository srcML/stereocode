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
