// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file type_index_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "model/type_index.hpp"

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {
    // Gets a type of a language at a path, defined in a unit
    //
    indexedType typeAt(const std::string& language, std::vector<std::string> path, int unitNumber, const std::string& fileName,
                       const std::string& arguments = "", int arity = 0) {
        indexedType type;
        type.language   = language;
        type.path       = std::move(path);
        type.arguments  = arguments;
        type.arity      = arity;
        type.unitNumber = unitNumber;
        type.fileName   = fileName;
        return type;
    }

    // Gets a name that code of a language writes in a scope of a unit
    //
    typeReference nameIn(const std::string& language, std::vector<std::string> path, std::vector<std::string> scope, int unitNumber,
                         const std::string& fileName, const std::string& arguments = "", int arity = 0) {
        typeReference reference;
        reference.language   = language;
        reference.path       = std::move(path);
        reference.arguments  = arguments;
        reference.arity      = arity;
        reference.scope      = std::move(scope);
        reference.unitNumber = unitNumber;
        reference.fileName   = fileName;
        return reference;
    }

    // Gets a C++ name at a place of a unit, the qualifier of a method defined outside its type, with the using directives of the unit
    //
    typeReference qualifierAt(std::vector<std::string> path, std::vector<std::string> scope, int unitNumber, elementPath position,
                              std::vector<usingDirective> directives, const std::string& arguments = "") {
        typeReference reference = nameIn("C++", std::move(path), std::move(scope), unitNumber, "unit.cpp", arguments, arguments.empty() ? 0 : 1);
        reference.position   = std::move(position);
        reference.directives = std::move(directives);
        return reference;
    }

    // Gets a using directive of a namespace at a place of its unit, inside the namespaces of a scope
    //
    usingDirective directiveAt(std::vector<std::string> name, std::vector<std::string> scope, elementPath position) {
        usingDirective directive;
        directive.name     = std::move(name);
        directive.scope    = std::move(scope);
        directive.position = std::move(position);
        return directive;
    }

    // Gets a C++ name that the type at a place of a unit writes in a scope, such as the type of a data member
    //
    typeReference cppNameAt(std::vector<std::string> path, std::vector<std::string> scope, int unitNumber, elementPath position) {
        typeReference reference = nameIn("C++", std::move(path), std::move(scope), unitNumber, "unit.cpp");
        reference.position = std::move(position);
        return reference;
    }

    // Gets a C++ type at a path, defined at a place of a unit
    //
    indexedType cppTypeAt(std::vector<std::string> path, int unitNumber, elementPath position, const std::string& arguments = "") {
        indexedType type = typeAt("C++", std::move(path), unitNumber, "unit.cpp", arguments, arguments.empty() ? 0 : 1);
        type.position = std::move(position);
        return type;
    }

    // Gets an alias of a language at a path, declared in a unit, with the named type of its target and, for an alias of a pointer, of its pointee
    //
    indexedAlias aliasAt(const std::string& language, std::vector<std::string> path, int unitNumber, const std::string& fileName,
                         std::vector<std::string> target, std::vector<std::string> pointee = {}) {
        const std::vector<std::string> scope(path.begin(), path.end() - 1);
        indexedAlias alias;
        alias.place   = typeAt(language, std::move(path), unitNumber, fileName);
        alias.target  = nameIn(language, std::move(target), scope, unitNumber, fileName);
        alias.pointee = nameIn(language, std::move(pointee), scope, unitNumber, fileName);
        return alias;
    }

    // Gets a type as written from its text, where each name of a type stands in brackets, as const[Count]& of const Count&
    //
    writtenType writtenFrom(const std::string& text) {
        writtenType type;
        for (std::size_t start = 0; start < text.size();) {
            const std::size_t open = text.find('[', start);
            addText(type, text.substr(start, open == std::string::npos ? std::string::npos : open - start));
            if (open == std::string::npos) break;
            const std::size_t close = text.find(']', open);
            type.push_back({text.substr(open + 1, close - open - 1), true});
            start = close + 1;
        }
        return type;
    }

    // Gets a C++ alias at a path, declared in a unit, with the named type of its target and the type that it stands for as written
    //
    indexedAlias spelledAliasAt(std::vector<std::string> path, int unitNumber, const std::string& fileName, std::vector<std::string> target,
                                const std::string& spelled) {
        indexedAlias alias = aliasAt("C++", std::move(path), unitNumber, fileName, std::move(target));
        alias.spelledType = writtenFrom(spelled);
        return alias;
    }

    // Gets the text of a type written at the place of a name, in which the names of aliases give way to their types
    //
    std::string spelledAt(const typeIndex& index, const std::string& written, const typeReference& place) {
        return typeText(index.withoutAliases(writtenFrom(written), place));
    }
}

