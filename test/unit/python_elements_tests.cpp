// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file python_elements_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "model/python_elements.hpp"
#include "archive/srcml_elements.hpp"
#include "parsed_srcml.hpp"

#include <string>
#include <vector>

using names = std::vector<std::string>;

namespace {
    const std::string signature = "def f(self, a: int, b: \"Engine\" = None, /, c: Optional[pkg.Engine] = None, *args, d: list[int], **kwargs) -> dict[str, Engine]:\n    pass\n";

    // Parses Python code into a unit
    //
    document python(const std::string& code) {
        return parsedCode("Python", code);
    }

    // Adds the elements with a name in a node, its later siblings, and their children, in document order
    //
    void addElements(const xmlNode* node, const std::string& name, std::vector<const xmlNode*>& elements) {
        for (; node; node = node->next) {
            if (isSrcElement(node, name)) elements.push_back(node);
            addElements(node->children, name, elements);
        }
    }

    // Gets the elements with a name in a parsed unit, in document order
    //
    std::vector<const xmlNode*> elementsOf(const document& unit, const std::string& name) {
        std::vector<const xmlNode*> elements;
        addElements(xmlDocGetRootElement(unit.get()), name, elements);
        return elements;
    }

    // Gets the types of the annotations of Python code, in document order
    //
    names annotatedTypes(const std::string& code) {
        const document unit = python(code);
        names types;
        for (const xmlNode* annotation : elementsOf(unit, "annotation")) types.push_back(pythonTypeOf(annotation));
        return types;
    }

    // Gets what a statement binds the name at a position among the parts of its expression to
    //
    boundValue boundBy(const std::string& statement, int target = 0) {
        const document unit = python(statement + "\n");
        const xmlNode* part = xmlFirstElementChild(const_cast<xmlNode*>(firstElement(unit, "expr")));
        for (int i = 0; i < target; ++i) part = xmlNextElementSibling(const_cast<xmlNode*>(part));
        return pythonBoundValue(part);
    }

    // Gets the numbers of arguments that the parameter list of a function takes
    //
    argumentRange argumentsOf(const std::string& function, bool firstBound) {
        const document unit = python(function);
        return pythonArgumentRange(firstElement(unit, "parameter_list"), firstBound);
    }

    // A function that returns an expression
    //
    struct returned {
        explicit returned(const std::string& expression) : unit{python("def f(self, a, b, c):\n    return " + expression + "\n")} {}

        const xmlNode* expression() const { return firstElement(firstElement(unit, "return"), "expr"); }

        document unit;
    };

    // Gets the name of the call that alone is the value of a returned expression, or nothing without such a call
    //
    std::string soleCallNameOf(const std::string& expression) {
        const returned function(expression);
        const xmlNode* call = soleCallOf(function.expression());
        return call ? pythonCallName(call) : "";
    }

    // Gets the name of the call at a position among the calls of a statement, in document order
    //
    std::string callNameOf(const std::string& statement, std::size_t position = 0) {
        const document unit = python(statement + "\n");
        return pythonCallName(elementsOf(unit, "call").at(position));
    }

    // Gets the object of the call at a position among the calls of a statement, which the variables with an index before the call name
    //
    std::string indexedObjectOf(const std::string& statement, std::size_t position = 0) {
        const document unit = python(statement + "\n");
        return pythonIndexedObject(elementsOf(unit, "call").at(position));
    }

    // Tells whether the call at a position among the calls of a statement names a method on the result of super()
    //
    bool callFollowsSuper(const std::string& statement, std::size_t position) {
        const document unit = python(statement + "\n");
        return followsSuperCall(elementsOf(unit, "call").at(position));
    }
}

TEST_CASE("the type of an annotation takes angle brackets around its arguments and drops the quotes of a forward reference") {
    CHECK(annotatedTypes(signature) == names{"int", "Engine", "Optional<pkg.Engine>", "list<int>", "dict<str, Engine>"});
    CHECK(annotatedTypes("def f(e: Optional[\"Engine\"]) -> None:\n    pass\n") == names{"Optional<Engine>", "None"});
}

TEST_CASE("the type of an annotation drops the values of a Literal") {
    CHECK(annotatedTypes("def f(mode: Literal[\"r\", \"w\"], size: Optional[Literal[1, 2]]):\n    pass\n") == names{"Literal", "Optional<Literal>"});
}

TEST_CASE("a type names its class alone, in Optional, or beside None, without its module and its arguments") {
    CHECK(pythonNamedType("Engine") == "Engine");
    CHECK(pythonNamedType("pkg.parts.Engine") == "Engine");
    CHECK(pythonNamedType("Optional<pkg.Engine>") == "Engine");
    CHECK(pythonNamedType("Engine | None") == "Engine");
    CHECK(pythonNamedType("None | Engine") == "Engine");
    CHECK(pythonNamedType("Box<int>") == "Box");
}

