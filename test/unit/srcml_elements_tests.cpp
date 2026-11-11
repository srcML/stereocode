// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file srcml_elements_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "archive/srcml_elements.hpp"
#include "archive/node_text.hpp"

#include <libxml/parser.h>

#include <memory>
#include <string>
#include <vector>

namespace {
    // A parsed srcML unit that frees its document
    //
    using document = std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)>;

    // Parses the srcML of a parameter list of a language into a unit
    //
    document parsedList(const std::string& language, const std::string& parameterList) {
        const std::string srcml = R"(<unit xmlns="http://www.srcML.org/srcML/src" language=")" + language + R"(">)" + parameterList + "</unit>";
        return document(xmlReadMemory(srcml.c_str(), static_cast<int>(srcml.size()), nullptr, nullptr, 0), &xmlFreeDoc);
    }

    // Gets the parameter list of a parsed unit
    //
    const xmlNode* listOf(const document& unit) {
        return firstChildElement(xmlDocGetRootElement(unit.get()), "parameter_list");
    }

    // Gets the text of a written type, where each name of a type stands in brackets, as const[Count]& of const Count&
    //
    std::string bracketed(const writtenType& type) {
        std::string text;
        for (const typePart& part : type) text += part.isName ? "[" + part.text + "]" : part.text;
        return text;
    }

    // Gets the type of the C++ declaration of a parsed unit
    //
    const xmlNode* declaredTypeOf(const document& unit) {
        return firstChildElement(firstChildElement(xmlDocGetRootElement(unit.get()), "decl"), "type");
    }

    // Parses the srcML of the type of a C++ declaration and gets the type
    //
    struct parsedType {
        explicit parsedType(const std::string& type) : unit{parsedList("C++", "<decl>" + type + R"( <name>a</name></decl>)")} {}

        const xmlNode*  type() const { return declaredTypeOf(unit); }
        const xmlNode*  name() const { return firstChildElement(type(), "name"); }

        document        unit;
    };

    // Gets the first element below an element that meets a condition, in document order
    //
    template <typename condition>
    const xmlNode* firstBelow(const xmlNode* element, condition meets) {
        for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(element)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child))) {
            if (meets(child)) return child;
            if (const xmlNode* found = firstBelow(child, meets)) return found;
        }
        return nullptr;
    }

    // Parses the srcML of a C# expression statement and gets its calls and names by their text
    //
    struct parsedStatement {
        explicit parsedStatement(const std::string& statement) : unit{parsedList("C#", statement)} {}

        const xmlNode*  call(const std::string& name) const {
            return firstBelow(xmlDocGetRootElement(unit.get()), [&name](const xmlNode* node) {
                return isSrcElement(node, "call") && sourceOf(firstChildElement(node, "name")) == name;
            });
        }
        const xmlNode*  name(const std::string& text) const {
            return firstBelow(xmlDocGetRootElement(unit.get()), [&text](const xmlNode* node) { return isSrcElement(node, "name") && sourceOf(node) == text; });
        }

        document        unit;
    };
}

TEST_CASE("the parameter types of a signature leave out the names, specifiers, attributes, and default values") {
    const document cpp = parsedList("C++",
        R"(<parameter_list>(<parameter><decl><type><specifier>const</specifier> <name><name>std</name><operator>::</operator><name>string</name></name><modifier>&amp;</modifier></type> <name>name</name></decl></parameter>, )"
        R"(<parameter><decl><type><name>int</name></type> <name>x</name> <init>= <expr><literal type="number">5</literal></expr></init></decl></parameter>, )"
        R"(<parameter><decl><type><modifier>...</modifier></type></decl></parameter>)</parameter_list>)");
    CHECK(signatureTypesOf(listOf(cpp)) == "(std::string&,int,...)");

    const document csharp = parsedList("C#",
        R"(<parameter_list>(<parameter><decl><type><specifier>in</specifier> <name>Point</name></type> <name>p</name></decl></parameter>, )"
        R"(<parameter><decl><type><modifier>ref</modifier> <name>int</name></type> <name>a</name></decl></parameter>, )"
        R"(<parameter><decl><type><attribute>[<expr><name>NotNull</name></expr>]</attribute> <name>string</name></type> <name>s</name> <init>= <expr><literal type="null">null</literal></expr></init></decl></parameter>, )"
        R"(<parameter><decl><type><specifier>params</specifier> <name><name>int</name><index>[]</index></name></type> <name>rest</name></decl></parameter>)</parameter_list>)");
    CHECK(signatureTypesOf(listOf(csharp)) == "(in Point,ref int,string,int[])");

    const document java = parsedList("Java",
        R"(<parameter_list>(<parameter><decl><type><annotation>@<name>NonNull</name></annotation> <specifier>final</specifier> <name><name>List</name><argument_list type="generic">&lt;<argument><name>String</name></argument>&gt;</argument_list></name></type> <name>xs</name></decl></parameter>, )"
        R"(<parameter><decl><type><name>String</name><modifier>...</modifier></type> <name>rest</name></decl></parameter>)</parameter_list>)");
    CHECK(signatureTypesOf(listOf(java)) == "(List<String>,String...)");
}

