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
#include <unordered_set>
#include <vector>

// The methods of the run that the calls of a method to the object itself and on its fields may reach
// The methods of the run follow the order of their types, with the methods of each type in their order
//
class calledMethods {
public:
    // Finds the types of the archive that a type name written in the code of a type, in a unit, refers to equally well, for a call through -> or not
    using typeLookup = std::function<std::vector<std::size_t>(std::size_t, const std::string&, int, bool)>;

                                calledMethods       (const std::vector<typeModel>&, std::vector<std::vector<std::size_t>>,
                                                     std::vector<std::optional<std::size_t>>, typeLookup, const staticOverloads&, extensionMethods);

    bool                        callsStaticMethod   (std::size_t, const callModel&) const;
    bool                        callsImplicitAccessor(std::size_t, const callModel&) const;
    bool                        callsExtensionMethod(std::size_t, const callModel&) const;
    std::vector<std::size_t>    ofCallToType        (std::size_t, std::size_t, const callModel&) const;
    std::vector<std::size_t>    ofCallOnField       (std::size_t, std::size_t, const callModel&) const;
private:
    // A method that calls may reach, with the types of its parameters without the names that qualify them, which a method of a nearer type
    // with these types hides, as a method of the type hides a method of the body of one of its enum constants
    struct namedMethod {
        overload                callable;
        std::string             parameterTypes;
        bool                    inConstantBody;
    };

    std::vector<std::size_t>    typesNamed          (std::size_t, const std::string&) const;
    bool                        hasMethodNamed      (std::size_t, const std::string&) const;
    std::size_t                 holderOf            (std::size_t, const std::string&) const;
    std::vector<std::size_t>    reachedMethods      (const std::vector<std::size_t>&, const std::string&, int, std::size_t) const;

    std::vector<std::unordered_map<std::string, std::vector<namedMethod>>>  methods;        // Methods of each type by name, which calls may reach
    std::vector<argumentsByName>                                            staticMethods;  // Numbers of arguments that the static methods of each type take
    std::vector<std::unordered_set<std::string>>                            implicitAccessors; // Components of each record whose accessors the record does not define
    std::vector<declaredMethods>                                            undefinedMethods; // Methods that each type declares without a definition
    std::vector<bool>                                                       unknownParents; // Whether a parent outside the archive of each type or of an ancestor may define a method under the name of a method of the type
    std::vector<std::string>                                                names;          // Name of each type without namespaces and generic arguments
    std::vector<std::vector<std::size_t>>                                   parents;        // Types of the archive that each type names as parents
    std::vector<std::optional<std::size_t>>                                 enclosing;      // Type whose object encloses an object of each Java inner class
    std::vector<std::unordered_map<std::string, std::string>>               parentAliases;  // Member aliases of each type that stand for the type or a parent, with the name of that type
    typeLookup                                                              findTypes;      // Finds the types that the type name of a field fits
    extensionMethods                                                        extensions;     // C# extension methods by the language and the name of the method
    std::vector<std::string>                                                languages;      // Language of each type
    std::vector<std::string>                                                genericNames;   // Name of each type with its generic parameters reduced to commas, as Map<,>
};

#endif