TEST_CASE("a type of several names or of no name names no class") {
    CHECK(pythonNamedType("int | str").empty());
    CHECK(pythonNamedType("Engine | Motor | None").empty());
    CHECK(pythonNamedType("").empty());
}

TEST_CASE("an assignment binds a name to the name or to the call that alone is its value") {
    const boundValue declared = boundBy("engine: Engine = Engine(4)");
    CHECK(declared.type == "Engine");
    CHECK(declared.callName == "Engine");
    CHECK(declared.name.empty());

    const boundValue member = boundBy("self.owner = a");
    CHECK(member.name == "a");
    CHECK(member.type.empty());
    CHECK(member.callName.empty());

    CHECK(boundBy("x = pkg.make(1)").callName == "pkg.make");
    CHECK(boundBy("y = type(self)(1)").callName == "self.__class__");
}

TEST_CASE("an annotation gives the type of a name without a value") {
    const boundValue declared = boundBy("count: int");
    CHECK(declared.type == "int");
    CHECK(declared.name.empty());
    CHECK(declared.callName.empty());
}

TEST_CASE("an augmented assignment, a chain of assignments, or a value of several parts binds a name to no name and no call") {
    for (const char* statement : {"self.n += 1", "w = a = b", "t = a + f()", "v = (a)", "x = await pkg.make(1)"}) {
        CAPTURE(statement);
        const boundValue value = boundBy(statement);
        CHECK(value.name.empty());
        CHECK(value.callName.empty());
    }

    const boundValue second = boundBy("w = a = b", 2);
    CHECK(second.name.empty());
}

TEST_CASE("a parameter list takes its named parameters as arguments, without the object, the markers / and *, and the variadic parameters") {
    const argumentRange method = argumentsOf(signature, true);
    CHECK(method.minimum == 2);
    CHECK(method.maximum == 4);
    CHECK(method.variadic);

    const argumentRange function = argumentsOf(signature, false);
    CHECK(function.minimum == 3);
    CHECK(function.maximum == 5);

    const argumentRange keywordOnly = argumentsOf("def g(a, *, b=1):\n    pass\n", false);
    CHECK(keywordOnly.minimum == 1);
    CHECK(keywordOnly.maximum == 2);
    CHECK_FALSE(keywordOnly.variadic);

    const argumentRange none = argumentsOf("def h(self):\n    pass\n", true);
    CHECK(none.minimum == 0);
    CHECK(none.maximum == 0);
}

TEST_CASE("the call that alone is the value of an expression may stand in parentheses or after await") {
    CHECK(soleCallNameOf("Engine(4)") == "Engine");
    CHECK(soleCallNameOf("await make()") == "make");
    CHECK(soleCallNameOf("(await self.load())") == "self.load");

    CHECK(soleCallNameOf("a").empty());
    CHECK(soleCallNameOf("a + f()").empty());
    CHECK(soleCallNameOf("f().g()").empty());
}

TEST_CASE("the name of a call holds the names before it, and type(x)(...) calls the class of x") {
    CHECK(callNameOf("pkg.make(1)") == "pkg.make");
    CHECK(callNameOf("self.a.b(1)") == "self.a.b");
    CHECK(callNameOf("type(self)(1)") == "self.__class__");
}

TEST_CASE("a call of the result of another call has no name") {
    CHECK(callNameOf("f()()").empty());
    CHECK(callNameOf("type(a, b)(1)").empty());
}

TEST_CASE("a call after the access operator has the variables with an index before it as its object") {
    CHECK(indexedObjectOf("rows[0].strip()") == "rows[0].");
    CHECK(indexedObjectOf("self.rows[i][j].strip()") == "self.rows[i][j].");
    CHECK(indexedObjectOf("a[0].b[1].c()") == "a[0].b[1].");
    CHECK(indexedObjectOf("x = a[ 0 ].b.c(1)") == "a[0].");
}

TEST_CASE("a call on the result of a call or on a value, and a call without an access operator before it, have no variable with an index as their object") {
    CHECK(indexedObjectOf("f(1).strip()", 1).empty());
    CHECK(indexedObjectOf("f(1).rows[0].strip()", 1).empty());
    CHECK(indexedObjectOf("\"\".join(a)").empty());
    CHECK(indexedObjectOf("(a).strip()").empty());
    CHECK(indexedObjectOf("a.strip()").empty());
    CHECK(indexedObjectOf("a[0](1)").empty());
}