TEST_CASE("a name resolves through the namespaces around it, from the innermost") {
    typeIndex index;
    index.add(typeAt("C++", {"list", "Node"}, 1, "list.cpp"));
    index.add(typeAt("C++", {"tree", "Node"}, 2, "tree.cpp"));
    index.add(typeAt("C++", {"Node"}, 3, "node.cpp"));

    CHECK(index.find(nameIn("C++", {"Node"}, {"tree"}, 4, "use.cpp")) == std::optional<std::size_t>(1));
    CHECK(index.find(nameIn("C++", {"Node"}, {"list"}, 4, "use.cpp")) == std::optional<std::size_t>(0));
    CHECK(index.find(nameIn("C++", {"Node"}, {}, 4, "use.cpp")) == std::optional<std::size_t>(2));
    CHECK(index.find(nameIn("C++", {"list", "Node"}, {"tree"}, 4, "use.cpp")) == std::optional<std::size_t>(0));
}

TEST_CASE("a name that no namespace around it explains names the only type whose path ends with it") {
    typeIndex index;
    index.add(typeAt("Java", {"a", "Node"}, 1, "a/Node.java"));
    index.add(typeAt("Java", {"b", "Node"}, 2, "b/Node.java"));
    index.add(typeAt("Java", {"c", "Shape"}, 3, "c/Shape.java"));

    CHECK(index.find(nameIn("Java", {"Shape"}, {"d"}, 4, "d/Square.java")) == std::optional<std::size_t>(2));
    CHECK_FALSE(index.find(nameIn("Java", {"Node"}, {"d"}, 4, "d/Tree.java")));
    CHECK(index.find(nameIn("Java", {"b", "Node"}, {"d"}, 4, "d/Tree.java")) == std::optional<std::size_t>(1));
}

TEST_CASE("a name never resolves to a type of another language or to an excluded type") {
    typeIndex index;
    index.add(typeAt("Java", {"Node"}, 1, "Node.java"));
    index.add(typeAt("C++", {"Widget"}, 2, "widget.hpp"));

    CHECK_FALSE(index.find(nameIn("C++", {"Node"}, {}, 3, "node.cpp")));
    CHECK_FALSE(index.find(nameIn("C++", {"ui", "Widget"}, {}, 2, "widget.hpp"), std::size_t{1}));
    CHECK_FALSE(index.find(nameIn("C++", {"Widget"}, {}, 2, "widget.hpp"), std::size_t{1}));
}

TEST_CASE("among copies of a type, the copy of the unit of the name and then the copy in the nearest folder wins") {
    typeIndex index;
    index.add(typeAt("C++", {"Catch", "Session"}, 1, "lexilla/test/unit/catch.hpp"));
    index.add(typeAt("C++", {"Catch", "Session"}, 2, "scintilla/test/unit/catch.hpp"));

    CHECK(index.find(nameIn("C++", {"Session"}, {"Catch"}, 2, "scintilla/test/unit/catch.hpp")) == std::optional<std::size_t>(1));
    CHECK(index.find(nameIn("C++", {"Session"}, {"Catch"}, 5, "lexilla/test/unit/main.cxx")) == std::optional<std::size_t>(0));
}

TEST_CASE("C# and Java tell generic types apart by their arity, while C++ prefers a specialization with the written arguments") {
    typeIndex index;
    index.add(typeAt("C#", {"Box"}, 1, "Box.cs"));
    index.add(typeAt("C#", {"Box"}, 2, "BoxT.cs", "<T>", 1));
    index.add(typeAt("C++", {"Box"}, 3, "box.hpp"));
    index.add(typeAt("C++", {"Box"}, 3, "box.hpp", "<int>", 1));

    CHECK(index.find(nameIn("C#", {"Box"}, {}, 4, "Use.cs", "<int>", 1)) == std::optional<std::size_t>(1));
    CHECK(index.find(nameIn("C#", {"Box"}, {}, 4, "Use.cs")) == std::optional<std::size_t>(0));
    CHECK(index.find(nameIn("C++", {"Box"}, {}, 4, "use.cpp", "<int>", 1)) == std::optional<std::size_t>(3));
    CHECK(index.find(nameIn("C++", {"Box"}, {}, 4, "use.cpp", "<T>", 1)) == std::optional<std::size_t>(2));
}

