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

// The types and free functions of one unit, in the order of the unit
//
struct unitModels {
    std::vector<typeModel>                  types;
    std::vector<std::optional<std::size_t>> enclosingTypes;         // For each type, the earlier type whose object encloses an object of a Java inner class
    std::vector<functionModel>              freeFunctions;
    std::vector<std::vector<std::string>>   freeFunctionScopes;     // Namespaces around each free function, outermost first
    std::vector<std::vector<usingDirective>> freeFunctionDirectives; // Using directives of the unit at namespace scope for each free function whose C++ name has a qualifier
    globalVariables                         globals;                // Variables at file or namespace scope and static fields
    std::unordered_set<std::string>         globalStaticImports;    // Types of the C# static imports that hold for every unit, as Limits of global using static Shop.Limits;
    std::unordered_set<std::string>         valueTypes;             // Names of the C# structs and enums, whose values a parameter copies
    std::vector<std::string>                cClassNames;            // Classes that the C markup of a C++ header hides
    std::vector<indexedAlias>               aliases;                // Aliases of types, such as IStreamingReporterPtr of using IStreamingReporterPtr = std::unique_ptr<IStreamingReporter>;
    std::unordered_set<std::string>         declaredTypeNames;      // Names of the C++ types and aliases of the unit, whatever the options, such as MODE_T of enum MODE_T { };
    std::string                             warning;                // Damaged markup of the unit, if any
};

unitModels findUnitModels (queryEngine&, const languageTables&, std::string_view, const std::string&, bool, int);

#endif