TEST_CASE("a call follows super() only as a method on the result of super()") {
    CHECK(callFollowsSuper("super().up(1)", 1));
    CHECK_FALSE(callFollowsSuper("super().up(1)", 0));
    CHECK_FALSE(callFollowsSuper("other().up(1)", 1));
    CHECK_FALSE(callFollowsSuper("self.up(1)", 0));
}

TEST_CASE("a Boolean literal, a negation, a comparison, and a call of a function that tests its argument are Boolean by form") {
    for (const char* expression : {"True", "False", "not a", "a == b", "a + 1 > 2", "a is not None", "b not in a", "isinstance(c, int)", "callable(a)", "(a != b)"}) {
        CAPTURE(expression);
        CHECK(isBooleanByForm(returned(expression).expression()));
    }
    for (const char* expression : {"a", "None", "a + 1", "len(a)", "self.ready", "a.is_ready()"}) {
        CAPTURE(expression);
        CHECK_FALSE(isBooleanByForm(returned(expression).expression()));
    }
}

TEST_CASE("a call of a method of the built-in types that tests its object is Boolean by form, on a name, an element, or a result") {
    for (const char* expression : {"a.startswith(\"x\")", "a[:1].isupper()", "self.name.lower().endswith(\".py\")", "\"\".join(a).isalpha()",
                                   "(a + b).isdigit()", "a.strip().isdigit() and not a.startswith(\"0\")", "b.issubset(c)"}) {
        CAPTURE(expression);
        CHECK(isBooleanByForm(returned(expression).expression()));
    }
    for (const char* expression : {"startswith(a)", "a.is_ready()", "a.startswith", "a.isdigit() + 1", "a + b.isdigit()", "a.startswith(\"x\").real"}) {
        CAPTURE(expression);
        CHECK_FALSE(isBooleanByForm(returned(expression).expression()));
    }
}

TEST_CASE("a conjunction, a disjunction, or a conditional expression is Boolean by form only when each of its results is") {
    for (const char* expression : {"not a or b is None or isinstance(c, int)", "(a == 1 and callable(a))", "(a is not None) if b not in a else (True)"}) {
        CAPTURE(expression);
        CHECK(isBooleanByForm(returned(expression).expression()));
    }
    for (const char* expression : {"a and b", "a == 1 or b", "(a) or (not b)", "a if a else True", "True if a else b"}) {
        CAPTURE(expression);
        CHECK_FALSE(isBooleanByForm(returned(expression).expression()));
    }
}

TEST_CASE("only the literal None alone gives the caller no value") {
    CHECK(isNoneLiteral(returned("None").expression()));
    CHECK_FALSE(isNoneLiteral(returned("a").expression()));
    CHECK_FALSE(isNoneLiteral(returned("a or None").expression()));
}

TEST_CASE("a name of a class follows CapWords after its leading underscores") {
    for (const char* name : {"Shape", "HTTPServer", "URL", "_Private", "X1"}) {
        CAPTURE(name);
        CHECK(namesPythonClass(name));
    }
    for (const char* name : {"shape", "make_shape", "MAX_SIZE", "_cache", "__", ""}) {
        CAPTURE(name);
        CHECK_FALSE(namesPythonClass(name));
    }
}

TEST_CASE("the holes of an f-string give their names, also in a format, while a doubled brace is text") {
    CHECK(formattedStringNames(R"(f"{self.x:>{width}} {a!r} {{b}}")") == names{"self.x", "width", "a"});
    CHECK(formattedStringNames(R"(f"{value:{width}.{precision}}")") == names{"value", "width", "precision"});
    CHECK(formattedStringNames(R"(rf'{a != b}\d')") == names{"a", "b"});
    CHECK(formattedStringNames("f\"\"\"{a}\n{b}\"\"\"") == names{"a", "b"});
}

TEST_CASE("a call in a hole uses only its object and its arguments, and a string or a keyword names nothing") {
    CHECK(formattedStringNames(R"(f"{len(self.items)} {self.items.count(a)} {f().name}")") == names{"self.items", "self.items", "a"});
    CHECK(formattedStringNames(R"(f"{d['key']} {x if y else 'none'} {1.5 + z}")") == names{"d", "x", "y", "z"});
}

TEST_CASE("a file defines the module of its name, and __init__.py defines the package of its folder") {
    CHECK(moduleNameOf("settings.py") == "settings");
    CHECK(moduleNameOf("app/settings.py") == "settings");
    CHECK(moduleNameOf("app/__init__.py") == "app");
    CHECK(moduleNameOf("C:\\src\\app\\models.py") == "models");
}
