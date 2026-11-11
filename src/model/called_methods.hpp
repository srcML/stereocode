// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file called_methods.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef CALLED_METHODS_HPP
#define CALLED_METHODS_HPP

#include "model/overloads.hpp"
#include "model/type_model.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// The methods of the run that the calls of a method to the object itself and on its fields may reach
// The methods of the run follow the order of their types, with the methods of each type in their order
//
class calledMethods {
public:
    // Finds the type of the archive that a type name written in the code of a type, in a unit, refers to, for a call through -> or not
    using typeLookup = std::function<std::optional<std::size_t>(std::size_t, const std::string&, int, bool)>;

                                calledMethods       (const std::vector<typeModel>&, std::vector<std::vector<std::size_t>>, typeLookup);

    std::vector<std::size_t>    ofCallToType        (std::size_t, const callModel&) const;
    std::vector<std::size_t>    ofCallOnField       (std::size_t, const callModel&) const;
private:
    // A method that calls may reach, with the types of its parameters in its signature, which a method of a nearer type with these types hides
    struct namedMethod {
        overload                callable;
        std::string             signatureTypes;
    };

    std::vector<std::size_t>    typesNamed          (std::size_t, const std::string&) const;
    std::vector<std::size_t>    reachedMethods      (const std::vector<std::size_t>&, const std::string&, int) const;

    std::vector<std::unordered_map<std::string, std::vector<namedMethod>>>  methods;        // Methods of each type by name, which calls may reach
    std::vector<std::string>                                                names;          // Name of each type without namespaces and generic arguments
    std::vector<std::vector<std::size_t>>                                   parents;        // Types of the archive that each type names as parents
    typeLookup                                                              findType;       // Finds the type of a field
};

#endif
