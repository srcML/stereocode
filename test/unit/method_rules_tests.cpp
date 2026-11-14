// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file method_rules_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "joined_names.hpp"
#include "rules/stereotype_classifier.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace {
    // A method with one statement that reads a field and returns int, which no rule labels
    //
    methodFacts readingMethod(const std::string& language = "C++") {
        methodFacts f;
        f.language             = language;
        f.returnType           = "int";
        f.declaredReturnType   = "int";
        f.nonCommentStatements = 1;
        f.fieldUsed            = true;
        return f;
    }

    // Gets the stereotypes of a method as stereocode writes them
    //
    std::string labelsOf(const methodFacts& f) {
        return joinedNames(classifyMethod(f));
    }

    // A call that may reach the methods at positions among the methods of the run
    //
    methodCall callReaching(std::vector<std::size_t> callees) {
        return {std::move(callees)};
    }

    // Gets the stereotypes of the methods of a run as stereocode writes them, in the order of the methods
    //
    std::vector<std::string> runLabelsOf(const std::vector<methodFacts>& methods) {
        std::vector<std::string> names;
        for (const stereotypeList& labels : classifyMethods(methods)) names.push_back(joinedNames(labels));
        return names;
    }
}

TEST_CASE("a method that reads a field and matches no rule is unclassified") {
    CHECK(labelsOf(readingMethod()) == "unclassified");
}

TEST_CASE("constructors and destructors get only their creational stereotype") {
    methodFacts f = readingMethod();
    f.simpleReturn   = true;
    f.fieldsModified = 2;

    f.constructorOrDestructor = "constructor";
    CHECK(labelsOf(f) == "constructor");
    f.constructorOrDestructor = "copy-constructor";
    CHECK(labelsOf(f) == "copy-constructor");
    f.constructorOrDestructor = "destructor";
    CHECK(labelsOf(f) == "destructor");
}

TEST_CASE("a method without statements is empty whatever its other facts") {
    methodFacts f = readingMethod();
    f.nonCommentStatements = 0;
    f.simpleReturn         = true;
    CHECK(labelsOf(f) == "empty");
}

TEST_CASE("a simple return of a field makes a get") {
    methodFacts f = readingMethod();
    f.simpleReturn = true;
    CHECK(labelsOf(f) == "get");

    SUBCASE("a further complex return adds property") {
        f.complexReturn = true;
        CHECK(labelsOf(f) == "get property");
    }
}

TEST_CASE("get and set read neither const nor the return type") {
    methodFacts f = readingMethod();
    f.simpleReturn = true;
    f.isConst      = true;
    CHECK(labelsOf(f) == "get");

    SUBCASE("a method that writes one field and returns the field is a get and a set") {
        f.isConst        = false;
        f.fieldsModified = 1;
        CHECK(labelsOf(f) == "get set");
    }
    SUBCASE("a const method that writes one mutable field is a set") {
        f.simpleReturn   = false;
        f.fieldsModified = 1;
        f.returnType = f.declaredReturnType = "void";
        CHECK(labelsOf(f) == "set");
    }
    SUBCASE("a set may return a Boolean") {
        f.simpleReturn   = false;
        f.isConst        = false;
        f.fieldsModified = 1;
        f.returnType = f.declaredReturnType = "bool";
        CHECK(labelsOf(f) == "set");
    }
}

