// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_comments_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "output/stereotype_comments.hpp"

#include <stdexcept>
#include <string>

namespace {
    // Gets the start tag of a unit of a language
    std::string unitStartIn(const std::string& language) {
        return R"(<unit xmlns="http://www.srcML.org/srcML/src" xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language=")" + language +
               R"(" filename="a">)";
    }

    const std::string unitStart = unitStartIn("C++");

    // Gets the content of the unit element after the insertion of the comments into a unit of a language with the given content
    std::string commented(const std::string& content, const std::string& language = "C++") {
        const std::string start = unitStartIn(language);
        const std::string srcml = withStereotypeComments("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n" + start + content + "</unit>\n");
        const std::size_t first = srcml.find("<unit");
        const std::size_t end = srcml.rfind("</unit>");
        REQUIRE(first != std::string::npos);
        REQUIRE(end != std::string::npos);
        return srcml.substr(first + start.size(), end - first - start.size());
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

TEST_CASE("An element after code on its line moves below its comment, and the comment and the element take the indentation of that line") {
    const std::string get = "<function st:stereotype=\"get\"><name>get</name> <block>{ <return>return <expr><name>x</name></expr>;</return> }</block></function>";
    const std::string set = "<function st:stereotype=\"set\"><name>set</name> <block>{ <expr_stmt><expr><name>x</name> <operator>=</operator> <name>value</name></expr>;</expr_stmt> }</block></function>";

    CHECK(commented("<block>{\n    <property>int <name>X</name> {" + get + " " + set + " }</property>\n}</block>\n") ==
          "<block>{\n    <property>int <name>X</name> {\n    <comment type=\"line\">// @stereotype get</comment>\n    " + get +
          "\n    <comment type=\"line\">// @stereotype set</comment>\n    " + set + " }</property>\n}</block>\n");
}

TEST_CASE("The blanks between the code and an element that moves below its comment leave the line of the code") {
    CHECK(commented("<block>{\n\t<decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt> \t <function st:stereotype=\"get\"><name>getX</name></function>\n}</block>\n") ==
          "<block>{\n\t<decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>\n"
          "\t<comment type=\"line\">// @stereotype get</comment>\n\t<function st:stereotype=\"get\"><name>getX</name></function>\n}</block>\n");
    CHECK(commented("<decl_stmt><decl><type><name>int</name></type> <name>a</name></decl>;</decl_stmt> <typedef>typedef <type><struct st:stereotype=\"empty\">struct <block>{}</block></struct></type> <name>S</name>;</typedef>\n") ==
          "<decl_stmt><decl><type><name>int</name></type> <name>a</name></decl>;</decl_stmt> <typedef>typedef<type>\n"
          "<comment type=\"line\">// @stereotype empty</comment>\n<struct st:stereotype=\"empty\">struct <block>{}</block></struct></type> <name>S</name>;</typedef>\n");
}

TEST_CASE("An element after code on the first line of the unit moves below its comment without indentation") {
    CHECK(commented("<decl_stmt><decl><type><name>int</name></type> <name>counter</name></decl>;</decl_stmt> <function st:stereotype=\"global-command\"><name>next</name></function>\n") ==
          "<decl_stmt><decl><type><name>int</name></type> <name>counter</name></decl>;</decl_stmt>\n"
          "<comment type=\"line\">// @stereotype global-command</comment>\n<function st:stereotype=\"global-command\"><name>next</name></function>\n");
}

TEST_CASE("An element that starts its line after a line break inside an earlier element gets its comment at the start of that line") {
    CHECK(commented("<macro><name>M</name>\n  </macro><function st:stereotype=\"get\"><name>f</name></function>\n") ==
          "<macro><name>M</name>\n  </macro><comment type=\"line\">// @stereotype get</comment>\n  <function st:stereotype=\"get\"><name>f</name></function>\n");
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

    CHECK(once == "<public>public:\n  <comment type=\"line\">// @stereotype get</comment>\n  <function st:stereotype=\"get\"><name>getY</name></function>\n"
                  "  <comment type=\"line\">// @stereotype set</comment>\n  <function st:stereotype=\"set\"><name>setY</name></function>\n</public>\n");
    CHECK(commented(once) == once);
}

TEST_CASE("The comment of an element after a block comment that ends on its line never goes into the block comment") {
    CHECK(commented("<comment type=\"block\">/* a\n b */</comment><function st:stereotype=\"get\"><name>f</name></function>\n") ==
          "<comment type=\"block\">/* a\n b */</comment>\n <comment type=\"line\">// @stereotype get</comment>\n <function st:stereotype=\"get\"><name>f</name></function>\n");
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

TEST_CASE("A unit without labeled elements keeps a stereotype comment that stands above no element") {
    const std::string content = "<function><name>f</name><block>{\n  <comment type=\"line\">// @stereotype get</comment>\n}</block></function>\n";
    CHECK(commented(content) == content);
}

TEST_CASE("A stereotype comment on the line before an element without stereotypes goes with its line, while other comments stay") {
    CHECK(commented("<block>{\n  <comment type=\"line\">// Returns x</comment>\n  <comment type=\"line\">// @stereotype get</comment>\n"
                    "  <function><name>getX</name></function>\n}</block>\n") ==
          "<block>{\n  <comment type=\"line\">// Returns x</comment>\n  <function><name>getX</name></function>\n}</block>\n");
}

TEST_CASE("A stereotype comment after code on the line before an element without stereotypes goes, while the code keeps its line") {
    CHECK(commented("<public>public: <comment type=\"line\">// @stereotype get</comment>\n  <function><name>getY</name></function>\n</public>\n") ==
          "<public>public:\n  <function><name>getY</name></function>\n</public>\n");
}

TEST_CASE("A stereotype comment on the first line of a unit before a type without stereotypes goes with its line") {
    CHECK(commented("  <comment type=\"line\">// @stereotype data-class</comment>\n  <class>class <name>A</name> <block>{}</block>;</class>\n") ==
          "  <class>class <name>A</name> <block>{}</block>;</class>\n");
}

TEST_CASE("A unit of a language that stereocode does not analyze keeps its stereotype comments") {
    const std::string content = "<comment type=\"line\">// @stereotype get</comment>\n"
                                "<function><type><name>int</name></type> <name>size</name><parameter_list>()</parameter_list> <block>{}</block></function>\n";
    CHECK(commented(content, "Objective-C") == content);
}

TEST_CASE("A stereotype comment stays above an enum of C++ or C# and a static class of C#, which get no stereotypes, and goes above a Java enum") {
    const std::string comment = "<comment type=\"line\">// @stereotype data-enum</comment>\n";
    const std::string colors  = "<enum>enum <name>Color</name> <block>{ <decl><name>Red</name></decl> }</block></enum>\n";
    const std::string tools   = "<class><specifier>static</specifier> class <name>Tools</name> <block>{ }</block></class>\n";
    CHECK(commented(comment + colors) == comment + colors);
    CHECK(commented(comment + colors, "C#") == comment + colors);
    CHECK(commented(comment + tools, "C#") == comment + tools);
    CHECK(commented(comment + colors, "Java") == colors);
}

TEST_CASE("A stereotype comment stays above a local type, its methods, a local function, and a type without a name, also one that a typedef names only through a pointer") {
    const std::string local =
        "<function><type><name>int</name></type> <name>outer</name><parameter_list>()</parameter_list> <block>{<block_content>\n"
        "    <comment type=\"line\">// @stereotype data-class</comment>\n"
        "    <struct>struct <name>Local</name> <block>{<public type=\"default\">\n"
        "        <comment type=\"line\">// @stereotype get</comment>\n"
        "        <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <block>{}</block></function>\n"
        "    </public>}</block>;</struct>\n"
        "</block_content>}</block></function>\n"
        "<comment type=\"line\">// @stereotype empty</comment>\n"
        "<struct>struct <block>{<public type=\"default\">\n"
        "    <comment type=\"line\">// @stereotype get</comment>\n"
        "    <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <block>{}</block></function>\n"
        "</public>}</block> <decl><name>anonymous</name></decl>;</struct>\n"
        "<comment type=\"line\">// @stereotype empty</comment>\n"
        "<typedef>typedef <type><struct>struct <block>{<public type=\"default\">\n"
        "    <comment type=\"line\">// @stereotype get</comment>\n"
        "    <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <block>{}</block></function>\n"
        "</public>}</block></struct></type> *<name>PointPtr</name>;</typedef>\n";
    CHECK(commented("<comment type=\"line\">// @stereotype global-command</comment>\n" + local) == local);

    const std::string shop =
        "<class>class <name>Shop</name> <block>{\n"
        "    <function><type><name>int</name></type> <name>Size</name><parameter_list>()</parameter_list> <block>{<block_content>\n"
        "        <comment type=\"line\">// @stereotype get</comment>\n"
        "        <function><type><name>int</name></type> <name>Count</name><parameter_list>()</parameter_list> <block>{}</block></function>\n"
        "    </block_content>}</block></function>\n"
        "}</block></class>\n";
    CHECK(commented(shop, "C#") == shop);
}

TEST_CASE("A stereotype comment stays above a C# or Java function outside every type, which stereocode does not analyze") {
    const std::string helper = "<comment type=\"line\">// @stereotype get</comment>\n"
                               "<function><type><name>int</name></type> <name>Helper</name><parameter_list>()</parameter_list> <block>{}</block></function>\n";
    CHECK(commented(helper, "C#") == helper);
    CHECK(commented(helper, "Java") == helper);
}

TEST_CASE("Stacked stereotype comments before a labeled element give way to one comment with its stereotypes") {
    CHECK(commented("<block>{\n  <comment type=\"line\">// @stereotype command</comment>\n  <comment type=\"line\">// @stereotype set</comment>\n"
                    "  <function st:stereotype=\"get\"><name>getX</name></function>\n}</block>\n") ==
          "<block>{\n  <comment type=\"line\">// @stereotype get</comment>\n  <function st:stereotype=\"get\"><name>getX</name></function>\n}</block>\n");
}

TEST_CASE("Stereotype comments above a declaration or an empty line and comments that only start like one stay") {
    const std::string content =
        "<block>{\n  <comment type=\"line\">// @stereotype get</comment>\n  <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>\n"
        "  <comment type=\"line\">// @stereotype set</comment>\n\n  <function><name>f</name></function>\n"
        "  <comment type=\"line\">// @stereotypes follow</comment>\n  <function><name>g</name></function>\n}</block>\n";
    CHECK(commented(content) == content);
}

TEST_CASE("Earlier comments give way in one run, and a second run keeps its output") {
    const std::string content =
        "<comment type=\"line\">// @stereotype data-class</comment>\n<class st:stereotype=\"empty\">class <name>B</name> <block>{<public>public: "
        "<comment type=\"line\">// @stereotype command</comment>\n<comment type=\"line\">// @stereotype set</comment>\n"
        "<function st:stereotype=\"get\"><name>getY</name></function> <comment type=\"line\">// @stereotype get</comment>\n"
        "<friend>friend <function><name>f</name></function></friend>\n</public>}</block>;</class>\n";
    const std::string once = commented(content);

    CHECK(once == "<comment type=\"line\">// @stereotype empty</comment>\n<class st:stereotype=\"empty\">class <name>B</name> <block>{<public>public:\n"
                  "<comment type=\"line\">// @stereotype get</comment>\n<function st:stereotype=\"get\"><name>getY</name></function>\n"
                  "<friend>friend <function><name>f</name></function></friend>\n</public>}</block>;</class>\n");
    CHECK(commented(once) == once);
}

TEST_CASE("A labeled Python element gets a comment that starts with #, above its first decorator and at its indentation") {
    const std::string method =
        "    <function st:stereotype=\"property\"><attribute>@<expr><name>staticmethod</name></expr></attribute>\n"
        "    def <name>area</name><parameter_list>(<parameter><name>side</name></parameter>)</parameter_list><block>:<block_content> "
        "<return>return <expr><name>side</name></expr></return></block_content></block></function>\n";
    const std::string once = commented("<class st:stereotype=\"empty\">class <name>Shape</name><block>:<block_content>\n" + method + "</block_content></block></class>\n", "Python");

    CHECK(once == "<comment type=\"line\"># @stereotype empty</comment>\n<class st:stereotype=\"empty\">class <name>Shape</name><block>:<block_content>\n"
                  "    <comment type=\"line\"># @stereotype property</comment>\n" + method + "</block_content></block></class>\n");
    CHECK(commented(once, "Python") == once);
}

TEST_CASE("A Python method that starts its line after the line break in the body of the method before it gets its comment at the start of that line") {
    const std::string first  = "<function st:stereotype=\"get\">def <name>f</name><parameter_list>()</parameter_list><block>:<block_content>\n        <pass>pass</pass>\n</block_content></block></function>";
    const std::string second = "<function st:stereotype=\"set\">def <name>g</name><parameter_list>()</parameter_list><block>:<block_content>\n        <pass>pass</pass>\n</block_content></block></function>";
    const std::string once = commented("<class>class <name>A</name><block>:<block_content>\n    " + first + "    " + second + "</block_content></block></class>\n", "Python");

    CHECK(once == "<class>class <name>A</name><block>:<block_content>\n    <comment type=\"line\"># @stereotype get</comment>\n    " + first +
                  "    <comment type=\"line\"># @stereotype set</comment>\n    " + second + "</block_content></block></class>\n");
    CHECK(commented(once, "Python") == once);
}

TEST_CASE("A Python stereotype comment goes above a method without stereotypes and stays above a function or a class in a function") {
    const std::string method = "    <function>def <name>f</name><parameter_list>()</parameter_list><block>:<block_content> <pass>pass</pass></block_content></block></function>\n";
    CHECK(commented("<class>class <name>A</name><block>:<block_content>\n    <comment type=\"line\"># @stereotype get</comment>\n" + method + "</block_content></block></class>\n", "Python") ==
          "<class>class <name>A</name><block>:<block_content>\n" + method + "</block_content></block></class>\n");

    const std::string local =
        "<function st:stereotype=\"factory\">def <name>outer</name><parameter_list>()</parameter_list><block>:<block_content>\n"
        "    <comment type=\"line\"># @stereotype get</comment>\n" + method +
        "    <comment type=\"line\"># @stereotype empty</comment>\n"
        "    <class>class <name>Local</name><block>:<block_content>\n"
        "    <comment type=\"line\"># @stereotype set</comment>\n" + method +
        "</block_content></block></class></block_content></block></function>\n";
    CHECK(commented(local, "Python") == "<comment type=\"line\"># @stereotype factory</comment>\n" + local);
}

TEST_CASE("A comment is a stereotype comment only with the comment start of the language of its unit") {
    const std::string function = "<function>def <name>f</name><parameter_list>()</parameter_list><block>:<block_content> <pass>pass</pass></block_content></block></function>\n";
    const std::string other    = "<comment type=\"line\">// @stereotype get</comment>\n" + function;
    CHECK(commented(other, "Python") == other);
}

TEST_CASE("Only srcML that holds the start of a stereotype comment of some language may hold stereotype comments") {
    CHECK(mayHoldStereotypeComments(unitStart + "<comment type=\"line\">// @stereotype get</comment></unit>"));
    CHECK(mayHoldStereotypeComments(unitStartIn("Python") + "<comment type=\"line\"># @stereotype get</comment></unit>"));
    CHECK_FALSE(mayHoldStereotypeComments(unitStart + "<comment type=\"line\">// Returns x</comment></unit>"));
    CHECK_FALSE(mayHoldStereotypeComments(unitStartIn("Python") + "<comment type=\"line\"># Returns x @stereotype</comment></unit>"));
}

TEST_CASE("srcML that is not well formed stops the insertion with an error") {
    CHECK_THROWS_AS(withStereotypeComments(unitStart + "<function>"), std::runtime_error);
}
