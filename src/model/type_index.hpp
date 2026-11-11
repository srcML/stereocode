// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file type_index.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef TYPE_INDEX_HPP
#define TYPE_INDEX_HPP

#include "archive/element_path.hpp"
#include "model/written_type.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// The place of a type, which name lookups compare with the names that the code writes
//
struct indexedType {
    std::string                 language;               // Language of the queries (C++ for C and C++ units, C#, or Java)
    std::vector<std::string>    path;                   // Enclosing namespaces or package, then the parts of the type name, without generic arguments
    std::string                 arguments;              // Generic argument lists of the last part, such as <int> of a C++ specialization
    int                         arity{0};               // Generic parameters of the last part
    int                         unitNumber{0};          // Unit that defines the type
    std::string                 fileName;               // File that defines the type
    elementPath                 position;               // Place of the type in its unit
};

// A C++ using directive at namespace scope, as using namespace Lib::Detail;, which brings the names of a namespace into the code after it
//
struct usingDirective {
    std::vector<std::string>    name;                   // Parts of the name of the namespace, as the directive writes it
    std::vector<std::string>    scope;                  // Namespaces around the directive, outermost first
    elementPath                 position;               // Place of the directive in its unit
};

// A name of a type as the code writes it, such as a parent or the qualifier of a method defined outside its type
//
struct typeReference {
    std::string                 language;               // Language of the queries of the code that writes the name
    std::vector<std::string>    path;                   // Parts of the name, without generic arguments
    std::string                 arguments;              // Generic argument lists of the last part
    int                         arity{0};               // Generic parameters of the last part
    std::vector<std::string>    scope;                  // Namespaces or package around the name, outermost first
    int                         unitNumber{0};          // Unit that writes the name
    std::string                 fileName;               // File that writes the name
    std::vector<usingDirective> directives;             // C++ using directives at namespace scope of the unit, for a method defined outside its type
    elementPath                 position;               // Place of the name in its unit, which no directive or type of the unit after it reaches
};

// A name that a C++ alias declaration or typedef, or a C# using alias directive, gives a type that it writes,
// as IStreamingReporterPtr of using IStreamingReporterPtr = std::unique_ptr<IStreamingReporter>;
//
struct indexedAlias {
    indexedType                 place;                  // Place of the alias, which has no generic parameters
    typeReference               target;                 // Named type that the alias stands for, written in the scope of the alias, without a path for a type of several names
    writtenType                 spelledType;            // Type that the alias stands for as written, with the modifiers of its declarator, as int and * of *CountPtr of typedef int Count, *CountPtr;
    typeReference               pointee;                // Named type of the object that -> reaches through an alias of a pointer, as IStreamingReporter, or none
    std::size_t                 enclosingTypes{0};      // Types around the alias, the last parts of the scope of its place, which a name outside them writes
    bool                        otherUnits{false};      // Holds in other units too: a C++ alias of a named namespace or type in a header, or a C# global using alias directive
};

typeReference referenceOf(const std::vector<std::string>&, const std::string&, const std::vector<std::string>&, int, const std::string&);

// Finds the type that a written name refers to among the types of an archive, also through the aliases of the archive
// A name resolves through the namespaces around it, from the innermost, then through the C++ using directives in effect at it,
// and otherwise to the only type whose path ends with it
//
class typeIndex {
public:
    void                        add                 (indexedType);
    void                        addAlias            (indexedAlias);
    std::optional<std::size_t>  find                (const typeReference&, std::optional<std::size_t> excluded = std::nullopt) const;
    std::optional<std::size_t>  findThroughAliases  (typeReference, bool) const;
    writtenType                 withoutAliases      (const writtenType&, const typeReference&) const;
private:
    std::optional<std::size_t>  aliasOf             (const typeReference&, std::optional<std::size_t>) const;
    writtenType                 replaceAliases      (const writtenType&, const typeReference&, std::optional<std::size_t>, std::unordered_set<std::size_t>&) const;

    std::vector<indexedType>                                    types;          // In the order of add(), which gives their positions
    std::unordered_map<std::string, std::vector<std::size_t>>   byName;         // Positions of the types by language and the last part of their name
    std::vector<indexedAlias>                                   aliases;        // In the order of addAlias()
    std::unordered_map<std::string, std::vector<std::size_t>>   aliasesByName;  // Positions of the aliases by language and name
};

#endif
