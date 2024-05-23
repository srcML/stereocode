// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file variable.hpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include "utils.hpp"

// Used to store data members (attributes or fields), locals, and parameters
//
class variable {
public:
    void        setName                 (const std::string& t)         { name = t;                   }
    void        setType                 (const std::string& t)         { type = t;                   }
    void        setNonPrimitiveExternal (const bool m)                 { nonPrimitiveExternal = m;   }

    std::string getName                 () const                       { return name;                }
    std::string getType                 () const                       { return type;                }
    bool        getNonPrimitiveExternal () const                       { return nonPrimitiveExternal;}

private:
    std::string name;                  // Variable name
    std::string type;                  // Variable type
    bool nonPrimitiveExternal{false};  // True if variable is non-primitive and not of same type as class it belongs to
};

#endif