TEST_CASE("the C# words ref, out, and in of a signature keep a space before the type, also inside a qualified type name") {
    const document csharp = parsedList("C#",
        R"(<parameter_list>(<parameter><decl><type><name><modifier>ref</modifier> <name>System</name><operator>.</operator><name>Drawing</name><operator>.</operator><name>Point</name></name></type> <name>q</name></decl></parameter>, )"
        R"(<parameter><decl><type><modifier>out</modifier> <name><name>List</name><argument_list type="generic">&lt;<argument><name>int</name></argument>&gt;</argument_list></name></type> <name>xs</name></decl></parameter>, )"
        R"(<parameter><decl><type><specifier>this</specifier> <name>string</name></type> <name>s</name></decl></parameter>)</parameter_list>)");
    CHECK(signatureTypesOf(listOf(csharp)) == "(ref System.Drawing.Point,out List<int>,string)");
}

TEST_CASE("the parameter types of a signature keep the array brackets after a name and the parameters of a function pointer") {
    const document cpp = parsedList("C++",
        R"(<parameter_list>(<parameter><decl><type><name>int</name></type> <name><name>a</name><index>[]</index></name></decl></parameter>, )"
        R"(<parameter><decl><type><name>A</name> <specifier>const</specifier><modifier>*</modifier></type> <name>c</name></decl></parameter>, )"
        R"(<parameter><function_decl><type><name>int</name></type> (<modifier>*</modifier><name>fp</name>)<parameter_list>(<parameter><decl><type><name>int</name></type></decl></parameter>)</parameter_list></function_decl></parameter>)</parameter_list>)");
    CHECK(signatureTypesOf(listOf(cpp)) == "(int[],A*,int(*)(int))");

    const document empty = parsedList("C", R"(<parameter_list>(<parameter><decl><type><name>void</name></type></decl></parameter>)</parameter_list>)");
    CHECK(signatureTypesOf(listOf(empty)) == "()");
}

TEST_CASE("the parameter types that tell overloads apart keep the specifiers, unlike those of a signature") {
    const document cpp = parsedList("C++",
        R"(<parameter_list>(<parameter><decl><type><specifier>const</specifier> <name>Node</name><modifier>*</modifier></type> <name>node</name></decl></parameter>)</parameter_list>)");
    CHECK(parameterTypesOf(listOf(cpp)) == "(constNode*)");
    CHECK(signatureTypesOf(listOf(cpp)) == "(Node*)");
}

