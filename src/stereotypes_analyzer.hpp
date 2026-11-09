// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotypes_analyzer.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef STEREOTYPES_ANALYZER_HPP
#define STEREOTYPES_ANALYZER_HPP

#include "options.hpp"
#include "global_variables.hpp"
#include "language_tables.hpp"
#include "stereotype_rules.hpp"
#include "type_index.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class outputArchive;
struct srcml_archive;
struct unitModels;

class stereotypesAnalyzer {
public:
    explicit             stereotypesAnalyzer            (const options&);
    // The models keep pointers to the tables of the analyzer, so the analyzer is never copied
                         stereotypesAnalyzer            (const stereotypesAnalyzer&) = delete;
    stereotypesAnalyzer& operator=                      (const stereotypesAnalyzer&) = delete;

    void                 run                            ();
private:
    // Progress of a type in the walk that collects inherited data members
    enum class inheritance { pending, onPath, done };

    options                                             settings;           // Settings of the run
    languageTables                                      tables;             // XPaths and lists that the models read
    std::vector<typeModel>                              types;              // Types in the order of the units, each C# partial type merged into its first part
    std::vector<std::optional<std::size_t>>             enclosingTypes;     // For each type, the earlier type whose object encloses an object of a Java inner class
    std::unordered_map<std::string, std::size_t>        partialTypes;       // Positions of the C# partial types by namespace and name
    typeIndex                                           typeNames;          // Names of the types, for parents and methods defined outside their types, and the aliases of types
    std::vector<functionModel>                          freeFunctions;      // List of free functions
    std::vector<std::vector<std::string>>               freeFunctionScopes; // Namespaces around each free function, until analyzeFreeFunctions()
    std::vector<std::vector<usingDirective>>            freeFunctionDirectives; // Using directives for each free function whose C++ name has a qualifier, until analyzeFreeFunctions()
    globalVariables                                     globals;            // Globals of every unit, since a unit may use a global of another unit
    std::unordered_set<std::string>                     globalStaticImports; // Types of the C# static imports that hold for every unit
    std::unordered_set<std::string>                     valueTypes;         // Names of the C# structs and enums, whose values a parameter copies
    std::unordered_set<std::string>                     cClassNames;        // Classes that the C markup of C++ headers hides
    std::vector<functionModel>                          cClassMethods;      // Definitions of the methods of these classes, which are unclassified

    int                  findModels                     (srcml_archive*);
    void                 mergeModels                    (unitModels);
    void                 indexTypes                     ();
    void                 findInheritedDataMembers       (std::size_t, std::vector<inheritance>&, std::vector<std::vector<std::size_t>>&);
    std::optional<std::size_t> findParentType           (std::size_t, std::size_t) const;
    std::optional<std::size_t> findDefinitionType       (const functionModel&, const std::vector<std::string>&, const std::vector<usingDirective>&) const;
    std::optional<std::size_t> findWrittenType          (std::size_t, const std::string&, int, bool) const;
    bool                 isCClassMethod                 (const functionModel&) const;
    bool                 isLabeled                      (const typeModel&) const;

    void                 analyzeFreeFunctions           ();
    void                 addGlobalStaticImports         ();
    void                 addAncestorStaticHolders       ();

    void                 outputStereotypes              (outputArchive&) const;
    template <typename report>
    void                 outputReport                   (report&) const;
};

#endif
