// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file bound_value.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef BOUND_VALUE_HPP
#define BOUND_VALUE_HPP

#include <string>

// What an assignment of a dynamic language tells about the name that it binds, such as engine: Engine = Engine(4) or self.owner = owner of Python
// and this.engine = new Engine(4) of JavaScript
//
struct boundValue {
    std::string     type;       // Type of the annotation of the name or of the object that the value creates, if any
    std::string     name;       // The name that alone is the value, as owner of self.owner = owner
    std::string     callName;   // Name of the Python call that alone is the value, as Engine of engine = Engine(4)
};

#endif
