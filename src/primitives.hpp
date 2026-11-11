// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file primitives.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef PRIMITIVES_HPP
#define PRIMITIVES_HPP

#include "user_lists.hpp"
#include "variable_model.hpp"

#include <string>
#include <unordered_set>
#include <unordered_map>

class specifiers;

// Stores primitives for data members, locals, parameters, and globals data types
// Stores primitives for function return types
//
class primitives {
public:
    bool                 isNonPrimitive           (variableModel&, const std::string&, const std::string&, const specifiers&) const;
    bool                 isPrimitive              (const std::string&, const std::string&) const;
    void                 initializePrimitivesList ();
    void                 outputPrimitives         () const;
    friend std::istream& operator>>               (std::istream&, primitives&);
private:
    std::unordered_map<std::string, std::unordered_set<std::string>>     primitivesList;         // List of primitives
    userEntries                                                          userPrimitivesList;     // List of user-defined primitives
};

#endif
