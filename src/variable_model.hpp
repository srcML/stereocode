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

// Used to store fields (data members), locals, parameters, and globals
// auto-properties are also stored and treated as fields
//
class variableModel {
public:
    void        setName                 (const std::string& name_)     { name = name_;               }
    void        setType                 (const std::string& type_)     { type = type_;               }
    void        setNamedType            (const std::string& named)     { namedType = named;          }
    void        setPointeeType          (const std::string& pointee)   { pointeeType = pointee;      }
    void        setUnitNumber           (int unitNumber_)              { unitNumber = unitNumber_;   }
    void        setNonPrimitiveExternal (const bool m)                 { nonPrimitiveExternal = m;   }
    void        setNonPrimitive         (const bool m)                 { nonPrimitive = m;           }

    std::string getName                 () const                       { return name;                }
    std::string getType                 () const                       { return type;                }
    std::string getNamedType            () const                       { return namedType;           }
    std::string getPointeeType          () const                       { return pointeeType;         }
    int         getUnitNumber           () const                       { return unitNumber;          }

    bool        isNonPrimitiveExternal  () const                       { return nonPrimitiveExternal;}
    bool        isNonPrimitive          () const                       { return nonPrimitive;        }
private:
    std::string name;                                // Variable name
    std::string type;                                // Variable type
    std::string namedType;                           // Name of a type of one name, such as Engine of const Engine*, which may name a type of the archive (fields only)
    std::string pointeeType;                         // Named type of the object that -> reaches for a pointer type, such as Engine of std::unique_ptr<Engine> (fields only)
    int         unitNumber           {0};            // Unit that declares the field, where its type name is written (fields only)
    bool        nonPrimitiveExternal {false};        // True if variable is non-primitive and not of same type as the type (e.g., class) it belongs to
    bool        nonPrimitive         {false};        // True if variable is non-primitive and is of same type as the type (e.g., class) it belongs to
};

#endif