TEST_CASE("a written parameter type keeps each name of a type without template arguments apart, and the parts join to the text of the parameter types") {
    const document cpp = parsedList("C++",
        R"(<parameter_list>(<parameter><decl><type><specifier>const</specifier> <name>Count</name><modifier>&amp;</modifier></type> <name>c</name></decl></parameter>, )"
        R"(<parameter><decl><type><name><name>std</name><operator>::</operator><name>size_t</name></name></type> <name>n</name></decl></parameter>, )"
        R"(<parameter><decl><type><name>Count</name> <specifier>const</specifier><modifier>*</modifier></type> <name>p</name></decl></parameter>, )"
        R"(<parameter><decl><type><name>unsigned</name> <name>int</name></type> <name>u</name></decl></parameter>, )"
        R"(<parameter><function_decl><type><name>int</name></type> (<modifier>*</modifier><name>g</name>)<parameter_list>(<parameter><decl><type><name>Count</name></type></decl></parameter>)</parameter_list></function_decl></parameter>, )"
        R"(<parameter><decl><type><specifier>const</specifier> <name><name>std</name><operator>::</operator><name>vector</name><argument_list type="generic">&lt;<argument><expr><name>Count</name></expr></argument>&gt;</argument_list></name><modifier>&amp;</modifier></type> <name>v</name></decl></parameter>, )"
        R"(<parameter><decl><type><name><name><name>Grid</name><argument_list type="generic">&lt;<argument><expr><specifier>const</specifier> <name>Count</name><modifier>*</modifier></expr></argument>&gt;</argument_list></name><operator>::</operator><name>Row</name></name></type> <name>r</name></decl></parameter>)</parameter_list>)");
    const std::vector<writtenType> types = writtenTypesOf(listOf(cpp));
    REQUIRE(types.size() == 7);
    CHECK(bracketed(types[0]) == "const[Count]&");
    CHECK(bracketed(types[1]) == "[std::size_t]");
    CHECK(bracketed(types[2]) == "[Count]const*");
    CHECK(bracketed(types[3]) == "[unsigned][int]");
    CHECK(bracketed(types[4]) == "int(*g)(Count)");
    CHECK(bracketed(types[5]) == "conststd::vector<[Count]>&");
    CHECK(bracketed(types[6]) == "Grid<const[Count]*>::Row");
    CHECK(parameterTypesOf(listOf(cpp)) == "(constCount&,std::size_t,Countconst*,unsignedint,int(*g)(Count),conststd::vector<Count>&,Grid<constCount*>::Row)");

    const document empty = parsedList("C", R"(<parameter_list>(<parameter><decl><type><name>void</name></type></decl></parameter>)</parameter_list>)");
    CHECK(writtenTypesOf(listOf(empty)).empty());
}

TEST_CASE("a parameter list takes its parameters without default values up to all its parameters") {
    const document defaults = parsedList("C++",
        R"(<parameter_list>(<parameter><decl><type><name>int</name></type> <name>a</name></decl></parameter>, )"
        R"(<parameter><decl><type><name>int</name></type> <name>b</name> <init>= <expr><literal type="number">5</literal></expr></init></decl></parameter>)</parameter_list>)");
    const argumentRange range = argumentRangeOf(listOf(defaults));
    CHECK(range.minimum == 1);
    CHECK(range.maximum == 2);
    CHECK_FALSE(range.variadic);

    const document none = parsedList("C", R"(<parameter_list>(<parameter><decl><type><name>void</name></type></decl></parameter>)</parameter_list>)");
    CHECK(argumentRangeOf(listOf(none)).maximum == 0);
}

TEST_CASE("a variadic parameter takes any number of arguments, also none") {
    SUBCASE("... of C and C++") {
        const document cpp = parsedList("C++",
            R"(<parameter_list>(<parameter><decl><type><name>int</name></type> <name>first</name></decl></parameter>, )"
            R"(<parameter><decl><type><modifier>...</modifier></type></decl></parameter>)</parameter_list>)");
        const argumentRange range = argumentRangeOf(listOf(cpp));
        CHECK(range.minimum == 1);
        CHECK(range.variadic);
    }
    SUBCASE("a parameter pack of C++") {
        const document cpp = parsedList("C++",
            R"(<parameter_list>(<parameter><decl><type><name>Ts</name><modifier>&amp;&amp;</modifier><modifier>...</modifier></type> <name>args</name></decl></parameter>)</parameter_list>)");
        const argumentRange range = argumentRangeOf(listOf(cpp));
        CHECK(range.minimum == 0);
        CHECK(range.variadic);
    }
    SUBCASE("params of C#") {
        const document csharp = parsedList("C#",
            R"(<parameter_list>(<parameter><decl><type><specifier>params</specifier> <name><name>int</name><index>[]</index></name></type> <name>rest</name></decl></parameter>)</parameter_list>)");
        const argumentRange range = argumentRangeOf(listOf(csharp));
        CHECK(range.minimum == 0);
        CHECK(range.variadic);
    }
    SUBCASE("int... of Java") {
        const document java = parsedList("Java",
            R"(<parameter_list>(<parameter><decl><type><name>String</name></type> <name>a</name></decl></parameter>, )"
            R"(<parameter><decl><type><name>int</name><modifier>...</modifier></type> <name>rest</name></decl></parameter>)</parameter_list>)");
        const argumentRange range = argumentRangeOf(listOf(java));
        CHECK(range.minimum == 1);
        CHECK(range.maximum == 2);
        CHECK(range.variadic);
    }
}

