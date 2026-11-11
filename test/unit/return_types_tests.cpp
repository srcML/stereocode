// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file return_types_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "rules/return_types.hpp"

TEST_CASE("Boolean return types are the Boolean types of each language") {
    CHECK(isBooleanType("C++", "bool"));
    CHECK(isBooleanType("C++", "_Bool"));
    CHECK(isBooleanType("C#", "bool"));
    CHECK(isBooleanType("C#", "Boolean"));
    CHECK(isBooleanType("C#", "System.Boolean"));
    CHECK(isBooleanType("Java", "boolean"));
    CHECK(isBooleanType("Java", "Boolean"));
    CHECK(isBooleanType("Java", "java.lang.Boolean"));

    CHECK_FALSE(isBooleanType("C++", "Boolean"));
    CHECK_FALSE(isBooleanType("C#", "boolean"));
    CHECK_FALSE(isBooleanType("Java", "bool"));
    CHECK_FALSE(isBooleanType("Java", "BooleanSupplier"));
}

TEST_CASE("void return types are void in each language and Void in C# and Java") {
    CHECK(isVoidType("C++", "void"));
    CHECK(isVoidType("C#", "void"));
    CHECK(isVoidType("C#", "Void"));
    CHECK(isVoidType("Java", "void"));
    CHECK(isVoidType("Java", "Void"));

    CHECK_FALSE(isVoidType("C++", "Void"));
    CHECK_FALSE(isVoidType("C++", "int"));
    CHECK_FALSE(isVoidType("Java", ""));
}

TEST_CASE("a declared return type that contains void* points to void outside Java") {
    CHECK(isVoidPointerType("C++", "void*"));
    CHECK(isVoidPointerType("C++", "constvoid*"));
    CHECK(isVoidPointerType("C++", "void**"));
    CHECK(isVoidPointerType("C#", "void*"));

    CHECK_FALSE(isVoidPointerType("C++", "void"));
    CHECK_FALSE(isVoidPointerType("C++", "int*"));
    CHECK_FALSE(isVoidPointerType("Java", "void*"));
}

TEST_CASE("a return type is non-void unless it is void and the declared type is no void pointer") {
    CHECK(isNonVoidType("C++", "int", "int"));
    CHECK(isNonVoidType("C++", "", ""));
    CHECK(isNonVoidType("C++", "void", "void*"));
    CHECK(isNonVoidType("C++", "Void", "Void"));

    CHECK_FALSE(isNonVoidType("C++", "void", "void"));
    CHECK_FALSE(isNonVoidType("Java", "Void", "Void"));
    CHECK_FALSE(isNonVoidType("C#", "void", "void"));
}

TEST_CASE("a name is spelled as a macro when it holds only capital letters, digits, and underscores") {
    CHECK(isSpelledAsMacro("SCI_METHOD"));
    CHECK(isSpelledAsMacro("WINAPI"));
    CHECK(isSpelledAsMacro("CATCH_INTERNAL_CDECL"));
    CHECK(isSpelledAsMacro("D2D1_API2"));

    CHECK_FALSE(isSpelledAsMacro(""));
    CHECK_FALSE(isSpelledAsMacro("fastcall"));
    CHECK_FALSE(isSpelledAsMacro("__stdcall"));
    CHECK_FALSE(isSpelledAsMacro("Sci_Position"));
    CHECK_FALSE(isSpelledAsMacro("Sci::POSITION"));
    CHECK_FALSE(isSpelledAsMacro("VECTOR<INT>"));
}