TEST_CASE("a C++ name resolves through the using directives before it when they reach exactly one type") {
    typeIndex index;
    index.add(cppTypeAt({"Gtk", "Canvas"}, 1, {1}));
    index.add(cppTypeAt({"Qt", "Canvas"}, 2, {1}));

    const usingDirective gtk = directiveAt({"Gtk"}, {}, {1});
    const usingDirective qt  = directiveAt({"Qt"}, {}, {2});
    CHECK_FALSE(index.find(qualifierAt({"Canvas"}, {}, 3, {5}, {})));
    CHECK(index.find(qualifierAt({"Canvas"}, {}, 3, {5}, {gtk})) == std::optional<std::size_t>(0));
    CHECK(index.find(qualifierAt({"Canvas"}, {}, 3, {5}, {qt})) == std::optional<std::size_t>(1));
    CHECK_FALSE(index.find(qualifierAt({"Canvas"}, {}, 3, {5}, {gtk, qt})));
    CHECK_FALSE(index.find(qualifierAt({"Canvas"}, {}, 3, {5}, {directiveAt({"Gtk"}, {}, {6})})));
}

TEST_CASE("a using directive holds in its namespace and the namespaces inside it, also in a later block, but not outside it") {
    typeIndex index;
    index.add(cppTypeAt({"Lib", "Detail", "Widget"}, 1, {1}));
    index.add(cppTypeAt({"App", "Widget"}, 2, {1}));
    index.add(cppTypeAt({"Lib", "Inner", "Detail", "Gauge"}, 1, {2}));

    const usingDirective inLib = directiveAt({"Lib", "Detail"}, {"Lib"}, {2, 1});
    const usingDirective intoInner = directiveAt({"Inner", "Detail"}, {"Lib"}, {2, 2});
    CHECK(index.find(qualifierAt({"Widget"}, {"Lib"}, 3, {2, 3}, {inLib})) == std::optional<std::size_t>(0));
    CHECK(index.find(qualifierAt({"Widget"}, {"Lib"}, 3, {4, 1}, {inLib})) == std::optional<std::size_t>(0));
    CHECK(index.find(qualifierAt({"Gauge"}, {"Lib", "Inner"}, 3, {4, 1, 1}, {intoInner})) == std::optional<std::size_t>(2));
    CHECK_FALSE(index.find(qualifierAt({"Widget"}, {}, 3, {5}, {inLib})));
    CHECK_FALSE(index.find(qualifierAt({"Widget"}, {"Tools"}, 3, {5, 1}, {inLib})));
}

TEST_CASE("a using directive never attaches a definition to a class outside the namespaces around the definition") {
    typeIndex index;
    index.add(cppTypeAt({"Lib", "Widget"}, 1, {1}));
    index.add(cppTypeAt({"Gui", "Widget"}, 1, {2}));

    const std::vector<usingDirective> directives{directiveAt({"Lib"}, {}, {3})};
    CHECK(index.find(qualifierAt({"Widget"}, {}, 1, {4}, directives)) == std::optional<std::size_t>(0));
    CHECK_FALSE(index.find(qualifierAt({"Widget"}, {"App"}, 1, {5, 1}, directives)));
}

TEST_CASE("a using directive names its namespace from the namespaces around the directive, from the innermost") {
    typeIndex index;
    index.add(cppTypeAt({"Lib", "Detail", "Widget"}, 1, {1}));
    index.add(cppTypeAt({"Detail", "Widget"}, 2, {1}));
    index.add(cppTypeAt({"App", "Widget"}, 3, {1}));

    CHECK(index.find(qualifierAt({"Widget"}, {"Lib"}, 4, {1, 2}, {directiveAt({"Detail"}, {"Lib"}, {1, 1})})) == std::optional<std::size_t>(0));
    CHECK(index.find(qualifierAt({"Widget"}, {}, 4, {2}, {directiveAt({"Detail"}, {}, {1})})) == std::optional<std::size_t>(1));
}