TEST_CASE("a smart pointer of the standard library is known by its name, with or without std::") {
    CHECK(standardSmartPointer(parsedType(R"(<type><name><name>std</name><operator>::</operator><name>shared_ptr</name><argument_list type="generic">&lt;<argument><expr><name>Widget</name></expr></argument>&gt;</argument_list></name></type>)").name()) == "shared_ptr");
    CHECK(standardSmartPointer(parsedType(R"(<type><name><operator>::</operator><name>std</name><operator>::</operator><name>weak_ptr</name><argument_list type="generic">&lt;<argument><expr><name>Widget</name></expr></argument>&gt;</argument_list></name></type>)").name()) == "weak_ptr");
    CHECK(standardSmartPointer(parsedType(R"(<type><name><name>unique_ptr</name><argument_list type="generic">&lt;<argument><expr><name>Widget</name></expr></argument>&gt;</argument_list></name></type>)").name()) == "unique_ptr");
    CHECK(standardSmartPointer(parsedType(R"(<type><name><name>boost</name><operator>::</operator><name>shared_ptr</name><argument_list type="generic">&lt;<argument><expr><name>Widget</name></expr></argument>&gt;</argument_list></name></type>)").name()).empty());
    CHECK(standardSmartPointer(parsedType(R"(<type><name><name>std</name><operator>::</operator><name>vector</name><argument_list type="generic">&lt;<argument><expr><name>Widget</name></expr></argument>&gt;</argument_list></name></type>)").name()).empty());
    CHECK(standardSmartPointer(parsedType(R"(<type><name>Widget</name></type>)").name()).empty());
}

TEST_CASE("-> reaches the type that a std::unique_ptr or a std::shared_ptr points to, without its specifiers") {
    CHECK(pointeeTypeOf(parsedType(R"(<type><name><name>std</name><operator>::</operator><name>unique_ptr</name><argument_list type="generic">&lt;<argument><expr><specifier>const</specifier> <name>Engine</name></expr></argument>&gt;</argument_list></name></type>)").type()) == "Engine");
    CHECK(pointeeTypeOf(parsedType(R"(<type><name><name>std</name><operator>::</operator><name>shared_ptr</name><argument_list type="generic">&lt;<argument><expr><name>IConfig</name> <specifier>const</specifier></expr></argument>&gt;</argument_list></name></type>)").type()) == "IConfig");
    CHECK(pointeeTypeOf(parsedType(R"(<type><name><name>shared_ptr</name><argument_list type="generic">&lt;<argument><expr><name><name>ns</name><operator>::</operator><name>Box</name><argument_list type="generic">&lt;<argument><expr><name>int</name></expr></argument>&gt;</argument_list></name></expr></argument>&gt;</argument_list></name><modifier>&amp;</modifier></type>)").type()) == "ns::Box<int>");
    CHECK(pointeeTypeOf(parsedType(R"(<type><name><name>std</name><operator>::</operator><name>unique_ptr</name><argument_list type="generic">&lt;<argument><expr><name>gchar</name></expr></argument>, <argument><expr><name>GFreeReleaser</name></expr></argument>&gt;</argument_list></name></type>)").type()) == "gchar");
}

TEST_CASE("-> reaches the named type of a raw pointer, also of a pointer to a smart pointer") {
    CHECK(pointeeTypeOf(parsedType(R"(<type><name>Engine</name><modifier>*</modifier></type>)").type()) == "Engine");
    CHECK(pointeeTypeOf(parsedType(R"(<type><name><name>std</name><operator>::</operator><name>unique_ptr</name><argument_list type="generic">&lt;<argument><expr><name>Engine</name></expr></argument>&gt;</argument_list></name><modifier>*</modifier></type>)").type()) == "std::unique_ptr<Engine>");
}

