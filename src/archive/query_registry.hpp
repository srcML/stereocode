// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file query_registry.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef QUERY_REGISTRY_HPP
#define QUERY_REGISTRY_HPP

#include "options.hpp"

#include <array>
#include <cstddef>
#include <string>

// The queries of the analysis, each an XPath per language that has it
//
enum class query {
    // On a unit
    type, freeFunction, functionOutsideTypes, staticMethodDeclaration, cClassName, globalVariable, importedName, staticImport, globalStaticImport, valueTypeName,
    usingDirective, typeAliasName, typeImport, globalTypeImport,
    // On a type
    structureType, partialType, parentName, fieldDeclaration, staticFieldName, method, functionDeclName, functionDeclParameters, staticDeclaration,
    staticMethodName, nestedTypeName, misreadMethodName, memberAliasName, property,
    // On a C# property
    propertyType, propertyMethod,
    // On a method or a free function
    methodName, methodParameterList, constructorOrDestructor, methodSpecifiers, methodAttributes, constMethod, returnType, returnTypeName,
    parameterDeclaration, localDeclaration, expressionVariable, returnExpression, newReturnExpression,
    functionCall, methodCall, constructorCall, callOnResult, nullConditionalCall, localFunction, selfOperator, shiftOperator,
    newAssignedVariable, nonCommentStatements, expressionName, expressionAssignment, interpolatedString, globalName, objectParameter, classParameter,
    count // Number of queries
};

// The languages that have queries (C units use the C++ queries)
//
enum class queryLanguage { cpp, csharp, java, python, javascript, count };

constexpr std::size_t queryCount    = static_cast<std::size_t>(query::count);
constexpr std::size_t languageCount = static_cast<std::size_t>(queryLanguage::count);

queryLanguage toQueryLanguage(const std::string&);

// The XPaths of every query and language, with the type kinds that the options select
//
class queryRegistry {
public:
    explicit                    queryRegistry   (const options&);

    const std::string&          xpath           (queryLanguage, query)            const;
private:
    std::array<std::array<std::string, queryCount>, languageCount> xpaths;   // Empty where a language has no such query
};

#endif