TEST_CASE("the namespaces around a C++ name come before its using directives") {
    typeIndex index;
    index.add(cppTypeAt({"Lib", "Widget"}, 1, {1}));
    index.add(cppTypeAt({"App", "Widget"}, 2, {1}));

    CHECK(index.find(qualifierAt({"Widget"}, {"App"}, 3, {2, 1}, {directiveAt({"Lib"}, {}, {1})})) == std::optional<std::size_t>(1));
}

TEST_CASE("among the types that using directives reach, the types that the unit of the name declares before it come first") {
    typeIndex index;
    index.add(cppTypeAt({"Lib", "Detail", "Surface"}, 1, {1}));
    index.add(cppTypeAt({"Lib", "Surface"}, 2, {4}));
    index.add(cppTypeAt({"Lib", "Detail", "Pen"}, 1, {2}));
    index.add(cppTypeAt({"Lib", "Pen"}, 2, {9}));
    index.add(cppTypeAt({"Win", "Brush"}, 2, {5}));
    index.add(cppTypeAt({"Lib", "Brush"}, 2, {6}));

    const std::vector<usingDirective> directives{directiveAt({"Lib"}, {}, {2}), directiveAt({"Lib", "Detail"}, {}, {3}), directiveAt({"Win"}, {}, {7})};
    CHECK(index.find(qualifierAt({"Surface"}, {}, 2, {8}, directives)) == std::optional<std::size_t>(1));
    CHECK(index.find(qualifierAt({"Pen"}, {}, 2, {8}, directives)) == std::optional<std::size_t>(2));
    CHECK_FALSE(index.find(qualifierAt({"Brush"}, {}, 2, {8}, directives)));
}

TEST_CASE("a using directive reaches a C++ class template and the specialization with the written arguments") {
    typeIndex index;
    index.add(cppTypeAt({"Lib", "Box"}, 1, {1, 1}));
    index.add(cppTypeAt({"Lib", "Box"}, 1, {1, 2}, "<int>"));
    index.add(cppTypeAt({"Other", "Box"}, 1, {2, 1}));

    const std::vector<usingDirective> directives{directiveAt({"Lib"}, {}, {4})};
    CHECK(index.find(qualifierAt({"Box"}, {}, 1, {5}, directives, "<T>")) == std::optional<std::size_t>(0));
    CHECK(index.find(qualifierAt({"Box"}, {}, 1, {6}, directives, "<int>")) == std::optional<std::size_t>(1));
}

TEST_CASE("an alias leads to the type that it names, also through other aliases, and through -> to the type that its smart pointer points to") {
    typeIndex index;
    index.add(typeAt("C++", {"Catch", "IStreamingReporter"}, 1, "catch.hpp"));
    index.add(typeAt("C++", {"Engine"}, 2, "engine.hpp"));
    index.addAlias(aliasAt("C++", {"Catch", "IStreamingReporterPtr"}, 1, "catch.hpp", {"std", "unique_ptr"}, {"IStreamingReporter"}));
    index.addAlias(aliasAt("C++", {"Motor"}, 2, "engine.hpp", {"Engine"}));
    index.addAlias(aliasAt("C++", {"Drive"}, 2, "engine.hpp", {"Motor"}));

    const typeReference reporter = nameIn("C++", {"IStreamingReporterPtr"}, {"Catch"}, 1, "catch.hpp");
    CHECK(index.findThroughAliases(reporter, true) == std::vector<std::size_t>{0});
    CHECK(index.findThroughAliases(reporter, false).empty());
    CHECK(index.findThroughAliases(nameIn("C++", {"Drive"}, {}, 3, "car.cpp"), false) == std::vector<std::size_t>{1});
    CHECK(index.findThroughAliases(nameIn("C++", {"Engine"}, {}, 3, "car.cpp"), false) == std::vector<std::size_t>{1});
    CHECK_FALSE(index.find(nameIn("C++", {"Drive"}, {}, 3, "car.cpp")));
}

TEST_CASE("-> reaches the object of the first alias of a pointer on the way, after which aliases lead to the types that they name") {
    typeIndex index;
    index.add(typeAt("C++", {"Engine"}, 1, "engine.hpp"));
    index.addAlias(aliasAt("C++", {"EnginePtr"}, 1, "engine.hpp", {"std", "unique_ptr"}, {"Engine"}));
    index.addAlias(aliasAt("C++", {"Motor"}, 1, "engine.hpp", {"EnginePtr"}));
    index.addAlias(aliasAt("C++", {"Slot"}, 1, "engine.hpp", {"std", "unique_ptr"}, {"EnginePtr"}));

    CHECK(index.findThroughAliases(nameIn("C++", {"Motor"}, {}, 1, "engine.hpp"), true) == std::vector<std::size_t>{0});
    CHECK(index.findThroughAliases(nameIn("C++", {"Slot"}, {}, 1, "engine.hpp"), true).empty());
}