TEST_CASE("-> reaches no named object through a type that is no raw pointer, std::unique_ptr, or std::shared_ptr, whose own operator it calls") {
    CHECK(pointeeTypeOf(parsedType(R"(<type><name>IStreamingReporterPtr</name></type>)").type()).empty());
    CHECK(pointeeTypeOf(parsedType(R"(<type><name><name>boost</name><operator>::</operator><name>shared_ptr</name><argument_list type="generic">&lt;<argument><expr><name>Engine</name></expr></argument>&gt;</argument_list></name></type>)").type()).empty());
    CHECK(pointeeTypeOf(parsedType(R"(<type><name><operator>::</operator><name>std</name><operator>::</operator><name>weak_ptr</name><argument_list type="generic">&lt;<argument><expr><name>Engine</name></expr></argument>&gt;</argument_list></name></type>)").type()).empty());
    CHECK(pointeeTypeOf(parsedType(R"(<type><name><name>std</name><operator>::</operator><name>unique_ptr</name><argument_list type="generic">&lt;<argument><expr><name><name>Engine</name><index>[]</index></name></expr></argument>&gt;</argument_list></name></type>)").type()).empty());
}

TEST_CASE("a call after a C# null-conditional chain of names is a call on the names, each with a dot after it, as after a chain of dots") {
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><name>x</name></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr>;</expr_stmt>)").call("Z")) == "x.");
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><name>x</name></expr>?</condition><then><expr><ternary><condition><expr><operator>.</operator><name>y</name></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr></then></ternary></expr>;</expr_stmt>)").call("Z")) == "x.y.");
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><name>x</name></expr>?</condition><then><expr><ternary><condition><expr><operator>.</operator><name>y</name></expr>?</condition><then><expr><ternary><condition><expr><operator>.</operator><name>w</name></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr></then></ternary></expr></then></ternary></expr>;</expr_stmt>)").call("Z")) == "x.y.w.");
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><name>x</name></expr>?</condition><then><expr><operator>.</operator><call><name><name>y</name><operator>.</operator><name>Z</name></name><argument_list>()</argument_list></call></expr></then></ternary></expr>;</expr_stmt>)").call("y.Z")) == "x.");
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><name><name>x</name><operator>.</operator><name>y</name></name></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr>;</expr_stmt>)").call("Z")) == "x.y.");
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><name><name>this</name><operator>.</operator><name>x</name></name></expr>?</condition><then><expr><ternary><condition><expr><operator>.</operator><name>y</name></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr></then></ternary></expr>;</expr_stmt>)").call("Z")) == "this.x.y.");
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><name>x</name></expr>?</condition><then><expr><ternary><condition><expr><operator>.</operator><name><name>y</name><index>[<expr><literal type="number">0</literal></expr>]</index></name></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr></then></ternary></expr>;</expr_stmt>)").call("Z")) == "x.y[0].");
}

TEST_CASE("a call before a later ?. of a C# null-conditional chain is a call on the names before it, as F() of x?.F()?.Z() is on x") {
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><name>x</name></expr>?</condition><then><expr><ternary><condition><expr><operator>.</operator><call><name>F</name><argument_list>()</argument_list></call></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr></then></ternary></expr>;</expr_stmt>)").call("F")) == "x.");
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><name>x</name></expr>?</condition><then><expr><ternary><condition><expr><operator>.</operator><name>y</name></expr>?</condition><then><expr><ternary><condition><expr><operator>.</operator><call><name>F</name><argument_list>()</argument_list></call></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr></then></ternary></expr></then></ternary></expr>;</expr_stmt>)").call("F")) == "x.y.");
}

