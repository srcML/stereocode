// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file global_variables_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "model/global_variables.hpp"

#include <string>

namespace {
    // Gets the declaration of a global of a unit that every unit reaches
    //
    globalDeclaration declared(const std::string& language, const std::string& name, const std::string& holder, bool namespaceScope, int unit = 1) {
        return {language, name, holder, unit, namespaceScope, false, false};
    }

    // Gets the declaration of a global that only its own unit reaches
    //
    globalDeclaration fileLocal(const std::string& name, const std::string& holder, int unit, bool immutable = false) {
        return {"C++", name, holder, unit, true, true, immutable};
    }

    // Gets the declaration of a global that no call changes, such as a const std::string
    //
    globalDeclaration constant(const std::string& language, const std::string& name, const std::string& holder, int unit = 1) {
        return {language, name, holder, unit, language == "C++", false, true};
    }
}

TEST_CASE("a global keeps each type or namespace that holds a global of its name") {
    globalVariables globals;
    globals.add(declared("C++", "count", "Registry", false));
    globals.add(declared("C++", "count", "", true));
    globals.add(declared("C++", "level", "settings", true));
    globals.add(declared("C++", "instances", "Registry", false));

    CHECK(globals.holds("C++", "Registry", "count", 2));
    CHECK(globals.holds("C++", "", "count", 2));
    CHECK(globals.holds("C++", "settings", "level", 2));
    CHECK_FALSE(globals.holds("C++", "", "level", 2));
    CHECK_FALSE(globals.holds("C++", "Registry", "level", 2));
    CHECK_FALSE(globals.holds("C++", "Registry", "width", 2));
}

TEST_CASE("a name without a qualifier reaches the globals that a namespace holds") {
    globalVariables globals;
    globals.add(declared("C++", "level", "settings", true));
    globals.add(declared("C++", "instances", "Registry", false));

    CHECK(globals.atNamespaceScope("C++", "level", 2));
    CHECK_FALSE(globals.atNamespaceScope("C++", "instances", 2));
    CHECK_FALSE(globals.atNamespaceScope("C++", "width", 2));
}

TEST_CASE("a unit uses only the globals of its own language") {
    globalVariables globals;
    globals.add(declared("Java", "COUNT", "Config", false));

    CHECK(globals.holds("Java", "Config", "COUNT", 2));
    CHECK_FALSE(globals.holds("C#", "Config", "COUNT", 2));
    CHECK_FALSE(globals.holds("C++", "Config", "COUNT", 2));
}

TEST_CASE("the globals of other units join the globals of the archive") {
    globalVariables archive;
    archive.add(declared("C++", "g_total", "metrics", false));

    globalVariables unit;
    unit.add(declared("C++", "g_total", "", true, 2));
    unit.add(declared("C#", "Level", "Settings", false, 2));
    archive.merge(unit);

    CHECK(archive.holds("C++", "", "g_total", 3));
    CHECK(archive.holds("C++", "metrics", "g_total", 3));
    CHECK(archive.atNamespaceScope("C++", "g_total", 3));
    CHECK(archive.holds("C#", "Settings", "Level", 3));
    CHECK_FALSE(archive.atNamespaceScope("C#", "Level", 3));
}

TEST_CASE("only the code of its own unit reaches a global with internal linkage") {
    globalVariables archive;
    globalVariables unit;
    unit.add(fileLocal("count", "", 2));
    unit.add(fileLocal("misses", "stats", 2));
    archive.merge(unit);

    CHECK(archive.atNamespaceScope("C++", "count", 2));
    CHECK(archive.holds("C++", "", "count", 2));
    CHECK(archive.holds("C++", "stats", "misses", 2));
    CHECK_FALSE(archive.atNamespaceScope("C++", "count", 3));
    CHECK_FALSE(archive.holds("C++", "", "count", 3));
    CHECK_FALSE(archive.holds("C++", "stats", "misses", 3));
}

TEST_CASE("a unit declares the globals at namespace scope that it defines or declares, extern declarations included") {
    globalVariables archive;
    archive.add(declared("C++", "width", "", true, 1));
    archive.add(declared("C++", "height", "", true, 1));
    archive.add(declared("C++", "height", "", true, 2));
    archive.add(fileLocal("count", "", 1));
    archive.add(declared("C++", "s_items", "Registry", false, 2));

    CHECK(archive.declaredAtNamespaceScope(1, "width"));
    CHECK(archive.declaredAtNamespaceScope(1, "count"));
    CHECK(archive.declaredAtNamespaceScope(2, "height"));
    CHECK_FALSE(archive.declaredAtNamespaceScope(2, "width"));
    CHECK_FALSE(archive.declaredAtNamespaceScope(2, "count"));
    CHECK_FALSE(archive.declaredAtNamespaceScope(2, "s_items"));
}

TEST_CASE("a global is immutable when every declaration of its name that the unit reaches is immutable") {
    globalVariables archive;
    archive.add(constant("Java", "PREFIX", "Service"));
    archive.add(constant("C#", "Name", "Names"));
    archive.add(declared("C#", "Name", "Other", false));
    archive.add(constant("C++", "g_name", ""));
    archive.add(fileLocal("g_name", "", 2));
    archive.add(fileLocal("kMax", "", 3, true));

    CHECK(archive.isImmutable("Java", "PREFIX", 4));
    CHECK_FALSE(archive.isImmutable("C#", "Name", 4));
    CHECK(archive.isImmutable("C++", "g_name", 3));
    CHECK_FALSE(archive.isImmutable("C++", "g_name", 2));
    CHECK(archive.isImmutable("C++", "kMax", 3));
    CHECK_FALSE(archive.isImmutable("C++", "kMax", 4));
    CHECK_FALSE(archive.isImmutable("Java", "COUNT", 4));
}
