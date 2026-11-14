// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file source_language_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "archive/source_language.hpp"

TEST_CASE("the languages with declarations write a line comment with // and have none of the traits of a dynamic language") {
    for (const char* name : {"C++", "C#", "Java"}) {
        CAPTURE(name);
        const sourceLanguage& language = languageOf(name);
        CHECK(language.name == name);
        CHECK(language.lineComment == "//");
        CHECK_FALSE(language.dynamic);
        CHECK_FALSE(language.modules);
        CHECK_FALSE(language.callsCreate);
    }
}

TEST_CASE("Python writes a line comment with #, declares no data member, keeps its globals in modules, and creates an object by a call") {
    const sourceLanguage& python = languageOf("Python");
    CHECK(python.name == "Python");
    CHECK(python.lineComment == "#");
    CHECK(python.dynamic);
    CHECK(python.modules);
    CHECK(python.callsCreate);
}

TEST_CASE("a call of a variable by its name calls the call operator of its language, which Java does not have") {
    CHECK(languageOf("C++").callOperator == "operator()");
    CHECK(languageOf("C#").callOperator == "Invoke");
    CHECK(languageOf("Python").callOperator == "__call__");
    CHECK(languageOf("Java").callOperator.empty());
    CHECK(languageOf("Go").callOperator.empty());
}

TEST_CASE("C# and Java have no functions outside types, so a call without an object names a method of a type") {
    CHECK(languageOf("C#").methodsOnly);
    CHECK(languageOf("Java").methodsOnly);
    CHECK_FALSE(languageOf("C++").methodsOnly);
    CHECK_FALSE(languageOf("Python").methodsOnly);
}

TEST_CASE("C++ and C# have #if alternatives, so a type may define one method several times") {
    CHECK(languageOf("C++").preprocessor);
    CHECK(languageOf("C#").preprocessor);
    CHECK_FALSE(languageOf("Java").preprocessor);
    CHECK_FALSE(languageOf("Python").preprocessor);
    CHECK_FALSE(languageOf("Go").preprocessor);
}

TEST_CASE("C# returns the value of an async method through Task and ValueTask") {
    CHECK(languageOf("C#").asyncResults == std::vector<std::string_view>{"Task", "ValueTask"});
    CHECK(languageOf("C++").asyncResults.empty());
    CHECK(languageOf("Java").asyncResults.empty());
    CHECK(languageOf("Python").asyncResults.empty());
    CHECK(languageOf("Go").asyncResults.empty());
}

TEST_CASE("C# has the discard _, which takes a value and names no variable") {
    CHECK(languageOf("C#").discard == "_");
    CHECK(languageOf("C++").discard.empty());
    CHECK(languageOf("Java").discard.empty());
    CHECK(languageOf("Python").discard.empty());
    CHECK(languageOf("Go").discard.empty());
}

TEST_CASE("C++, C#, and Java have overloads, so a call may name a method of a parent under the name of a method of the type") {
    CHECK(languageOf("C++").overloads);
    CHECK(languageOf("C#").overloads);
    CHECK(languageOf("Java").overloads);
    CHECK_FALSE(languageOf("Python").overloads);
    CHECK_FALSE(languageOf("Go").overloads);
}

TEST_CASE("a Python call may name the class object, the parent of every class, with the object as its argument") {
    CHECK(languageOf("Python").rootClass == "object");
    CHECK(languageOf("Java").rootClass.empty());
    CHECK(languageOf("Go").rootClass.empty());
}

TEST_CASE("a language without analysis has no name and none of the traits") {
    const sourceLanguage& unknown = languageOf("Go");
    CHECK(unknown.name.empty());
    CHECK_FALSE(unknown.dynamic);
    CHECK_FALSE(unknown.modules);
    CHECK_FALSE(unknown.callsCreate);
    CHECK_FALSE(unknown.methodsOnly);
}

TEST_CASE("a C unit takes the analysis of C++, and a unit of a language without analysis takes none") {
    CHECK(analysisLanguageOf("C") == "C++");
    CHECK(analysisLanguageOf("C++") == "C++");
    CHECK(analysisLanguageOf("Python") == "Python");
    CHECK(analysisLanguageOf("JavaScript") == "JavaScript");
    CHECK(analysisLanguageOf("Go").empty());
    CHECK(analysisLanguageOf("").empty());
}

TEST_CASE("the section headers of the user lists name each language of the analysis") {
    CHECK(sectionHeadersText() == "[C++], [C#], [Java], [Python], or [JavaScript]");
}
