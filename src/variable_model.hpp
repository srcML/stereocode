// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file variable_model.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef VARIABLE_MODEL_HPP
#define VARIABLE_MODEL_HPP

#include <string>

// Used to store data members, locals, parameters, and globals
//
// Terminology:
//   A data member =  field (C++, C#, Java) or an auto-property (C#)
//
class variableModel {
public:
    void        setName                 (const std::string& name_)     { name = name_;               }
    void        setType                 (const std::string& type_)     { type = type_;               }
    void        setNonPrimitiveExternal (const bool m)                 { nonPrimitiveExternal = m;   }
    void        setNonPrimitive         (const bool m)                 { nonPrimitive = m;           }

    std::string getName                 () const                       { return name;                }
    std::string getType                 () const                       { return type;                }

    bool        isNonPrimitiveExternal  () const                       { return nonPrimitiveExternal;} 
    bool        isNonPrimitive          () const                       { return nonPrimitive;        }
private:
    std::string name;                                // Variable name
    std::string type;                                // Variable type
    bool        nonPrimitiveExternal {false};        // True if variable is non-primitive and not of same type as the type (e.g., class) it belongs to
    bool        nonPrimitive         {false};        // True if variable is non-primitive and is of same type as the type (e.g., class) it belongs to
};

#endif
