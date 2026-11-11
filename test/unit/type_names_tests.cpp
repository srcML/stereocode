// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file type_names_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "rules/primitives.hpp"
#include "rules/specifier_pattern.hpp"
#include "rules/specifiers.hpp"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

using names = std::vector<std::string>;

namespace {
    // Gets the specifiers and the primitives of the defaults with the lines of user files
    //
    struct typeTables {
        specifiers typeSpecifiers;
        primitives primitiveTypes;

        explicit typeTables(const std::string& userSpecifiers = "", const std::string& userPrimitives = "") {
            std::istringstream specifierFile(userSpecifiers);
            std::istringstream primitiveFile(userPrimitives);
            specifierFile >> typeSpecifiers;
            primitiveFile >> primitiveTypes;
            typeSpecifiers.initializeSpecifiersList();
            primitiveTypes.initializePrimitivesList();
        }

        // Tells whether a variable of a type is non-primitive and whether it is external to a type
        //
        std::pair<bool, bool> facts(const std::string& type, const std::string& language, const std::string& typeName) const {
            variableModel variable;
            variable.setType(type);
            primitiveTypes.isNonPrimitive(variable, language, typeName, typeSpecifiers);
            return {variable.isNonPrimitive(), variable.isNonPrimitiveExternal()};
        }
    };
}

TEST_CASE("a pattern replaces whole words, escaped characters, and annotations as the regular expression of the defaults does") {
    const specifierPattern pattern({"const", "\\*", "&", "@\\w+", "\\.\\.\\.", "::iterator"}, {});
    CHECK(pattern.replace("const int*") == "  int ");
    CHECK(pattern.replace("constant") == "constant");
    CHECK(pattern.replace("@Nullable String...") == "  String ");
    CHECK(pattern.replace("std::vector<int>::iterator") == "std::vector<int> ");
}

TEST_CASE("a pattern splits a type between its specifiers instead of gluing the names on both sides") {
    const specifierPattern pattern({"<", ">", "\\*", "array"}, {});
    CHECK(pattern.split("Node<T>*") == names{"Node", "T", "", ""});
    CHECK(pattern.split("std::array<int, 3>") == names{"std::", "", "int, 3", ""});
}

TEST_CASE("a user specifier is plain text, whatever characters of a regular expression it holds") {
    const specifierPattern pattern({"const"}, {"*", "[[nodiscard]]", "(", "Q_DECL"});
    CHECK(pattern.replace("[[nodiscard]] int*") == "  int ");
    CHECK(pattern.replace("f(") == "f ");
    CHECK(pattern.replace("Q_DECL Q_DECLARE") == "  Q_DECLARE");
}

TEST_CASE("a container that also exists without generic arguments is a specifier only before its argument list") {
    const specifierPattern pattern({"IEnumerable(?=<)", "<", ">"}, {});
    CHECK(pattern.replace("IEnumerable<int>") == "  int ");
    CHECK(pattern.replace("IEnumerable") == "IEnumerable");
    CHECK(pattern.replace("MyIEnumerable<int>") == "MyIEnumerable int ");
}

TEST_CASE("the names of a type drop namespaces, array brackets, and integer arguments") {
    const typeTables tables;
    CHECK(tables.typeSpecifiers.typeNames("Node<T>*", "C++") == names{"Node", "T"});
    CHECK(tables.typeSpecifiers.typeNames("std::array<int, 3>", "C++") == names{"int"});
    CHECK(tables.typeSpecifiers.typeNames("const std::map<std::string, Foo>&", "C++") == names{"string", "Foo"});
    CHECK(tables.typeSpecifiers.typeNames("unsigned long int[4]", "C++") == names{"longint"});
    CHECK(tables.typeSpecifiers.typeNames("[NotNullWhen(true)] out Version?", "C#") == names{"Version"});
}

TEST_CASE("a field of the class's own generic type or of a container of primitives is no external object") {
    const typeTables tables;
    CHECK(tables.facts("Node<T>*", "C++", "Node") == std::pair<bool, bool>{true, false});
    CHECK(tables.facts("Node<T>", "C#", "Node") == std::pair<bool, bool>{true, false});
    CHECK(tables.facts("std::array<int, 3>", "C++", "Dims") == std::pair<bool, bool>{false, false});
    CHECK(tables.facts("SplitVector<std::string>", "C++", "Holder") == std::pair<bool, bool>{true, true});
}

TEST_CASE("the defaults hold the string types of C++, Java Boolean, and the collection interfaces") {
    const typeTables tables;
    CHECK(tables.facts("std::wstring", "C++", "Label") == std::pair<bool, bool>{false, false});
    CHECK(tables.facts("std::string_view", "C++", "Label") == std::pair<bool, bool>{false, false});
    CHECK(tables.facts("Boolean", "Java", "Calc") == std::pair<bool, bool>{false, false});
    CHECK(tables.facts("Collection<Integer>", "Java", "Calc") == std::pair<bool, bool>{false, false});
    CHECK(tables.facts("IEnumerable<int>", "C#", "Calc") == std::pair<bool, bool>{false, false});
    CHECK(tables.facts("IEnumerable", "C#", "Calc") == std::pair<bool, bool>{true, true});
}

TEST_CASE("the calling conventions of Windows are C++ specifiers, which leave the names of a type") {
    const typeTables tables;
    for (const std::string convention : {"__stdcall", "__cdecl", "__fastcall", "__thiscall", "__vectorcall", "__clrcall", "WINAPI", "WINAPIV",
                                         "APIENTRY", "CALLBACK", "PASCAL", "STDMETHODCALLTYPE", "STDAPICALLTYPE"}) {
        CAPTURE(convention);
        CHECK(tables.typeSpecifiers.typeNames("int " + convention, "C++") == names{"int"});
    }
    CHECK(tables.typeSpecifiers.typeNames("LRESULT CALLBACK", "C++") == names{"LRESULT"});
    CHECK(tables.facts("int WINAPI", "C++", "Window") == std::pair<bool, bool>{false, false});
    CHECK(tables.typeSpecifiers.typeNames("int WINAPI", "C#") == names{"intWINAPI"});
}

TEST_CASE("user files add primitives and specifiers whatever their line ends") {
    const typeTables tables("Q_DECL\r\n", "# Qt types\r\nQString \r\n\r\n");
    CHECK(tables.facts("QString", "C++", "Label") == std::pair<bool, bool>{false, false});
    CHECK(tables.facts("Q_DECL int", "C++", "Label") == std::pair<bool, bool>{false, false});
}

TEST_CASE("a primitive or a specifier under a section header holds only for the units of its language") {
    const typeTables tables("[C#]\nQ_DECL\n", "[C++]\nQString\n");
    CHECK(tables.facts("QString", "C++", "Label") == std::pair<bool, bool>{false, false});
    CHECK(tables.facts("QString", "C#", "Label") == std::pair<bool, bool>{true, true});
    CHECK(tables.typeSpecifiers.typeNames("Q_DECL Size", "C#") == names{"Size"});
    CHECK(tables.typeSpecifiers.typeNames("Q_DECL Size", "C++") == names{"Q_DECLSize"});
}