TEST_CASE("a cycle of aliases and an alias of a type outside the archive lead to no type") {
    typeIndex index;
    index.addAlias(aliasAt("C++", {"Loop"}, 1, "ring.hpp", {"Ring"}));
    index.addAlias(aliasAt("C++", {"Ring"}, 1, "ring.hpp", {"Loop"}));
    index.addAlias(aliasAt("C++", {"XYPOSITION"}, 1, "ring.hpp", {"double"}));

    CHECK(index.findThroughAliases(nameIn("C++", {"Loop"}, {}, 1, "ring.hpp"), false).empty());
    CHECK(index.findThroughAliases(nameIn("C++", {"XYPOSITION"}, {}, 1, "ring.hpp"), false).empty());
}

TEST_CASE("a name of an alias in a written type gives way to the type that the alias stands for, also through other aliases and in template arguments") {
    typeIndex index;
    index.addAlias(spelledAliasAt({"Count"}, 1, "count.hpp", {"int"}, "[int]"));
    index.addAlias(spelledAliasAt({"Total"}, 1, "count.hpp", {"Count"}, "[Count]"));
    index.addAlias(spelledAliasAt({"Size"}, 1, "count.hpp", {"std", "size_t"}, "[std::size_t]"));
    index.addAlias(spelledAliasAt({"CountPtr"}, 1, "count.hpp", {"int"}, "[int]*"));
    index.addAlias(spelledAliasAt({"TotalCPtr"}, 1, "count.hpp", {"Total"}, "const[Total]*"));
    index.addAlias(spelledAliasAt({"Byte"}, 1, "count.hpp", {}, "[unsigned][char]"));
    const typeReference meter = nameIn("C++", {}, {"Meter"}, 2, "meter.cpp");

    CHECK(spelledAt(index, "const[Total]&", meter) == "constint&");
    CHECK(spelledAt(index, "[Size]", meter) == "std::size_t");
    CHECK(spelledAt(index, "[CountPtr]", meter) == "int*");
    CHECK(spelledAt(index, "[TotalCPtr]", meter) == "constint*");
    CHECK(spelledAt(index, "[Byte]", meter) == "unsignedchar");
    CHECK(spelledAt(index, "std::map<[Count],[Byte]>", meter) == "std::map<int,unsignedchar>");
    CHECK(spelledAt(index, "[unsigned][int]", meter) == "unsignedint");
}

TEST_CASE("a name after const or volatile keeps an alias of a pointer or a reference, unlike an alias of a template of pointers") {
    typeIndex index;
    index.addAlias(spelledAliasAt({"CountPtr"}, 1, "count.hpp", {"int"}, "[int]*"));
    index.addAlias(spelledAliasAt({"CountRef"}, 1, "count.hpp", {"int"}, "[int]&"));
    index.addAlias(spelledAliasAt({"Counts"}, 1, "count.hpp", {"std", "vector"}, "std::vector<[int]*>"));
    const typeReference meter = nameIn("C++", {}, {"Meter"}, 2, "meter.cpp");

    CHECK(spelledAt(index, "const[CountPtr]", meter) == "constCountPtr");
    CHECK(spelledAt(index, "volatile[CountRef]", meter) == "volatileCountRef");
    CHECK(spelledAt(index, "[CountPtr]const", meter) == "int*const");
    CHECK(spelledAt(index, "const[Counts]&", meter) == "conststd::vector<int*>&");
}

TEST_CASE("an alias that a type declares stands for its type in the names in the scope of the type and in the names that write the type") {
    typeIndex index;
    indexedAlias offset = spelledAliasAt({"Buffer", "Offset"}, 1, "buffer.hpp", {"long"}, "[long]");
    offset.enclosingTypes = 1;
    index.addAlias(offset);

    CHECK(spelledAt(index, "[Offset]", nameIn("C++", {}, {"Buffer"}, 2, "buffer.cpp")) == "long");
    CHECK(spelledAt(index, "[Buffer::Offset]", nameIn("C++", {}, {"Buffer"}, 2, "buffer.cpp")) == "long");
    CHECK(spelledAt(index, "[Offset]", nameIn("C++", {}, {"Stream"}, 2, "stream.cpp")) == "Offset");
}

