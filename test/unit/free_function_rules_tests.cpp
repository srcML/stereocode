// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file free_function_rules_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "joined_names.hpp"
#include "stereotype_classifier.hpp"

#include <string>

namespace {
    // A function with one statement that uses a parameter and returns int, which no rule labels
    //
    freeFunctionFacts parameterFunction(const std::string& language = "C++") {
        freeFunctionFacts f;
        f.language             = language;
        f.name                 = "compute";
        f.returnType           = "int";
        f.declaredReturnType   = "int";
        f.nonCommentStatements = 1;
        f.parameterUsed        = true;
        return f;
    }

    // Gets the stereotypes of a free function as stereocode writes them
    //
    std::string labelsOf(const freeFunctionFacts& f) {
        return joinedNames(classifyFreeFunction(f));
    }
}

TEST_CASE("a function that uses a parameter and matches no rule is unclassified") {
    CHECK(labelsOf(parameterFunction()) == "unclassified");
}

TEST_CASE("a function named main or Main is only main") {
    freeFunctionFacts f = parameterFunction();
    f.parameterUsed = false;
    f.calls         = 2;

    f.name = "main";
    CHECK(labelsOf(f) == "main");
    f.name = "Main";
    CHECK(labelsOf(f) == "main");

    f.nonCommentStatements = 0;
    CHECK(labelsOf(f) == "main");
}

TEST_CASE("the Windows entry points wmain, WinMain, wWinMain, _tmain, and _tWinMain are main") {
    freeFunctionFacts f = parameterFunction();
    f.calls = 1;
    for (const char* name : {"wmain", "WinMain", "wWinMain", "_tmain", "_tWinMain"}) {
        f.name = name;
        CHECK(labelsOf(f) == "main");
    }

    SUBCASE("a name that only contains an entry point is no entry point") {
        f.name = "mainLoop";
        CHECK(labelsOf(f) == "wrapper");
        f.name = "WinMainCRTStartup";
        CHECK(labelsOf(f) == "wrapper");
    }
}

TEST_CASE("a function without statements is empty") {
    freeFunctionFacts f = parameterFunction();
    f.nonCommentStatements = 0;
    f.calls                = 1;
    CHECK(labelsOf(f) == "empty");
}

TEST_CASE("a Boolean return derived from a used parameter makes a predicate in each language") {
    freeFunctionFacts f = parameterFunction();
    f.parameterComplexReturn = true;

    SUBCASE("C++ bool") {
        f.returnType = "bool";
        CHECK(labelsOf(f) == "predicate");
    }
    SUBCASE("C# bool and Boolean") {
        f.language   = "C#";
        f.returnType = "bool";
        CHECK(labelsOf(f) == "predicate");
        f.returnType = "Boolean";
        CHECK(labelsOf(f) == "predicate");
    }
    SUBCASE("Java boolean") {
        f.language   = "Java";
        f.returnType = "boolean";
        CHECK(labelsOf(f) == "predicate");
    }
    SUBCASE("Java Boolean and C# System.Boolean") {
        f.language   = "Java";
        f.returnType = "Boolean";
        CHECK(labelsOf(f) == "predicate");
        f.language   = "C#";
        f.returnType = "System.Boolean";
        CHECK(labelsOf(f) == "predicate");
    }
    SUBCASE("C _Bool, which runs as C++") {
        f.returnType = "_Bool";
        CHECK(labelsOf(f) == "predicate");
    }
}

TEST_CASE("a non-Boolean return derived from a used parameter makes a property") {
    freeFunctionFacts f = parameterFunction();
    f.parameterComplexReturn = true;
    CHECK(labelsOf(f) == "property");

    SUBCASE("a void return gives no property") {
        f.returnType = "void";
        CHECK(labelsOf(f) == "unclassified");
        f.language   = "Java";
        f.returnType = "Void";
        CHECK(labelsOf(f) == "unclassified");
    }
    SUBCASE("a missing return type gives no property") {
        f.returnType = "";
        CHECK(labelsOf(f) == "unclassified");
    }
    SUBCASE("a void* return counts as a value") {
        f.returnType         = "void";
        f.declaredReturnType = "void*";
        CHECK(labelsOf(f) == "property");
    }
    SUBCASE("an unused parameter gives a literal instead") {
        f.parameterUsed = false;
        CHECK(labelsOf(f) == "literal");
    }
}

TEST_CASE("a returned parameter counts as derived from the parameters") {
    freeFunctionFacts f = parameterFunction();
    f.parameterReturned = true;
    CHECK(labelsOf(f) == "property");

    f.returnType = f.declaredReturnType = "bool";
    CHECK(labelsOf(f) == "predicate");

    SUBCASE("a void function that uses a parameter stays unclassified") {
        f.returnType = f.declaredReturnType = "void";
        CHECK(labelsOf(f) == "unclassified");
    }
}

TEST_CASE("a change to a global or a static variable makes a global-command") {
    freeFunctionFacts f = parameterFunction();
    f.globalOrStaticVariableModified = true;
    CHECK(labelsOf(f) == "global-command");
}

TEST_CASE("a change to a parameter passed by reference makes a command and no wrapper") {
    freeFunctionFacts f = parameterFunction();
    f.parameterRefModified = true;
    CHECK(labelsOf(f) == "command");

    f.calls = 1;
    CHECK(labelsOf(f) == "command");
}

TEST_CASE("a non-primitive return of an object created with new makes a factory") {
    freeFunctionFacts f = parameterFunction();
    f.nonPrimitiveReturnType = true;
    f.newReturned            = true;
    CHECK(labelsOf(f) == "factory");

    f.newReturned                       = false;
    f.variableCreatedWithNewAndReturned = true;
    CHECK(labelsOf(f) == "factory");

    f.nonPrimitiveReturnType = false;
    CHECK(labelsOf(f) == "unclassified");
}

TEST_CASE("a function that uses no parameter is a literal") {
    freeFunctionFacts f = parameterFunction();
    f.parameterUsed = false;
    CHECK(labelsOf(f) == "literal");
}

TEST_CASE("a call without a change to a reference parameter makes a wrapper") {
    freeFunctionFacts f = parameterFunction();
    f.calls = 1;
    CHECK(labelsOf(f) == "wrapper");

    f.parameterUsed = false;
    CHECK(labelsOf(f) == "literal wrapper");
}

TEST_CASE("the stereotypes of a free function follow the order of the rules") {
    freeFunctionFacts f = parameterFunction();
    f.returnType                     = "bool";
    f.parameterComplexReturn         = true;
    f.globalOrStaticVariableModified = true;
    f.parameterRefModified           = true;
    f.nonPrimitiveReturnType         = true;
    f.newReturned                    = true;
    CHECK(labelsOf(f) == "predicate global-command command factory");

    f.parameterRefModified = false;
    f.parameterUsed        = false;
    f.calls                = 1;
    CHECK(labelsOf(f) == "global-command factory literal wrapper");
}

TEST_CASE("a literal uses no parameter, no global, and no static") {
    freeFunctionFacts f = parameterFunction();
    f.parameterUsed = false;
    f.calls         = 1;
    CHECK(labelsOf(f) == "literal wrapper");

    f.globalUsed = true;
    CHECK(labelsOf(f) == "wrapper");

    f.globalOrStaticVariableModified = true;
    CHECK(labelsOf(f) == "global-command wrapper");
}
