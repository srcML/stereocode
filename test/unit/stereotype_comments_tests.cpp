// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_comments_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "stereotype_comments.hpp"

#include <stdexcept>
#include <string>

namespace {
    const std::string unitStart =
        R"(<unit xmlns="http://www.srcML.org/srcML/src" xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="C++" filename="a.cpp">)";

    // Gets the content of the unit element after the insertion of the comments into a unit with the given content
    std::string commented(const std::string& content) {
        const std::string srcml = withStereotypeComments("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n" + unitStart + content + "</unit>\n");
        const std::size_t start = srcml.find("<unit");
        const std::size_t end = srcml.rfind("</unit>");
        REQUIRE(start != std::string::npos);
        REQUIRE(end != std::string::npos);
        return srcml.substr(start + unitStart.size(), end - start - unitStart.size());
    }
}

TEST_CASE("Each labeled element gets a line comment with its stereotypes on the line before it, at its indentation") {
    const std::string content =
        "<class st:stereotype=\"data-provider data-class small-class\">class <name>A</name> <block>{<private type=\"default\">\n"
        "    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>\n"
        "</private><public>public:\n"
        "    <function st:stereotype=\"get\"><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <block>{}</block></function>\n"
        "</public>}</block>;</class>\n";

    CHECK(commented(content) ==
        "<comment type=\"line\">// @stereotype data-provider data-class small-class</comment>\n"
        "<class st:stereotype=\"data-provider data-class small-class\">class <name>A</name> <block>{<private type=\"default\">\n"
        "    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>\n"
        "</private><public>public:\n"
        "    <comment type=\"line\">// @stereotype get</comment>\n"
        "    <function st:stereotype=\"get\"><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <block>{}</block></function>\n"
        "</public>}</block>;</class>\n");
}

TEST_CASE("The comment keeps the tabs of the indentation") {
    CHECK(commented("<block>{\n\t\t<function st:stereotype=\"set\"><name>f</name></function>\n}</block>\n") ==
          "<block>{\n\t\t<comment type=\"line\">// @stereotype set</comment>\n\t\t<function st:stereotype=\"set\"><name>f</name></function>\n}</block>\n");
}

TEST_CASE("An element after code on its line moves to a line of its own at the indentation of that line") {
    const std::string get = "<function st:stereotype=\"get\"><name>get</name> <block>{ <return>return <expr><name>x</name></expr>;</return> }</block></function>";
    const std::string set = "<function st:stereotype=\"set\"><name>set</name> <block>{ <expr_stmt><expr><name>x</name> <operator>=</operator> <name>value</name></expr>;</expr_stmt> }</block></function>";

    CHECK(commented("<block>{\n    <property>int <name>X</name> {" + get + " " + set + " }</property>\n}</block>\n") ==
          "<block>{\n    <property>int <name>X</name> { <comment type=\"line\">// @stereotype get</comment>\n    " + get +
          " <comment type=\"line\">// @stereotype set</comment>\n    " + set + " }</property>\n}</block>\n");
}

TEST_CASE("Specifiers that srcML leaves before an element keep their line below the comment") {
    CHECK(commented("<block>{\n\n    final <class st:stereotype=\"boundary\">class <name>Inner</name> <block>{}</block></class>\n}</block>\n") ==
          "<block>{\n\n    <comment type=\"line\">// @stereotype boundary</comment>\n    final <class st:stereotype=\"boundary\">class <name>Inner</name> <block>{}</block></class>\n}</block>\n");
    CHECK(commented("Sci<function st:stereotype=\"unclassified\"><type><name><operator>::</operator><name>Position</name></name></type> <name>end</name></function>\n") ==
          "<comment type=\"line\">// @stereotype unclassified</comment>\nSci<function st:stereotype=\"unclassified\"><type><name><operator>::</operator><name>Position</name></name></type> <name>end</name></function>\n");
}

TEST_CASE("An element after an earlier member on its line moves to a line of its own, and the comment of the earlier member stays") {
    const std::string content = "<public>public:\n  <function st:stereotype=\"get\"><name>getY</name></function> <function st:stereotype=\"set\"><name>setY</name></function>\n</public>\n";
    const std::string once = commented(content);

    CHECK(once == "<public>public:\n  <comment type=\"line\">// @stereotype get</comment>\n  <function st:stereotype=\"get\"><name>getY</name></function> "
                  "<comment type=\"line\">// @stereotype set</comment>\n  <function st:stereotype=\"set\"><name>setY</name></function>\n</public>\n");
    CHECK(commented(once) == once);
}