TEST_CASE("the names of a cycle of aliases stay as written, as does a name of an alias that a type of the same or a nearer scope hides") {
    typeIndex index;
    index.add(typeAt("C++", {"plant", "Pump"}, 1, "plant.hpp"));
    index.addAlias(spelledAliasAt({"Loop"}, 1, "ring.hpp", {"Ring"}, "[Ring]"));
    index.addAlias(spelledAliasAt({"Ring"}, 1, "ring.hpp", {"Loop"}, "[Loop]"));
    index.addAlias(spelledAliasAt({"Pump"}, 1, "plant.hpp", {"Engine"}, "[Engine]"));

    CHECK(spelledAt(index, "[Loop]", nameIn("C++", {}, {}, 1, "ring.hpp")) == "Loop");
    CHECK(spelledAt(index, "[Ring]", nameIn("C++", {}, {}, 1, "ring.hpp")) == "Ring");
    CHECK(spelledAt(index, "[Pump]", nameIn("C++", {}, {"plant", "Station"}, 1, "plant.hpp")) == "Pump");
    CHECK(spelledAt(index, "[Pump]", nameIn("C++", {}, {"depot"}, 1, "plant.hpp")) == "Engine");
}

TEST_CASE("an alias that takes the name of the type that it names leads to that type") {
    typeIndex index;
    index.add(typeAt("C++", {"Engine"}, 1, "garage.hpp"));
    index.addAlias(aliasAt("C++", {"garage", "Engine"}, 1, "garage.hpp", {"Engine"}));

    CHECK(index.findThroughAliases(nameIn("C++", {"Engine"}, {"garage", "Car"}, 1, "garage.hpp"), false) == std::vector<std::size_t>{0});
}

TEST_CASE("an alias that a type declares holds outside the type only for a name that writes the type") {
    typeIndex index;
    index.add(typeAt("C++", {"Pool"}, 1, "arena.hpp"));
    indexedAlias allocator = aliasAt("C++", {"store", "Arena", "Allocator"}, 1, "arena.hpp", {"Pool"});
    allocator.enclosingTypes = 1;
    index.addAlias(allocator);

    CHECK(index.findThroughAliases(nameIn("C++", {"Allocator"}, {"store", "Arena"}, 1, "arena.hpp"), false) == std::vector<std::size_t>{0});
    CHECK(index.findThroughAliases(nameIn("C++", {"Arena", "Allocator"}, {}, 2, "depot.cpp"), false) == std::vector<std::size_t>{0});
    CHECK(index.findThroughAliases(nameIn("C++", {"Allocator"}, {"Vector"}, 2, "vector.hpp"), false).empty());
}

TEST_CASE("an alias of a namespace around the name hides a type that only a farther scope or a using directive reaches, unlike a type of the same or a nearer scope") {
    typeIndex index;
    index.add(typeAt("C++", {"Engine"}, 1, "plant.hpp"));
    index.add(typeAt("C++", {"parts", "Motor"}, 1, "plant.hpp"));
    index.add(typeAt("C++", {"plant", "Pump"}, 1, "plant.hpp"));
    index.add(typeAt("C++", {"Point"}, 1, "plant.hpp"));
    index.addAlias(aliasAt("C++", {"fleet", "Motor"}, 1, "plant.hpp", {"Engine"}));
    index.addAlias(aliasAt("C++", {"Pump"}, 1, "plant.hpp", {"Engine"}));
    index.addAlias(aliasAt("C++", {"Point"}, 1, "plant.hpp", {"Engine"}));

    CHECK(index.findThroughAliases(nameIn("C++", {"Motor"}, {"fleet"}, 1, "plant.hpp"), false) == std::vector<std::size_t>{0});
    CHECK(index.findThroughAliases(nameIn("C++", {"Motor"}, {"depot"}, 1, "plant.hpp"), false) == std::vector<std::size_t>{1});
    CHECK(index.findThroughAliases(nameIn("C++", {"Pump"}, {"plant"}, 1, "plant.hpp"), false) == std::vector<std::size_t>{2});
    CHECK(index.findThroughAliases(nameIn("C++", {"Point"}, {}, 1, "plant.hpp"), false) == std::vector<std::size_t>{3});
}

