// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file javascript_elements_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "model/javascript_elements.hpp"
#include "archive/node_text.hpp"
#include "archive/srcml_elements.hpp"
#include "parsed_srcml.hpp"

#include <string>
#include <vector>

using names = std::vector<std::string>;

namespace {
    // Parses JavaScript or TypeScript code into a unit, which srcML marks as JavaScript
    //
    document javascript(const std::string& code) {
        return parsedCode("JavaScript", code);
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

    // Gets the types of the declarations of the statements of TypeScript code, in document order
    //
    names declaredTypes(const std::string& code) {
        const document unit = javascript(code);
        names types;
        for (const xmlNode* declaration : elementsOf(unit, "decl"))
            if (declaration->parent->parent == xmlDocGetRootElement(unit.get())) types.push_back(javascriptDeclaredType(declaration));
        return types;
    }

    // A function that returns an expression
    //
    struct returned {
        explicit returned(const std::string& expression) : unit{javascript("async function f(a, b, c) {\n    return " + expression + ";\n}\n")} {}

        const xmlNode* expression() const { return firstElement(firstElement(unit, "return"), "expr"); }

        document unit;
    };

    // Gets what a statement binds the name that starts it to
    //
    boundValue boundBy(const std::string& statement) {
        const document unit = javascript(statement + ";\n");
        return javascriptBoundValue(xmlFirstElementChild(const_cast<xmlNode*>(firstElement(unit, "expr"))));
    }

    // Gets the numbers of arguments that the parameter list of a function takes
    //
    argumentRange argumentsOf(const std::string& function) {
        const document unit = javascript(function);
        return javascriptArgumentRange(firstElement(unit, "parameter_list"));
    }

    // Gets the names that the parameters of a function declare, in document order
    //
    names parameterNames(const std::string& function) {
        const document unit = javascript(function);
        names declared;
        for (const xmlNode* parameter : elementsOf(unit, "parameter")) {
            for (const xmlNode* declaration : patternDeclarations(firstChildElement(parameter, "decl")))
                declared.push_back(sourceOf(firstChildElement(declaration, "name")));
        }
        return declared;
    }

    // Gets the functions and the constructors of the body of a class, in document order
    //
    struct classBody {
        explicit classBody(const std::string& members) : unit{javascript("class A {\n" + members + "\n}\n")} {
            for (const xmlNode* function : elementsOf(unit, "function")) methods.push_back(function);
            for (const xmlNode* constructor : elementsOf(unit, "constructor")) methods.push_back(constructor);
        }

        document unit;
        std::vector<const xmlNode*> methods;
    };
}

TEST_CASE("the type of an annotation drops its colon, while a function type holds code and an object type holds no object of a class") {
    CHECK(declaredTypes("let a: number;\nlet b: Map<string, Engine>;\nlet c: (x: number) => void;\nlet d: { x: number };\n") ==
          names{"number", "Map<string, Engine>", "Function", "object"});
}

TEST_CASE("a declaration without an annotation has the class of the object that its initializer creates, or no type") {
    CHECK(declaredTypes("const a = new Engine(4);\nconst b = (new Engine);\nconst c = await new Engine();\nconst d = make();\nlet e;\n") ==
          names{"Engine", "Engine", "Engine", "", ""});
}

TEST_CASE("a type names its class alone or beside undefined or null, without its namespace and its arguments") {
    CHECK(javascriptNamedType("Engine") == "Engine");
    CHECK(javascriptNamedType("parts.Engine") == "Engine");
    CHECK(javascriptNamedType("Engine | undefined") == "Engine");
    CHECK(javascriptNamedType("null | Engine") == "Engine");
    CHECK(javascriptNamedType("Box<number>") == "Box");
}

TEST_CASE("a type of several names, of an array, or of no name names no class") {
    CHECK(javascriptNamedType("number | string").empty());
    CHECK(javascriptNamedType("Engine[]").empty());
    CHECK(javascriptNamedType("[number, number]").empty());
    CHECK(javascriptNamedType("").empty());
}

TEST_CASE("an assignment binds a name to the name or to the class of the created object that alone is its value") {
    CHECK(boundBy("this.owner = owner").name == "owner");
    CHECK(boundBy("this.engine = new Engine(4)").type == "Engine");
    CHECK(boundBy("this.engine = new parts.Engine").type == "parts.Engine");

    for (const char* statement : {"this.count += 1", "this.a = b + c", "this.a = make()", "this.a = b = c"}) {
        CAPTURE(statement);
        const boundValue value = boundBy(statement);
        CHECK(value.name.empty());
        CHECK(value.type.empty());
    }
}

TEST_CASE("a parameter with a default value or with ? may get no argument, and a rest parameter takes any number") {
    const argumentRange range = argumentsOf("function f(a, b = 1, c?: number, { d }, ...rest) {}\n");
    CHECK(range.minimum == 2);
    CHECK(range.maximum == 4);
    CHECK(range.variadic);

    const argumentRange none = argumentsOf("function g() {}\n");
    CHECK(none.minimum == 0);
    CHECK(none.maximum == 0);
    CHECK_FALSE(none.variadic);
}

TEST_CASE("a parameter declares its name, or the names of the keys and the elements of its destructuring pattern") {
    CHECK(parameterNames("function f(plain, { x, y = 1 }, [first, ...others], ...rest) {}\n") == names{"plain", "x", "y", "first", "others", "rest"});
    CHECK(parameterNames("function g({ a: b, inner: { z }, list: [c] }) {}\n") == names{"b", "z", "c"});
}

TEST_CASE("an expression creates an object of a class as its whole value, also in parentheses or after await") {
    CHECK(createdClassOf(returned("new Engine(4)").expression()) == "Engine");
    CHECK(createdClassOf(returned("(new Engine)").expression()) == "Engine");
    CHECK(createdClassOf(returned("await new Engine()").expression()) == "Engine");

    for (const char* expression : {"make()", "new Engine().start()", "a || new Engine()", "Engine"}) {
        CAPTURE(expression);
        CHECK(createdClassOf(returned(expression).expression()).empty());
    }
}

TEST_CASE("a Boolean literal, a negation, a comparison, and a call of a function that tests its argument are Boolean by form") {
    for (const char* expression : {"true", "false", "!a", "a === b", "a + 1 > 2", "a instanceof Engine", "\"x\" in a", "Array.isArray(a)", "isNaN(a)",
                                   "(a !== b)"}) {
        CAPTURE(expression);
        CHECK(isJavaScriptBooleanByForm(returned(expression).expression()));
    }
    for (const char* expression : {"a", "undefined", "a + 1", "a.length", "this.ready", "a.isReady()", "a ?? false"}) {
        CAPTURE(expression);
        CHECK_FALSE(isJavaScriptBooleanByForm(returned(expression).expression()));
    }
}

TEST_CASE("a call of a method of the built-in types that tests its object is Boolean by form, unless the call names a method through this") {
    for (const char* expression : {"a.startsWith(\"x\")", "this.items.includes(b)", "a.trim().endsWith(\".js\")", "this.map?.has(b)", "pattern.test(a)"}) {
        CAPTURE(expression);
        CHECK(isJavaScriptBooleanByForm(returned(expression).expression()));
    }
    for (const char* expression : {"this.has(b)", "includes(a)", "a.includes(b) + 1", "a.indexOf(b)"}) {
        CAPTURE(expression);
        CHECK_FALSE(isJavaScriptBooleanByForm(returned(expression).expression()));
    }
}

TEST_CASE("a conjunction, a disjunction, or a conditional expression is Boolean by form only when each of its results is") {
    for (const char* expression : {"!a || b === null || Array.isArray(c)", "(a > 1 && b < 2)", "a ? b === 1 : false"}) {
        CAPTURE(expression);
        CHECK(isJavaScriptBooleanByForm(returned(expression).expression()));
    }
    for (const char* expression : {"a && b", "a === 1 || b", "a ? b : true"}) {
        CAPTURE(expression);
        CHECK_FALSE(isJavaScriptBooleanByForm(returned(expression).expression()));
    }
}

TEST_CASE("only the literal undefined alone gives the caller no value") {
    CHECK(isUndefinedLiteral(returned("undefined").expression()));
    CHECK_FALSE(isUndefinedLiteral(returned("a").expression()));
    CHECK_FALSE(isUndefinedLiteral(returned("a || undefined").expression()));
}

TEST_CASE("a get or set accessor of the objects names a property, unlike a static accessor and a method named get or set") {
    const classBody body("get size() { return 1; }\nset size(v) {}\nstatic get count() { return 1; }\nget(key) { return key; }\nstatic total() { return 1; }");
    REQUIRE(body.methods.size() == 5);
    CHECK(isPropertyAccessor(body.methods[0]));
    CHECK(isPropertyAccessor(body.methods[1]));
    CHECK_FALSE(isPropertyAccessor(body.methods[2]));
    CHECK_FALSE(isPropertyAccessor(body.methods[3]));
    CHECK_FALSE(isPropertyAccessor(body.methods[4]));

    CHECK_FALSE(isStaticMethod(body.methods[0]));
    CHECK(isStaticMethod(body.methods[2]));
    CHECK(isStaticMethod(body.methods[4]));
}

TEST_CASE("a constructor and a method named get or set, which srcML marks without a name element, take their names from their kinds") {
    const classBody body("constructor() {}\nget(key) { return key; }\nset(key, value) {}\nget size() { return 1; }");
    REQUIRE(body.methods.size() == 4);
    CHECK(unnamedMethodName(body.methods[0]) == "get");
    CHECK(unnamedMethodName(body.methods[1]) == "set");
    CHECK(unnamedMethodName(body.methods[2]).empty());
    CHECK(unnamedMethodName(body.methods[3]) == "constructor");
}

TEST_CASE("the holes of a template literal give their names, while an escaped dollar sign opens no hole") {
    CHECK(templateLiteralNames("`${this.a} and ${b + c}`") == names{"this.a", "b", "c"});
    CHECK(templateLiteralNames("`\\${a} ${b}`") == names{"b"});
    CHECK(templateLiteralNames("`${ {}[k] }`") == names{"k"});
}

TEST_CASE("a call in a hole uses only its object and its arguments, and a string or a keyword names nothing") {
    CHECK(templateLiteralNames("`${this.items.join(sep)} ${f().name} ${\"a\".repeat(n)}`") == names{"this.items", "sep", "n"});
    CHECK(templateLiteralNames("`${a ? 'yes' : \"no\"} ${true} ${typeof c}`") == names{"a", "c"});
}

TEST_CASE("optional chaining reaches the member that the plain access operator reaches") {
    CHECK(withoutOptionalChaining("this.cache?.get") == "this.cache.get");
    CHECK(withoutOptionalChaining("a?.b?.c") == "a.b.c");
    CHECK(withoutOptionalChaining("a.b") == "a.b");
}