TEST_CASE("a Boolean complex return that uses a field makes a predicate in each language") {
    methodFacts f = readingMethod();
    f.complexReturn = true;

    SUBCASE("C++ bool") {
        f.returnType = f.declaredReturnType = "bool";
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
}

TEST_CASE("Java Boolean and java.lang.Boolean and C# System.Boolean returns make a predicate") {
    methodFacts f = readingMethod("Java");
    f.complexReturn = true;
    f.returnType    = "Boolean";
    CHECK(labelsOf(f) == "predicate");
    f.returnType = "java.lang.Boolean";
    CHECK(labelsOf(f) == "predicate");

    f.language   = "C#";
    f.returnType = "System.Boolean";
    CHECK(labelsOf(f) == "predicate");
}

TEST_CASE("a predicate or a property needs a field or a call to an accessor of the type") {
    methodFacts f = readingMethod();
    f.complexReturn = true;
    f.fieldUsed     = false;
    CHECK(labelsOf(f) == "incidental");

    SUBCASE("a Boolean return without a field or a call is no predicate") {
        f.returnType = "bool";
        CHECK(labelsOf(f) == "incidental");
    }
    SUBCASE("a call to a method of the type that is no accessor makes a command of a non-const method and no accessor") {
        f.typeMethodCalls = {methodCall{}};
        CHECK(labelsOf(f) == "non-void-command");
        f.returnType = "bool";
        CHECK(labelsOf(f) == "non-void-command");
    }
    SUBCASE("a const method with such a call is unclassified") {
        f.typeMethodCalls    = {methodCall{}};
        f.isConst            = true;
        CHECK(labelsOf(f) == "unclassified");
    }
    SUBCASE("a call to an accessor of the type stands for the field") {
        methodFacts getter = readingMethod();
        getter.simpleReturn = true;
        f.typeMethodCalls   = {callReaching({0})};
        CHECK(runLabelsOf({getter, f}) == std::vector<std::string>{"get", "property"});
        f.returnType = "bool";
        CHECK(runLabelsOf({getter, f}) == std::vector<std::string>{"get", "predicate"});
    }
}

TEST_CASE("a non-Boolean complex return that uses a field makes a property") {
    methodFacts f = readingMethod();
    f.complexReturn = true;
    CHECK(labelsOf(f) == "property");

    SUBCASE("a void return gives no property") {
        f.returnType = f.declaredReturnType = "void";
        CHECK(labelsOf(f) == "unclassified");
    }
    SUBCASE("a missing return type gives no property") {
        f.returnType = f.declaredReturnType = "";
        CHECK(labelsOf(f) == "unclassified");
    }
    SUBCASE("a void* return counts as a value and makes a collaborator") {
        f.returnType         = "void";
        f.declaredReturnType = "void*";
        CHECK(labelsOf(f) == "property collaborator");
    }
    SUBCASE("Void is void in C# and Java but a type name in C++") {
        f.returnType = "Void";
        CHECK(labelsOf(f) == "property");
        f.language = "Java";
        CHECK(labelsOf(f) == "unclassified");
        f.language = "C#";
        CHECK(labelsOf(f) == "unclassified");
    }
}

TEST_CASE("a void method that writes a reference parameter and reads a field is a void-accessor") {
    methodFacts f = readingMethod();
    f.returnType = f.declaredReturnType = "void";
    f.parameterRefModified = true;
    CHECK(labelsOf(f) == "void-accessor");

    SUBCASE("a call to an accessor of the type stands for the field") {
        methodFacts getter = readingMethod();
        getter.simpleReturn = true;
        f.fieldUsed         = false;
        f.typeMethodCalls   = {callReaching({0})};
        f.isConst           = true;
        CHECK(runLabelsOf({getter, f}) == std::vector<std::string>{"get", "void-accessor"});
    }
    SUBCASE("without a field or a call the method is incidental") {
        f.fieldUsed = false;
        CHECK(labelsOf(f) == "incidental");
    }
    SUBCASE("a void* return is no void-accessor") {
        f.declaredReturnType = "void*";
        CHECK(labelsOf(f) == "collaborator");
    }
    SUBCASE("a Java Void return counts as void") {
        f.language   = "Java";
        f.returnType = "Void";
        CHECK(labelsOf(f) == "void-accessor");
    }
}

TEST_CASE("one changed field without calls on fields or to the type makes a set") {
    methodFacts f = readingMethod();
    f.returnType = f.declaredReturnType = "void";
    f.fieldsModified = 1;
    CHECK(labelsOf(f) == "set");

    SUBCASE("a call to a method of the type turns it into a command") {
        f.typeMethodCalls = {methodCall{}};
        CHECK(labelsOf(f) == "command");
    }
    SUBCASE("a call on a field turns it into a command") {
        f.callsOnFields = {methodCall{}};
        CHECK(labelsOf(f) == "command");
    }
}

TEST_CASE("one call on a field without a changed field makes a command, not a set") {
    methodFacts f = readingMethod();
    f.returnType = f.declaredReturnType = "void";
    f.callsOnFields = {methodCall{}};
    CHECK(labelsOf(f) == "command");

    SUBCASE("a const method that returns the result of the call is only a property") {
        f.returnType = f.declaredReturnType = "int";
        f.isConst       = true;
        f.complexReturn = true;
        CHECK(labelsOf(f) == "property");
    }
    SUBCASE("a Java method that returns the result of the call is also a non-void-command") {
        f.language   = "Java";
        f.returnType = f.declaredReturnType = "int";
        f.complexReturn = true;
        CHECK(labelsOf(f) == "property non-void-command");
    }
    SUBCASE("a call on a field of another type makes a command collaborator") {
        f.newConstructorCalls       = 1;
        f.nonPrimitiveFieldExternal = true;
        CHECK(labelsOf(f) == "command collaborator");
    }
}

TEST_CASE("command covers several changed fields, one changed field with calls, and calls without a changed field") {
    methodFacts f = readingMethod();
    f.returnType = f.declaredReturnType = "void";

    SUBCASE("case 1: two changed fields") {
        f.fieldsModified = 2;
        CHECK(labelsOf(f) == "command");
    }
    SUBCASE("case 2: one changed field and a call on a field") {
        f.fieldsModified = 1;
        f.callsOnFields  = {methodCall{}};
        CHECK(labelsOf(f) == "command");
    }
    SUBCASE("case 3: one call on a field") {
        f.callsOnFields = {methodCall{}};
        CHECK(labelsOf(f) == "command");
    }
    SUBCASE("case 3: two calls on fields") {
        f.callsOnFields = {methodCall{}, methodCall{}};
        CHECK(labelsOf(f) == "command");
    }
    SUBCASE("case 3: one call to a method of the type") {
        f.typeMethodCalls = {methodCall{}};
        CHECK(labelsOf(f) == "command");
    }
}

TEST_CASE("a const method is a command only when it changes several fields") {
    methodFacts f = readingMethod();
    f.returnType = f.declaredReturnType = "void";
    f.isConst        = true;
    f.fieldsModified = 1;
    f.callsOnFields  = {methodCall{}};
    CHECK(labelsOf(f) == "unclassified");

    f.fieldsModified = 2;
    CHECK(labelsOf(f) == "command");
}

TEST_CASE("a command with a return value is a non-void-command") {
    methodFacts f = readingMethod();
    f.fieldsModified = 2;
    CHECK(labelsOf(f) == "non-void-command");

    SUBCASE("a Boolean return counts as a value") {
        f.returnType = f.declaredReturnType = "bool";
        CHECK(labelsOf(f) == "non-void-command");
    }
    SUBCASE("a void* return counts as a value and adds collaborator") {
        f.returnType         = "void";
        f.declaredReturnType = "void*";
        CHECK(labelsOf(f) == "non-void-command collaborator");
    }
    SUBCASE("a void* method that returns a field is a get as well") {
        f.returnType         = "void";
        f.declaredReturnType = "void*";
        f.simpleReturn       = true;
        CHECK(labelsOf(f) == "get non-void-command collaborator");
    }
    SUBCASE("Void counts as void in C# and Java and as a type name in C++") {
        f.returnType = "Void";
        CHECK(labelsOf(f) == "non-void-command");
        f.language = "Java";
        CHECK(labelsOf(f) == "command");
        f.language = "C#";
        CHECK(labelsOf(f) == "command");
    }
}

TEST_CASE("a non-primitive return of an object created with new makes a factory") {
    methodFacts f = readingMethod();
    f.nonPrimitiveReturnType = true;
    f.newReturned            = true;
    CHECK(labelsOf(f) == "factory");

    f.newReturned                       = false;
    f.variableCreatedWithNewAndReturned = true;
    CHECK(labelsOf(f) == "factory");

    f.nonPrimitiveReturnType = false;
    CHECK(labelsOf(f) == "unclassified");
}

TEST_CASE("a free-function call without changes or other calls makes a wrapper") {
    methodFacts f = readingMethod();
    f.callsOnFreeFunctions = 1;
    CHECK(labelsOf(f) == "wrapper");

    SUBCASE("a call to a method of another type makes a controller instead") {
        f.callsToOtherTypeMethods = 1;
        CHECK(labelsOf(f) == "controller");
    }
    SUBCASE("a wrapper is no collaborator") {
        f.nonPrimitiveParameterExternal = true;
        CHECK(labelsOf(f) == "wrapper");
    }
}

TEST_CASE("a call to another type or a changed non-primitive local or parameter makes a controller") {
    methodFacts f = readingMethod();
    f.callsToOtherTypeMethods = 1;
    CHECK(labelsOf(f) == "controller");

    f.callsToOtherTypeMethods              = 0;
    f.nonPrimitiveLocalOrParameterModified = true;
    CHECK(labelsOf(f) == "controller");

    SUBCASE("a changed field rules out controller") {
        f.fieldsModified = 1;
        f.returnType = f.declaredReturnType = "void";
        CHECK(labelsOf(f) == "set");
    }
}

TEST_CASE("a non-primitive of another type makes a collaborator") {
    methodFacts f = readingMethod();

    SUBCASE("field") {
        f.nonPrimitiveFieldExternal = true;
        CHECK(labelsOf(f) == "collaborator");
    }
    SUBCASE("local") {
        f.nonPrimitiveLocalExternal = true;
        CHECK(labelsOf(f) == "collaborator");
    }
    SUBCASE("parameter") {
        f.nonPrimitiveParameterExternal = true;
        CHECK(labelsOf(f) == "collaborator");
    }
    SUBCASE("return type") {
        f.nonPrimitiveReturnTypeExternal = true;
        CHECK(labelsOf(f) == "collaborator");
    }
}

TEST_CASE("a method that uses no field and makes no call is incidental") {
    methodFacts f = readingMethod();
    f.fieldUsed = false;
    CHECK(labelsOf(f) == "incidental");

    SUBCASE("a constructor call with new makes it stateless") {
        f.newConstructorCalls = 1;
        CHECK(labelsOf(f) == "stateless");
    }
}

TEST_CASE("a method that uses no field and calls outside its type is stateless") {
    methodFacts f = readingMethod();
    f.fieldUsed = false;

    SUBCASE("a free-function call also makes a wrapper") {
        f.callsOnFreeFunctions = 1;
        CHECK(labelsOf(f) == "wrapper stateless");
    }
    SUBCASE("a call to another type also makes a controller") {
        f.callsToOtherTypeMethods = 1;
        CHECK(labelsOf(f) == "controller stateless");
    }
    SUBCASE("a call on a field rules out stateless") {
        f.callsOnFreeFunctions = 1;
        f.callsOnFields        = {methodCall{}};
        f.returnType = f.declaredReturnType = "void";
        CHECK(labelsOf(f) == "command");
    }
}

TEST_CASE("the stereotypes of a method follow the order of the rules") {
    methodFacts f = readingMethod();
    f.simpleReturn                   = true;
    f.complexReturn                  = true;
    f.fieldsModified                 = 1;
    f.nonPrimitiveReturnType         = true;
    f.newReturned                    = true;
    f.nonPrimitiveReturnTypeExternal = true;
    CHECK(labelsOf(f) == "get property set factory collaborator");
}

TEST_CASE("after the first pass, a call to a get method of the type counts as a read of a field") {
    methodFacts getter = readingMethod("Java");
    getter.simpleReturn = true;

    methodFacts caller = readingMethod("Java");
    caller.fieldUsed       = false;
    caller.complexReturn   = true;
    caller.typeMethodCalls = {callReaching({0})};
    CHECK(labelsOf(caller) == "non-void-command");
    CHECK(runLabelsOf({getter, caller}) == std::vector<std::string>{"get", "property"});

    SUBCASE("a Boolean return makes a predicate") {
        caller.returnType = caller.declaredReturnType = "boolean";
        CHECK(runLabelsOf({getter, caller}) == std::vector<std::string>{"get", "predicate"});
    }
}

TEST_CASE("the methods are classified again until no stereotype changes") {
    methodFacts getter = readingMethod("Java");
    getter.simpleReturn = true;

    methodFacts middle = readingMethod("Java");
    middle.fieldUsed       = false;
    middle.complexReturn   = true;
    middle.typeMethodCalls = {callReaching({2})};

    methodFacts outer = middle;
    outer.typeMethodCalls = {callReaching({1})};

    CHECK(runLabelsOf({outer, middle, getter}) == std::vector<std::string>{"property", "property", "get"});
}

TEST_CASE("the stereotypes of the methods of a run do not depend on the order of the methods") {
    methodFacts getter = readingMethod("Java");
    getter.simpleReturn = true;

    methodFacts setter = readingMethod("Java");
    setter.returnType = setter.declaredReturnType = "void";
    setter.fieldsModified = 1;

    methodFacts reader = readingMethod("Java");
    reader.fieldUsed     = false;
    reader.complexReturn = true;

    methodFacts writer = reader;

    // The positions of the callees follow the order of each run
    reader.typeMethodCalls = {callReaching({1})};
    writer.typeMethodCalls = {callReaching({0}), callReaching({1})};
    const std::vector<std::string> forward = runLabelsOf({setter, getter, reader, writer});

    reader.typeMethodCalls = {callReaching({2})};
    writer.typeMethodCalls = {callReaching({3}), callReaching({2})};
    const std::vector<std::string> backward = runLabelsOf({writer, reader, getter, setter});

    CHECK(forward == std::vector<std::string>{"set", "get", "property", "property non-void-command"});
    CHECK(backward == std::vector<std::string>{forward[3], forward[2], forward[1], forward[0]});
}

TEST_CASE("a call to a degenerate method of the type neither reads nor changes the object and counts like a call of a free function") {
    methodFacts degenerate = readingMethod("Java");
    degenerate.fieldUsed = false;

    methodFacts caller = readingMethod("Java");
    caller.fieldUsed       = false;
    caller.complexReturn   = true;
    caller.typeMethodCalls = {callReaching({0})};
    CHECK(runLabelsOf({degenerate, caller}) == std::vector<std::string>{"incidental", "wrapper stateless"});

    SUBCASE("a void method that only calls an empty method of the type is a wrapper") {
        degenerate.nonCommentStatements = 0;
        caller.returnType = caller.declaredReturnType = "void";
        caller.complexReturn = false;
        CHECK(runLabelsOf({degenerate, caller}) == std::vector<std::string>{"empty", "wrapper stateless"});
    }
    SUBCASE("a call to a stateless method of the type is no call to the type") {
        degenerate.callsOnFreeFunctions = 1;
        caller.fieldUsed = true;
        CHECK(runLabelsOf({degenerate, caller}) == std::vector<std::string>{"wrapper stateless", "property wrapper"});
    }
    SUBCASE("a call that may also reach a set may change the object") {
        methodFacts setter = readingMethod("Java");
        setter.returnType = setter.declaredReturnType = "void";
        setter.fieldsModified  = 1;
        caller.typeMethodCalls = {callReaching({0, 2})};
        CHECK(runLabelsOf({degenerate, caller, setter}) == std::vector<std::string>{"incidental", "non-void-command", "set"});
    }
}

TEST_CASE("a call on a field counts by the stereotypes of the method of the type of the field that it calls") {
    methodFacts getter = readingMethod();
    getter.simpleReturn = true;

    methodFacts setter = readingMethod();
    setter.returnType = setter.declaredReturnType = "void";
    setter.fieldsModified = 1;

    methodFacts degenerate = readingMethod();
    degenerate.returnType = degenerate.declaredReturnType = "void";
    degenerate.fieldUsed            = false;
    degenerate.callsOnFreeFunctions = 1;

    methodFacts caller = readingMethod();
    caller.complexReturn = true;

    SUBCASE("a call to an accessor reads the field") {
        caller.callsOnFields = {callReaching({0})};
        CHECK(runLabelsOf({getter, setter, degenerate, caller})[3] == "property");
    }
    SUBCASE("a call to a set may change the field") {
        caller.callsOnFields = {callReaching({1})};
        CHECK(runLabelsOf({getter, setter, degenerate, caller})[3] == "property non-void-command");
    }
    SUBCASE("a call to a degenerate method counts like a call of a free function") {
        caller.callsOnFields = {callReaching({2})};
        CHECK(runLabelsOf({getter, setter, degenerate, caller})[3] == "property wrapper");
    }
    SUBCASE("a call that reaches no method with stereotypes may change the field") {
        caller.callsOnFields = {methodCall{}};
        CHECK(runLabelsOf({getter, setter, degenerate, caller})[3] == "property non-void-command");
    }
}

TEST_CASE("a write through the result of a call to an accessor changes the object") {
    methodFacts getter = readingMethod();
    getter.simpleReturn = true;

    methodFacts writer = readingMethod();
    writer.returnType = writer.declaredReturnType = "void";
    writer.fieldUsed = false;

    SUBCASE("a call to the type") {
        writer.typeMethodCalls = {methodCall{{0}, true}};
        CHECK(runLabelsOf({getter, writer}) == std::vector<std::string>{"get", "command"});
    }
    SUBCASE("a call on a field") {
        writer.fieldUsed     = true;
        writer.callsOnFields = {methodCall{{0}, true}};
        CHECK(runLabelsOf({getter, writer}) == std::vector<std::string>{"get", "command"});
    }
}

TEST_CASE("a call to a method that both reads and changes the object makes an accessor and a command") {
    methodFacts both = readingMethod("Java");
    both.simpleReturn   = true;
    both.fieldsModified = 1;

    methodFacts caller = readingMethod("Java");
    caller.fieldUsed       = false;
    caller.complexReturn   = true;
    caller.typeMethodCalls = {callReaching({0})};
    CHECK(runLabelsOf({both, caller}) == std::vector<std::string>{"get set", "property non-void-command"});
}

TEST_CASE("a call to a method of the type that may change the object stays a call") {
    methodFacts setter = readingMethod("Java");
    setter.returnType = setter.declaredReturnType = "void";
    setter.fieldsModified = 1;

    methodFacts caller = readingMethod("Java");
    caller.returnType = caller.declaredReturnType = "void";
    caller.fieldUsed       = false;
    caller.typeMethodCalls = {callReaching({0})};
    CHECK(runLabelsOf({setter, caller}) == std::vector<std::string>{"set", "command"});

    SUBCASE("a call that may reach a get and a set reads and may change the object") {
        methodFacts getter = readingMethod("Java");
        getter.simpleReturn    = true;
        caller.typeMethodCalls = {callReaching({0, 2})};
        CHECK(runLabelsOf({setter, caller, getter}) == std::vector<std::string>{"set", "command", "get"});
    }
    SUBCASE("a call that reaches no method with stereotypes stays a call") {
        methodFacts getter = readingMethod("Java");
        getter.simpleReturn    = true;
        caller.typeMethodCalls = {methodCall{}};
        CHECK(runLabelsOf({getter, caller}) == std::vector<std::string>{"get", "command"});
    }
}

TEST_CASE("a call that reaches the calling method itself counts by the stereotypes that the method has without the call") {
    methodFacts recursive = readingMethod("Java");
    recursive.complexReturn   = true;
    recursive.typeMethodCalls = {callReaching({0})};
    CHECK(runLabelsOf({recursive}) == std::vector<std::string>{"property"});

    SUBCASE("a call that also reaches an accessor reads the object") {
        methodFacts getter = readingMethod("Java");
        getter.simpleReturn = true;
        methodFacts overload = readingMethod("Java");
        overload.fieldUsed       = false;
        overload.complexReturn   = true;
        overload.typeMethodCalls = {callReaching({0, 1})};
        CHECK(runLabelsOf({getter, overload}) == std::vector<std::string>{"get", "property"});
    }
    SUBCASE("a call that also reaches a set may change the object") {
        methodFacts setter = readingMethod("Java");
        setter.returnType = setter.declaredReturnType = "void";
        setter.fieldsModified = 1;
        methodFacts overload = readingMethod("Java");
        overload.returnType = overload.declaredReturnType = "void";
        overload.fieldUsed       = false;
        overload.typeMethodCalls = {callReaching({0, 1})};
        CHECK(runLabelsOf({setter, overload}) == std::vector<std::string>{"set", "command"});
    }
    SUBCASE("a method that changes a field and calls itself may change the object through the call") {
        methodFacts draining = readingMethod("Java");
        draining.returnType = draining.declaredReturnType = "void";
        draining.fieldsModified  = 1;
        draining.typeMethodCalls = {callReaching({0})};
        CHECK(runLabelsOf({draining}) == std::vector<std::string>{"command"});
    }
    SUBCASE("a call on a field of the own type to the calling method reads the field when the method only reads") {
        methodFacts counting = readingMethod("Java");
        counting.complexReturn = true;
        counting.callsOnFields = {callReaching({0})};
        CHECK(runLabelsOf({counting}) == std::vector<std::string>{"property"});
    }
}

TEST_CASE("the rounds end when the stereotypes of two methods cycle") {
    // Two const methods that change a mutable field and call each other are properties until each call counts as a read, which makes a set
    methodFacts first = readingMethod("C++");
    first.isConst         = true;
    first.fieldsModified  = 1;
    first.complexReturn   = true;
    first.typeMethodCalls = {callReaching({1})};
    methodFacts second = first;
    second.typeMethodCalls = {callReaching({0})};
    CHECK(labelsOf(first) == "property");

    for (const std::string& labels : runLabelsOf({first, second})) CHECK((labels == "property" || labels == "property set"));
}

TEST_CASE("a method that returns the object itself and changes no state is a get") {
    methodFacts f = readingMethod();
    f.returnType = f.declaredReturnType = "Node&";
    f.returnsObject = true;
    CHECK(labelsOf(f) == "get");

    SUBCASE("a const method as well") {
        f.isConst = true;
        CHECK(labelsOf(f) == "get");
    }
    SUBCASE("a call to a method of the type that may change the object leaves a command") {
        f.typeMethodCalls = {methodCall{}};
        CHECK(labelsOf(f) == "command");
    }
}

TEST_CASE("a set or a command that returns the object itself keeps its label and is no non-void-command") {
    methodFacts f = readingMethod();
    f.returnType = f.declaredReturnType = "Node&";
    f.returnsObject  = true;
    f.fieldsModified = 1;
    CHECK(labelsOf(f) == "set");

    f.fieldsModified = 2;
    CHECK(labelsOf(f) == "command");

    f.returnsObject = false;
    CHECK(labelsOf(f) == "non-void-command");
}

TEST_CASE("a write of the whole object, as *this = other, makes a command and no set") {
    methodFacts f = readingMethod();
    f.returnType = f.declaredReturnType = "void";
    f.objectReplaced = true;
    CHECK(labelsOf(f) == "command");

    f.fieldsModified = 1;
    CHECK(labelsOf(f) == "command");

    f.returnType = f.declaredReturnType = "int";
    CHECK(labelsOf(f) == "non-void-command");

    SUBCASE("the write changes the object, which no wrapper or controller does") {
        f.fieldsModified       = 0;
        f.callsOnFreeFunctions = 1;
        CHECK(labelsOf(f) == "non-void-command");
        f.callsToOtherTypeMethods = 1;
        CHECK(labelsOf(f) == "non-void-command");
    }
}