TEST_CASE("an alias hides the types of a farther scope only from the code of its unit after it and from the type that declares it") {
    typeIndex index;
    index.add(cppTypeAt({"Widget"}, 1, {1}));
    index.add(cppTypeAt({"Engine"}, 1, {2}));
    indexedAlias inNamespace = aliasAt("C++", {"app", "Widget"}, 1, "unit.cpp", {"Engine"});
    inNamespace.place.position = {4, 2, 1};
    indexedAlias inType = aliasAt("C++", {"Truck", "Widget"}, 1, "unit.cpp", {"Engine"});
    inType.place.position = {6, 2, 1};
    index.addAlias(inNamespace);
    index.addAlias(inType);

    CHECK(index.findThroughAliases(cppNameAt({"Widget"}, {"app", "Panel"}, 1, {4, 2, 0}), false) == std::vector<std::size_t>{0});
    CHECK(index.findThroughAliases(cppNameAt({"Widget"}, {"app", "Gauge"}, 1, {4, 2, 2}), false) == std::vector<std::size_t>{1});
    CHECK(index.findThroughAliases(cppNameAt({"Widget"}, {"Truck"}, 1, {6}), false) == std::vector<std::size_t>{1});
}

TEST_CASE("an alias of another unit hides the types of a farther scope only where that unit may be included, unlike an alias of a source file") {
    typeIndex index;
    index.add(typeAt("C++", {"parts", "Motor"}, 1, "fleet.hpp"));
    index.add(typeAt("C++", {"Engine"}, 1, "fleet.hpp"));
    indexedAlias inHeader = aliasAt("C++", {"fleet", "Motor"}, 1, "fleet.hpp", {"Engine"});
    inHeader.otherUnits = true;
    index.addAlias(inHeader);
    index.addAlias(aliasAt("C++", {"app", "Motor"}, 2, "render.cpp", {"Engine"}));

    CHECK(index.findThroughAliases(nameIn("C++", {"Motor"}, {"fleet", "Van"}, 3, "van.cpp"), false) == std::vector<std::size_t>{1});
    CHECK(index.findThroughAliases(nameIn("C++", {"Motor"}, {"app", "Panel"}, 3, "panel.hpp"), false) == std::vector<std::size_t>{0});
    CHECK(index.findThroughAliases(nameIn("C++", {"Motor"}, {"app", "Panel"}, 2, "render.cpp"), false) == std::vector<std::size_t>{1});
}

TEST_CASE("a name that types of one path in several folders fit equally well names each of them, and the nearest folder alone where one is nearer") {
    typeIndex index;
    index.add(typeAt("C++", {"priv", "CursorImpl"}, 1, "window/android/CursorImpl.hpp"));
    index.add(typeAt("C++", {"priv", "CursorImpl"}, 2, "window/win32/CursorImpl.hpp"));

    const typeReference fromApi = nameIn("C++", {"priv", "CursorImpl"}, {}, 3, "window/Cursor.hpp");
    CHECK(index.findThroughAliases(fromApi, false) == std::vector<std::size_t>{0, 1});
    CHECK(index.find(fromApi) == std::optional<std::size_t>(0));
    CHECK(index.findThroughAliases(nameIn("C++", {"priv", "CursorImpl"}, {}, 4, "window/win32/Window.cpp"), false) == std::vector<std::size_t>{1});
}

TEST_CASE("an alias of the global namespace hides a type that a using directive reaches only in the unit of the alias") {
    typeIndex index;
    index.add(typeAt("C++", {"Scintilla", "Document"}, 1, "scintilla/Document.h"));
    index.add(typeAt("C++", {"Engine"}, 2, "editor/Buffer.h"));
    index.addAlias(aliasAt("C++", {"Document"}, 2, "editor/Buffer.h", {"Engine"}));

    CHECK(index.findThroughAliases(nameIn("C++", {"Document"}, {}, 3, "boostregex/Search.cxx"), false) == std::vector<std::size_t>{0});
    CHECK(index.findThroughAliases(nameIn("C++", {"Document"}, {}, 2, "editor/Buffer.h"), false) == std::vector<std::size_t>{1});
}