TEST_CASE("a name of a C# null-conditional chain after the . or the index that opens the then of its ?. starts at that element") {
    const parsedStatement member(R"(<expr_stmt><expr><ternary><condition><expr><name>part</name></expr>?</condition><then><expr><operator>.</operator><name>b</name> <operator>=</operator> <name>v</name></expr></then></ternary></expr>;</expr_stmt>)");
    CHECK(sourceOf(nullConditionalStart(member.name("b"))) == ".");
    CHECK_FALSE(nullConditionalStart(member.name("part")));
    CHECK_FALSE(nullConditionalStart(member.name("v")));

    const parsedStatement element(R"(<expr_stmt><expr><ternary><condition><expr><name>part</name></expr>?</condition><then><expr><index>[<expr><literal type="number">0</literal></expr>]</index><operator>.</operator><name>b</name> <operator>=</operator> <name>v</name></expr></then></ternary></expr>;</expr_stmt>)");
    CHECK(sourceOf(nullConditionalStart(element.name("b"))) == "[0]");

    const parsedStatement dot(R"(<expr_stmt><expr><name><name>part</name><operator>.</operator><name>b</name></name> <operator>=</operator> <name>v</name></expr>;</expr_stmt>)");
    CHECK_FALSE(nullConditionalStart(dot.name("b")));
}

TEST_CASE("a C# null-conditional chain goes on after the condition of each ?. at the element that opens its then") {
    const parsedStatement once(R"(<expr_stmt><expr><ternary><condition><expr><call><name>F</name><argument_list>()</argument_list></call></expr>?</condition><then><expr><operator>.</operator><name>b</name> <operator>=</operator> <name>v</name></expr></then></ternary></expr>;</expr_stmt>)");
    const xmlNode* next = nullConditionalNext(once.call("F"));
    REQUIRE(next);
    CHECK(sourceOf(next) == ".");
    CHECK_FALSE(nullConditionalNext(once.name("b")));

    const parsedStatement twice(R"(<expr_stmt><expr><ternary><condition><expr><call><name>F</name><argument_list>()</argument_list></call></expr>?</condition><then><expr><ternary><condition><expr><operator>.</operator><name>c</name></expr>?</condition><then><expr><operator>.</operator><name>b</name> <operator>=</operator> <name>v</name></expr></then></ternary></expr></then></ternary></expr>;</expr_stmt>)");
    CHECK(nullConditionalNext(twice.call("F")) == xmlPreviousElementSibling(const_cast<xmlNode*>(twice.name("c"))));
    CHECK(nullConditionalNext(twice.name("c")) == xmlPreviousElementSibling(const_cast<xmlNode*>(twice.name("b"))));

    const parsedStatement dot(R"(<expr_stmt><expr><call><name>F</name><argument_list>()</argument_list></call><operator>.</operator><name>b</name> <operator>=</operator> <name>v</name></expr>;</expr_stmt>)");
    CHECK_FALSE(nullConditionalNext(dot.call("F")));
}

TEST_CASE("a C# null-conditional chain that starts at or passes through more than a name gives no names, since its call is on a result") {
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><call><name>F</name><argument_list>()</argument_list></call></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr>;</expr_stmt>)").call("Z")).empty());
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><call><name>F</name><argument_list>()</argument_list></call></expr>?</condition><then><expr><ternary><condition><expr><operator>.</operator><name>y</name></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr></then></ternary></expr>;</expr_stmt>)").call("Z")).empty());
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><call><name>F</name><argument_list>()</argument_list></call></expr>?</condition><then><expr><operator>.</operator><call><name><name>y</name><operator>.</operator><name>Z</name></name><argument_list>()</argument_list></call></expr></then></ternary></expr>;</expr_stmt>)").call("y.Z")).empty());
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><name>x</name></expr>?</condition><then><expr><ternary><condition><expr><operator>.</operator><call><name>F</name><argument_list>()</argument_list></call></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr></then></ternary></expr>;</expr_stmt>)").call("Z")).empty());
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><operator>(</operator><name>x</name> <operator>as</operator> <name>T</name><operator>)</operator></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr>;</expr_stmt>)").call("Z")).empty());
    CHECK(nullConditionalObject(parsedStatement(R"(<expr_stmt><expr><ternary><condition><expr><name>x</name></expr>?</condition><then><expr><ternary><condition><expr><index>[<expr><literal type="number">0</literal></expr>]</index></expr>?</condition><then><expr><operator>.</operator><call><name>Z</name><argument_list>()</argument_list></call></expr></then></ternary></expr></then></ternary></expr>;</expr_stmt>)").call("Z")).empty());
}
