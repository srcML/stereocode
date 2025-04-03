// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file variable.hpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include "utils.hpp"

// Used to store fields, locals, and parameters
//
class variable {
public:
    void        setName                 (const std::string& t)         { name = t;                   }
    void        setType                 (const std::string& t)         { type = t;                   }
    void        setNonPrimitiveExternal (const bool m)                 { nonPrimitiveExternal = m;   }
    void        setNonPrimitive         (const bool m)                 { nonPrimitive = m;           }
    void        setPos                  (const int p)                  { pos = p;                    }

    std::string getName                 () const                       { return name;                }
    std::string getType                 () const                       { return type;                }
    bool        getNonPrimitiveExternal () const                       { return nonPrimitiveExternal;} 
    bool        getNonPrimitive         () const                       { return nonPrimitive;        }
    int         getPos                  () const                       { return pos;                 }
    
private:
    std::string name;                         // Variable name
    std::string type;                         // Variable type
    bool        nonPrimitiveExternal{false};  // True if variable is non-primitive and not of same type as class it belongs to
    bool        nonPrimitive{false};          // True if variable is non-primitive
    int         pos{-1};                      // Position of variable (Starting at 0)         
};

#endif
