// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file PrimitiveTypes.hpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */


#ifndef PRIMITIVETYPES_HPP
#define PRIMITIVETYPES_HPP

#include <iostream>
#include <unordered_set>

class primitiveTypes {
public:
                   primitiveTypes () : language(), ptypes(), usertypes() {};

    bool           isPrimitive    (const std::string&) const;
    
    void           addPrimitive   (const std::string&);
    void           setLanguage    (const std::string&);

    std::string    getLanguage    () const { return language; }

    friend         std::istream& operator>>(std::istream&, primitiveTypes&);

private:
    std::string                           language;      // Language: "C++", "C#"
    std::unordered_set<std::string>       ptypes;        // List of language primitives
    std::unordered_set<std::string>       usertypes;     // List of user defined primitives 
};

#endif