TEST_CASE("The comment of an element after a block comment that ends on its line never goes into the block comment") {
    CHECK(commented("<comment type=\"block\">/* a\n b */</comment><function st:stereotype=\"get\"><name>f</name></function>\n") ==
          "<comment type=\"block\">/* a\n b */</comment> <comment type=\"line\">// @stereotype get</comment>\n <function st:stereotype=\"get\"><name>f</name></function>\n");
}

TEST_CASE("A friend function gets its comment before the keyword friend") {
    CHECK(commented("<public>public:\n  <friend>friend <function st:stereotype=\"property\"><name>peek</name></function></friend>\n</public>\n") ==
          "<public>public:\n  <comment type=\"line\">// @stereotype property</comment>\n"
          "  <friend>friend <function st:stereotype=\"property\"><name>peek</name></function></friend>\n</public>\n");
}

TEST_CASE("Other comments before an element stay in place") {
    CHECK(commented("<block>{\n  <comment type=\"line\">// Returns x</comment>\n  <function st:stereotype=\"get\"><name>getX</name></function>\n}</block>\n") ==
          "<block>{\n  <comment type=\"line\">// Returns x</comment>\n  <comment type=\"line\">// @stereotype get</comment>\n"
          "  <function st:stereotype=\"get\"><name>getX</name></function>\n}</block>\n");
}

TEST_CASE("A stereotype comment on the line before an element gives way to the comment of its current stereotypes") {
    CHECK(commented("<block>{\n  <comment type=\"line\">// @stereotype set command</comment>\n  <function st:stereotype=\"get\"><name>getX</name></function>\n}</block>\n") ==
          "<block>{\n  <comment type=\"line\">// @stereotype get</comment>\n  <function st:stereotype=\"get\"><name>getX</name></function>\n}</block>\n");
}

TEST_CASE("Comments added twice equal the comments added once") {
    const std::string content =
        "<class st:stereotype=\"empty\">class <name>B</name> <block>{<public>public: "
        "<function st:stereotype=\"get\"><name>getY</name><parameter_list>()</parameter_list> <block>{ <return>return <expr><name>y</name></expr>;</return> }</block></function> "
        "<friend>friend <function st:stereotype=\"property\"><name>f</name><parameter_list>()</parameter_list> <block>{}</block></function></friend> "
        "</public>}</block>;</class>\n";

    const std::string once = withStereotypeComments(unitStart + content + "</unit>");
    CHECK(withStereotypeComments(once) == once);
}

TEST_CASE("The declaration of the stereotype namespace stays the only one, whatever the spelling of the start tag") {
    const std::string starts[] = {
        "<unit xmlns=\"http://www.srcML.org/srcML/src\"\txmlns:st=\"http://www.srcML.org/srcML/stereotype\" filename=\"a.cpp\">",
        "<unit xmlns=\"http://www.srcML.org/srcML/src\" filename=\"a>b.cpp\" xmlns:st=\"http://www.srcML.org/srcML/stereotype\">"
    };
    for (const std::string& start : starts) {
        const std::string srcml = withStereotypeComments(start + R"(<function st:stereotype="get"><name>f</name></function></unit>)");

        CHECK(srcml.find(R"(<comment type="line">// @stereotype get</comment>)" "\n" R"(<function st:stereotype="get">)") != std::string::npos);
        const std::size_t declaration = srcml.find(R"(xmlns:st="http://www.srcML.org/srcML/stereotype")");
        CHECK(declaration != std::string::npos);
        CHECK(srcml.find("xmlns:st=", declaration + 1) == std::string::npos);
    }
}

TEST_CASE("A unit without labeled elements keeps its srcML") {
    const std::string content = "<function><name>f</name><block>{\n  <comment type=\"line\">// @stereotype get</comment>\n}</block></function>\n";
    CHECK(commented(content) == content);
}

TEST_CASE("srcML that is not well formed stops the insertion with an error") {
    CHECK_THROWS_AS(withStereotypeComments(unitStart + "<function>"), std::runtime_error);
}
