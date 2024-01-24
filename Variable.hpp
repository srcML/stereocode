// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file Variable.hpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <iostream>
#include <string>

class Variable {
public:
    void        setName                 (const std::string& t)         { name = t;                   }
    void        setNameParsed           (const std::string& t)         { nameParsed = t;             }
    void        setType                 (const std::string& t)         { type = t;                   }
    void        setTypeParsed           (const std::string& t)         { typeParsed = t;             }
    void        setNonPrimitive         (const bool m)                 { nonPrimitive = m;           }
    void        setNonPrimitiveExternal (const bool m)                 { nonPrimitiveExternal = m;   }

    std::string getName                 () const                       { return name;                }
    std::string getNameParsed           () const                       { return nameParsed;          }
    std::string getType                 () const                       { return type;                }
    std::string getTypeParsed           () const                       { return typeParsed;          }
    bool        getNonPrimitive         () const                       { return nonPrimitive;        }
    bool        getNonPrimitiveExternal () const                       { return nonPrimitiveExternal;}

private:
    std::string name{};           
    std::string nameParsed{};          // Name without [] (if any)
    std::string type{};           
    std::string typeParsed{};          // Type with containers and specifiers removed
    bool nonPrimitive{false};          // True if variable is non-primitive
    bool nonPrimitiveExternal{false};  // True if variable is non-primitive and not of same type as class it belongs to
};

#endif
