// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_attributes_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "stereotype_attributes.hpp"

#include <stdexcept>
#include <string>

namespace {
    const std::string srcNamespaceDeclaration = R"(xmlns="http://www.srcML.org/srcML/src")";
    const std::string stereotypeDeclaration   = R"(xmlns:st="http://www.srcML.org/srcML/stereotype")";

    // Gets the number of times that a text holds a part
    //
    int occurrences(const std::string& text, const std::string& part) {
        int count = 0;
        for (std::size_t position = text.find(part); position != std::string::npos; position = text.find(part, position + 1)) ++count;
        return count;
    }
}

TEST_CASE("The stereotypes of an earlier run give way to the stereotypes of the annotations") {
    const std::string srcml = withStereotypeAttributes(
        "<unit " + srcNamespaceDeclaration + " " + stereotypeDeclaration + R"( language="C++">)"
        R"(<class st:stereotype="data-class">class <name>A</name> <block>{<function st:stereotype="get"><name>f</name></function>}</block>;</class></unit>)",
        {{{1, 2, 1}, "set"}});

    CHECK(srcml.find(R"(<class>class <name>A</name>)") != std::string::npos);
    CHECK(srcml.find(R"(<function st:stereotype="set"><name>f</name></function>)") != std::string::npos);
    CHECK(occurrences(srcml, "stereotype=") == 1);
    CHECK(occurrences(srcml, "xmlns:st=") == 1);
}

TEST_CASE("Without annotations, the unit keeps no stereotype") {
    const std::string srcml = withStereotypeAttributes(
        "<unit " + srcNamespaceDeclaration + " " + stereotypeDeclaration + R"(><function st:stereotype="get"><name>f</name></function></unit>)", {});

    CHECK(srcml.find(R"(<function><name>f</name></function>)") != std::string::npos);
    CHECK(occurrences(srcml, "stereotype=") == 0);
}

TEST_CASE("A unit without the stereotype namespace gets its declaration for the attributes") {
    const std::string srcml = withStereotypeAttributes("<unit " + srcNamespaceDeclaration + R"(><function><name>f</name></function></unit>)", {{{1}, "get"}});

    CHECK(srcml.find(R"(<function st:stereotype="get"><name>f</name></function>)") != std::string::npos);
    CHECK(occurrences(srcml, stereotypeDeclaration) == 1);
}

TEST_CASE("The attributes take the prefix under which the unit declares the stereotype namespace") {
    const std::string srcml = withStereotypeAttributes(
        "<unit " + srcNamespaceDeclaration + "\t" + R"(xmlns:s="http://www.srcML.org/srcML/stereotype" filename="a>b.cpp">)"
        R"(<function s:stereotype="set"><name>f</name></function></unit>)",
        {{{1}, "get"}});

    CHECK(srcml.find(R"(<function s:stereotype="get"><name>f</name></function>)") != std::string::npos);
    CHECK(occurrences(srcml, "xmlns:st=") == 0);
    CHECK(occurrences(srcml, "xmlns:s=") == 1);
}

TEST_CASE("The input may hold stereotypes only when it names the stereotype namespace") {
    CHECK(mayHoldStereotypes("<unit " + srcNamespaceDeclaration + " " + stereotypeDeclaration + "/>"));
    CHECK_FALSE(mayHoldStereotypes("<unit " + srcNamespaceDeclaration + R"(><comment type="line">// @stereotype get</comment></unit>)"));
}

TEST_CASE("An annotation of an element that the unit lacks or srcML that is not well formed stops with an error") {
    CHECK_THROWS_AS(withStereotypeAttributes("<unit " + srcNamespaceDeclaration + R"(><function><name>f</name></function></unit>)", {{{2}, "get"}}),
                    std::runtime_error);
    CHECK_THROWS_AS(withStereotypeAttributes("<unit " + srcNamespaceDeclaration + "><function>", {}), std::runtime_error);
}
