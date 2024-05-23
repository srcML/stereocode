// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file PrimitiveTypes.hpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef PRIMITIVETYPES_HPP
#define PRIMITIVETYPES_HPP

#include <string>
#include <fstream>
#include <unordered_set>

class primitiveTypes {
public:
    bool           isPrimitive    (const std::string&) const;
    
    void           addPrimitive   (const std::string&);
    void           setLanguage    (const std::string&);

    friend         std::istream& operator>>(std::istream&, primitiveTypes&);

private:
    std::string                           unitLanguage;   // Language
    std::unordered_set<std::string>       ptypes;         // List of language primitives
    std::unordered_set<std::string>       userTypes;      // List of user defined language primitives 
};

#endif
