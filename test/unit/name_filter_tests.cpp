// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file name_filter_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "helper_functions.hpp"

#include <string>
#include <vector>

namespace {
    // Gets the four forms of a name that nameFilter gives
    //
    std::vector<std::string> filtered(std::string name, const char* language, bool removeNamespace) {
        std::vector<std::string> forms;
        HELPERS::nameFilter(name, forms, language, removeNamespace);
        return forms;
    }
}

TEST_CASE("a member of a class template keeps its own name") {
    CHECK(filtered("Box<T>::touch", "C++", true) == std::vector<std::string>{"Box<T>::touch", "Box<T>::touch", "touch", "touch"});
    CHECK(filtered("Holder<int>::Inner", "C++", true)[3] == "Inner");
    CHECK(filtered("IEnumerable<T>.GetEnumerator", "C#", true)[3] == "GetEnumerator");
}

TEST_CASE("an operator keeps its symbol") {
    CHECK(filtered("operator<", "C++", true)[3] == "operator<");
    CHECK(filtered("operator<<", "C++", true)[3] == "operator<<");
    CHECK(filtered("Point::operator==", "C++", true)[3] == "operator==");
}

TEST_CASE("a generic type keeps the commas of its argument list in the third form") {
    CHECK(filtered("ns::Map<int, std::pair<int, int>>", "C++", true) == std::vector<std::string>{"ns::Map<int, std::pair<int, int>>", "ns::Map<int,std::pair<int,int>>", "Map<,>", "Map"});
    CHECK(filtered("std::max<int>", "C++", false)[3] == "std::max");
}

TEST_CASE("the line of a constructor is the line of its declaration after its attributes or annotations") {
    const std::string attributed = R"(<constructor pos:start="4:5" pos:end="6:25"><attribute pos:start="4:5" pos:end="4:14">[<expr pos:start="4:6" pos:end="4:13"><name pos:start="4:6" pos:end="4:13">Obsolete</name></expr>]</attribute>)"
                                   "\n    "
                                   R"(<attribute pos:start="5:5" pos:end="5:18">[<expr pos:start="5:6" pos:end="5:17"><name pos:start="5:6" pos:end="5:17">Serializable</name></expr>]</attribute>)"
                                   "\n    "
                                   R"(<specifier pos:start="6:5" pos:end="6:10">public</specifier> <name pos:start="6:12" pos:end="6:12">A</name><parameter_list pos:start="6:13" pos:end="6:14">()</parameter_list> <block pos:start="6:16" pos:end="6:25">{}</block></constructor>)";
    CHECK(HELPERS::extractFirstLineNumber(attributed) == 6);

    const std::string annotated = R"(<constructor pos:start="3:5" pos:end="5:18"><annotation pos:start="3:5" pos:end="3:38">@<name pos:start="3:6" pos:end="3:21">SuppressWarnings</name><argument_list pos:start="3:22" pos:end="3:38">(<argument><expr><annotation pos:start="3:23" pos:end="3:26">@<name pos:start="3:24" pos:end="3:26">Foo</name></annotation></expr></argument>)</argument_list></annotation>)"
                                  "\n    "
                                  R"(<name pos:start="5:5" pos:end="5:5">B</name><parameter_list pos:start="5:6" pos:end="5:7">()</parameter_list> <block pos:start="5:9" pos:end="5:18">{}</block></constructor>)";
    CHECK(HELPERS::extractFirstLineNumber(annotated) == 5);

    const std::string plain = R"(<constructor pos:start="2:5" pos:end="2:15"><name pos:start="2:5" pos:end="2:5">A</name><parameter_list pos:start="2:6" pos:end="2:7">()</parameter_list> <block pos:start="2:9" pos:end="2:15">{}</block></constructor>)";
    CHECK(HELPERS::extractFirstLineNumber(plain) == 2);
}

TEST_CASE("the structure of a type comes without whitespace and semicolons") {
    CHECK(HELPERS::typeStructure("class ;") == "class");
    CHECK(HELPERS::typeStructure("\n struct  ") == "struct");
    CHECK(HELPERS::typeStructure("record") == "record");
    CHECK(HELPERS::typeStructure(" ;") == "class");
}
