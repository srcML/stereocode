// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file overloads.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef OVERLOADS_HPP
#define OVERLOADS_HPP

#include "model/argument_range.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

// A method that the calls of its name may reach, with the numbers of arguments that it takes
//
struct overload {
    std::size_t     position{0};    // Position of the method among the methods of the run
    argumentRange   arguments;      // Numbers of arguments that the method takes
};

// A method that a type declares without a definition, as an abstract method
//
struct declaredMethod {
    argumentRange   arguments;      // Numbers of arguments that the method takes
    std::string     parameterTypes; // Types of its parameters without the names that qualify them
};

// The declared methods of a type by name
//
using declaredMethods = std::unordered_map<std::string, std::vector<declaredMethod>>;

// The numbers of arguments that the static methods of the types take, by the language and the name of their type, as "Java Registry",
// and by the name of the method
using staticOverloads = std::unordered_map<std::string, argumentsByName>;

// A C# extension method, a static method whose first parameter takes the object that a call names before the method, as basket of
// basket.IsEmpty() for IsEmpty(this Basket basket)
//
struct extensionMethod {
    std::string     extendedType;   // Type of the first parameter without qualifiers and with its generic arguments reduced to commas, as Basket or Map<,>
    argumentRange   arguments;      // Numbers of arguments that a call on an object passes besides the object
};

// The extension methods of the archive by the language and the name of the method, as "C# IsEmpty"
using extensionMethods = std::unordered_map<std::string, std::vector<extensionMethod>>;

std::vector<std::size_t> overloadsTaking (const std::vector<overload>&, int);

#endif
