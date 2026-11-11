// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file interpolated_strings_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "model/interpolated_strings.hpp"
#include "archive/node_text.hpp"
#include "archive/srcml_elements.hpp"

#include <libxml/parser.h>

#include <memory>
#include <string>
#include <vector>

using names = std::vector<std::string>;

namespace {
    // A parsed srcML unit that frees its document
    //
    using document = std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)>;

    // Parses srcML into the children of a C# unit
    //
    document parsed(const std::string& children) {
        const std::string srcml = R"(<unit xmlns="http://www.srcML.org/srcML/src" language="C#">)" + children + "</unit>";
        return document(xmlReadMemory(srcml.c_str(), static_cast<int>(srcml.size()), nullptr, nullptr, 0), &xmlFreeDoc);
    }

    // Escapes the characters of a text that XML marks up
    //
    std::string escaped(const std::string& text) {
        std::string escapedText;
        for (char c : text) {
            if      (c == '&') escapedText += "&amp;";
            else if (c == '<') escapedText += "&lt;";
            else if (c == '>') escapedText += "&gt;";
            else               escapedText += c;
        }
        return escapedText;
    }

    // Gets the first element with a name in a unit, in document order
    //
    const xmlNode* firstElement(const xmlNode* node, const std::string& name) {
        for (; node; node = node->next) {
            if (isSrcElement(node, name)) return node;
            if (const xmlNode* found = firstElement(node->children, name)) return found;
        }
        return nullptr;
    }

    // Reads the interpolated string of a unit, which starts at its first name
    //
    interpolatedString readFirst(const document& unit) {
        return readInterpolatedString(firstElement(xmlDocGetRootElement(unit.get()), "name"));
    }

    // Gets the names of the holes of an interpolated string, whose prefix srcML marks as a name before a literal with the rest of its text
    //
    names namesOf(const std::string& text) {
        const std::size_t quote = text.find('"');
        const document unit = parsed("<expr><name>" + escaped(text.substr(0, quote)) + R"(</name><literal type="string">)" + escaped(text.substr(quote)) + "</literal></expr>");
        return readFirst(unit).names;
    }
}

TEST_CASE("the holes of an interpolated string give their names, while the alignment and the format of a hole name nothing") {
    CHECK(namesOf(R"($"{owner} has {balance,10:C2} and {count:D3}")") == names{"owner", "balance", "count"});
    CHECK(namesOf(R"($"{total, -8} {width:owner}")") == names{"total", "width"});
}

TEST_CASE("doubled braces of a regular or a verbatim string are text") {
    CHECK(namesOf(R"($"{{owner}} {count}")") == names{"count"});
    CHECK(namesOf(R"($@"{{owner}}")").empty());
    CHECK(namesOf(R"($"{{{count}}}")") == names{"count"});
}

TEST_CASE("a verbatim string doubles its quotes, and a regular string escapes them with a backslash") {
    CHECK(namesOf(R"($@"C:\""{owner}""\")") == names{"owner"});
    CHECK(namesOf(R"(@$"{owner}")") == names{"owner"});
    CHECK(namesOf(R"($"\"{owner}\\")") == names{"owner"});
}

TEST_CASE("a raw string opens a hole with as many braces as its prefix has $ signs") {
    CHECK(namesOf(R"($"""The owner is "{owner}" """)") == names{"owner"});
    CHECK(namesOf(R"($$"""{"owner": "{{owner}}"}""")") == names{"owner"});
    CHECK(namesOf(R"($$"""{{{count}}}""")") == names{"count"});
    CHECK(namesOf("$\"\"\"\n    line {count}\n    \"\"\"") == names{"count"});
}

TEST_CASE("a name keeps its chain of access operators, while a call uses only its object") {
    CHECK(namesOf(R"($"{this.owner.Length} {items?.Count} {arr[i].x}")") == names{"this.owner.Length", "items?.Count", "arr", "i"});
    CHECK(namesOf(R"($"{items.Clear()} {Describe()} {this.Refresh()} {base.ToString()} {f().Name}")") == names{"items"});
    CHECK(namesOf(R"($"{@class} {Max(left, right)}")") == names{"@class", "left", "right"});
}

TEST_CASE("a literal in a hole holds no name, while a nested interpolated string gives the names of its holes") {
    CHECK(namesOf(R"($"{(count > 0 ? "some" : "none")}")") == names{"count"});
    CHECK(namesOf(R"($"{Join(", ", limits)} {'"'} {@"a""b"}")") == names{"limits"});
    CHECK(namesOf(R"($"{$"[{owner}]"}")") == names{"owner"});
}

TEST_CASE("the closing quote of a string that srcML splits lies in the literal that ends the string, before the code after the string") {
    const document unit = parsed(R"(<expr><call><name>Wrap</name><argument_list>(<argument><expr><name>$</name><literal type="string">"{Join("</literal></expr></argument>, )"
                                 R"(<argument><expr><literal type="string">", xs)}"</literal> <operator>+</operator> <name>count</name></expr></argument>)</argument_list></call></expr>)");
    const interpolatedString string = readInterpolatedString(firstElement(xmlDocGetRootElement(unit.get()), "argument")->children->children);

    CHECK(string.names == names{"xs"});
    REQUIRE(string.closingNode);
    CHECK(isSrcElement(string.closingNode->parent, "literal"));
    CHECK(sourceOf(string.closingNode) == R"(", xs)}")");
}

TEST_CASE("the closing quotes of a raw string lie in the literal after the code that srcML reads in the string") {
    const document unit = parsed(R"(<expr><name>$</name><literal type="string">"""</literal><name>The</name> <name>owner</name> <operator>is</operator> )"
                                 R"(<literal type="string">"{owner}"</literal> <literal type="string">"""</literal> <operator>+</operator> <name>tail</name></expr>)");
    const interpolatedString string = readFirst(unit);

    CHECK(string.names == names{"owner"});
    REQUIRE(string.closingNode);
    CHECK(isSrcElement(string.closingNode->parent, "literal"));
    CHECK(sourceOf(string.closingNode) == R"(""")");
    CHECK(isSrcElement(xmlNextElementSibling(string.closingNode->parent), "operator"));
}

TEST_CASE("a string whose text ends before its closing quote has no closing node") {
    const document unit = parsed(R"(<expr><name>$</name><literal type="string">"{Join("</literal></expr>)");
    CHECK_FALSE(readFirst(unit).closingNode);
}