TEST_CASE("a C# alias holds only in the unit that declares it") {
    typeIndex index;
    index.add(typeAt("C#", {"Shop", "Parts", "Meter"}, 1, "Dashboard.cs"));
    index.add(typeAt("C#", {"Shop", "Other", "Gauge"}, 2, "Panel.cs"));
    index.addAlias(aliasAt("C#", {"Gauge"}, 1, "Dashboard.cs", {"Shop", "Parts", "Meter"}));

    CHECK(index.findThroughAliases(nameIn("C#", {"Gauge"}, {"Shop", "Cars"}, 1, "Dashboard.cs"), false) == std::vector<std::size_t>{0});
    CHECK(index.findThroughAliases(nameIn("C#", {"Gauge"}, {"Shop", "Panels"}, 2, "Panel.cs"), false) == std::vector<std::size_t>{1});
}

TEST_CASE("among copies of an alias, the copy of the unit of the name wins, and its target resolves from the unit of the alias") {
    typeIndex index;
    index.add(typeAt("C++", {"Catch", "IStreamingReporter"}, 1, "lexilla/test/unit/catch.hpp"));
    index.add(typeAt("C++", {"Catch", "IStreamingReporter"}, 2, "scintilla/test/unit/catch.hpp"));
    index.addAlias(aliasAt("C++", {"Catch", "IStreamingReporterPtr"}, 1, "lexilla/test/unit/catch.hpp", {"std", "unique_ptr"}, {"IStreamingReporter"}));
    index.addAlias(aliasAt("C++", {"Catch", "IStreamingReporterPtr"}, 2, "scintilla/test/unit/catch.hpp", {"std", "unique_ptr"}, {"IStreamingReporter"}));

    CHECK(index.findThroughAliases(nameIn("C++", {"IStreamingReporterPtr"}, {"Catch"}, 2, "scintilla/test/unit/catch.hpp"), true) == std::vector<std::size_t>{1});
    CHECK(index.findThroughAliases(nameIn("C++", {"IStreamingReporterPtr"}, {"Catch"}, 5, "lexilla/test/unit/main.cxx"), true) == std::vector<std::size_t>{0});
}

TEST_CASE("an import of the unit of a name picks its type among the types of the name in several packages") {
    typeIndex index;
    index.add(typeAt("Java", {"model", "Tag"}, 0, "model/Tag.java"));
    index.add(typeAt("Java", {"query", "Evaluator", "Tag"}, 1, "query/Evaluator.java"));
    index.add(typeAt("Java", {"parser", "Tag"}, 2, "parser/Tag.java"));
    const typeReference written = nameIn("Java", {"Tag"}, {"view", "Element"}, 3, "view/Element.java");

    SUBCASE("without an import, the types of several paths leave the name without a type") {
        CHECK_FALSE(index.find(written));
    }
    SUBCASE("an import of one type names the type") {
        index.addImport({{"model", "Tag"}, false, 3, false});
        CHECK(index.find(written) == 0);
    }
    SUBCASE("an import of a type brings in the types inside it by a name of two parts") {
        index.addImport({{"query", "Evaluator"}, false, 3, false});
        CHECK(index.find(nameIn("Java", {"Evaluator", "Tag"}, {"view", "Element"}, 3, "view/Element.java")) == 1);
        CHECK_FALSE(index.find(written));
    }
    SUBCASE("an import of every type of a package or of a type names the type of that path") {
        index.addImport({{"parser"}, true, 3, false});
        CHECK(index.find(written) == 2);
    }
    SUBCASE("an import of one type comes before an import of every type") {
        index.addImport({{"parser"}, true, 3, false});
        index.addImport({{"query", "Evaluator", "Tag"}, false, 3, false});
        CHECK(index.find(written) == 1);
    }
    SUBCASE("an import holds in its own unit alone, unless it holds in every unit") {
        index.addImport({{"model", "Tag"}, false, 4, false});
        CHECK_FALSE(index.find(written));
        index.addImport({{"parser"}, true, 4, true});
        CHECK(index.find(written) == 2);
    }
    SUBCASE("a type of the package of the name comes before an import of every type and after an import of one type") {
        index.add(typeAt("Java", {"view", "Tag"}, 5, "view/Tag.java"));
        index.addImport({{"parser"}, true, 3, false});
        CHECK(index.find(written) == 3);
        index.addImport({{"model", "Tag"}, false, 3, false});
        CHECK(index.find(written) == 0);
    }
    SUBCASE("a type of the unit of the name comes before an import of one type") {
        index.add(typeAt("Java", {"view", "Element", "Tag"}, 3, "view/Element.java"));
        index.addImport({{"model", "Tag"}, false, 3, false});
        CHECK(index.find(written) == 3);
    }
}
