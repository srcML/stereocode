// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file unit_models.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef UNIT_MODELS_HPP
#define UNIT_MODELS_HPP

#include "model/argument_range.hpp"
#include "model/global_variables.hpp"
#include "model/type_index.hpp"
#include "model/type_model.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

class queryEngine;

// A static method of a type, which a call without an object names in the type and in the types that it holds
//
struct staticMethod {
    std::string     holder;         // Language and name of the type that holds the method, as "Java Registry"
    std::string     name;
    argumentRange   arguments;
    std::string     extendedType;   // Type that a C# extension method extends, as Basket of IsEmpty(this Basket basket), if any
};

// The types and free functions of one unit, in the order of the unit
//
struct unitModels {
    std::vector<typeModel>                  types;
    std::vector<std::optional<std::size_t>> enclosingTypes;         // For each type, the earlier type whose object encloses an object of a Java inner class
    std::vector<functionModel>              freeFunctions;
    std::vector<std::vector<std::string>>   freeFunctionScopes;     // Namespaces around each free function, outermost first
    std::vector<std::vector<usingDirective>> freeFunctionDirectives; // Using directives of the unit at namespace scope for each free function whose C++ name has a qualifier
    globalVariables                         globals;                // Variables at file or namespace scope and static fields
    std::vector<staticMethod>               staticMethods;          // Static methods of the types of a unit of C# or Java, which are free functions as well
    std::unordered_set<std::string>         globalStaticImports;    // Types of the C# static imports that hold for every unit, as Limits of global using static Shop.Limits;
    std::unordered_set<std::string>         valueTypes;             // Names of the C# structs and enums, whose values a parameter copies
    std::vector<std::string>                cClassNames;            // Classes that the C markup of a C++ header hides
    std::vector<indexedAlias>               aliases;                // Aliases of types, such as IStreamingReporterPtr of using IStreamingReporterPtr = std::unique_ptr<IStreamingReporter>;
    std::vector<typeImport>                 typeImports;            // Imports of C# and Java that bring types into the unit by their names
    std::unordered_set<std::string>         declaredTypeNames;      // Names of the C++ types and aliases of the unit, whatever the options, such as MODE_T of enum MODE_T { };
    std::string                             warning;                // Damaged markup of the unit, if any
};

unitModels findUnitModels (queryEngine&, const languageTables&, std::string_view, const std::string&, bool, int);

#endif
