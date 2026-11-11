// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file language_tables.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef LANGUAGE_TABLES_HPP
#define LANGUAGE_TABLES_HPP

#include "options.hpp"
#include "archive/query_registry.hpp"
#include "rules/primitives.hpp"
#include "rules/calls.hpp"
#include "rules/query_methods.hpp"
#include "rules/specifiers.hpp"

// The tables per language that the type and function models read
//
struct languageTables {
    explicit             languageTables  (const options&);

    queryRegistry        queries;               // XPaths of the selected types and their members
    primitives           primitiveTypes;        // Default and user primitive types
    calls                ignorableCalls;        // Default and user calls to ignore
    specifiers           typeSpecifiers;        // Default and user specifiers to remove from types
    queryMethods         fieldQueries;          // Default and user query methods, whose calls on data members read them
};

#endif
